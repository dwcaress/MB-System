/*--------------------------------------------------------------------
 *    The MB-system:	mbr_kemkmall.c	5/01/2018
 *	$Id: mbr_kemkmall.c $
 *
 *    Copyright (c) 2014-2017 by
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
 * mbr_kemkmall.c contains the functions for reading and writing
 * multibeam data in the KEMKMALL format.
 * These functions include:
 *   mbr_alm_kemkmall	- allocate read/write memory
 *   mbr_dem_kemkmall 	- deallocate read/write memory
 *   mbr_rt_kemkmall	 	- read and translate data
 *   mbr_wt_kemkmall	 	- translate and write data
 *
 * Author:	B. Y. Raanan
 * Date:	May 25, 2018
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_kmbes.h"

/* include for byte swapping */
#include "mb_swap.h"

/* turn on debug statements here */
/* #define MBR_KEMKMALL_DEBUG 1 */

/* essential function prototypes */
int mbr_register_kemkmall(int verbose, void *mbio_ptr, int *error);
int mbr_info_kemkmall(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
					 char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
					 int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
					 int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
					 double *beamwidth_ltrack, int *error);
int mbr_alm_kemkmall(int verbose, void *mbio_ptr, int *error);
int mbr_dem_kemkmall(int verbose, void *mbio_ptr, int *error);
int mbr_rt_kemkmall(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_kemkmall(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_kemkmall_index_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_kemkmall_create_dgm_index_table(int verbose, void *mbio_ptr, void *store_ptr, int *error);    //size_t block_size
int mbr_kemkmall_add_dgm_to_dgm_index_table(int verbose, void *index_table_ptr, void *new_index_ptr, int *error);
int mbr_kemkmall_indextable_compare_time_d(const void *a, const void *b);
int mbr_kemkmall_indextable_compare_rx_index(const void *a, const void *b);
int mbr_kemkmall_id_dgm(int verbose, void *dgm_index_ptr, int *error);
int mbr_kemkmall_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_kemkmall_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_kemkmall_rd_hdr(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_spo(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_skm(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_svp(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_svt(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_scl(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_sde(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_shi(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_sha(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_iip(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_iop(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_mrz(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_mwc(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_cpo(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_che(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_rd_unknown(int verbose, char *buffer, void *store_ptr, int *error);
int mbr_kemkmall_wr_spo(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_skm(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_svp(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_svt(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_scl(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_sde(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_shi(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_sha(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_iip(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_iop(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_mrz(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_mwc(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_cpo(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_che(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);
int mbr_kemkmall_wr_unknown(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);

static char rcs_id[] = "$Id: mbr_kemkmall.c 2308 2018-05-25 19:55:48Z caress $";

/*--------------------------------------------------------------------*/
int mbr_register_kemkmall(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_register_kemkmall";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	status = mbr_info_kemkmall(
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
	mb_io_ptr->mb_io_sidescantype = NULL;
	mb_io_ptr->mb_io_extract = &mbsys_kmbes_extract;
	mb_io_ptr->mb_io_insert = &mbsys_kmbes_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_kmbes_extract_nav;
//	mb_io_ptr->mb_io_extract_nnav = &mbsys_kmbes_extract_nnav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_kmbes_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_kmbes_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_kmbes_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_kmbes_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_kmbes_ttimes;
//	mb_io_ptr->mb_io_detects = &mbsys_kmbes_detects;
//	mb_io_ptr->mb_io_gains = &mbsys_kmbes_gains;
	mb_io_ptr->mb_io_copyrecord = &mbsys_kmbes_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;
	mb_io_ptr->mb_io_extract_segytraceheader = NULL;
	mb_io_ptr->mb_io_extract_segy = NULL;
	mb_io_ptr->mb_io_insert_segy = NULL;
	mb_io_ptr->mb_io_ctd = NULL;
	mb_io_ptr->mb_io_ancilliarysensor = NULL;

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

	/* return status */
	return (status);
}

/*--------------------------------------------------------------------*/
int mbr_info_kemkmall(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
					 char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
					 int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
					 int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
					 double *beamwidth_ltrack, int *error) {
	char *function_name = "mbr_info_kemkmall";
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
	*system = MB_SYS_KMBES;
	*beams_bath_max = MBSYS_KMBES_MAX_NUM_BEAMS;
	*beams_amp_max = MBSYS_KMBES_MAX_NUM_BEAMS;
	//*pixels_ss_max = MBSYS_KMBES_MAX_PIXELS;
	strncpy(format_name, "KEMKMALL", MB_NAME_LENGTH);
	strncpy(system_name, "KMBES", MB_NAME_LENGTH);
	strncpy(format_description,
			"Format name:          MBF_KEMKMALL\nInformal Description: Kongsberg multibeam echosounder system EM datagram format\nAttributes:           "
			"sensor(s), \n                      what data types are supported\n                      how many beams and pixels, "
			"file type (ascii, binary, netCDF), Organization that defined this format.\n",
			MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_SINGLE;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 1.0;
	*beamwidth_ltrack = 1.0;

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
int mbr_alm_kemkmall(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_alm_kemkmall";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
    char **bufferptr;
    int *bufferalloc;

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
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mbsys_kmbes_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

    /* allocate starting memory for data record buffer */
    bufferptr = (char **)&mb_io_ptr->raw_data;
    bufferalloc = (int *)&mb_io_ptr->structure_size;

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
int mbr_dem_kemkmall(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_dem_kemkmall";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_index_table *dgm_index_table;
	int *dgm_count;
	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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
		dgm_count = (int *)mb_io_ptr->save1;

		if (dgm_index_table->size >0) {
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&dgm_index_table->indextable), error);
		}
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&dgm_index_table), error);
		dgm_index_table = NULL;
		*dgm_count = 0;
	}

	/* deallocate memory for data descriptor */
	status = mbsys_kmbes_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_rt_kemkmall(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_rt_kemkmall";
	int status = MB_SUCCESS;
	int interp_status;
	int interp_error = MB_ERROR_NO_ERROR;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
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

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

#ifdef MBR_KEMKMALL_DEBUG
	fprintf(stderr, "About to call mbr_kemkmall_rd_data...\n");
#endif

	/* get saved values */
	file_indexed = (int *)&mb_io_ptr->save2;

	/* if needed index the file */
	if (*file_indexed == MB_NO) {
		status = mbr_kemkmall_index_data(verbose, mbio_ptr, store_ptr, error);
	}

	/* read next data from file */
	status = mbr_kemkmall_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* get pointers to data structures */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

#ifdef MBR_KEMKMALL_DEBUG
	fprintf(stderr, "Done with mbr_kemkmall_rd_data: status:%d error:%d record kind:%d\n", status, *error, store->kind);
#endif

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
int mbr_wt_kemkmall(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_wt_kemkmall";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;

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

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;

#ifdef MBR_KEMKMALL_DEBUG
	fprintf(stderr, "About to call mbr_kemkmall_wr_data record kind:%d\n", store->kind);
#endif

	/* write next data to file */
	status = mbr_kemkmall_wr_data(verbose, mbio_ptr, store_ptr, error);

#ifdef MBR_KEMKMALL_DEBUG
	fprintf(stderr, "Done with mbr_kemkmall_wr_data: status:%d error:%d\n", status, *error);
#endif

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

int mbr_kemkmall_index_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_index_data";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index_table *dgm_index_table;
	struct mbsys_kmbes_index *dgm_index;
	int *file_indexed;
	char buffer[256];
	size_t read_len, offset;
	int index, skip, valid_id, num_bytes_dgm_end;
	size_t i;
	const int HEADER_SKIP = 8;


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

	/* create a datagram index table in mbio descriptor */
	mbr_kemkmall_create_dgm_index_table(verbose, mbio_ptr, store_ptr, error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* now get the index table from the mbio descriptor field saveptr1 */
	dgm_index_table = (struct mbsys_kmbes_index_table *)mb_io_ptr->saveptr1;
	file_indexed = (int *)&mb_io_ptr->save2;

	/* get an index instance. We'll use it to populate the table later */
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];

	/* set file position to the start */
	fseek(mb_io_ptr->mbfp, 0, SEEK_SET);  // TODO: check if file should be set to fpos
	mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

	/* set status */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	valid_id = MB_YES;

	while (*error <= MB_ERROR_NO_ERROR) {

		memset(&buffer, 0, sizeof(buffer));
		memset(dgm_index, 0, sizeof(struct mbsys_kmbes_index));
		dgm_index->emdgm_type = UNKNOWN;


		/* get a datagram header */
		read_len = (size_t)MBSYS_KMBES_HEADER_SIZE;
		status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);

		if (status == MB_SUCCESS) {
			/* extract datagram header and determine its type */
			mbr_kemkmall_rd_hdr(verbose, &buffer[0], store_ptr, error);

			/* verify datagram is intact */
			if (dgm_index->emdgm_type != UNKNOWN) {

				/* seek to end of the datagram and read last int */
				offset = (dgm_index->header.numBytesDgm - MBSYS_KMBES_HEADER_SIZE - sizeof(int));
				fseek(mb_io_ptr->mbfp, offset, SEEK_CUR);

				read_len = sizeof(int);
				status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);

				if (status == MB_SUCCESS) {
					/* Confirm that that byte count in the last int matches start int */
					mb_get_binary_int(MB_YES, &buffer[0], &num_bytes_dgm_end);

					if (dgm_index->header.numBytesDgm != num_bytes_dgm_end) {
						/* No match. Move file pointer past corrupted packet header and set datagram type to unknown. */
						mb_io_ptr->file_pos += HEADER_SKIP;
						dgm_index->emdgm_type = UNKNOWN;
						valid_id = MB_NO;
					}
				}


				/* handle the datagram */
				switch (dgm_index->emdgm_type) {

					case UNKNOWN:
						/* Unknown, invalid, or corrupted datagram: */
						/* read byte by byte until a sync character is found */
						/* once found, confirm a valid EM datagram type and start back at the top */
                        // TODO: add unknown chunk to the index table as an UNKNOWN datagram

						skip = 0;
						do {
							read_len = 1;
							status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);

							if (buffer[0]==MBSYS_KMBES_SYNC_CHAR) {

								read_len = 3;  // read 3 more bytes
								status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[1], &read_len, error);
								memcpy(&dgm_index->header.dgmType, &buffer[0], 4);

								mbr_kemkmall_id_dgm(verbose, dgm_index, error);

								if (dgm_index->emdgm_type != UNKNOWN) {

									/* valid datagram. Set file pointer back to start of valid datagram header */
									fseek(mb_io_ptr->mbfp, -HEADER_SKIP, SEEK_CUR);
									valid_id = MB_YES;
								}
							}
							else
								skip++;

						} while (status == MB_SUCCESS && valid_id == MB_NO);

						/* report problem */
						if (skip > 0 && verbose >= 0) {
							fprintf(stderr, "\nThe MBF_KEMKMALL module skipped data between identified\n"
											"data records. Something is broken, most likely the data...\n"
											"However, the data may include a data record type that we\n"
											"haven't seen yet, or there could be an error in the code.\n"
											"If skipped data are reported multiple times, \n"
											"we recommend you send a data sample and problem \n"
											"description to the MB-System team \n"
											"(caress@mbari.org and dale@ldeo.columbia.edu)\n"
											"Have a nice day...\n");
							fprintf(stderr, "MBF_KEMKMALL skipped %d bytes before record %4s\n", skip, dgm_index->header.dgmType);
						}
						break;

					case MWC:
					case MRZ:
						/* Valid multibeam datagram: */
						/* parse the dgm to get additional info about ping. */
						/* this is necessary to insure multi-TX/RX and dual-swath modes are handled correctly. */

						/* get ping info */
						offset = (mb_io_ptr->file_pos + MBSYS_KMBES_HEADER_SIZE + sizeof(int));
						fseek(mb_io_ptr->mbfp, offset, SEEK_SET);

						read_len = 8;
						status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);

						if (status == MB_SUCCESS) {

							index = 2;
							mb_get_binary_short(MB_YES, &buffer[index], &(dgm_index->ping_num));
							index += 2;
							dgm_index->rx_per_ping = buffer[index];
							index++;
							dgm_index->rx_index = buffer[index];
							index++;
							dgm_index->swaths_per_ping = buffer[index];

							/* update the datagram index table */
							dgm_index->time_d = dgm_index->header.time_nanosec * MBSYS_KMBES_NANO;
							dgm_index->time_d += dgm_index->header.time_sec;
							dgm_index->file_pos = mb_io_ptr->file_pos;
							dgm_index->rx_per_ping = dgm_index->rx_per_ping * dgm_index->swaths_per_ping;

							status = mbr_kemkmall_add_dgm_to_dgm_index_table(verbose, dgm_index_table, dgm_index, error);
						}

						if (status == MB_SUCCESS) {
							offset = (size_t) (mb_io_ptr->file_pos + dgm_index->header.numBytesDgm);
							fseek(mb_io_ptr->mbfp, offset, SEEK_SET);
						}
						// TODO: what happens if alloc fails - while condition?
						break;

					default:
						/* Other valid datagram: */

						/* update the datagram index table */
						dgm_index->ping_num = 0;
						dgm_index->time_d = dgm_index->header.time_nanosec * MBSYS_KMBES_NANO;
						dgm_index->time_d += dgm_index->header.time_sec;
						dgm_index->file_pos = mb_io_ptr->file_pos;

						status = mbr_kemkmall_add_dgm_to_dgm_index_table(verbose, dgm_index_table, dgm_index, error);

						if (status == MB_SUCCESS) {
							offset = (size_t) (mb_io_ptr->file_pos + dgm_index->header.numBytesDgm);
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
	*file_indexed = MB_YES;
	if ((dgm_index_table->dgm_count > 0) && (*error = MB_ERROR_EOF)) {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}

	/* sort the datagram index table */
	if (status == MB_SUCCESS) {

		/* first sort by rx_index */
		qsort((void *)dgm_index_table->indextable, dgm_index_table->dgm_count,
			  sizeof(struct mbsys_kmbes_index), (void *)mbr_kemkmall_indextable_compare_rx_index);

		/* then sort by timestamp. */
		qsort((void *)dgm_index_table->indextable, dgm_index_table->dgm_count,
			  sizeof(struct mbsys_kmbes_index), (void *)mbr_kemkmall_indextable_compare_time_d);

		/* probably not the most efficient way to sort... */
		/* but this will order the dgm's by time with secondary order by rxIndex. */
	}

#ifdef MBR_KEMKMALL_DEBUG
	printf("\n\nIndexed %ld valid EM datagrams:\n", dgm_index_table->dgm_count);
	for (i=0; i<dgm_index_table->dgm_count; i++)
	{
		printf("ID: %4zu, ", i);
		printf("file_pos: %8.zu, ", dgm_index_table->indextable[i].file_pos);
		printf("dgm: %.4s, ", &dgm_index_table->indextable[i].header.dgmType[0]);
		printf("type: %2d, ", dgm_index_table->indextable[i].emdgm_type);
		printf("size: %6u, ", dgm_index_table->indextable[i].header.numBytesDgm);
		printf("time: %9.3f, ", dgm_index_table->indextable[i].time_d);
		printf("ping: %5d, ", dgm_index_table->indextable[i].ping_num);
		printf("rxIndex: %u/%u.\n", dgm_index_table->indextable[i].rx_index, dgm_index_table->indextable[i].rx_per_ping);
	}
	printf("\n");
#endif

	/* set file position back to the start */
	fseek(mb_io_ptr->mbfp, 0, SEEK_SET);

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

};

int mbr_kemkmall_create_dgm_index_table(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_create_dgm_index_table";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index_table *dgm_index_table;
	size_t size_bytes;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* we will store the datagram index table in mbio descriptor field saveptr1 */
	dgm_index_table = (struct mbsys_kmbes_index_table *)mb_io_ptr->saveptr1;

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* allocate the datagram index table struct (vector struct) */
	status = mb_mallocd(verbose, __FILE__, __LINE__,
						 sizeof(struct mbsys_kmbes_index_table), (void **)(&dgm_index_table), error);

    size_bytes = MBSYS_KMBES_INDEX_TABLE_BLOCK_SIZE * sizeof(struct mbsys_kmbes_index);
    if (status == MB_SUCCESS) {
		dgm_index_table->dgm_count = 0;
		dgm_index_table->size = MBSYS_KMBES_INDEX_TABLE_BLOCK_SIZE;

		/* allocate the datagram index table array */
		status = mb_mallocd(verbose, __FILE__, __LINE__, size_bytes,
							 (void **)(&dgm_index_table->indextable), error);
    }


	/* init internal data structure variables */
	store->dgm_count = 0;
	store->dgm_count_id = 0;

    /* print output debug statements */
    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
        fprintf(stderr, "dbg2  Return values:\n");
        fprintf(stderr, "dbg2       error:      %d\n", *error);
        fprintf(stderr, "dbg2  Return status:\n");
        fprintf(stderr, "dbg2       status:  %d\n", status);
    }

	/* return status */
	return(status);
};


int mbr_kemkmall_add_dgm_to_dgm_index_table(int verbose, void *index_table_ptr, void *new_index_ptr, int *error) {
	char *function_name = "mbr_kemkmall_add_dgm_to_dgm_index_table";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_index_table *dgm_index_table;
	struct mbsys_kmbes_index *new_dgm_index;
	size_t dgm_count;
	size_t new_size;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       index_table_ptr: %p\n", (void *)index_table_ptr);
		fprintf(stderr, "dbg2       new_index_ptr:   %p\n", (void *)new_index_ptr);
	}

	/* get pointer to datagram index table */
	dgm_index_table = (struct mbsys_kmbes_index_table *)index_table_ptr;

	/* get pointer to the new datagram index structure */
	new_dgm_index = (struct mbsys_kmbes_index *)new_index_ptr;

	/* reallocate the datagram index table array if needed */
	dgm_count = dgm_index_table->dgm_count;
	if (dgm_count >= (dgm_index_table->size-1)) {
		new_size = dgm_index_table->size + MBSYS_KMBES_INDEX_TABLE_BLOCK_SIZE;

		/* reallocate the datagram index table array */
		status = mb_reallocd(verbose, __FILE__, __LINE__,
							 sizeof(struct mbsys_kmbes_index)*new_size,
							 (void **)(&dgm_index_table->indextable), error);

		if (status == MB_SUCCESS) {
			dgm_index_table->size = new_size;
		}
	}

	if (status == MB_SUCCESS) {
		dgm_index_table->indextable[dgm_count] = *new_dgm_index;
		dgm_index_table->dgm_count++;
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
	return(status);
};

int mbr_kemkmall_indextable_compare_time_d(const void *a, const void *b) {
	struct mbsys_kmbes_index *aa;
	struct mbsys_kmbes_index *bb;
	int result = 0;

	aa = (struct mbsys_kmbes_index*) a;
	bb = (struct mbsys_kmbes_index*) b;

	if (aa->time_d < bb->time_d)
		result = -1;
	else if (aa->time_d > bb->time_d)
		result = 1;

	return(result);
};

int mbr_kemkmall_indextable_compare_rx_index(const void *a, const void *b) {
	struct mbsys_kmbes_index *aa;
	struct mbsys_kmbes_index *bb;
	int result = 0;

	aa = (struct mbsys_kmbes_index*) a;
	bb = (struct mbsys_kmbes_index*) b;

	if (aa->rx_index < bb->rx_index)
		result = -1;
	else if (aa->rx_index > bb->rx_index)
		result = 1;

	return(result);
};

int mbr_kemkmall_id_dgm(int verbose, void *dgm_index_ptr, int *error) {
	char *function_name = "mbr_kemkmall_id_dgm";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_index *dgm_index;
	mbsys_kmbes_emdgm_type *emdgm_type;
	const char *dgm_type_char;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       dgm_index_ptr:  %p\n", (void *)dgm_index_ptr);
	}

	/* get pointer to data structure */
	dgm_index = (struct mbsys_kmbes_index *)&dgm_index_ptr;
	dgm_type_char = (const char *)&(dgm_index->header.dgmType[0]);
	emdgm_type = &(dgm_index->emdgm_type);

	if (strcmp(dgm_type_char, MBSYS_KMBES_I_INSTALLATION_PARAM) == 0 ) {
		*emdgm_type = IIP;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_I_OP_RUNTIME)==0) {
		*emdgm_type = IOP;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_S_POSITION)==0) {
		*emdgm_type = SPO;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_S_KM_BINARY)==0) {
		*emdgm_type = SKM;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_S_SOUND_VELOCITY_PROFILE)==0) {
		*emdgm_type = SVP;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_S_SOUND_VELOCITY_TRANSDUCER)==0) {
		*emdgm_type = SVT;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_S_CLOCK)==0) {
		*emdgm_type = SCL;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_S_DEPTH)==0) {
		*emdgm_type = SDE;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_S_HEIGHT)==0) {
		*emdgm_type = SHI;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_S_HEADING)==0) {
		*emdgm_type = SHA;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_M_RANGE_AND_DEPTH)==0) {
		*emdgm_type = MRZ;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_M_WATER_COLUMN)==0) {
		*emdgm_type = MWC;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_C_POSITION)==0) {
		*emdgm_type = CPO;
	}
	else if (strcmp(dgm_type_char, MBSYS_KMBES_C_HEAVE)==0) {
		*emdgm_type = CHE;
	}
	else {
		*emdgm_type = UNKNOWN;
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
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_data";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index_table *dgm_index_table;
	struct mbsys_kmbes_index *dgm_index;
	size_t read_len;
	char **bufferptr;
	char *buffer;
	int *bufferalloc;
	int *dgm_id;
	int ping_num;
	int skip;
	int done;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get buffer and related vars from mbio saved values */
	bufferptr = (char **)&mb_io_ptr->raw_data;
	bufferalloc = (int *)&mb_io_ptr->structure_size;
	buffer = (char *)*bufferptr;

	/* get the datagram index table */
	dgm_index_table = (struct mbsys_kmbes_index_table *)&mb_io_ptr->saveptr1;
	dgm_id = (int *)&mb_io_ptr->save1;

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&(store->dgm_index[store->dgm_count_id]); // first dgm index

	*dgm_index = dgm_index_table->indextable[*dgm_id];
	ping_num = dgm_index->ping_num;

	store->time_d = dgm_index->time_d;
	mb_get_date(verbose, store->time_d, store->time_i);

	/* loop over reading data until a record is ready for return */
	skip = 0;
    done = MB_NO;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO) {

		/* set the file offset */
		fseek(mb_io_ptr->mbfp, dgm_index->file_pos, SEEK_SET);
		mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

		/* allocate memory to read the record if necessary */
		read_len = (size_t)dgm_index->header.numBytesDgm;
		if (*bufferalloc <= read_len) {

			status = mb_reallocd(verbose, __FILE__, __LINE__, (read_len + 1), (void **)bufferptr, error);
			if (status != MB_SUCCESS) {
				*bufferalloc = 0;
				done = MB_YES;
			}
			else {
				*bufferalloc = (int)(read_len + 1);
				buffer = (char *)*bufferptr;
			}
		}

		/* read the next record header - set read_kind value */
		status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);

		/* if valid read the record type */
		if (status == MB_SUCCESS) {

			switch (dgm_index->emdgm_type) {

				case IIP:
				    /* #IIP - Info Installation PU */
					status = mbr_kemkmall_rd_iip(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case IOP:
				    /* #IOP -  Runtime datagram */
					status = mbr_kemkmall_rd_iop(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case SPO:
				    /* #SPO - Sensor POsition data */
					status = mbr_kemkmall_rd_spo(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case SKM:
				    /* #SKM - KM binary sensor data */
					status = mbr_kemkmall_rd_skm(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case SVP:
				    /* #SVP - Sound Velocity Profile */
					status = mbr_kemkmall_rd_svp(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case SVT:
					/* #SVP - Sensor sound Velocity measured at Transducer */
					status = mbr_kemkmall_rd_svt(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case SCL:
				    /* #SCL - Sensor CLock datagram */
					status = mbr_kemkmall_rd_scl(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case SDE:
				    /* #SDE - Sensor DEpth data */
					status = mbr_kemkmall_rd_sde(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case SHI:
                    /* #SHI - Sensor HeIght data */
					status = mbr_kemkmall_rd_shi(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case SHA:
				    /* #SHA - Sensor HeAding */
					status = mbr_kemkmall_rd_sha(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case MRZ:
				    /* #MRZ - multibeam data for raw range,
                    depth, reflectivity, seabed image(SI) etc. */
					status = mbr_kemkmall_rd_mrz(verbose, buffer, store_ptr, error);
					/* not done yet, keep going to insure multi-TX/RX and dual-swath modes are handled correctly. */
					break;

				case MWC:
				    /* #MWC - multibeam water column datagram */
					status = mbr_kemkmall_rd_mwc(verbose, buffer, store_ptr, error);
					/* not done yet, keep going to insure multi-TX/RX and dual-swath modes are handled correctly. */
					break;

				case CPO:
					/* #CPO - Compatibility position sensor data */
					status = mbr_kemkmall_rd_cpo(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case CHE:
					/* #CHE - Compatibility heave data */
					status = mbr_kemkmall_rd_che(verbose, buffer, store_ptr, error);
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				case UNKNOWN:
                    /* Unknown datagram format */
					status = mbr_kemkmall_rd_unknown(verbose, buffer, store_ptr, error); // TODO: implement!
					if (status == MB_SUCCESS)
						done = MB_YES;
					break;

				default:
					/* should never get here */
                    status = MB_FAILURE;
                    done = MB_YES;
					break;

			}

            if (status != MB_SUCCESS) {
                skip++;
                /* report problem */
                if (verbose >= 0) {
                    fprintf(stderr, "\nThe MBF_KEMKMALL module skipped data between identified\n"
                                    "data records. Something is broken, most probably the data...\n"
                                    "However, the data may include a data record type that we\n"
                                    "haven't seen yet, or there could be an error in the code.\n"
                                    "If skipped data are reported multiple times, \n"
                                    "we recommend you send a data sample and problem \n"
                                    "description to the MB-System team \n"
                                    "(caress@mbari.org and dale@ldeo.columbia.edu)\n"
                                    "Have a nice day...\n");
                    fprintf(stderr, "MBF_KEMKMALL skipped %d record(s). Last skipped record was of type %s.\n",
                            skip, dgm_index->header.dgmType);
                }

            }

			/* increment the count */
            store->dgm_count++;
            (*dgm_id)++;

            /* and get the next datagram */
            dgm_index++;
            *dgm_index = dgm_index_table->indextable[*dgm_id];

            /* wrap up if the next dgm is associated with a different ping. */
            if (dgm_index->ping_num != ping_num)
                done = MB_YES;
		}

		/* set done if read failure */
		else {
			done = MB_YES;
		}
	}

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

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
int mbr_kemkmall_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_wr_data";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
    // struct mbsys_kmbes_index_table *dgm_index_table;
    struct mbsys_kmbes_index *dgm_index;
    size_t write_len;
    char **bufferptr;
    char *buffer;
    int *bufferalloc;
    int size;


	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

    /* get pointer to mbio descriptor */
    mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

    /* get buffer and related vars from mbio saved values */
    bufferptr = (char **)&mb_io_ptr->raw_data;
    bufferalloc = (int *)&mb_io_ptr->structure_size;
	buffer = (char *)*bufferptr;

    /* get pointer to raw data structure */
    store = (struct mbsys_kmbes_struct *)store_ptr;

    for (int i=0; i<store->dgm_count; i++) {

        size = 0;
        store->dgm_count_id = i;
        dgm_index = &store->dgm_index[i];

        /* write the current data record type */
        switch (dgm_index->emdgm_type) {

            case IIP:
                /* #IIP - Info Installation PU */
                status = mbr_kemkmall_wr_iip(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

            case IOP:
                /* #IOP -  Runtime datagram */
                status = mbr_kemkmall_wr_iop(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

            case SPO:
                /* #SPO - Sensor POsition data */
                status = mbr_kemkmall_wr_spo(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

            case SKM:
                /* #SKM - KM binary sensor data */
                status = mbr_kemkmall_wr_skm(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

            case SVP:
                /* #SVP - Sound Velocity Profile */
                status = mbr_kemkmall_wr_svp(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

			case SVT:
				/* #SVT - Sensor sound Velocity measured at Transducer */
				status = mbr_kemkmall_wr_svt(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
				break;

            case SCL:
                /* #SCL - Sensor CLock datagram */
                status = mbr_kemkmall_wr_scl(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

            case SDE:
                /* #SDE - Sensor DEpth data */
                status = mbr_kemkmall_wr_sde(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

            case SHI:
                /* #SHI - Sensor HeIght data */
                status = mbr_kemkmall_wr_shi(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

            case SHA:
                /* #SHA - Sensor HeAding */
                status = mbr_kemkmall_wr_sha(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

            case MRZ:
                /* #MRZ - multibeam data for raw range,
                depth, reflectivity, seabed image(SI) etc. */
                status = mbr_kemkmall_wr_mrz(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

            case MWC:
                /* #MWC - multibeam water column datagram */
                status = mbr_kemkmall_wr_mwc(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
                break;

			case CPO:
				/* #CPO - Compatibility position sensor data */
				status = mbr_kemkmall_wr_cpo(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
				break;

			case CHE:
				/* #CHE - Compatibility heave data */
				status = mbr_kemkmall_wr_che(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
				break;

            case UNKNOWN:
                /* Unknown datagram format */
                status = mbr_kemkmall_wr_unknown(verbose, bufferalloc, bufferptr, store_ptr, &size, error); // TODO: implement!
                break;

            default:
                /* should never get here */
                status = MB_FAILURE;
                break;
        }

		if (status == MB_SUCCESS) {
			buffer = (char *)*bufferptr;
			write_len = (size_t)size;
			status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
		}
    }


#ifdef MBR_KEMKMALL_DEBUG
	fprintf(stderr, "KEMKMALL DATA WRITTEN: type:%d status:%d error:%d\n\n", store->kind, status, *error);
#endif

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
int mbr_kemkmall_rd_hdr(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_hdr";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_header *header;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	header = &(dgm_index->header);

	/* extract the data */
	index = 0;
	mb_get_binary_int(MB_YES, &buffer[index], &(header->numBytesDgm));
	index += 4;
	memcpy(&(header->dgmType), &buffer[index], sizeof(header->dgmType));
	index += 4;
	header->dgmVersion = buffer[index];
	index++;
	header->systemID = buffer[index];
	index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(header->echoSounderID));
	index += 2;
	mb_get_binary_int(MB_YES, &buffer[index], &(header->time_sec));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(header->time_nanosec));

	/* determine datagram type */
	mbr_kemkmall_id_dgm(verbose, dgm_index, error);

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_HEADER;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       header->numBytesDgm:                    %u\n", header->numBytesDgm);
		fprintf(stderr, "dbg5       header->dgmType:                        %s\n", header->dgmType);
		fprintf(stderr, "dbg5       header->dgmVersion:                     %u\n", header->dgmVersion);
		fprintf(stderr, "dbg5       header->systemID:                       %u\n", header->systemID);
		fprintf(stderr, "dbg5       header->echoSounderID:                  %u\n", header->echoSounderID);
		fprintf(stderr, "dbg5       header->time_sec:                       %u\n", header->time_sec);
		fprintf(stderr, "dbg5       header->time_nanosec:                   %u\n", header->time_nanosec);
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
};

int mbr_kemkmall_rd_spo(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_spo";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_spo *spo;
	size_t numBytesRawSensorData;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	spo = &(store->spo);

	/* get header */
	spo->header = dgm_index->header;

	/* calc number of bytes for raw sensor data */
	numBytesRawSensorData = spo->header.numBytesDgm - MBSYS_KMBES_SPO_VAR_OFFSET;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	/* common part */
	mb_get_binary_short(MB_YES, &buffer[index], &(spo->cmnPart.numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(spo->cmnPart.sensorSystem));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(spo->cmnPart.sensorStatus));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(spo->cmnPart.padding));
	index += 2;

	/* sensor data block */
	mb_get_binary_int(MB_YES, &buffer[index], &(spo->sensorData.timeFromSensor_sec));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(spo->sensorData.timeFromSensor_nanosec));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(spo->sensorData.posFixQuality_m));
	index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(spo->sensorData.correctedLat_deg));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(spo->sensorData.correctedLong_deg));
	index += 8;
	mb_get_binary_float(MB_YES, &buffer[index], &(spo->sensorData.speedOverGround_mPerSec));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(spo->sensorData.courseOverGround_deg));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(spo->sensorData.ellipsoidHeightReRefPoint_m));
	index += 4;
	memcpy(&(spo->sensorData.posDataFromSensor), &buffer[index], numBytesRawSensorData);

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_NAV;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       spo->header.numBytesDgm:                %u\n", spo->header.numBytesDgm);
		fprintf(stderr, "dbg5       spo->header.dgmType:                    %s\n", spo->header.dgmType);
		fprintf(stderr, "dbg5       spo->header.dgmVersion:                 %u\n", spo->header.dgmVersion);
		fprintf(stderr, "dbg5       spo->header.systemID:                   %u\n", spo->header.systemID);
		fprintf(stderr, "dbg5       spo->header.echoSounderID:              %u\n", spo->header.echoSounderID);
		fprintf(stderr, "dbg5       spo->header.time_sec:                   %u\n", spo->header.time_sec);
		fprintf(stderr, "dbg5       spo->header.time_nanosec:               %u\n", spo->header.time_nanosec);

		fprintf(stderr, "dbg5       spo->cmnPart.numBytesCmnPart:           %u\n", spo->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       spo->cmnPart.sensorSystem:              %u\n", spo->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       spo->cmnPart.sensorStatus:              %u\n", spo->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       spo->cmnPart.padding:                   %u\n", spo->cmnPart.padding);

		fprintf(stderr, "dbg5       spo->sensorData.timeFromSensor_sec:     %u\n", spo->sensorData.timeFromSensor_sec);
		fprintf(stderr, "dbg5       spo->sensorData.timeFromSensor_nanosec: %u\n", spo->sensorData.timeFromSensor_nanosec);
		fprintf(stderr, "dbg5       spo->sensorData.posFixQuality_m:        %f\n", spo->sensorData.posFixQuality_m);
		fprintf(stderr, "dbg5       spo->sensorData.correctedLat_deg:       %f\n", spo->sensorData.correctedLat_deg);
		fprintf(stderr, "dbg5       spo->sensorData.correctedLong_deg:      %f\n", spo->sensorData.correctedLong_deg);
		fprintf(stderr, "dbg5       spo->sensorData.speedOverGround_mPerSec:%f\n", spo->sensorData.speedOverGround_mPerSec);
		fprintf(stderr, "dbg5       spo->sensorData.courseOverGround_deg:   %f\n", spo->sensorData.courseOverGround_deg);
		fprintf(stderr, "dbg5       spo->sensorData.ellipsoidHeightReRefPoint_m:   %f\n", spo->sensorData.ellipsoidHeightReRefPoint_m);
		fprintf(stderr, "dbg5       spo->sensorData.posDataFromSensor:      %s\n", spo->sensorData.posDataFromSensor);
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
};

int mbr_kemkmall_rd_skm(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_skm";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_skm *skm;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	skm = &(store->skm);

	/* get header */
	skm->header = dgm_index->header;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	/* info part */
	mb_get_binary_short(MB_YES, &buffer[index], &(skm->infoPart.numBytesInfoPart));
	index += 2;
	skm->infoPart.sensorSystem = buffer[index];
	index++;
	skm->infoPart.sensorStatus = buffer[index];
	index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(skm->infoPart.sensorInputFormat));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(skm->infoPart.numSamplesArray));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(skm->infoPart.numBytesPerSample));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(skm->infoPart.padding));
	index += 2;

	for (int i=0; i<(skm->infoPart.numSamplesArray); i++ ) {

		/* KMbinary */
		memcpy(&(skm->sample[i].KMdefault.dgmType), &buffer[index], 4);
		index += 4;
		mb_get_binary_short(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.numBytesDgm));
		index += 2;
		mb_get_binary_short(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.dgmVersion));
		index += 2;
		mb_get_binary_int(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.time_sec));
		index += 4;
		mb_get_binary_int(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.time_nanosec));
		index += 4;
		mb_get_binary_int(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.status));
		index += 4;
		mb_get_binary_double(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.latitude_deg));
		index += 8;
		mb_get_binary_double(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.longitude_deg));
		index += 8;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.ellipsoidHeight_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.roll_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.pitch_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.heading_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.heave_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.rollRate));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.pitchRate));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.yawRate));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.velNorth));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.velEast));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.velDown));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.latitudeError_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.longitudeError_m));
		index += 4;
        mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.ellipsoidHeightError_m));
        index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.rollError_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.pitchError_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.headingError_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.heaveError_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.northAcceleration));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.eastAcceleration));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].KMdefault.downAcceleration));
		index += 4;

		/* KMdelayedHeave */
		mb_get_binary_int(MB_YES, &buffer[index], &(skm->sample[i].delayedHeave.time_sec));
		index += 4;
		mb_get_binary_int(MB_YES, &buffer[index], &(skm->sample[i].delayedHeave.time_nanosec));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(skm->sample[i].delayedHeave.delayedHeave_m));
		index += 4;
	}

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_GEN_SENS;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       skm->header.numBytesDgm:                %u\n", skm->header.numBytesDgm);
		fprintf(stderr, "dbg5       skm->header.dgmType:                    %s\n", skm->header.dgmType);
		fprintf(stderr, "dbg5       skm->header.dgmVersion:                 %u\n", skm->header.dgmVersion);
		fprintf(stderr, "dbg5       skm->header.systemID:                   %u\n", skm->header.systemID);
		fprintf(stderr, "dbg5       skm->header.echoSounderID:              %u\n", skm->header.echoSounderID);
		fprintf(stderr, "dbg5       skm->header.time_sec:                   %u\n", skm->header.time_sec);
		fprintf(stderr, "dbg5       skm->header.time_nanosec:               %u\n", skm->header.time_nanosec);

		fprintf(stderr, "dbg5       skm->infoPart.numBytesInfoPart:         %u\n", skm->infoPart.numBytesInfoPart);
		fprintf(stderr, "dbg5       skm->infoPart.sensorSystem:             %u\n", skm->infoPart.sensorSystem);
		fprintf(stderr, "dbg5       skm->infoPart.sensorStatus:             %u\n", skm->infoPart.sensorStatus);
		fprintf(stderr, "dbg5       skm->infoPart.sensorInputFormat:        %u\n", skm->infoPart.sensorInputFormat);
		fprintf(stderr, "dbg5       skm->infoPart.numSamplesArray:          %u\n", skm->infoPart.numSamplesArray);
		fprintf(stderr, "dbg5       skm->infoPart.numBytesPerSample:        %u\n", skm->infoPart.numBytesPerSample);
		fprintf(stderr, "dbg5       skm->infoPart.padding:                  %u\n", skm->infoPart.padding);

		for (int i=0; i<(skm->infoPart.numSamplesArray); i++ ) {
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.dgmType:                %s\n", i, skm->sample[i].KMdefault.dgmType);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.numBytesDgm:            %u\n", i, skm->sample[i].KMdefault.numBytesDgm);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.dgmVersion:             %u\n", i, skm->sample[i].KMdefault.dgmVersion);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.time_sec:               %u\n", i, skm->sample[i].KMdefault.time_sec);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.time_nanosec:           %u\n", i, skm->sample[i].KMdefault.time_nanosec);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.status:                 %u\n", i, skm->sample[i].KMdefault.status);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.latitude_deg:           %f\n", i, skm->sample[i].KMdefault.latitude_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.longitude_deg:          %f\n", i, skm->sample[i].KMdefault.longitude_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.ellipsoidHeight_m:      %f\n", i, skm->sample[i].KMdefault.ellipsoidHeight_m);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.roll_deg:               %f\n", i, skm->sample[i].KMdefault.roll_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.pitch_deg:              %f\n", i, skm->sample[i].KMdefault.pitch_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.heading_deg:            %f\n", i, skm->sample[i].KMdefault.heading_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.heave_m:                %f\n", i, skm->sample[i].KMdefault.heave_m);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.rollRate:               %f\n", i, skm->sample[i].KMdefault.rollRate);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.pitchRate:              %f\n", i, skm->sample[i].KMdefault.pitchRate);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.yawRate:                %f\n", i, skm->sample[i].KMdefault.yawRate);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.velNorth:               %f\n", i, skm->sample[i].KMdefault.velNorth);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.velEast:                %f\n", i, skm->sample[i].KMdefault.velEast);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.velDown:                %f\n", i, skm->sample[i].KMdefault.velDown);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.latitudeError_m:        %f\n", i, skm->sample[i].KMdefault.latitudeError_m);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.longitudeError_m:       %f\n", i, skm->sample[i].KMdefault.longitudeError_m);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.ellipsoidHeightError_m: %f\n", i, skm->sample[i].KMdefault.ellipsoidHeightError_m);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.rollError_deg:          %f\n", i, skm->sample[i].KMdefault.rollError_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.pitchError_deg:         %f\n", i, skm->sample[i].KMdefault.pitchError_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.headingError_deg:       %f\n", i, skm->sample[i].KMdefault.headingError_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.heaveError_m:           %f\n", i, skm->sample[i].KMdefault.heaveError_m);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.northAcceleration:      %f\n", i, skm->sample[i].KMdefault.northAcceleration);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.eastAcceleration:       %f\n", i, skm->sample[i].KMdefault.eastAcceleration);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.downAcceleration:       %f\n", i, skm->sample[i].KMdefault.downAcceleration);

			//
			fprintf(stderr, "dbg5       skm->sample[%3d].delayedHeave.time_sec:            %u\n", i, skm->sample[i].delayedHeave.time_sec);
			fprintf(stderr, "dbg5       skm->sample[%3d].delayedHeave.time_nanosec:        %u\n", i, skm->sample[i].delayedHeave.time_nanosec);
			fprintf(stderr, "dbg5       skm->sample[%3d].delayedHeave.delayedHeave_m:      %f\n", i, skm->sample[i].delayedHeave.delayedHeave_m);
		}

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
};

int mbr_kemkmall_rd_svp(int verbose, char *buffer, void *store_ptr, int *error){
	char *function_name = "mbr_kemkmall_rd_svp";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_svp *svp;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	svp = &(store->svp);

	/* get header */
	svp->header = dgm_index->header;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	/* svp common part */
	mb_get_binary_short(MB_YES, &buffer[index], &(svp->numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(svp->numSamples));
	index += 2;
	memcpy(&svp->sensorFormat, &buffer[index], 4);
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(svp->time_sec));
	index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &(svp->latitude_deg));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(svp->longitude_deg));
	index += 8;

	/* svp data block */
	for (int i=0; i<(svp->numSamples); i++ ) {
		mb_get_binary_float(MB_YES, &buffer[index], &(svp->sensorData[i].depth_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(svp->sensorData[i].soundVelocity_mPerSec));
		index += 4;
		mb_get_binary_int(MB_YES, &buffer[index], &(svp->sensorData[i].padding));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(svp->sensorData[i].temp_C));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(svp->sensorData[i].salinity));
		index += 4;
	}

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_SSV;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       svp->header.numBytesDgm:                %u\n", svp->header.numBytesDgm);
		fprintf(stderr, "dbg5       svp->header.dgmType:                    %s\n", svp->header.dgmType);
		fprintf(stderr, "dbg5       svp->header.dgmVersion:                 %u\n", svp->header.dgmVersion);
		fprintf(stderr, "dbg5       svp->header.systemID:                   %u\n", svp->header.systemID);
		fprintf(stderr, "dbg5       svp->header.echoSounderID:              %u\n", svp->header.echoSounderID);
		fprintf(stderr, "dbg5       svp->header.time_sec:                   %u\n", svp->header.time_sec);
		fprintf(stderr, "dbg5       svp->header.time_nanosec:               %u\n", svp->header.time_nanosec);

		fprintf(stderr, "dbg5       svp->numBytesCmnPart:            %u\n", svp->numBytesCmnPart);
		fprintf(stderr, "dbg5       svp->numSamples:                 %u\n", svp->numSamples);
		fprintf(stderr, "dbg5       svp->sensorFormat:               %s\n", svp->sensorFormat);
		fprintf(stderr, "dbg5       svp->time_sec:                   %u\n", svp->time_sec);
		fprintf(stderr, "dbg5       svp->latitude_deg:               %f\n", svp->latitude_deg);
		fprintf(stderr, "dbg5       svp->longitude_deg:              %f\n", svp->longitude_deg);

		for (int i = 0; i < (svp->numSamples); i++) {
			fprintf(stderr, "dbg5       svp->sensorData[%3d].depth_m:                      %f\n", i, svp->sensorData[i].depth_m);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].soundVelocity_mPerSec:        %f\n", i, svp->sensorData[i].soundVelocity_mPerSec);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].padding:                      %d\n", i, svp->sensorData[i].padding);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].temp_C:                       %f\n", i, svp->sensorData[i].temp_C);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].salinity:                     %f\n", i, svp->sensorData[i].salinity);
		}
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
};

int mbr_kemkmall_rd_svt(int verbose, char *buffer, void *store_ptr, int *error){
	char *function_name = "mbr_kemkmall_rd_svt";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_svt *svt;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	svt = &(store->svt);

	/* get header */
	svt->header = dgm_index->header;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	/* svp info */
	mb_get_binary_short(MB_YES, &buffer[index], &(svt->infoPart.numBytesInfoPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(svt->infoPart.sensorStatus));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(svt->infoPart.sensorInputFormat));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(svt->infoPart.numSamplesArray));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(svt->infoPart.numBytesPerSample));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(svt->infoPart.sensorDataContents));
	index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(svt->infoPart.filterTime_sec));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(svt->infoPart.soundVelocity_mPerSec_offset));
	index += 4;

	/* svt data blocks */
	for (int i=0; i<(svt->infoPart.numSamplesArray); i++ ) {
		mb_get_binary_int(MB_YES, &buffer[index], &(svt->sensorData[i].time_sec));
		index += 4;
		mb_get_binary_int(MB_YES, &buffer[index], &(svt->sensorData[i].time_nanosec));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(svt->sensorData[i].soundVelocity_mPerSec));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(svt->sensorData[i].temp_C));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(svt->sensorData[i].pressure_Pa));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(svt->sensorData[i].salinity));
		index += 4;
	}

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_SSV;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       svt->header.numBytesDgm:                %u\n", svt->header.numBytesDgm);
		fprintf(stderr, "dbg5       svt->header.dgmType:                    %s\n", svt->header.dgmType);
		fprintf(stderr, "dbg5       svt->header.dgmVersion:                 %u\n", svt->header.dgmVersion);
		fprintf(stderr, "dbg5       svt->header.systemID:                   %u\n", svt->header.systemID);
		fprintf(stderr, "dbg5       svt->header.echoSounderID:              %u\n", svt->header.echoSounderID);
		fprintf(stderr, "dbg5       svt->header.time_sec:                   %u\n", svt->header.time_sec);
		fprintf(stderr, "dbg5       svt->header.time_nanosec:               %u\n", svt->header.time_nanosec);

		fprintf(stderr, "dbg5       svt->infoPart.numBytesInfoPart:         %u\n", svt->infoPart.numBytesInfoPart);
		fprintf(stderr, "dbg5       svt->infoPart.sensorStatus:             %u\n", svt->infoPart.sensorStatus);
		fprintf(stderr, "dbg5       svt->infoPart.sensorInputFormat:        %u\n", svt->infoPart.sensorInputFormat);
		fprintf(stderr, "dbg5       svt->infoPart.numSamplesArray:          %u\n", svt->infoPart.numSamplesArray);
		fprintf(stderr, "dbg5       svt->infoPart.sensorDataContents:       %u\n", svt->infoPart.sensorDataContents);
		fprintf(stderr, "dbg5       svt->infoPart.filterTime_sec:           %f\n", svt->infoPart.filterTime_sec);
		fprintf(stderr, "dbg5       svt->infoPart.soundVelocity_mPerSec_offset: %f\n", svt->infoPart.soundVelocity_mPerSec_offset);

		for (int i = 0; i < (svt->infoPart.numSamplesArray); i++) {
			fprintf(stderr, "dbg5       svp->sensorData[%3d].time_sec:                     %u\n", i, svt->sensorData[i].time_sec);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].time_nanosec:                 %u\n", i, svt->sensorData[i].time_nanosec);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].soundVelocity_mPerSec:        %f\n", i, svt->sensorData[i].soundVelocity_mPerSec);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].temp_C:                       %f\n", i, svt->sensorData[i].temp_C);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].pressure_Pa:                  %f\n", i, svt->sensorData[i].pressure_Pa);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].salinity:                     %f\n", i, svt->sensorData[i].salinity);
		}
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
};

int mbr_kemkmall_rd_scl(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_scl";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_scl *scl;
	size_t numBytesRawSensorData;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	scl = &(store->scl);

	/* get header */
	scl->header = dgm_index->header;

	/* calc number of bytes for raw sensor data */
	numBytesRawSensorData = scl->header.numBytesDgm - MBSYS_KMBES_SCL_VAR_OFFSET;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	// common part
	mb_get_binary_short(MB_YES, &buffer[index], &(scl->cmnPart.numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(scl->cmnPart.sensorSystem));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(scl->cmnPart.sensorStatus));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(scl->cmnPart.padding));
	index += 2;

	// sensor data block
	mb_get_binary_float(MB_YES, &buffer[index], &(scl->sensorData.offset_sec));
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &(scl->sensorData.clockDevPU_nanosec));
	index += 4;
	memcpy(&(scl->sensorData.dataFromSensor), &buffer[index], numBytesRawSensorData);

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_CLOCK;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       scl->header.numBytesDgm:                %u\n", scl->header.numBytesDgm);
		fprintf(stderr, "dbg5       scl->header.dgmType:                    %s\n", scl->header.dgmType);
		fprintf(stderr, "dbg5       scl->header.dgmVersion:                 %u\n", scl->header.dgmVersion);
		fprintf(stderr, "dbg5       scl->header.systemID:                   %u\n", scl->header.systemID);
		fprintf(stderr, "dbg5       scl->header.echoSounderID:              %u\n", scl->header.echoSounderID);
		fprintf(stderr, "dbg5       scl->header.time_sec:                   %u\n", scl->header.time_sec);
		fprintf(stderr, "dbg5       scl->header.time_nanosec:               %u\n", scl->header.time_nanosec);

		fprintf(stderr, "dbg5       scl->cmnPart.numBytesCmnPart:           %u\n", scl->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       scl->cmnPart.sensorSystem:              %u\n", scl->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       scl->cmnPart.sensorStatus:              %u\n", scl->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       scl->cmnPart.padding:                   %u\n", scl->cmnPart.padding);

		fprintf(stderr, "dbg5       scl->sensorData.offset_sec:             %f\n", scl->sensorData.offset_sec);
		fprintf(stderr, "dbg5       scl->sensorData.clockDevPU_nanosec:     %d\n", scl->sensorData.clockDevPU_nanosec);
		fprintf(stderr, "dbg5       scl->sensorData.dataFromSensor:         %s\n", scl->sensorData.dataFromSensor);
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
};

int mbr_kemkmall_rd_sde(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_sde";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_sde *sde;
	size_t numBytesRawSensorData;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	sde = &(store->sde);

	/* get header */
	sde->header = dgm_index->header;

	/* calc number of bytes for raw sensor data */
	numBytesRawSensorData = sde->header.numBytesDgm - MBSYS_KMBES_SDE_VAR_OFFSET;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	// common part
	mb_get_binary_short(MB_YES, &buffer[index], &(sde->cmnPart.numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(sde->cmnPart.sensorSystem));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(sde->cmnPart.sensorStatus));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(sde->cmnPart.padding));
	index += 2;

	// sensor data block
	mb_get_binary_float(MB_YES, &buffer[index], &(sde->sensorData.depthUsed_m));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sde->sensorData.offset));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sde->sensorData.scale));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sde->sensorData.latitude_deg));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(sde->sensorData.longitude_deg));
	index += 4;
	memcpy(&(sde->sensorData.dataFromSensor), &buffer[index], numBytesRawSensorData);

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_SONARDEPTH;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       sde->header.numBytesDgm:                %u\n", sde->header.numBytesDgm);
		fprintf(stderr, "dbg5       sde->header.dgmType:                    %s\n", sde->header.dgmType);
		fprintf(stderr, "dbg5       sde->header.dgmVersion:                 %u\n", sde->header.dgmVersion);
		fprintf(stderr, "dbg5       sde->header.systemID:                   %u\n", sde->header.systemID);
		fprintf(stderr, "dbg5       sde->header.echoSounderID:              %u\n", sde->header.echoSounderID);
		fprintf(stderr, "dbg5       sde->header.time_sec:                   %u\n", sde->header.time_sec);
		fprintf(stderr, "dbg5       sde->header.time_nanosec:               %u\n", sde->header.time_nanosec);

		fprintf(stderr, "dbg5       sde->cmnPart.numBytesCmnPart:           %u\n", sde->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       sde->cmnPart.sensorSystem:              %u\n", sde->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       sde->cmnPart.sensorStatus:              %u\n", sde->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       sde->cmnPart.padding:                   %u\n", sde->cmnPart.padding);

		fprintf(stderr, "dbg5       sde->sensorData.depthUsed_m:            %f\n", sde->sensorData.depthUsed_m);
		fprintf(stderr, "dbg5       sde->sensorData.offset:                 %f\n", sde->sensorData.offset);
		fprintf(stderr, "dbg5       sde->sensorData.scale:                  %f\n", sde->sensorData.scale);
		fprintf(stderr, "dbg5       sde->sensorData.latitude_deg:           %f\n", sde->sensorData.latitude_deg);
		fprintf(stderr, "dbg5       sde->sensorData.longitude_deg:          %f\n", sde->sensorData.longitude_deg);
		fprintf(stderr, "dbg5       sde->sensorData.dataFromSensor:         %s\n", sde->sensorData.dataFromSensor);
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
};

int mbr_kemkmall_rd_shi(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_shi";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_shi *shi;
	size_t numBytesRawSensorData;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	shi = &(store->shi);

	/* get header */
	shi->header = dgm_index->header;

	/* calc number of bytes for raw sensor data */
	numBytesRawSensorData = shi->header.numBytesDgm - MBSYS_KMBES_SHI_VAR_OFFSET;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	// common part
	mb_get_binary_short(MB_YES, &buffer[index], &(shi->cmnPart.numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(shi->cmnPart.sensorSystem));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(shi->cmnPart.sensorStatus));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(shi->cmnPart.padding));
	index += 2;

	// sensor data block
	mb_get_binary_short(MB_YES, &buffer[index], &(shi->sensorData.sensorType));
	index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(shi->sensorData.heigthUsed_m));
	index += 4;
	memcpy(&(shi->sensorData.dataFromSensor), &buffer[index], numBytesRawSensorData);

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_HEIGHT;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       shi->header.numBytesDgm:                %u\n", shi->header.numBytesDgm);
		fprintf(stderr, "dbg5       shi->header.dgmType:                    %s\n", shi->header.dgmType);
		fprintf(stderr, "dbg5       shi->header.dgmVersion:                 %u\n", shi->header.dgmVersion);
		fprintf(stderr, "dbg5       shi->header.systemID:                   %u\n", shi->header.systemID);
		fprintf(stderr, "dbg5       shi->header.echoSounderID:              %u\n", shi->header.echoSounderID);
		fprintf(stderr, "dbg5       shi->header.time_sec:                   %u\n", shi->header.time_sec);
		fprintf(stderr, "dbg5       shi->header.time_nanosec:               %u\n", shi->header.time_nanosec);

		fprintf(stderr, "dbg5       shi->cmnPart.numBytesCmnPart:           %u\n", shi->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       shi->cmnPart.sensorSystem:              %u\n", shi->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       shi->cmnPart.sensorStatus:              %u\n", shi->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       shi->cmnPart.padding:                   %u\n", shi->cmnPart.padding);

		fprintf(stderr, "dbg5       shi->sensorData.sensorType:             %u\n", shi->sensorData.sensorType);
		fprintf(stderr, "dbg5       shi->sensorData.heigthUsed_m:           %f\n", shi->sensorData.heigthUsed_m);
		fprintf(stderr, "dbg5       shi->sensorData.dataFromSensor:         %s\n", shi->sensorData.dataFromSensor);
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
};


int mbr_kemkmall_rd_sha(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_sha";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_sha *sha;
	int index;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	sha = &(store->sha);

	/* get header */
	sha->header = dgm_index->header;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	// common part
	mb_get_binary_short(MB_YES, &buffer[index], &(sha->cmnPart.numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(sha->cmnPart.sensorSystem));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(sha->cmnPart.sensorStatus));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(sha->cmnPart.padding));
	index += 2;

	// sensor info
	mb_get_binary_short(MB_YES, &buffer[index], &(sha->dataInfo.numBytesInfoPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(sha->dataInfo.numSamplesArray));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(sha->dataInfo.numBytesPerSample));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(sha->dataInfo.numBytesRawSensorData));
	index += 2;

	// sensor data blocks
	for (i = 0; i<(sha->dataInfo.numSamplesArray); i++) {
		mb_get_binary_int(MB_YES, &buffer[index], &(sha->sensorData[i].timeSinceRecStart_nanosec));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(sha->sensorData[i].headingCorrected_deg));
		index += 4;
		memcpy(&(sha->sensorData[i].dataFromSensor), &buffer[index], sha->dataInfo.numBytesRawSensorData);
		index += sha->dataInfo.numBytesRawSensorData;
	}


	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_HEADING;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       sha->header.numBytesDgm:                %u\n", sha->header.numBytesDgm);
		fprintf(stderr, "dbg5       sha->header.dgmType:                    %s\n", sha->header.dgmType);
		fprintf(stderr, "dbg5       sha->header.dgmVersion:                 %u\n", sha->header.dgmVersion);
		fprintf(stderr, "dbg5       sha->header.systemID:                   %u\n", sha->header.systemID);
		fprintf(stderr, "dbg5       sha->header.echoSounderID:              %u\n", sha->header.echoSounderID);
		fprintf(stderr, "dbg5       sha->header.time_sec:                   %u\n", sha->header.time_sec);
		fprintf(stderr, "dbg5       sha->header.time_nanosec:               %u\n", sha->header.time_nanosec);

		fprintf(stderr, "dbg5       sha->cmnPart.numBytesCmnPart:           %u\n", sha->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       sha->cmnPart.sensorSystem:              %u\n", sha->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       sha->cmnPart.sensorStatus:              %u\n", sha->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       sha->cmnPart.padding:                   %u\n", sha->cmnPart.padding);

		fprintf(stderr, "dbg5       sha->dataInfo.numBytesInfoPart:         %u\n", sha->dataInfo.numBytesInfoPart);
		fprintf(stderr, "dbg5       sha->dataInfo.numSamplesArray:          %u\n", sha->dataInfo.numSamplesArray);
		fprintf(stderr, "dbg5       sha->dataInfo.numBytesPerSample:        %u\n", sha->dataInfo.numBytesPerSample);
		fprintf(stderr, "dbg5       sha->dataInfo.numBytesRawSensorData:    %u\n", sha->dataInfo.numBytesRawSensorData);

		for (int i = 0; i<(sha->dataInfo.numSamplesArray); i++) {
			fprintf(stderr, "dbg5       sha->sensorData[%3d].timeSinceRecStart_nanosec: %u\n", i, sha->sensorData[i].timeSinceRecStart_nanosec);
			fprintf(stderr, "dbg5       sha->sensorData[%3d].headingCorrected_deg:      %f\n", i, sha->sensorData[i].headingCorrected_deg);
			fprintf(stderr, "dbg5       sha->sensorData[%3d].dataFromSensor:            %s\n", i, sha->sensorData[i].dataFromSensor);
		}
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
};

int mbr_kemkmall_rd_mrz(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_mrz";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_mrz *mrz;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *) buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *) store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *) store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	mrz = &(store->mrz[dgm_index->rx_index]);

	/* EMdgmHeader - data header information */
	mrz->header = dgm_index->header;

	/* get the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	/* EMdgmMpartition - data partition information */
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->partition.numOfDgms));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->partition.dgmNum));
	index += 2;

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr,"dbg5       numOfDgms = %d\n", mrz->partition.numOfDgms);
		fprintf(stderr,"dbg5       dgmNum    = %d\n", mrz->partition.dgmNum);
	}

	/* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->cmnPart.numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->cmnPart.pingCnt));
	index += 2;
	mrz->cmnPart.rxFansPerPing = buffer[index];
	index++;
	mrz->cmnPart.rxFanIndex = buffer[index];
	index++;
	mrz->cmnPart.swathsPerPing = buffer[index];
	index++;
	mrz->cmnPart.swathAlongPosition = buffer[index];
	index++;
	mrz->cmnPart.txTransducerInd = buffer[index];
	index++;
	mrz->cmnPart.rxTransducerInd = buffer[index];
	index++;
	mrz->cmnPart.numRxTransducers = buffer[index];
	index++;
	mrz->cmnPart.algorithmType = buffer[index];
	index++;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr,"dbg5       numBytesCmnPart     = %d\n", mrz->cmnPart.numBytesCmnPart);
		fprintf(stderr,"dbg5       pingCnt             = %d\n", mrz->cmnPart.pingCnt);
		fprintf(stderr,"dbg5       rxFansPerPing       = %d\n", mrz->cmnPart.rxFansPerPing);
		fprintf(stderr,"dbg5       rxFanIndex          = %d\n", mrz->cmnPart.rxFanIndex);
		fprintf(stderr,"dbg5       swathsPerPing       = %d\n", mrz->cmnPart.swathsPerPing);
		fprintf(stderr,"dbg5       swathAlongPosition  = %d\n", mrz->cmnPart.swathAlongPosition);
		fprintf(stderr,"dbg5       txTransducerInd     = %d\n", mrz->cmnPart.txTransducerInd);
		fprintf(stderr,"dbg5       rxTransducerInd     = %d\n", mrz->cmnPart.rxTransducerInd);
		fprintf(stderr,"dbg5       numRxTransducers    = %d\n", mrz->cmnPart.numRxTransducers);
		fprintf(stderr,"dbg5       algorithmType       = %d\n", mrz->cmnPart.algorithmType);
	}

	/* EMdgmMRZ_pingInfo - ping info */
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.numBytesInfoData));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.padding0));
	index += 2;

	/* Ping info */
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.pingRate_Hz));
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

	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.padding1));
	index += 2;

	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.frequencyMode_Hz));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.freqRangeLowLim_Hz));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.freqRangeHighLim_Hz));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.maxTotalTxPulseLength_sec));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.maxEffTxPulseLength_sec));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.maxEffTxBandWidth_Hz));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.absCoeff_dBPerkm));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.portSectorEdge_deg));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.starbSectorEdge_deg));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.portMeanCov_deg));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.starbMeanCov_deg));
	index += 4;

	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.portMeanCov_m));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.starbMeanCov_m));
	index += 2;

	mrz->pingInfo.modeAndStabilisation = buffer[index];
	index++;
	mrz->pingInfo.runtimeFilter1 = buffer[index];
	index++;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.runtimeFilter2));
	index += 2;
	mb_get_binary_int(MB_YES, &buffer[index], &(mrz->pingInfo.pipeTrackingStatus));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.transmitArraySizeUsed_deg));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.receiveArraySizeUsed_deg));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.transmitPower_dB));
	index += 4;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.SLrampUpTimeRemaining));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.padding2));
	index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.yawAngle_deg));
	index += 4;

	/* Info of tx sector data block, EMdgmMRZ_txSectorInfo */
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.numTxSectors));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->pingInfo.numBytesPerTxSector));
	index += 2;

	/* Info at time of midpoint of first tx pulse */
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.headingVessel_deg));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.soundSpeedAtTxDepth_mPerSec));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.txTransducerDepth_m));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.z_waterLevelReRefPoint_m));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.x_kmallToall_m));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.y_kmallToall_m));
	index += 4;

	mrz->pingInfo.latLongInfo = buffer[index];
	index++;
	mrz->pingInfo.posSensorStatus = buffer[index];
	index++;
	mrz->pingInfo.attitudeSensorStatus = buffer[index];
	index++;
	mrz->pingInfo.padding2 = buffer[index];
	index++;

	mb_get_binary_double(MB_YES, &buffer[index], &(mrz->pingInfo.latitude_deg));
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &(mrz->pingInfo.longitude_deg));
	index += 8;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->pingInfo.ellipsoidHeightReRefPoint_m));
	index += 4;

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr,"dbg5       numBytesInfoData            = %d\n", mrz->pingInfo.numBytesInfoData);
		fprintf(stderr,"dbg5       padding0                    = %d\n", mrz->pingInfo.padding0);
		fprintf(stderr,"dbg5       pingRate_Hz                 = %f\n", mrz->pingInfo.pingRate_Hz);
		fprintf(stderr,"dbg5       beamSpacing                 = %d\n", mrz->pingInfo.beamSpacing);
		fprintf(stderr,"dbg5       depthMode                   = %d\n", mrz->pingInfo.depthMode);
		fprintf(stderr,"dbg5       subDepthMode                = %d\n", mrz->pingInfo.subDepthMode);
		fprintf(stderr,"dbg5       distanceBtwSwath            = %d\n", mrz->pingInfo.distanceBtwSwath);
		fprintf(stderr,"dbg5       detectionMode               = %d\n", mrz->pingInfo.detectionMode);
		fprintf(stderr,"dbg5       pulseForm                   = %d\n", mrz->pingInfo.pulseForm);
		fprintf(stderr,"dbg5       padding1                    = %d\n", mrz->pingInfo.padding1);
		fprintf(stderr,"dbg5       frequencyMode_Hz            = %f\n", mrz->pingInfo.frequencyMode_Hz);
		fprintf(stderr,"dbg5       freqRangeLowLim_Hz          = %f\n", mrz->pingInfo.freqRangeLowLim_Hz);
		fprintf(stderr,"dbg5       freqRangeHighLim_Hz         = %f\n", mrz->pingInfo.freqRangeHighLim_Hz);
		fprintf(stderr,"dbg5       maxEffTxPulseLength_sec     = %f\n", mrz->pingInfo.maxEffTxPulseLength_sec);
		fprintf(stderr,"dbg5       maxTotalTxPulseLength_sec   = %f\n", mrz->pingInfo.maxTotalTxPulseLength_sec);
		fprintf(stderr,"dbg5       maxEffTxBandWidth_Hz        = %f\n", mrz->pingInfo.maxEffTxBandWidth_Hz);
		fprintf(stderr,"dbg5       absCoeff_dBPerkm            = %f\n", mrz->pingInfo.absCoeff_dBPerkm);
		fprintf(stderr,"dbg5       portSectorEdge_deg          = %f\n", mrz->pingInfo.portSectorEdge_deg);
		fprintf(stderr,"dbg5       starbSectorEdge_deg         = %f\n", mrz->pingInfo.starbSectorEdge_deg);
		fprintf(stderr,"dbg5       portMeanCov_m               = %d\n", mrz->pingInfo.portMeanCov_m);
		fprintf(stderr,"dbg5       starbMeanCov_m              = %d\n", mrz->pingInfo.starbMeanCov_m);
		fprintf(stderr,"dbg5       modeAndStabilisation        = %d\n", mrz->pingInfo.modeAndStabilisation);
		fprintf(stderr,"dbg5       runtimeFilter1              = %d\n", mrz->pingInfo.runtimeFilter1);
		fprintf(stderr,"dbg5       runtimeFilter2              = %d\n", mrz->pingInfo.runtimeFilter2);
		fprintf(stderr,"dbg5       pipeTrackingStatus          = %d\n", mrz->pingInfo.pipeTrackingStatus);
		fprintf(stderr,"dbg5       transmitArraySizeUsed_deg   = %f\n", mrz->pingInfo.transmitArraySizeUsed_deg);
		fprintf(stderr,"dbg5       receiveArraySizeUsed_deg    = %f\n", mrz->pingInfo.receiveArraySizeUsed_deg);
		fprintf(stderr,"dbg5       transmitPower_dB            = %f\n", mrz->pingInfo.transmitPower_dB);
		fprintf(stderr,"dbg5       SLrampUpTimeRemaining       = %d\n", mrz->pingInfo.SLrampUpTimeRemaining);
		fprintf(stderr,"dbg5       padding2                    = %d\n", mrz->pingInfo.padding2);
		fprintf(stderr,"dbg5       yawAngle_deg                = %f\n", mrz->pingInfo.yawAngle_deg);
		fprintf(stderr,"dbg5       numTxSectors                = %d\n", mrz->pingInfo.numTxSectors);
		fprintf(stderr,"dbg5       numBytesPerTxSector         = %d\n", mrz->pingInfo.numBytesPerTxSector);
		fprintf(stderr,"dbg5       headingVessel_deg           = %f\n", mrz->pingInfo.headingVessel_deg);
		fprintf(stderr,"dbg5       soundSpeedAtTxDepth_mPerSec = %f\n", mrz->pingInfo.soundSpeedAtTxDepth_mPerSec);
		fprintf(stderr,"dbg5       txTransducerDepth_m         = %f\n", mrz->pingInfo.txTransducerDepth_m);
		fprintf(stderr,"dbg5       z_waterLevelReRefPoint_m    = %f\n", mrz->pingInfo.z_waterLevelReRefPoint_m);
		fprintf(stderr,"dbg5       x_kmallToall_m              = %f\n", mrz->pingInfo.x_kmallToall_m);
		fprintf(stderr,"dbg5       y_kmallToall_m              = %f\n", mrz->pingInfo.y_kmallToall_m);
		fprintf(stderr,"dbg5       latLongInfo                 = %d\n", mrz->pingInfo.latLongInfo);
		fprintf(stderr,"dbg5       posSensorStatus             = %d\n", mrz->pingInfo.posSensorStatus);
		fprintf(stderr,"dbg5       attitudeSensorStatus        = %d\n", mrz->pingInfo.attitudeSensorStatus);
		fprintf(stderr,"dbg5       padding3                    = %d\n", mrz->pingInfo.padding3);
		fprintf(stderr,"dbg5       latitude_deg                = %f\n", mrz->pingInfo.latitude_deg);
		fprintf(stderr,"dbg5       longitude_deg               = %f\n", mrz->pingInfo.longitude_deg);
		fprintf(stderr,"dbg5       ellipsoidHeightReRefPoint_m = %f\n", mrz->pingInfo.ellipsoidHeightReRefPoint_m);
	}

	/* EMdgmMRZ_txSectorInfo - sector information */
	for (int i = 0; i<(mrz->pingInfo.numTxSectors); i++)
	{
		mrz->sectorInfo[i].txSectorNumb = buffer[index];
		index++;
		mrz->sectorInfo[i].txArrNumber = buffer[index];
		index++;
		mrz->sectorInfo[i].txSubArray = buffer[index];
		index++;
		mrz->sectorInfo[i].padding0 = buffer[index];
		index++;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sectorInfo[i].sectorTransmitDelay_sec));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sectorInfo[i].tiltAngleReTx_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sectorInfo[i].txNominalSourceLevel_dB));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sectorInfo[i].txFocusRange_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sectorInfo[i].centreFreq_Hz));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sectorInfo[i].signalBandWidth_Hz));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sectorInfo[i].totalSignalLength_sec));
		index += 4;
		mrz->sectorInfo[i].pulseShading = buffer[index];
		index++;
		mrz->sectorInfo[i].signalWaveForm = buffer[index];
		index++;
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->sectorInfo[i].padding1));
		index += 2;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       #MWC transmit sector %d/%d:\n", i + 1, mrz->pingInfo.numTxSectors);
			fprintf(stderr, "dbg5       txSectorNumb            = %d\n", mrz->sectorInfo[i].txSectorNumb);
			fprintf(stderr, "dbg5       txArrNumber             = %d\n", mrz->sectorInfo[i].txArrNumber);
			fprintf(stderr, "dbg5       txSubArray              = %d\n", mrz->sectorInfo[i].txSubArray);
			fprintf(stderr, "dbg5       padding0                = %d\n", mrz->sectorInfo[i].padding0);
			fprintf(stderr, "dbg5       sectorTransmitDelay_sec = %f\n", mrz->sectorInfo[i].sectorTransmitDelay_sec);
			fprintf(stderr, "dbg5       tiltAngleReTx_deg       = %f\n", mrz->sectorInfo[i].tiltAngleReTx_deg);
			fprintf(stderr, "dbg5       txNominalSourceLevel_dB = %f\n", mrz->sectorInfo[i].txNominalSourceLevel_dB);
			fprintf(stderr, "dbg5       txFocusRange_m          = %f\n", mrz->sectorInfo[i].txFocusRange_m);
			fprintf(stderr, "dbg5       centreFreq_Hz           = %f\n", mrz->sectorInfo[i].centreFreq_Hz);
			fprintf(stderr, "dbg5       signalBandWidth_Hz      = %f\n", mrz->sectorInfo[i].signalBandWidth_Hz);
			fprintf(stderr, "dbg5       totalSignalLength_sec   = %f\n", mrz->sectorInfo[i].totalSignalLength_sec);
			fprintf(stderr, "dbg5       pulseShading            = %d\n", mrz->sectorInfo[i].pulseShading);
			fprintf(stderr, "dbg5       signalWaveForm          = %d\n", mrz->sectorInfo[i].signalWaveForm);
			fprintf(stderr, "dbg5       padding1                = %d\n", mrz->sectorInfo[i].padding1);
		}
	}

	/* EMdgmMRZ_rxInfo - receiver specific info */
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->rxInfo.numBytesRxInfo));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->rxInfo.numSoundingsMaxMain));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->rxInfo.numSoundingsValidMain));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->rxInfo.numBytesPerSounding));
	index += 2;

	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->rxInfo.WCSampleRate));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->rxInfo.seabedImageSampleRate));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->rxInfo.BSnormal_dB));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mrz->rxInfo.BSoblique_dB));
	index += 4;

	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->rxInfo.extraDetectionAlarmFlag));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->rxInfo.numExtraDetections));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->rxInfo.numExtraDetectionClasses));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mrz->rxInfo.numBytesPerClass));
	index += 2;

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       numBytesInfoData            = %d\n", mrz->rxInfo.numBytesRxInfo);
		fprintf(stderr, "dbg5       numSoundingsMaxMain         = %d\n", mrz->rxInfo.numSoundingsMaxMain);
		fprintf(stderr, "dbg5       numSoundingsValidMain       = %d\n", mrz->rxInfo.numSoundingsValidMain);
		fprintf(stderr, "dbg5       numBytesPerSounding         = %d\n", mrz->rxInfo.numBytesPerSounding);
		fprintf(stderr, "dbg5       WCSampleRate                = %f\n", mrz->rxInfo.WCSampleRate);
		fprintf(stderr, "dbg5       seabedImageSampleRate       = %f\n", mrz->rxInfo.seabedImageSampleRate);
		fprintf(stderr, "dbg5       BSnormal_dB                 = %f\n", mrz->rxInfo.BSnormal_dB);
		fprintf(stderr, "dbg5       BSoblique_dB                = %f\n", mrz->rxInfo.BSoblique_dB);
		fprintf(stderr, "dbg5       extraDetectionAlarmFlag     = %d\n", mrz->rxInfo.extraDetectionAlarmFlag);
		fprintf(stderr, "dbg5       numExtraDetections          = %d\n", mrz->rxInfo.numExtraDetections);
		fprintf(stderr, "dbg5       numExtraDetectionClasses    = %d\n", mrz->rxInfo.numExtraDetectionClasses);
		fprintf(stderr, "dbg5       numBytesPerClass            = %d\n", mrz->rxInfo.numBytesPerClass);
	}

	/* EMdgmMRZ_extraDetClassInfo -  Extra detection class info */
	for (int i = 0; i<(mrz->rxInfo.numExtraDetectionClasses); i++)
	{
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->extraDetClassInfo[i].numExtraDetInClass));
		index += 2;
		mrz->extraDetClassInfo[i].padding = buffer[index];
		index++;
		mrz->extraDetClassInfo[i].alarmFlag = buffer[index];
		index++;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       numExtraDetInClass  = %d\n", mrz->extraDetClassInfo[i].numExtraDetInClass);
			fprintf(stderr, "dbg5       padding             = %d\n", mrz->extraDetClassInfo[i].padding);
			fprintf(stderr, "dbg5       alarmFlag           = %d\n", mrz->extraDetClassInfo[i].alarmFlag);
		}
	}


	/* EMdgmMRZ_sounding - Data for each sounding */
	int numSidescanSamples = 0;
	int numSoundings = mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections;
	for (int i = 0; i<numSoundings; i++)
	{
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->sounding[i].soundingIndex));
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
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->sounding[i].padding));
		index += 2;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].rangeFactor));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].qualityFactor));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].detectionUncertaintyVer_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].detectionUncertaintyHor_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].detectionWindowLength_sec));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].echoLength_sec));
		index += 4;

		/* Water column paramters. */
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->sounding[i].WCBeamNumb));
		index += 2;
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->sounding[i].WCrange_samples));
		index += 2;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].WCNomBeamAngleAcross_deg));
		index += 4;

		/* Reflectivity data (backscatter (BS) data). */
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].meanAbsCoeff_dBPerkm));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].reflectivity1_dB));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].reflectivity2_dB));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].receiverSensitivityApplied_dB));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].sourceLevelApplied_dB));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].BScalibration_dB));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].TVG_dB));
		index += 4;

		/* Range and angle data. */
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].beamAngleReRx_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].beamAngleCorrection_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].twoWayTravelTime_sec));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].twoWayTravelTimeCorrection_sec));
		index += 4;

		/* Georeferenced depth points. */
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].deltaLatitude_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].deltaLongitude_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].z_reRefPoint_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].y_reRefPoint_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].x_reRefPoint_m));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mrz->sounding[i].beamIncAngleAdj_deg));
		index += 4;
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->sounding[i].realTimeCleanInfo));
		index += 2;

		/* Seabed image. */
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->sounding[i].SIstartRange_samples));
		index += 2;
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->sounding[i].SIcentreSample));
		index += 2;
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->sounding[i].SInumSamples));
		index += 2;

		numSidescanSamples += mrz->sounding[i].SInumSamples;

        /* calculate beamflag */
        mrz->sounding[i].beamflag = MB_FLAG_NULL;
        if(mrz->sounding[i].qualityFactor > MBSYS_KMBES_QUAL_FACTOR_THRESHOLD) {
            mrz->sounding[i].beamflag = (MB_FLAG_FLAG + MB_FLAG_SONAR);
        }

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       soundingIndex                   = %d\n", mrz->sounding[i].soundingIndex);
			fprintf(stderr, "dbg5       txSectorNumb                    = %d\n", mrz->sounding[i].txSectorNumb);
			fprintf(stderr, "dbg5       detectionType                   = %d\n", mrz->sounding[i].detectionType);
			fprintf(stderr, "dbg5       detectionMethod                 = %d\n", mrz->sounding[i].detectionMethod);
			fprintf(stderr, "dbg5       rejectionInfo1                  = %d\n", mrz->sounding[i].rejectionInfo1);
			fprintf(stderr, "dbg5       rejectionInfo2                  = %d\n", mrz->sounding[i].rejectionInfo2);
			fprintf(stderr, "dbg5       postProcessingInfo              = %d\n", mrz->sounding[i].postProcessingInfo);
			fprintf(stderr, "dbg5       detectionClass                  = %d\n", mrz->sounding[i].detectionClass);
			fprintf(stderr, "dbg5       detectionConfidenceLevel        = %d\n", mrz->sounding[i].detectionConfidenceLevel);
			fprintf(stderr, "dbg5       padding                         = %d\n", mrz->sounding[i].padding);
			fprintf(stderr, "dbg5       rangeFactor                     = %f\n", mrz->sounding[i].rangeFactor);
			fprintf(stderr, "dbg5       qualityFactor                   = %f\n", mrz->sounding[i].qualityFactor);
			fprintf(stderr, "dbg5       detectionUncertaintyVer_m       = %f\n", mrz->sounding[i].detectionUncertaintyVer_m);
			fprintf(stderr, "dbg5       detectionUncertaintyHor_m       = %f\n", mrz->sounding[i].detectionUncertaintyHor_m);
			fprintf(stderr, "dbg5       detectionWindowLength_sec       = %f\n", mrz->sounding[i].detectionWindowLength_sec);
			fprintf(stderr, "dbg5       echoLength_sec                  = %f\n", mrz->sounding[i].echoLength_sec);
			fprintf(stderr, "dbg5       WCBeamNumb                      = %d\n", mrz->sounding[i].WCBeamNumb);
			fprintf(stderr, "dbg5       WCrange_samples                 = %d\n", mrz->sounding[i].WCrange_samples);
			fprintf(stderr, "dbg5       WCNomBeamAngleAcross_deg        = %f\n", mrz->sounding[i].WCNomBeamAngleAcross_deg);
			fprintf(stderr, "dbg5       meanAbsCoeff_dBPerkm            = %f\n", mrz->sounding[i].meanAbsCoeff_dBPerkm);
			fprintf(stderr, "dbg5       reflectivity1_dB                = %f\n", mrz->sounding[i].reflectivity1_dB);
			fprintf(stderr, "dbg5       reflectivity2_dB                = %f\n", mrz->sounding[i].reflectivity2_dB);
			fprintf(stderr, "dbg5       receiverSensitivityApplied_dB   = %f\n", mrz->sounding[i].receiverSensitivityApplied_dB);
			fprintf(stderr, "dbg5       sourceLevelApplied_dB           = %f\n", mrz->sounding[i].sourceLevelApplied_dB);
			fprintf(stderr, "dbg5       BScalibration_dB                = %f\n", mrz->sounding[i].BScalibration_dB);
			fprintf(stderr, "dbg5       TVG_dB                          = %f\n", mrz->sounding[i].TVG_dB);
			fprintf(stderr, "dbg5       beamAngleReRx_deg               = %f\n", mrz->sounding[i].beamAngleReRx_deg);
			fprintf(stderr, "dbg5       beamAngleCorrection_deg         = %f\n", mrz->sounding[i].beamAngleCorrection_deg);
			fprintf(stderr, "dbg5       twoWayTravelTime_sec            = %f\n", mrz->sounding[i].twoWayTravelTime_sec);
			fprintf(stderr, "dbg5       twoWayTravelTimeCorrection_sec  = %f\n", mrz->sounding[i].twoWayTravelTimeCorrection_sec);
			fprintf(stderr, "dbg5       deltaLatitude_deg               = %f\n", mrz->sounding[i].deltaLatitude_deg);
			fprintf(stderr, "dbg5       deltaLongitude_deg              = %f\n", mrz->sounding[i].deltaLongitude_deg);
			fprintf(stderr, "dbg5       z_reRefPoint_m                  = %f\n", mrz->sounding[i].z_reRefPoint_m);
			fprintf(stderr, "dbg5       y_reRefPoint_m                  = %f\n", mrz->sounding[i].y_reRefPoint_m);
			fprintf(stderr, "dbg5       x_reRefPoint_m                  = %f\n", mrz->sounding[i].x_reRefPoint_m);
			fprintf(stderr, "dbg5       beamIncAngleAdj_deg             = %f\n", mrz->sounding[i].beamIncAngleAdj_deg);
			fprintf(stderr, "dbg5       realTimeCleanInfo               = %d\n", mrz->sounding[i].realTimeCleanInfo);
			fprintf(stderr, "dbg5       SIstartRange_samples            = %d\n", mrz->sounding[i].SIstartRange_samples);
			fprintf(stderr, "dbg5       SIcentreSample                  = %d\n", mrz->sounding[i].SIcentreSample);
			fprintf(stderr, "dbg5       SInumSamples                    = %d\n", mrz->sounding[i].SInumSamples);
		}
	}

	for (int i = 0; i<numSidescanSamples; i++)
	{
		mb_get_binary_short(MB_YES, &buffer[index], &(mrz->SIsample_desidB[i]));
		index += 2;
	}

	/* Update data structure beam and pixel counts */
	store->num_soundings += numSoundings;
	store->num_sidescan_samples += numSidescanSamples;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_DATA;
	}
	else {
		store->kind = MB_DATA_NONE;
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

int mbr_kemkmall_rd_mwc(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_mwc";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_mwc *mwc;
	size_t alloc_size;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	mwc = &store->mwc[dgm_index->rx_index];

	mwc->header = dgm_index->header;

	/* get the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	/* EMdgmMpartition - data partition information */
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->partition.numOfDgms));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->partition.dgmNum));
	index += 2;

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       numOfDgms = %d\n", mwc->partition.numOfDgms);
		fprintf(stderr, "dbg5       dgmNum    = %d\n", mwc->partition.dgmNum);
	}

	/* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->cmnPart.numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->cmnPart.pingCnt));
	index += 2;
	mwc->cmnPart.rxFansPerPing = buffer[index];
	index++;
	mwc->cmnPart.rxFanIndex = buffer[index];
	index++;
	mwc->cmnPart.swathsPerPing = buffer[index];
	index++;
	mwc->cmnPart.swathAlongPosition = buffer[index];
	index++;
	mwc->cmnPart.txTransducerInd = buffer[index];
	index++;
	mwc->cmnPart.rxTransducerInd = buffer[index];
	index++;
	mwc->cmnPart.numRxTransducers = buffer[index];
	index++;
	mwc->cmnPart.algorithmType = buffer[index];
	index++;

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       numBytesCmnPart     = %d\n", mwc->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       pingCnt             = %d\n", mwc->cmnPart.pingCnt);
		fprintf(stderr, "dbg5       rxFansPerPing       = %d\n", mwc->cmnPart.rxFansPerPing);
		fprintf(stderr, "dbg5       rxFanIndex          = %d\n", mwc->cmnPart.rxFanIndex);
		fprintf(stderr, "dbg5       swathsPerPing       = %d\n", mwc->cmnPart.swathsPerPing);
		fprintf(stderr, "dbg5       swathAlongPosition  = %d\n", mwc->cmnPart.swathAlongPosition);
		fprintf(stderr, "dbg5       txTransducerInd     = %d\n", mwc->cmnPart.txTransducerInd);
		fprintf(stderr, "dbg5       rxTransducerInd     = %d\n", mwc->cmnPart.rxTransducerInd);
		fprintf(stderr, "dbg5       numRxTransducers    = %d\n", mwc->cmnPart.numRxTransducers);
		fprintf(stderr, "dbg5       algorithmType       = %d\n", mwc->cmnPart.algorithmType);
	}

	/* EMdgmMWCtxInfo - transmit sectors, general info for all sectors */
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->txInfo.numBytesTxInfo));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->txInfo.numTxSectors));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->txInfo.numBytesPerTxSector));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->txInfo.padding));
	index += 2;
	mb_get_binary_float(MB_YES, &buffer[index], &(mwc->txInfo.heave_m));
	index += 4;

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       numBytesTxInfo      = %d\n", mwc->txInfo.numBytesTxInfo);
		fprintf(stderr, "dbg5       numTxSectors        = %d\n", mwc->txInfo.numTxSectors);
		fprintf(stderr, "dbg5       numBytesPerTxSector = %d\n", mwc->txInfo.numBytesPerTxSector);
		fprintf(stderr, "dbg5       padding             = %d\n", mwc->txInfo.padding);
		fprintf(stderr, "dbg5       heave_m             = %f\n", mwc->txInfo.heave_m);
	}

	/* EMdgmMWCtxSectorData - transmit sector data, loop for all i = numTxSectors */
	for (int i=0; i<(mwc->txInfo.numTxSectors); i++) {
		mb_get_binary_float(MB_YES, &buffer[index], &(mwc->sectorData[i].tiltAngleReTx_deg));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mwc->sectorData[i].centreFreq_Hz));
		index += 4;
		mb_get_binary_float(MB_YES, &buffer[index], &(mwc->sectorData[i].txBeamWidthAlong_deg));
		index += 4;
		mb_get_binary_short(MB_YES, &buffer[index], &(mwc->sectorData[i].txSectorNum));
		index += 2;
		mb_get_binary_short(MB_YES, &buffer[index], &(mwc->sectorData[i].padding));
		index += 2;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       #MWC transmit sector %d/%d:\n", i + 1, mwc->txInfo.numTxSectors);
			fprintf(stderr, "dbg5       tiltAngleReTx_deg    = %f\n", mwc->sectorData[i].tiltAngleReTx_deg);
			fprintf(stderr, "dbg5       centreFreq_Hz        = %f\n", mwc->sectorData[i].centreFreq_Hz);
			fprintf(stderr, "dbg5       txBeamWidthAlong_deg = %f\n", mwc->sectorData[i].txBeamWidthAlong_deg);
			fprintf(stderr, "dbg5       txSectorNum          = %d\n", mwc->sectorData[i].txSectorNum);
			fprintf(stderr, "dbg5       padding              = %d\n", mwc->sectorData[i].padding);
		}
	}

	/* EMdgmMWCrxInfo - receiver, general info */
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->rxInfo.numBytesRxInfo));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(mwc->rxInfo.numBeams));
	index += 2;
	mwc->rxInfo.numBytesPerBeamEntry = buffer[index];
	index ++;
	mwc->rxInfo.phaseFlag = buffer[index];
	index ++;
	mwc->rxInfo.TVGfunctionApplied = buffer[index];
	index ++;
	mwc->rxInfo.TVGoffset_dB = buffer[index];
	index ++;
	mb_get_binary_float(MB_YES, &buffer[index], &(mwc->rxInfo.sampleFreq_Hz));
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &(mwc->rxInfo.soundVelocity_mPerSec));
	index += 4;

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       numBytesRxInfo        = %d\n", mwc->rxInfo.numBytesRxInfo);
		fprintf(stderr, "dbg5       numBeams              = %d\n", mwc->rxInfo.numBeams);
		fprintf(stderr, "dbg5       numBytesPerBeamEntry  = %d\n", mwc->rxInfo.numBytesPerBeamEntry);
		fprintf(stderr, "dbg5       phaseFlag             = %d\n", mwc->rxInfo.phaseFlag);
		fprintf(stderr, "dbg5       TVGfunctionApplied    = %d\n", mwc->rxInfo.TVGfunctionApplied);
		fprintf(stderr, "dbg5       TVGoffset_dB          = %d\n", mwc->rxInfo.TVGoffset_dB);
		fprintf(stderr, "dbg5       sampleFreq_Hz         = %f\n", mwc->rxInfo.sampleFreq_Hz);
		fprintf(stderr, "dbg5       soundVelocity_mPerSec = %f\n", mwc->rxInfo.soundVelocity_mPerSec);
	}

	/* EMdgmMWCrxBeamData - receiver, specific info for each beam */
	alloc_size = (size_t)(mwc->rxInfo.numBeams * sizeof(struct mbsys_kmbes_mwc_rx_beam_data));
	status = mb_mallocd(verbose, __FILE__, __LINE__, alloc_size, (void **)&(mwc->beamData_p), error);

	for (int i=0; i<(mwc->rxInfo.numBeams); i++)
	{
		mb_get_binary_float(MB_YES, &buffer[index], &(mwc->beamData_p[i].beamPointAngReVertical_deg));
		index += 4;
		mb_get_binary_short(MB_YES, &buffer[index], &(mwc->beamData_p[i].startRangeSampleNum));
		index += 2;
		mb_get_binary_short(MB_YES, &buffer[index], &(mwc->beamData_p[i].detectedRangeInSamples));
		index += 2;
		mb_get_binary_short(MB_YES, &buffer[index], &(mwc->beamData_p[i].beamTxSectorNum));
		index += 2;
		mb_get_binary_short(MB_YES, &buffer[index], &(mwc->beamData_p[i].numSampleData));
		index += 2;

		/* Allocate sample amplitude array. Sample amplitudes are in 0.5 dB resolution */
		alloc_size = (size_t)(mwc->beamData_p[i].numSampleData);
		status = mb_mallocd(verbose, __FILE__, __LINE__, alloc_size,
							(void **)&(mwc->beamData_p[i].sampleAmplitude05dB_p), error);

		/* now get the samples */
		memcpy(mwc->beamData_p[i].sampleAmplitude05dB_p, &buffer[index], mwc->beamData_p[i].numSampleData);
		index += mwc->beamData_p[i].numSampleData;

		/* Water Column data are followed by struct rxBeamPhase1 or struct rxBeamPhase2 */
		/* if indicated in the field phaseFlag in struct rxInfo */
		switch (mwc->rxInfo.phaseFlag) {
			case 0:
				break;
			case 1:
				/* Rx beam phase in 180/128 degree resolution. */
				mwc->beamData_p[i].rx_beam_phase1.rxBeamPhase = buffer[index];
				index++;
				break;
			case 2:
				/* Rx beam phase in 0.01 degree resolution */
				mb_get_binary_short(MB_YES, &buffer[index], &(mwc->beamData_p[i].rx_beam_phase2.rxBeamPhase));
				index += 2;
				break;
			default:
				break;
		}

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       #MWC receiver beam data %d/%d:\n", i + 1, mwc->rxInfo.numBeams);
			fprintf(stderr, "dbg5       tiltAngleReTx_deg       = %f\n", mwc->beamData_p[i].beamPointAngReVertical_deg);
			fprintf(stderr, "dbg5       startRangeSampleNum     = %d\n", mwc->beamData_p[i].startRangeSampleNum);
			fprintf(stderr, "dbg5       detectedRangeInSamples  = %d\n", mwc->beamData_p[i].detectedRangeInSamples);
			fprintf(stderr, "dbg5       beamTxSectorNum         = %d\n", mwc->beamData_p[i].beamTxSectorNum);
			fprintf(stderr, "dbg5       numSampleData           = %d\n", mwc->beamData_p[i].numSampleData);
			fprintf(stderr, "dbg5       sampleAmplitude05dB_p   = [");
			for (int k = 0; k < (mwc->beamData_p[i].numSampleData); k++)
				printf("%d, ", mwc->beamData_p[i].sampleAmplitude05dB_p[k]);
			printf("]\n");
		}

		// TODO: don't forget to free(mwc->beamData_p[i].sampleAmplitude05dB_p);!
		// printf("Freeing #MWC %d beamData_p data arrays.\n", EMdgm->mwc[dgmIndex->rx_index].rxInfo.numBeams);
		// for (int i=0;i<EMdgm->mwc[dgmIndex->rx_index].rxInfo.numBeams; i++)
		// 	free(EMdgm->mwc[dgmIndex->rx_index].beamData_p[i].sampleAmplitude05dB_p);
		// free(EMdgm->mwc[dgmIndex->rx_index].beamData_p);
	}


	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_WATER_COLUMN;
	}
	else {
		store->kind = MB_DATA_NONE;
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

int mbr_kemkmall_rd_cpo(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_cpo";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_cpo *cpo;
	size_t numBytesRawSensorData;
	int index;


	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	cpo = &(store->cpo);

	/* get header */
	cpo->header = dgm_index->header;

	numBytesRawSensorData = cpo->header.numBytesDgm - MBSYS_KMBES_CPO_VAR_OFFSET;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;


	// common part
	mb_get_binary_short(MB_YES, &buffer[index], &(cpo->cmnPart.numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(cpo->cmnPart.sensorSystem));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(cpo->cmnPart.sensorStatus));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(cpo->cmnPart.padding));
	index += 2;

	// sensor data block
	mb_get_binary_int(MB_YES, &buffer[index], &cpo->sensorData.timeFromSensor_sec);
	index += 4;
	mb_get_binary_int(MB_YES, &buffer[index], &cpo->sensorData.timeFromSensor_nanosec);
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &cpo->sensorData.posFixQuality_m);
	index += 4;
	mb_get_binary_double(MB_YES, &buffer[index], &cpo->sensorData.correctedLat_deg);
	index += 8;
	mb_get_binary_double(MB_YES, &buffer[index], &cpo->sensorData.correctedLong_deg);
	index += 8;
	mb_get_binary_float(MB_YES, &buffer[index], &cpo->sensorData.speedOverGround_mPerSec);
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &cpo->sensorData.courseOverGround_deg);
	index += 4;
	mb_get_binary_float(MB_YES, &buffer[index], &cpo->sensorData.ellipsoidHeightReRefPoint_m);
	index += 4;
	memcpy(&cpo->sensorData.posDataFromSensor, &buffer[index], numBytesRawSensorData);
	index += numBytesRawSensorData;


	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       cpo->header.numBytesDgm:                %u\n", cpo->header.numBytesDgm);
		fprintf(stderr, "dbg5       cpo->header.dgmType:                    %s\n", cpo->header.dgmType);
		fprintf(stderr, "dbg5       cpo->header.dgmVersion:                 %u\n", cpo->header.dgmVersion);
		fprintf(stderr, "dbg5       cpo->header.systemID:                   %u\n", cpo->header.systemID);
		fprintf(stderr, "dbg5       cpo->header.echoSounderID:              %u\n", cpo->header.echoSounderID);
		fprintf(stderr, "dbg5       cpo->header.time_sec:                   %u\n", cpo->header.time_sec);
		fprintf(stderr, "dbg5       cpo->header.time_nanosec:               %u\n", cpo->header.time_nanosec);

		fprintf(stderr, "dbg5       cpo->cmnPart.numBytesCmnPart:           %u\n", cpo->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       cpo->cmnPart.sensorSystem:              %u\n", cpo->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       cpo->cmnPart.sensorStatus:              %u\n", cpo->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       cpo->cmnPart.padding:                   %u\n", cpo->cmnPart.padding);

		fprintf(stderr, "dbg5       cpo->sensorData.timeFromSensor_sec:     %u\n", cpo->sensorData.timeFromSensor_sec);
		fprintf(stderr, "dbg5       cpo->sensorData.timeFromSensor_nanosec: %u\n", cpo->sensorData.timeFromSensor_nanosec);
		fprintf(stderr, "dbg5       cpo->sensorData.posFixQuality_m:        %f\n", cpo->sensorData.posFixQuality_m);
		fprintf(stderr, "dbg5       cpo->sensorData.correctedLat_deg:       %f\n", cpo->sensorData.correctedLat_deg);
		fprintf(stderr, "dbg5       cpo->sensorData.correctedLong_deg:      %f\n", cpo->sensorData.correctedLong_deg);
		fprintf(stderr, "dbg5       cpo->sensorData.speedOverGround_mPerSec:%f\n", cpo->sensorData.speedOverGround_mPerSec);
		fprintf(stderr, "dbg5       cpo->sensorData.correctedLat_deg:       %f\n", cpo->sensorData.courseOverGround_deg);
		fprintf(stderr, "dbg5       cpo->sensorData.correctedLong_deg:      %f\n", cpo->sensorData.ellipsoidHeightReRefPoint_m);
		fprintf(stderr, "dbg5       cpo->sensorData.speedOverGround_mPerSec:%s\n", cpo->sensorData.posDataFromSensor);
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
};

int mbr_kemkmall_rd_che(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_che";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_che *che;
	size_t numBytesRawSensorData;
	int index;


	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	che = &(store->che);

	/* get header */
	che->header = dgm_index->header;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	// common part
	mb_get_binary_short(MB_YES, &buffer[index], &(che->cmnPart.numBytesCmnPart));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(che->cmnPart.sensorSystem));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(che->cmnPart.sensorStatus));
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &(che->cmnPart.padding));
	index += 2;

	mb_get_binary_float(MB_YES, &buffer[index], &che->data.heave_m);
	index += 4;

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       che->header.numBytesDgm:                %u\n", che->header.numBytesDgm);
		fprintf(stderr, "dbg5       che->header.dgmType:                    %s\n", che->header.dgmType);
		fprintf(stderr, "dbg5       che->header.dgmVersion:                 %u\n", che->header.dgmVersion);
		fprintf(stderr, "dbg5       che->header.systemID:                   %u\n", che->header.systemID);
		fprintf(stderr, "dbg5       che->header.echoSounderID:              %u\n", che->header.echoSounderID);
		fprintf(stderr, "dbg5       che->header.time_sec:                   %u\n", che->header.time_sec);
		fprintf(stderr, "dbg5       che->header.time_nanosec:               %u\n", che->header.time_nanosec);

		fprintf(stderr, "dbg5       che->cmnPart.numBytesCmnPart:           %u\n", che->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       che->cmnPart.sensorSystem:              %u\n", che->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       che->cmnPart.sensorStatus:              %u\n", che->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       che->cmnPart.padding:                   %u\n", che->cmnPart.padding);

		fprintf(stderr, "dbg5       che->data.heave_m:                      %f\n", che->data.heave_m);
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
};

int mbr_kemkmall_rd_iip(int verbose, char *buffer, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_rd_iip";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_iip *iip;
	size_t numBytesRawSensorData;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	iip = &(store->iip);

	/* get header */
	iip->header = dgm_index->header;

	numBytesRawSensorData = iip->header.numBytesDgm - MBSYS_KMBES_IIP_VAR_OFFSET;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	mb_get_binary_short(MB_YES, &buffer[index], &iip->numBytesCmnPart);
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &iip->info);
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &iip->status);
	index += 2;
	memcpy(&iip->install_txt, &buffer[index], numBytesRawSensorData);

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_INSTALLATION; // TODO: check data kind
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       iip->header.numBytesDgm:                %u\n", iip->header.numBytesDgm);
		fprintf(stderr, "dbg5       iip->header.dgmType:                    %s\n", iip->header.dgmType);
		fprintf(stderr, "dbg5       iip->header.dgmVersion:                 %u\n", iip->header.dgmVersion);
		fprintf(stderr, "dbg5       iip->header.systemID:                   %u\n", iip->header.systemID);
		fprintf(stderr, "dbg5       iip->header.echoSounderID:              %u\n", iip->header.echoSounderID);
		fprintf(stderr, "dbg5       iip->header.time_sec:                   %u\n", iip->header.time_sec);
		fprintf(stderr, "dbg5       iip->header.time_nanosec:               %u\n", iip->header.time_nanosec);

		fprintf(stderr, "dbg5       iip->numBytesCmnPart:                   %u\n", iip->numBytesCmnPart);
		fprintf(stderr, "dbg5       iip->info:                              %u\n", iip->info);
		fprintf(stderr, "dbg5       iip->status:                            %u\n", iip->status);
		fprintf(stderr, "dbg5       iip->install_txt:                       %s\n", iip->install_txt);
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
};

int mbr_kemkmall_rd_iop(int verbose, char *buffer, void *store_ptr, int *error){
	char *function_name = "mbr_kemkmall_rd_iop";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_iop *iop;
	size_t numBytesRawSensorData;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	iop = &(store->iop);

	/* get header */
	iop->header = dgm_index->header;

	numBytesRawSensorData = iop->header.numBytesDgm - MBSYS_KMBES_IOP_VAR_OFFSET;

	/* extract the data */
	index = MBSYS_KMBES_HEADER_SIZE;

	mb_get_binary_short(MB_YES, &buffer[index], &iop->numBytesCmnPart);
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &iop->info);
	index += 2;
	mb_get_binary_short(MB_YES, &buffer[index], &iop->status);
	index += 2;
	memcpy(&iop->runtime_txt, &buffer[index], numBytesRawSensorData);

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_RUN_PARAMETER; // TODO: check data kind
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       iop->header.numBytesDgm:                %u\n", iop->header.numBytesDgm);
		fprintf(stderr, "dbg5       iop->header.dgmType:                    %s\n", iop->header.dgmType);
		fprintf(stderr, "dbg5       iop->header.dgmVersion:                 %u\n", iop->header.dgmVersion);
		fprintf(stderr, "dbg5       iop->header.systemID:                   %u\n", iop->header.systemID);
		fprintf(stderr, "dbg5       iop->header.echoSounderID:              %u\n", iop->header.echoSounderID);
		fprintf(stderr, "dbg5       iop->header.time_sec:                   %u\n", iop->header.time_sec);
		fprintf(stderr, "dbg5       iop->header.time_nanosec:               %u\n", iop->header.time_nanosec);

		fprintf(stderr, "dbg5       iop->iop->numBytesCmnPart:              %u\n", iop->numBytesCmnPart);
		fprintf(stderr, "dbg5       iop->info:                              %u\n", iop->info);
		fprintf(stderr, "dbg5       iop->status:                            %u\n", iop->status);
		fprintf(stderr, "dbg5       iop->runtime_txt:                       %s\n", iop->runtime_txt);
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
};

int mbr_kemkmall_rd_unknown(int verbose, char *buffer, void *store_ptr, int *error);

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_header(int verbose, char **bufferptr, void *store_ptr, int *error) {
	char *function_name = "mbr_kemkmall_wr_header";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_header *header;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	header = &(dgm_index->header);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       header->numBytesDgm:                    %u\n", header->numBytesDgm);
		fprintf(stderr, "dbg5       header->dgmType:                        %s\n", header->dgmType);
		fprintf(stderr, "dbg5       header->dgmVersion:                     %u\n", header->dgmVersion);
		fprintf(stderr, "dbg5       header->systemID:                       %u\n", header->systemID);
		fprintf(stderr, "dbg5       header->echoSounderID:                  %u\n", header->echoSounderID);
		fprintf(stderr, "dbg5       header->time_sec:                       %u\n", header->time_sec);
		fprintf(stderr, "dbg5       header->time_nanosec:                   %u\n", header->time_nanosec);
	}

	/* proceed to write if buffer allocated */
	if (status == MB_SUCCESS) {
		/* get buffer for writing */
		buffer = (char *) *bufferptr;

		/* insert the data */
		index = 0;

		mb_put_binary_int(MB_YES, header->numBytesDgm, &buffer[index]);
		index += 4;
		memcpy(&buffer[index], &(header->dgmType), sizeof(header->dgmType));
		index += 4;
		buffer[index] = header->dgmVersion;
		index++;
		buffer[index] = header->systemID;
		index++;
		mb_put_binary_short(MB_YES, header->echoSounderID, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, header->time_sec, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, header->time_nanosec, &buffer[index]);
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
};

int mbr_kemkmall_wr_spo(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error){
	char *function_name = "mbr_kemkmall_wr_spo";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_spo *spo;
	size_t numBytesRawSensorData;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&(store->dgm_index[store->dgm_count_id]);
	spo = &(store->spo);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       spo->header.numBytesDgm:                %u\n", spo->header.numBytesDgm);
		fprintf(stderr, "dbg5       spo->header.dgmType:                    %s\n", spo->header.dgmType);
		fprintf(stderr, "dbg5       spo->header.dgmVersion:                 %u\n", spo->header.dgmVersion);
		fprintf(stderr, "dbg5       spo->header.systemID:                   %u\n", spo->header.systemID);
		fprintf(stderr, "dbg5       spo->header.echoSounderID:              %u\n", spo->header.echoSounderID);
		fprintf(stderr, "dbg5       spo->header.time_sec:                   %u\n", spo->header.time_sec);
		fprintf(stderr, "dbg5       spo->header.time_nanosec:               %u\n", spo->header.time_nanosec);

		fprintf(stderr, "dbg5       spo->cmnPart.numBytesCmnPart:           %u\n", spo->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       spo->cmnPart.sensorSystem:              %u\n", spo->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       spo->cmnPart.sensorStatus:              %u\n", spo->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       spo->cmnPart.padding:                   %u\n", spo->cmnPart.padding);

		fprintf(stderr, "dbg5       spo->sensorData.timeFromSensor_sec:     %u\n", spo->sensorData.timeFromSensor_sec);
		fprintf(stderr, "dbg5       spo->sensorData.timeFromSensor_nanosec: %u\n", spo->sensorData.timeFromSensor_nanosec);
		fprintf(stderr, "dbg5       spo->sensorData.posFixQuality_m:        %f\n", spo->sensorData.posFixQuality_m);
		fprintf(stderr, "dbg5       spo->sensorData.correctedLat_deg:       %f\n", spo->sensorData.correctedLat_deg);
		fprintf(stderr, "dbg5       spo->sensorData.correctedLong_deg:      %f\n", spo->sensorData.correctedLong_deg);
		fprintf(stderr, "dbg5       spo->sensorData.speedOverGround_mPerSec:%f\n", spo->sensorData.speedOverGround_mPerSec);
		fprintf(stderr, "dbg5       spo->sensorData.courseOverGround_deg:   %f\n", spo->sensorData.courseOverGround_deg);
		fprintf(stderr, "dbg5       spo->sensorData.ellipsoidHeightReRefPoint_m:   %f\n", spo->sensorData.ellipsoidHeightReRefPoint_m);
		fprintf(stderr, "dbg5       spo->sensorData.posDataFromSensor:      %s\n", spo->sensorData.posDataFromSensor);
	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* common part */
		mb_put_binary_short(MB_YES, spo->cmnPart.numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, spo->cmnPart.sensorSystem, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, spo->cmnPart.sensorStatus, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, spo->cmnPart.padding, &buffer[index]);
		index += 2;

		/* sensor data block */
		mb_put_binary_int(MB_YES, spo->sensorData.timeFromSensor_sec, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, spo->sensorData.timeFromSensor_nanosec, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, spo->sensorData.posFixQuality_m, &buffer[index]);
		index += 4;
		mb_put_binary_double(MB_YES, spo->sensorData.correctedLat_deg, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, spo->sensorData.correctedLong_deg, &buffer[index]);
		index += 8;
		mb_put_binary_float(MB_YES, spo->sensorData.speedOverGround_mPerSec, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, spo->sensorData.courseOverGround_deg, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, spo->sensorData.ellipsoidHeightReRefPoint_m, &buffer[index]);
		index += 4;

		/* raw data msg from sensor */
		memcpy(&buffer[index], &(spo->sensorData.posDataFromSensor), numBytesRawSensorData);
		index += numBytesRawSensorData;

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, spo->header.numBytesDgm, &buffer[index]);
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
};

int mbr_kemkmall_wr_skm(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error){
	char *function_name = "mbr_kemkmall_wr_skm";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_skm *skm;
	char *buffer;
	int index;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	skm = &(store->skm);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       skm->header.numBytesDgm:                %u\n", skm->header.numBytesDgm);
		fprintf(stderr, "dbg5       skm->header.dgmType:                    %s\n", skm->header.dgmType);
		fprintf(stderr, "dbg5       skm->header.dgmVersion:                 %u\n", skm->header.dgmVersion);
		fprintf(stderr, "dbg5       skm->header.systemID:                   %u\n", skm->header.systemID);
		fprintf(stderr, "dbg5       skm->header.echoSounderID:              %u\n", skm->header.echoSounderID);
		fprintf(stderr, "dbg5       skm->header.time_sec:                   %u\n", skm->header.time_sec);
		fprintf(stderr, "dbg5       skm->header.time_nanosec:               %u\n", skm->header.time_nanosec);

		fprintf(stderr, "dbg5       skm->infoPart.numBytesInfoPart:         %u\n", skm->infoPart.numBytesInfoPart);
		fprintf(stderr, "dbg5       skm->infoPart.sensorSystem:             %u\n", skm->infoPart.sensorSystem);
		fprintf(stderr, "dbg5       skm->infoPart.sensorStatus:             %u\n", skm->infoPart.sensorStatus);
		fprintf(stderr, "dbg5       skm->infoPart.sensorInputFormat:        %u\n", skm->infoPart.sensorInputFormat);
		fprintf(stderr, "dbg5       skm->infoPart.numSamplesArray:          %u\n", skm->infoPart.numSamplesArray);
		fprintf(stderr, "dbg5       skm->infoPart.numBytesPerSample:        %u\n", skm->infoPart.numBytesPerSample);
		fprintf(stderr, "dbg5       skm->infoPart.sensorDataContents:       %u\n", skm->infoPart.sensorDataContents);

		for (int i=0; i<(skm->infoPart.numSamplesArray); i++ ) {
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.dgmType:                %s\n", i, skm->sample[i].KMdefault.dgmType);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.numBytesDgm:            %u\n", i, skm->sample[i].KMdefault.numBytesDgm);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.dgmVersion:             %u\n", i, skm->sample[i].KMdefault.dgmVersion);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.time_sec:               %u\n", i, skm->sample[i].KMdefault.time_sec);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.time_nanosec:           %u\n", i, skm->sample[i].KMdefault.time_nanosec);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.status:                 %u\n", i, skm->sample[i].KMdefault.status);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.latitude_deg:           %f\n", i, skm->sample[i].KMdefault.latitude_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.longitude_deg:          %f\n", i, skm->sample[i].KMdefault.longitude_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.ellipsoidHeight_m:      %f\n", i, skm->sample[i].KMdefault.ellipsoidHeight_m);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.roll_deg:               %f\n", i, skm->sample[i].KMdefault.roll_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.pitch_deg:              %f\n", i, skm->sample[i].KMdefault.pitch_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.heading_deg:            %f\n", i, skm->sample[i].KMdefault.heading_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.heave_m:                %f\n", i, skm->sample[i].KMdefault.heave_m);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.rollRate:               %f\n", i, skm->sample[i].KMdefault.rollRate);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.pitchRate:              %f\n", i, skm->sample[i].KMdefault.pitchRate);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.yawRate:                %f\n", i, skm->sample[i].KMdefault.yawRate);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.velNorth:               %f\n", i, skm->sample[i].KMdefault.velNorth);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.velEast:                %f\n", i, skm->sample[i].KMdefault.velEast);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.velDown:                %f\n", i, skm->sample[i].KMdefault.velDown);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.latitudeError_m:        %f\n", i, skm->sample[i].KMdefault.latitudeError_m);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.longitudeError_m:       %f\n", i, skm->sample[i].KMdefault.longitudeError_m);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.ellipsoidHeightError_m: %f\n", i, skm->sample[i].KMdefault.ellipsoidHeightError_m);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.rollError_deg:          %f\n", i, skm->sample[i].KMdefault.rollError_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.pitchError_deg:         %f\n", i, skm->sample[i].KMdefault.pitchError_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.headingError_deg:       %f\n", i, skm->sample[i].KMdefault.headingError_deg);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.heaveError_m:           %f\n", i, skm->sample[i].KMdefault.heaveError_m);

			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.northAcceleration:      %f\n", i, skm->sample[i].KMdefault.northAcceleration);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.eastAcceleration:       %f\n", i, skm->sample[i].KMdefault.eastAcceleration);
			fprintf(stderr, "dbg5       skm->sample[%3d].KMdefault.downAcceleration:       %f\n", i, skm->sample[i].KMdefault.downAcceleration);

			//
			fprintf(stderr, "dbg5       skm->sample[%3d].delayedHeave.time_sec:            %u\n", i, skm->sample[i].delayedHeave.time_sec);
			fprintf(stderr, "dbg5       skm->sample[%3d].delayedHeave.time_nanosec:        %u\n", i, skm->sample[i].delayedHeave.time_nanosec);
			fprintf(stderr, "dbg5       skm->sample[%3d].delayedHeave.delayedHeave_m:      %f\n", i, skm->sample[i].delayedHeave.delayedHeave_m);
		}

	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* info part */
		mb_put_binary_short(MB_YES, skm->infoPart.numBytesInfoPart, &buffer[index]);
		index += 2;
		buffer[index] = skm->infoPart.sensorSystem;
		index++;
		buffer[index] = skm->infoPart.sensorStatus;
		index++;
		mb_put_binary_short(MB_YES, skm->infoPart.sensorInputFormat, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, skm->infoPart.numSamplesArray, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, skm->infoPart.numBytesPerSample, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, skm->infoPart.sensorDataContents, &buffer[index]);
		index += 2;

		for (i=0; i<(skm->infoPart.numSamplesArray); i++ ) {

			/* KMbinary */
			memcpy(&buffer[index], &(skm->sample[i].KMdefault.dgmType), 4);
			index += 4;
			mb_put_binary_short(MB_YES, skm->sample[i].KMdefault.numBytesDgm, &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, skm->sample[i].KMdefault.dgmVersion, &buffer[index]);
			index += 2;
			mb_put_binary_int(MB_YES, skm->sample[i].KMdefault.time_sec, &buffer[index]);
			index += 4;
			mb_put_binary_int(MB_YES, skm->sample[i].KMdefault.time_nanosec, &buffer[index]);
			index += 4;
			mb_put_binary_int(MB_YES, skm->sample[i].KMdefault.status, &buffer[index]);
			index += 4;
			mb_put_binary_double(MB_YES, skm->sample[i].KMdefault.latitude_deg, &buffer[index]);
			index += 8;
			mb_put_binary_double(MB_YES, skm->sample[i].KMdefault.longitude_deg, &buffer[index]);
			index += 8;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.ellipsoidHeight_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.roll_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.pitch_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.heading_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.heave_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.rollRate, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.pitchRate, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.yawRate, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.velNorth, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.velEast, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.velDown, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.latitudeError_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.longitudeError_m, &buffer[index]);
			index += 4;
            mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.ellipsoidHeightError_m, &buffer[index]);
            index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.rollError_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.pitchError_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.headingError_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.heaveError_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.northAcceleration, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.eastAcceleration, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].KMdefault.downAcceleration, &buffer[index]);
			index += 4;

			/* KMdelayedHeave */
			mb_put_binary_int(MB_YES, skm->sample[i].delayedHeave.time_sec, &buffer[index]);
			index += 4;
			mb_put_binary_int(MB_YES, skm->sample[i].delayedHeave.time_nanosec, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, skm->sample[i].delayedHeave.delayedHeave_m, &buffer[index]);
			index += 4;
		}

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, skm->header.numBytesDgm, &buffer[index]);
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
};


int mbr_kemkmall_wr_svp(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_svp";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_svp *svp;
	char *buffer;
	int index;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	svp = &(store->svp);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       svp->header.numBytesDgm:                %u\n", svp->header.numBytesDgm);
		fprintf(stderr, "dbg5       svp->header.dgmType:                    %s\n", svp->header.dgmType);
		fprintf(stderr, "dbg5       svp->header.dgmVersion:                 %u\n", svp->header.dgmVersion);
		fprintf(stderr, "dbg5       svp->header.systemID:                   %u\n", svp->header.systemID);
		fprintf(stderr, "dbg5       svp->header.echoSounderID:              %u\n", svp->header.echoSounderID);
		fprintf(stderr, "dbg5       svp->header.time_sec:                   %u\n", svp->header.time_sec);
		fprintf(stderr, "dbg5       svp->header.time_nanosec:               %u\n", svp->header.time_nanosec);

		fprintf(stderr, "dbg5       svp->numBytesCmnPart:                   %u\n", svp->numBytesCmnPart);
		fprintf(stderr, "dbg5       svp->numSamples:                        %u\n", svp->numSamples);
		fprintf(stderr, "dbg5       svp->sensorFormat:                      %s\n", svp->sensorFormat);
		fprintf(stderr, "dbg5       svp->time_sec:                          %u\n", svp->time_sec);
		fprintf(stderr, "dbg5       svp->latitude_deg:                      %f\n", svp->latitude_deg);
		fprintf(stderr, "dbg5       svp->longitude_deg:                     %f\n", svp->longitude_deg);

		for (i = 0; i < (svp->numSamples); i++) {
			fprintf(stderr, "dbg5       svp->sensorData[%3d].depth_m:                      %f\n", i, svp->sensorData[i].depth_m);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].soundVelocity_mPerSec:        %f\n", i, svp->sensorData[i].soundVelocity_mPerSec);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].padding:                      %d\n", i, svp->sensorData[i].padding);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].temp_C:                       %f\n", i, svp->sensorData[i].temp_C);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].salinity:                     %f\n", i, svp->sensorData[i].salinity);
		}
	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
        mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* svp common part */
		mb_put_binary_short(MB_YES, svp->numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, svp->numSamples, &buffer[index]);
		index += 2;
		memcpy(&buffer[index], &svp->sensorFormat, 4);
		index += 4;
		mb_put_binary_int(MB_YES, svp->time_sec, &buffer[index]);
		index += 4;
		mb_put_binary_double(MB_YES, svp->latitude_deg, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, svp->longitude_deg, &buffer[index]);
		index += 8;

		/* svp data block */
		for (i = 0; i < (svp->numSamples); i++) {
			mb_put_binary_float(MB_YES, svp->sensorData[i].depth_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, svp->sensorData[i].soundVelocity_mPerSec, &buffer[index]);
			index += 4;
			mb_put_binary_int(MB_YES, svp->sensorData[i].padding, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, svp->sensorData[i].temp_C, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, svp->sensorData[i].salinity, &buffer[index]);
			index += 4;
		}

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, svp->header.numBytesDgm, &buffer[index]);
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
};

int mbr_kemkmall_wr_svt(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_svt";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_svt *svt;
	char *buffer;
	int index;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	svt = &(store->svt);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       svt->header.numBytesDgm:                %u\n", svt->header.numBytesDgm);
		fprintf(stderr, "dbg5       svt->header.dgmType:                    %s\n", svt->header.dgmType);
		fprintf(stderr, "dbg5       svt->header.dgmVersion:                 %u\n", svt->header.dgmVersion);
		fprintf(stderr, "dbg5       svt->header.systemID:                   %u\n", svt->header.systemID);
		fprintf(stderr, "dbg5       svt->header.echoSounderID:              %u\n", svt->header.echoSounderID);
		fprintf(stderr, "dbg5       svt->header.time_sec:                   %u\n", svt->header.time_sec);
		fprintf(stderr, "dbg5       svt->header.time_nanosec:               %u\n", svt->header.time_nanosec);

		fprintf(stderr, "dbg5       svt->infoPart.numBytesInfoPart:         %u\n", svt->infoPart.numBytesInfoPart);
		fprintf(stderr, "dbg5       svt->infoPart.sensorStatus:             %u\n", svt->infoPart.sensorStatus);
		fprintf(stderr, "dbg5       svt->infoPart.sensorInputFormat:        %u\n", svt->infoPart.sensorInputFormat);
		fprintf(stderr, "dbg5       svt->infoPart.numSamplesArray:          %u\n", svt->infoPart.numSamplesArray);
		fprintf(stderr, "dbg5       svt->infoPart.sensorDataContents:       %u\n", svt->infoPart.sensorDataContents);
		fprintf(stderr, "dbg5       svt->infoPart.filterTime_sec:           %f\n", svt->infoPart.filterTime_sec);
		fprintf(stderr, "dbg5       svt->infoPart.soundVelocity_mPerSec_offset: %f\n", svt->infoPart.soundVelocity_mPerSec_offset);

		for (int i = 0; i < (svt->infoPart.numSamplesArray); i++) {
			fprintf(stderr, "dbg5       svp->sensorData[%3d].time_sec:                     %u\n", i, svt->sensorData[i].time_sec);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].time_nanosec:                 %u\n", i, svt->sensorData[i].time_nanosec);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].soundVelocity_mPerSec:        %f\n", i, svt->sensorData[i].soundVelocity_mPerSec);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].temp_C:                       %f\n", i, svt->sensorData[i].temp_C);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].pressure_Pa:                  %f\n", i, svt->sensorData[i].pressure_Pa);
			fprintf(stderr, "dbg5       svp->sensorData[%3d].salinity:                     %f\n", i, svt->sensorData[i].salinity);
		}
	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* svt common part */
		mb_put_binary_short(MB_YES, svt->infoPart.numBytesInfoPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, svt->infoPart.sensorStatus, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, svt->infoPart.sensorInputFormat, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, svt->infoPart.numSamplesArray, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, svt->infoPart.sensorDataContents, &buffer[index]);
		index += 2;
		mb_put_binary_float(MB_YES, svt->infoPart.filterTime_sec, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, svt->infoPart.soundVelocity_mPerSec_offset, &buffer[index]);
		index += 4;

		/* svt data block */
		for( i=0; i<svt->infoPart.numSamplesArray; i++ )
		{
			mb_put_binary_int(MB_YES, svt->sensorData[i].time_sec, &buffer[index]);
			index += 4;
			mb_put_binary_int(MB_YES, svt->sensorData[i].time_nanosec, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, svt->sensorData[i].soundVelocity_mPerSec, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, svt->sensorData[i].temp_C, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, svt->sensorData[i].pressure_Pa, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, svt->sensorData[i].salinity, &buffer[index]);
			index += 4;
		}

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, svt->header.numBytesDgm, &buffer[index]);
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
};

int mbr_kemkmall_wr_scl(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_scl";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_scl *scl;
	size_t numBytesRawSensorData;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}


	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	scl = &(store->scl);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       scl->header.numBytesDgm:                %u\n", scl->header.numBytesDgm);
		fprintf(stderr, "dbg5       scl->header.dgmType:                    %s\n", scl->header.dgmType);
		fprintf(stderr, "dbg5       scl->header.dgmVersion:                 %u\n", scl->header.dgmVersion);
		fprintf(stderr, "dbg5       scl->header.systemID:                   %u\n", scl->header.systemID);
		fprintf(stderr, "dbg5       scl->header.echoSounderID:              %u\n", scl->header.echoSounderID);
		fprintf(stderr, "dbg5       scl->header.time_sec:                   %u\n", scl->header.time_sec);
		fprintf(stderr, "dbg5       scl->header.time_nanosec:               %u\n", scl->header.time_nanosec);

		fprintf(stderr, "dbg5       scl->cmnPart.numBytesCmnPart:           %u\n", scl->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       scl->cmnPart.sensorSystem:              %u\n", scl->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       scl->cmnPart.sensorStatus:              %u\n", scl->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       scl->cmnPart.padding:                   %u\n", scl->cmnPart.padding);

		fprintf(stderr, "dbg5       scl->sensorData.offset_sec:             %f\n", scl->sensorData.offset_sec);
		fprintf(stderr, "dbg5       scl->sensorData.clockDevPU_nanosec:     %d\n", scl->sensorData.clockDevPU_nanosec);
		fprintf(stderr, "dbg5       scl->sensorData.dataFromSensor:         %s\n", scl->sensorData.dataFromSensor);
	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* calc number of bytes for raw sensor data */
		numBytesRawSensorData = scl->header.numBytesDgm - MBSYS_KMBES_SCL_VAR_OFFSET;

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* common part */
		mb_put_binary_short(MB_YES, scl->cmnPart.numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, scl->cmnPart.sensorSystem, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, scl->cmnPart.sensorStatus, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, scl->cmnPart.padding, &buffer[index]);
		index += 2;

		/* sensor data block */
		mb_put_binary_float(MB_YES, scl->sensorData.offset_sec, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, scl->sensorData.clockDevPU_nanosec, &buffer[index]);
		index += 4;
		memcpy(&buffer[index], &(scl->sensorData.dataFromSensor), numBytesRawSensorData);
		index += numBytesRawSensorData;

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, scl->header.numBytesDgm, &buffer[index]);
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
};

int mbr_kemkmall_wr_sde(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error){
	char *function_name = "mbr_kemkmall_wr_sde";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_sde *sde;
	size_t numBytesRawSensorData;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	sde = &(store->sde);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       sde->header.numBytesDgm:                %u\n", sde->header.numBytesDgm);
		fprintf(stderr, "dbg5       sde->header.dgmType:                    %s\n", sde->header.dgmType);
		fprintf(stderr, "dbg5       sde->header.dgmVersion:                 %u\n", sde->header.dgmVersion);
		fprintf(stderr, "dbg5       sde->header.systemID:                   %u\n", sde->header.systemID);
		fprintf(stderr, "dbg5       sde->header.echoSounderID:              %u\n", sde->header.echoSounderID);
		fprintf(stderr, "dbg5       sde->header.time_sec:                   %u\n", sde->header.time_sec);
		fprintf(stderr, "dbg5       sde->header.time_nanosec:               %u\n", sde->header.time_nanosec);

		fprintf(stderr, "dbg5       sde->cmnPart.numBytesCmnPart:           %u\n", sde->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       sde->cmnPart.sensorSystem:              %u\n", sde->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       sde->cmnPart.sensorStatus:              %u\n", sde->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       sde->cmnPart.padding:                   %u\n", sde->cmnPart.padding);

		fprintf(stderr, "dbg5       sde->sensorData.depthUsed_m:            %f\n", sde->sensorData.depthUsed_m);
		fprintf(stderr, "dbg5       sde->sensorData.offset:                 %f\n", sde->sensorData.offset);
		fprintf(stderr, "dbg5       sde->sensorData.scale:                  %f\n", sde->sensorData.scale);
		fprintf(stderr, "dbg5       sde->sensorData.latitude_deg:           %f\n", sde->sensorData.latitude_deg);
		fprintf(stderr, "dbg5       sde->sensorData.longitude_deg:          %f\n", sde->sensorData.longitude_deg);
		fprintf(stderr, "dbg5       sde->sensorData.dataFromSensor:         %s\n", sde->sensorData.dataFromSensor);
	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* calc number of bytes for raw sensor data */
		numBytesRawSensorData = sde->header.numBytesDgm - MBSYS_KMBES_SDE_VAR_OFFSET;

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* common part */
		mb_put_binary_short(MB_YES, sde->cmnPart.numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sde->cmnPart.sensorSystem, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sde->cmnPart.sensorStatus, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sde->cmnPart.padding, &buffer[index]);
		index += 2;

		/* sensor data block */
		mb_put_binary_float(MB_YES, sde->sensorData.depthUsed_m, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, sde->sensorData.offset, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, sde->sensorData.scale, &buffer[index]);
		index += 4;
		mb_put_binary_double(MB_YES, sde->sensorData.latitude_deg, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, sde->sensorData.longitude_deg, &buffer[index]);
		index += 8;
		memcpy(&(sde->sensorData.dataFromSensor), &buffer[index], numBytesRawSensorData);
		index += numBytesRawSensorData;

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, sde->header.numBytesDgm, &buffer[index]);
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
};

int mbr_kemkmall_wr_shi(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error){
	char *function_name = "mbr_kemkmall_wr_shi";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_shi *shi;
	size_t numBytesRawSensorData;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	shi = &(store->shi);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       shi->header.numBytesDgm:                %u\n", shi->header.numBytesDgm);
		fprintf(stderr, "dbg5       shi->header.dgmType:                    %s\n", shi->header.dgmType);
		fprintf(stderr, "dbg5       shi->header.dgmVersion:                 %u\n", shi->header.dgmVersion);
		fprintf(stderr, "dbg5       shi->header.systemID:                   %u\n", shi->header.systemID);
		fprintf(stderr, "dbg5       shi->header.echoSounderID:              %u\n", shi->header.echoSounderID);
		fprintf(stderr, "dbg5       shi->header.time_sec:                   %u\n", shi->header.time_sec);
		fprintf(stderr, "dbg5       shi->header.time_nanosec:               %u\n", shi->header.time_nanosec);

		fprintf(stderr, "dbg5       shi->cmnPart.numBytesCmnPart:           %u\n", shi->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       shi->cmnPart.sensorSystem:              %u\n", shi->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       shi->cmnPart.sensorStatus:              %u\n", shi->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       shi->cmnPart.padding:                   %u\n", shi->cmnPart.padding);

		fprintf(stderr, "dbg5       shi->sensorData.sensorType:             %u\n", shi->sensorData.sensorType);
		fprintf(stderr, "dbg5       shi->sensorData.heigthUsed_m:           %f\n", shi->sensorData.heigthUsed_m);
		fprintf(stderr, "dbg5       shi->sensorData.dataFromSensor:         %s\n", shi->sensorData.dataFromSensor);
	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* calc number of bytes for raw sensor data */
		numBytesRawSensorData = shi->header.numBytesDgm - MBSYS_KMBES_SHI_VAR_OFFSET;

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* common part */
		mb_put_binary_short(MB_YES, shi->cmnPart.numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, shi->cmnPart.sensorSystem, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, shi->cmnPart.sensorStatus, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, shi->cmnPart.padding, &buffer[index]);
		index += 2;

		/* sensor data block */
		mb_put_binary_short(MB_YES, shi->sensorData.sensorType, &buffer[index]);
		index += 2;
		mb_put_binary_float(MB_YES, shi->sensorData.heigthUsed_m, &buffer[index]);
		index += 4;
		memcpy(&buffer[index], &(shi->sensorData.dataFromSensor), numBytesRawSensorData);
		index += numBytesRawSensorData;

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, shi->header.numBytesDgm, &buffer[index]);
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
};

int mbr_kemkmall_wr_sha(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_sha";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_sha *sha;
	char *buffer;
	int index;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	sha = &(store->sha);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       sha->header.numBytesDgm:                %u\n", sha->header.numBytesDgm);
		fprintf(stderr, "dbg5       sha->header.dgmType:                    %s\n", sha->header.dgmType);
		fprintf(stderr, "dbg5       sha->header.dgmVersion:                 %u\n", sha->header.dgmVersion);
		fprintf(stderr, "dbg5       sha->header.systemID:                   %u\n", sha->header.systemID);
		fprintf(stderr, "dbg5       sha->header.echoSounderID:              %u\n", sha->header.echoSounderID);
		fprintf(stderr, "dbg5       sha->header.time_sec:                   %u\n", sha->header.time_sec);
		fprintf(stderr, "dbg5       sha->header.time_nanosec:               %u\n", sha->header.time_nanosec);

		fprintf(stderr, "dbg5       sha->cmnPart.numBytesCmnPart:           %u\n", sha->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       sha->cmnPart.sensorSystem:              %u\n", sha->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       sha->cmnPart.sensorStatus:              %u\n", sha->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       sha->cmnPart.padding:                   %u\n", sha->cmnPart.padding);

		fprintf(stderr, "dbg5       sha->dataInfo.numBytesInfoPart:         %u\n", sha->dataInfo.numBytesInfoPart);
		fprintf(stderr, "dbg5       sha->dataInfo.numSamplesArray:          %u\n", sha->dataInfo.numSamplesArray);
		fprintf(stderr, "dbg5       sha->dataInfo.numBytesPerSample:        %u\n", sha->dataInfo.numBytesPerSample);
		fprintf(stderr, "dbg5       sha->dataInfo.numBytesRawSensorData:    %u\n", sha->dataInfo.numBytesRawSensorData);

		for (i = 0; i < (sha->dataInfo.numSamplesArray); i++) {
			fprintf(stderr, "dbg5       sha->sensorData[%3d].timeSinceRecStart_nanosec: %u\n", i, sha->sensorData[i].timeSinceRecStart_nanosec);
			fprintf(stderr, "dbg5       sha->sensorData[%3d].headingCorrected_deg:      %f\n", i, sha->sensorData[i].headingCorrected_deg);
			fprintf(stderr, "dbg5       sha->sensorData[%3d].dataFromSensor:            %s\n", i, sha->sensorData[i].dataFromSensor);
		}
	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* common part */
		mb_put_binary_short(MB_YES, sha->cmnPart.numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sha->cmnPart.sensorSystem, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sha->cmnPart.sensorStatus, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sha->cmnPart.padding, &buffer[index]);
		index += 2;

		/* sensor info */
		mb_put_binary_short(MB_YES, sha->dataInfo.numBytesInfoPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sha->dataInfo.numSamplesArray, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sha->dataInfo.numBytesPerSample, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sha->dataInfo.numBytesRawSensorData, &buffer[index]);
		index += 2;

		/* sensor data blocks */
		for (i = 0; i<(sha->dataInfo.numSamplesArray); i++) {
			mb_put_binary_int(MB_YES, sha->sensorData[i].timeSinceRecStart_nanosec, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, sha->sensorData[i].headingCorrected_deg, &buffer[index]);
			index += 4;
			memcpy(&buffer[index], &(sha->sensorData[i].dataFromSensor), sha->dataInfo.numBytesRawSensorData);
			index += sha->dataInfo.numBytesRawSensorData;
		}

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, sha->header.numBytesDgm, &buffer[index]);
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
};


int mbr_kemkmall_wr_mrz(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_mrz";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_mrz *mrz;
	char *buffer;
	int index;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to the data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	mrz = &(store->mrz[dgm_index->rx_index]);

	/* size of output record */
	*size = mrz->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* EMdgmMpartition - data partition information */
		mb_put_binary_short(MB_YES, mrz->partition.numOfDgms, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->partition.dgmNum, &buffer[index]);
		index += 2;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       numOfDgms = %d\n", mrz->partition.numOfDgms);
			fprintf(stderr, "dbg5       dgmNum    = %d\n", mrz->partition.dgmNum);
		}

		/* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
		mb_put_binary_short(MB_YES, mrz->cmnPart.numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->cmnPart.pingCnt, &buffer[index]);
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
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       numBytesCmnPart     = %d\n", mrz->cmnPart.numBytesCmnPart);
			fprintf(stderr, "dbg5       pingCnt             = %d\n", mrz->cmnPart.pingCnt);
			fprintf(stderr, "dbg5       rxFansPerPing       = %d\n", mrz->cmnPart.rxFansPerPing);
			fprintf(stderr, "dbg5       rxFanIndex          = %d\n", mrz->cmnPart.rxFanIndex);
			fprintf(stderr, "dbg5       swathsPerPing       = %d\n", mrz->cmnPart.swathsPerPing);
			fprintf(stderr, "dbg5       swathAlongPosition  = %d\n", mrz->cmnPart.swathAlongPosition);
			fprintf(stderr, "dbg5       txTransducerInd     = %d\n", mrz->cmnPart.txTransducerInd);
			fprintf(stderr, "dbg5       rxTransducerInd     = %d\n", mrz->cmnPart.rxTransducerInd);
			fprintf(stderr, "dbg5       numRxTransducers    = %d\n", mrz->cmnPart.numRxTransducers);
			fprintf(stderr, "dbg5       algorithmType       = %d\n", mrz->cmnPart.algorithmType);
		}

		/* EMdgmMRZ_pingInfo - ping info */
		mb_put_binary_short(MB_YES, mrz->pingInfo.numBytesInfoData, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->pingInfo.padding0, &buffer[index]);
		index += 2;

		/* ping info */
		mb_put_binary_float(MB_YES, mrz->pingInfo.pingRate_Hz, &buffer[index]);
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

		mb_put_binary_short(MB_YES, mrz->pingInfo.padding1, &buffer[index]);
		index += 2;
		mb_put_binary_float(MB_YES, mrz->pingInfo.frequencyMode_Hz, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.freqRangeLowLim_Hz, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.freqRangeHighLim_Hz, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.maxTotalTxPulseLength_sec, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.maxEffTxPulseLength_sec, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.maxEffTxBandWidth_Hz, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.absCoeff_dBPerkm, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.portSectorEdge_deg, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.starbSectorEdge_deg, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.portMeanCov_deg, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.starbMeanCov_deg, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, mrz->pingInfo.portMeanCov_m, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->pingInfo.starbMeanCov_m, &buffer[index]);
		index += 2;

		buffer[index] = mrz->pingInfo.modeAndStabilisation;
		index++;
		buffer[index] = mrz->pingInfo.runtimeFilter1;
		index++;

		mb_put_binary_short(MB_YES, mrz->pingInfo.runtimeFilter2, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, mrz->pingInfo.pipeTrackingStatus, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.transmitArraySizeUsed_deg, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.receiveArraySizeUsed_deg, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.transmitPower_dB, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, mrz->pingInfo.SLrampUpTimeRemaining, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->pingInfo.padding2, &buffer[index]);
		index += 2;
		mb_put_binary_float(MB_YES, mrz->pingInfo.yawAngle_deg, &buffer[index]);
		index += 4;

		// Info of tx sector data block, EMdgmMRZ_txSectorInfo
		mb_put_binary_short(MB_YES, mrz->pingInfo.numTxSectors, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->pingInfo.numBytesPerTxSector, &buffer[index]);
		index += 2;

		// Info at time of midpoint of first tx pulse
		mb_put_binary_float(MB_YES, mrz->pingInfo.headingVessel_deg, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.soundSpeedAtTxDepth_mPerSec, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.txTransducerDepth_m, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.z_waterLevelReRefPoint_m, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.x_kmallToall_m, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->pingInfo.y_kmallToall_m, &buffer[index]);
		index += 4;

		buffer[index] = mrz->pingInfo.latLongInfo;
		index++;
		buffer[index] = mrz->pingInfo.posSensorStatus;
		index++;
		buffer[index] = mrz->pingInfo.attitudeSensorStatus;
		index++;
		buffer[index] = mrz->pingInfo.padding3;
		index++;

		mb_put_binary_double(MB_YES, mrz->pingInfo.latitude_deg, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, mrz->pingInfo.longitude_deg, &buffer[index]);
		index += 8;
		mb_put_binary_float(MB_YES, mrz->pingInfo.ellipsoidHeightReRefPoint_m, &buffer[index]);
		index += 4;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       numBytesInfoData            = %d\n", mrz->pingInfo.numBytesInfoData);
			fprintf(stderr, "dbg5       padding0                    = %d\n", mrz->pingInfo.padding0);
			fprintf(stderr, "dbg5       pingRate_Hz                 = %f\n", mrz->pingInfo.pingRate_Hz);
			fprintf(stderr, "dbg5       beamSpacing                 = %d\n", mrz->pingInfo.beamSpacing);
			fprintf(stderr, "dbg5       depthMode                   = %d\n", mrz->pingInfo.depthMode);
			fprintf(stderr, "dbg5       subDepthMode                = %d\n", mrz->pingInfo.subDepthMode);
			fprintf(stderr, "dbg5       distanceBtwSwath            = %d\n", mrz->pingInfo.distanceBtwSwath);
			fprintf(stderr, "dbg5       detectionMode               = %d\n", mrz->pingInfo.detectionMode);
			fprintf(stderr, "dbg5       pulseForm                   = %d\n", mrz->pingInfo.pulseForm);
			fprintf(stderr, "dbg5       padding1                    = %d\n", mrz->pingInfo.padding1);
			fprintf(stderr, "dbg5       frequencyMode_Hz            = %f\n", mrz->pingInfo.frequencyMode_Hz);
			fprintf(stderr, "dbg5       freqRangeLowLim_Hz          = %f\n", mrz->pingInfo.freqRangeLowLim_Hz);
			fprintf(stderr, "dbg5       freqRangeHighLim_Hz         = %f\n", mrz->pingInfo.freqRangeHighLim_Hz);
			fprintf(stderr, "dbg5       maxEffTxPulseLength_sec     = %f\n", mrz->pingInfo.maxEffTxPulseLength_sec);
			fprintf(stderr, "dbg5       maxTotalTxPulseLength_sec   = %f\n", mrz->pingInfo.maxTotalTxPulseLength_sec);
			fprintf(stderr, "dbg5       maxEffTxBandWidth_Hz        = %f\n", mrz->pingInfo.maxEffTxBandWidth_Hz);
			fprintf(stderr, "dbg5       absCoeff_dBPerkm            = %f\n", mrz->pingInfo.absCoeff_dBPerkm);
			fprintf(stderr, "dbg5       portSectorEdge_deg          = %f\n", mrz->pingInfo.portSectorEdge_deg);
			fprintf(stderr, "dbg5       starbSectorEdge_deg         = %f\n", mrz->pingInfo.starbSectorEdge_deg);
			fprintf(stderr, "dbg5       portMeanCov_m               = %d\n", mrz->pingInfo.portMeanCov_m);
			fprintf(stderr, "dbg5       starbMeanCov_m              = %d\n", mrz->pingInfo.starbMeanCov_m);
			fprintf(stderr, "dbg5       modeAndStabilisation        = %d\n", mrz->pingInfo.modeAndStabilisation);
			fprintf(stderr, "dbg5       runtimeFilter1              = %d\n", mrz->pingInfo.runtimeFilter1);
			fprintf(stderr, "dbg5       runtimeFilter2              = %d\n", mrz->pingInfo.runtimeFilter2);
			fprintf(stderr, "dbg5       pipeTrackingStatus          = %d\n", mrz->pingInfo.pipeTrackingStatus);
			fprintf(stderr, "dbg5       transmitArraySizeUsed_deg   = %f\n", mrz->pingInfo.transmitArraySizeUsed_deg);
			fprintf(stderr, "dbg5       receiveArraySizeUsed_deg    = %f\n", mrz->pingInfo.receiveArraySizeUsed_deg);
			fprintf(stderr, "dbg5       transmitPower_dB            = %f\n", mrz->pingInfo.transmitPower_dB);
			fprintf(stderr, "dbg5       SLrampUpTimeRemaining       = %d\n", mrz->pingInfo.SLrampUpTimeRemaining);
			fprintf(stderr, "dbg5       padding2                    = %d\n", mrz->pingInfo.padding2);
			fprintf(stderr, "dbg5       yawAngle_deg                = %f\n", mrz->pingInfo.yawAngle_deg);
			fprintf(stderr, "dbg5       numTxSectors                = %d\n", mrz->pingInfo.numTxSectors);
			fprintf(stderr, "dbg5       numBytesPerTxSector         = %d\n", mrz->pingInfo.numBytesPerTxSector);
			fprintf(stderr, "dbg5       headingVessel_deg           = %f\n", mrz->pingInfo.headingVessel_deg);
			fprintf(stderr, "dbg5       soundSpeedAtTxDepth_mPerSec = %f\n", mrz->pingInfo.soundSpeedAtTxDepth_mPerSec);
			fprintf(stderr, "dbg5       txTransducerDepth_m         = %f\n", mrz->pingInfo.txTransducerDepth_m);
			fprintf(stderr, "dbg5       z_waterLevelReRefPoint_m    = %f\n", mrz->pingInfo.z_waterLevelReRefPoint_m);
			fprintf(stderr, "dbg5       x_kmallToall_m              = %f\n", mrz->pingInfo.x_kmallToall_m);
			fprintf(stderr, "dbg5       y_kmallToall_m              = %f\n", mrz->pingInfo.y_kmallToall_m);
			fprintf(stderr, "dbg5       latLongInfo                 = %d\n", mrz->pingInfo.latLongInfo);
			fprintf(stderr, "dbg5       posSensorStatus             = %d\n", mrz->pingInfo.posSensorStatus);
			fprintf(stderr, "dbg5       attitudeSensorStatus        = %d\n", mrz->pingInfo.attitudeSensorStatus);
			fprintf(stderr, "dbg5       padding3                    = %d\n", mrz->pingInfo.padding3);
			fprintf(stderr, "dbg5       latitude_deg                = %f\n", mrz->pingInfo.latitude_deg);
			fprintf(stderr, "dbg5       longitude_deg               = %f\n", mrz->pingInfo.longitude_deg);
			fprintf(stderr, "dbg5       ellipsoidHeightReRefPoint_m = %f\n", mrz->pingInfo.ellipsoidHeightReRefPoint_m);
		}

		/* EMdgmMRZ_txSectorInfo - sector information */
		for (i = 0; i < (mrz->pingInfo.numTxSectors); i++) {
			buffer[index] = mrz->sectorInfo[i].txSectorNumb;
			index++;
			buffer[index] = mrz->sectorInfo[i].txArrNumber;
			index++;
			buffer[index] = mrz->sectorInfo[i].txSubArray;
			index++;
			buffer[index] = mrz->sectorInfo[i].padding0;
			index++;
			mb_put_binary_float(MB_YES, mrz->sectorInfo[i].sectorTransmitDelay_sec, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sectorInfo[i].tiltAngleReTx_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sectorInfo[i].txNominalSourceLevel_dB, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sectorInfo[i].txFocusRange_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sectorInfo[i].centreFreq_Hz, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sectorInfo[i].signalBandWidth_Hz, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sectorInfo[i].totalSignalLength_sec, &buffer[index]);
			index += 4;
			buffer[index] = mrz->sectorInfo[i].pulseShading;
			index++;
			buffer[index] = mrz->sectorInfo[i].signalWaveForm;
			index++;
			mb_put_binary_short(MB_YES, mrz->sectorInfo[i].padding1, &buffer[index]);
			index += 2;

			/* print debug statements */
			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
				fprintf(stderr, "dbg5       #MWC transmit sector %d/%d:\n", i + 1, mrz->pingInfo.numTxSectors);
				fprintf(stderr, "dbg5       txSectorNumb            = %d\n", mrz->sectorInfo[i].txSectorNumb);
				fprintf(stderr, "dbg5       txArrNumber             = %d\n", mrz->sectorInfo[i].txArrNumber);
				fprintf(stderr, "dbg5       txSubArray              = %d\n", mrz->sectorInfo[i].txSubArray);
				fprintf(stderr, "dbg5       padding0                = %d\n", mrz->sectorInfo[i].padding0);
				fprintf(stderr, "dbg5       sectorTransmitDelay_sec = %f\n", mrz->sectorInfo[i].sectorTransmitDelay_sec);
				fprintf(stderr, "dbg5       tiltAngleReTx_deg       = %f\n", mrz->sectorInfo[i].tiltAngleReTx_deg);
				fprintf(stderr, "dbg5       txNominalSourceLevel_dB = %f\n", mrz->sectorInfo[i].txNominalSourceLevel_dB);
				fprintf(stderr, "dbg5       txFocusRange_m          = %f\n", mrz->sectorInfo[i].txFocusRange_m);
				fprintf(stderr, "dbg5       centreFreq_Hz           = %f\n", mrz->sectorInfo[i].centreFreq_Hz);
				fprintf(stderr, "dbg5       signalBandWidth_Hz      = %f\n", mrz->sectorInfo[i].signalBandWidth_Hz);
				fprintf(stderr, "dbg5       totalSignalLength_sec   = %f\n", mrz->sectorInfo[i].totalSignalLength_sec);
				fprintf(stderr, "dbg5       pulseShading            = %d\n", mrz->sectorInfo[i].pulseShading);
				fprintf(stderr, "dbg5       signalWaveForm          = %d\n", mrz->sectorInfo[i].signalWaveForm);
				fprintf(stderr, "dbg5       padding1                = %d\n", mrz->sectorInfo[i].padding1);
			}
		}

		/* EMdgmMRZ_rxInfo - receiver specific info */
		mb_put_binary_short(MB_YES, mrz->rxInfo.numBytesRxInfo, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->rxInfo.numSoundingsMaxMain, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->rxInfo.numSoundingsValidMain, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->rxInfo.numBytesPerSounding, &buffer[index]);
		index += 2;

		mb_put_binary_float(MB_YES, mrz->rxInfo.WCSampleRate, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->rxInfo.seabedImageSampleRate, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->rxInfo.BSnormal_dB, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mrz->rxInfo.BSoblique_dB, &buffer[index]);
		index += 4;

		mb_put_binary_short(MB_YES, mrz->rxInfo.extraDetectionAlarmFlag, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->rxInfo.numExtraDetections, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->rxInfo.numExtraDetectionClasses, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mrz->rxInfo.numBytesPerClass, &buffer[index]);
		index += 2;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       numBytesInfoData            = %d\n", mrz->rxInfo.numBytesRxInfo);
			fprintf(stderr, "dbg5       numSoundingsMaxMain         = %d\n", mrz->rxInfo.numSoundingsMaxMain);
			fprintf(stderr, "dbg5       numSoundingsValidMain       = %d\n", mrz->rxInfo.numSoundingsValidMain);
			fprintf(stderr, "dbg5       numBytesPerSounding         = %d\n", mrz->rxInfo.numBytesPerSounding);
			fprintf(stderr, "dbg5       WCSampleRate                = %f\n", mrz->rxInfo.WCSampleRate);
			fprintf(stderr, "dbg5       seabedImageSampleRate       = %f\n", mrz->rxInfo.seabedImageSampleRate);
			fprintf(stderr, "dbg5       BSnormal_dB                 = %f\n", mrz->rxInfo.BSnormal_dB);
			fprintf(stderr, "dbg5       BSoblique_dB                = %f\n", mrz->rxInfo.BSoblique_dB);
			fprintf(stderr, "dbg5       extraDetectionAlarmFlag     = %d\n", mrz->rxInfo.extraDetectionAlarmFlag);
			fprintf(stderr, "dbg5       numExtraDetections          = %d\n", mrz->rxInfo.numExtraDetections);
			fprintf(stderr, "dbg5       numExtraDetectionClasses    = %d\n", mrz->rxInfo.numExtraDetectionClasses);
			fprintf(stderr, "dbg5       numBytesPerClass            = %d\n", mrz->rxInfo.numBytesPerClass);
		}

		/* EMdgmMRZ_extraDetClassInfo -  Extra detection class info */
		for (i = 0; i < (mrz->rxInfo.numExtraDetectionClasses); i++) {
			mb_put_binary_short(MB_YES, mrz->extraDetClassInfo[i].numExtraDetInClass, &buffer[index]);
			index += 2;
			buffer[index] = mrz->extraDetClassInfo[i].padding;
			index++;
			buffer[index] = mrz->extraDetClassInfo[i].alarmFlag;
			index++;

			/* print debug statements */
			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
				fprintf(stderr, "dbg5       numExtraDetInClass  = %d\n", mrz->extraDetClassInfo[i].numExtraDetInClass);
				fprintf(stderr, "dbg5       padding             = %d\n", mrz->extraDetClassInfo[i].padding);
				fprintf(stderr, "dbg5       alarmFlag           = %d\n", mrz->extraDetClassInfo[i].alarmFlag);
			}
		}

		/* EMdgmMRZ_sounding - Data for each sounding */
		int numSidescanSamples = 0;
		int numSoundings = mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections;
		for (i = 0; i < numSoundings; i++) {
			mb_put_binary_short(MB_YES, mrz->sounding[i].soundingIndex, &buffer[index]);
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
			mb_put_binary_short(MB_YES, mrz->sounding[i].padding, &buffer[index]);
			index += 2;
			mb_put_binary_float(MB_YES, mrz->sounding[i].rangeFactor, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].qualityFactor, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].detectionUncertaintyVer_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].detectionUncertaintyHor_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].detectionWindowLength_sec, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].echoLength_sec, &buffer[index]);
			index += 4;

			/* Water column paramters */
			mb_put_binary_short(MB_YES, mrz->sounding[i].WCBeamNumb, &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, mrz->sounding[i].WCrange_samples, &buffer[index]);
			index += 2;
			mb_put_binary_float(MB_YES, mrz->sounding[i].WCNomBeamAngleAcross_deg, &buffer[index]);
			index += 4;

			/* Reflectivity data (backscatter (BS) data) */
			mb_put_binary_float(MB_YES, mrz->sounding[i].meanAbsCoeff_dBPerkm, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].reflectivity1_dB, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].reflectivity2_dB, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].receiverSensitivityApplied_dB, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].sourceLevelApplied_dB, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].BScalibration_dB, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].TVG_dB, &buffer[index]);
			index += 4;

			/* Range and angle data */
			mb_put_binary_float(MB_YES, mrz->sounding[i].beamAngleReRx_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].beamAngleCorrection_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].twoWayTravelTime_sec, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].twoWayTravelTimeCorrection_sec, &buffer[index]);
			index += 4;

			/* Georeferenced depth points */
			mb_put_binary_float(MB_YES, mrz->sounding[i].deltaLatitude_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].deltaLongitude_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].z_reRefPoint_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].y_reRefPoint_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].x_reRefPoint_m, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mrz->sounding[i].beamIncAngleAdj_deg, &buffer[index]);
			index += 4;
			mb_put_binary_short(MB_YES, mrz->sounding[i].realTimeCleanInfo, &buffer[index]);
			index += 2;

			/* Seabed image */
			mb_put_binary_short(MB_YES, mrz->sounding[i].SIstartRange_samples, &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, mrz->sounding[i].SIcentreSample, &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, mrz->sounding[i].SInumSamples, &buffer[index]);
			index += 2;

			numSidescanSamples += mrz->sounding[i].SInumSamples;

			/* print debug statements */
			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
				fprintf(stderr, "dbg5       soundingIndex                   = %d\n", mrz->sounding[i].soundingIndex);
				fprintf(stderr, "dbg5       txSectorNumb                    = %d\n", mrz->sounding[i].txSectorNumb);
				fprintf(stderr, "dbg5       detectionType                   = %d\n", mrz->sounding[i].detectionType);
				fprintf(stderr, "dbg5       detectionMethod                 = %d\n", mrz->sounding[i].detectionMethod);
				fprintf(stderr, "dbg5       rejectionInfo1                  = %d\n", mrz->sounding[i].rejectionInfo1);
				fprintf(stderr, "dbg5       rejectionInfo2                  = %d\n", mrz->sounding[i].rejectionInfo2);
				fprintf(stderr, "dbg5       postProcessingInfo              = %d\n", mrz->sounding[i].postProcessingInfo);
				fprintf(stderr, "dbg5       detectionClass                  = %d\n", mrz->sounding[i].detectionClass);
				fprintf(stderr, "dbg5       detectionConfidenceLevel        = %d\n", mrz->sounding[i].detectionConfidenceLevel);
				fprintf(stderr, "dbg5       padding                         = %d\n", mrz->sounding[i].padding);
				fprintf(stderr, "dbg5       rangeFactor                     = %f\n", mrz->sounding[i].rangeFactor);
				fprintf(stderr, "dbg5       qualityFactor                   = %f\n", mrz->sounding[i].qualityFactor);
				fprintf(stderr, "dbg5       detectionUncertaintyVer_m       = %f\n", mrz->sounding[i].detectionUncertaintyVer_m);
				fprintf(stderr, "dbg5       detectionUncertaintyHor_m       = %f\n", mrz->sounding[i].detectionUncertaintyHor_m);
				fprintf(stderr, "dbg5       detectionWindowLength_sec       = %f\n", mrz->sounding[i].detectionWindowLength_sec);
				fprintf(stderr, "dbg5       echoLength_sec                  = %f\n", mrz->sounding[i].echoLength_sec);
				fprintf(stderr, "dbg5       WCBeamNumb                      = %d\n", mrz->sounding[i].WCBeamNumb);
				fprintf(stderr, "dbg5       WCrange_samples                 = %d\n", mrz->sounding[i].WCrange_samples);
				fprintf(stderr, "dbg5       WCNomBeamAngleAcross_deg        = %f\n", mrz->sounding[i].WCNomBeamAngleAcross_deg);
				fprintf(stderr, "dbg5       meanAbsCoeff_dBPerkm            = %f\n", mrz->sounding[i].meanAbsCoeff_dBPerkm);
				fprintf(stderr, "dbg5       reflectivity1_dB                = %f\n", mrz->sounding[i].reflectivity1_dB);
				fprintf(stderr, "dbg5       reflectivity2_dB                = %f\n", mrz->sounding[i].reflectivity2_dB);
				fprintf(stderr, "dbg5       receiverSensitivityApplied_dB   = %f\n", mrz->sounding[i].receiverSensitivityApplied_dB);
				fprintf(stderr, "dbg5       sourceLevelApplied_dB           = %f\n", mrz->sounding[i].sourceLevelApplied_dB);
				fprintf(stderr, "dbg5       BScalibration_dB                = %f\n", mrz->sounding[i].BScalibration_dB);
				fprintf(stderr, "dbg5       TVG_dB                          = %f\n", mrz->sounding[i].TVG_dB);
				fprintf(stderr, "dbg5       beamAngleReRx_deg               = %f\n", mrz->sounding[i].beamAngleReRx_deg);
				fprintf(stderr, "dbg5       beamAngleCorrection_deg         = %f\n", mrz->sounding[i].beamAngleCorrection_deg);
				fprintf(stderr, "dbg5       twoWayTravelTime_sec            = %f\n", mrz->sounding[i].twoWayTravelTime_sec);
				fprintf(stderr, "dbg5       twoWayTravelTimeCorrection_sec  = %f\n", mrz->sounding[i].twoWayTravelTimeCorrection_sec);
				fprintf(stderr, "dbg5       deltaLatitude_deg               = %f\n", mrz->sounding[i].deltaLatitude_deg);
				fprintf(stderr, "dbg5       deltaLongitude_deg              = %f\n", mrz->sounding[i].deltaLongitude_deg);
				fprintf(stderr, "dbg5       z_reRefPoint_m                  = %f\n", mrz->sounding[i].z_reRefPoint_m);
				fprintf(stderr, "dbg5       y_reRefPoint_m                  = %f\n", mrz->sounding[i].y_reRefPoint_m);
				fprintf(stderr, "dbg5       x_reRefPoint_m                  = %f\n", mrz->sounding[i].x_reRefPoint_m);
				fprintf(stderr, "dbg5       beamIncAngleAdj_deg             = %f\n", mrz->sounding[i].beamIncAngleAdj_deg);
				fprintf(stderr, "dbg5       realTimeCleanInfo               = %d\n", mrz->sounding[i].realTimeCleanInfo);
				fprintf(stderr, "dbg5       SIstartRange_samples            = %d\n", mrz->sounding[i].SIstartRange_samples);
				fprintf(stderr, "dbg5       SIcentreSample                  = %d\n", mrz->sounding[i].SIcentreSample);
				fprintf(stderr, "dbg5       SInumSamples                    = %d\n", mrz->sounding[i].SInumSamples);
			}
		}

		for (i = 0; i < numSidescanSamples; i++) {
			mb_put_binary_short(MB_YES, mrz->SIsample_desidB[i], &buffer[index]);
			index += 2;
		}

		/* Insert closing byte count */
		mb_put_binary_int(MB_YES, mrz->header.numBytesDgm, &buffer[index]);
		// index += 4;
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
};

int mbr_kemkmall_wr_mwc(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_mwc";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_mwc *mwc;
	char *buffer;
	int index;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	mwc = &(store->mwc[dgm_index->rx_index]);

	/* size of output record */
	*size = mwc->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* EMdgmMpartition - data partition information */
		mb_put_binary_short(MB_YES, mwc->partition.numOfDgms, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mwc->partition.dgmNum, &buffer[index]);
		index += 2;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       numOfDgms = %d\n", mwc->partition.numOfDgms);
			fprintf(stderr, "dbg5       dgmNum    = %d\n", mwc->partition.dgmNum);
		}

		/* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
		mb_put_binary_short(MB_YES, mwc->cmnPart.numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mwc->cmnPart.pingCnt, &buffer[index]);
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

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       numBytesCmnPart     = %d\n", mwc->cmnPart.numBytesCmnPart);
			fprintf(stderr, "dbg5       pingCnt             = %d\n", mwc->cmnPart.pingCnt);
			fprintf(stderr, "dbg5       rxFansPerPing       = %d\n", mwc->cmnPart.rxFansPerPing);
			fprintf(stderr, "dbg5       rxFanIndex          = %d\n", mwc->cmnPart.rxFanIndex);
			fprintf(stderr, "dbg5       swathsPerPing       = %d\n", mwc->cmnPart.swathsPerPing);
			fprintf(stderr, "dbg5       swathAlongPosition  = %d\n", mwc->cmnPart.swathAlongPosition);
			fprintf(stderr, "dbg5       txTransducerInd     = %d\n", mwc->cmnPart.txTransducerInd);
			fprintf(stderr, "dbg5       rxTransducerInd     = %d\n", mwc->cmnPart.rxTransducerInd);
			fprintf(stderr, "dbg5       numRxTransducers    = %d\n", mwc->cmnPart.numRxTransducers);
			fprintf(stderr, "dbg5       algorithmType       = %d\n", mwc->cmnPart.algorithmType);
		}

		/* EMdgmMWCtxInfo - transmit sectors, general info for all sectors */
		mb_put_binary_short(MB_YES, mwc->txInfo.numBytesTxInfo, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mwc->txInfo.numTxSectors, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mwc->txInfo.numBytesPerTxSector, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mwc->txInfo.padding, &buffer[index]);
		index += 2;
		mb_put_binary_float(MB_YES, mwc->txInfo.heave_m, &buffer[index]);
		index += 4;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       numBytesTxInfo      = %d\n", mwc->txInfo.numBytesTxInfo);
			fprintf(stderr, "dbg5       numTxSectors        = %d\n", mwc->txInfo.numTxSectors);
			fprintf(stderr, "dbg5       numBytesPerTxSector = %d\n", mwc->txInfo.numBytesPerTxSector);
			fprintf(stderr, "dbg5       padding             = %d\n", mwc->txInfo.padding);
			fprintf(stderr, "dbg5       heave_m             = %f\n", mwc->txInfo.heave_m);
		}

		/* EMdgmMWCtxSectorData - transmit sector data, loop for all i = numTxSectors */
		for (i=0; i<(mwc->txInfo.numTxSectors); i++) {
			mb_put_binary_float(MB_YES, mwc->sectorData[i].tiltAngleReTx_deg, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mwc->sectorData[i].centreFreq_Hz, &buffer[index]);
			index += 4;
			mb_put_binary_float(MB_YES, mwc->sectorData[i].txBeamWidthAlong_deg, &buffer[index]);
			index += 4;
			mb_put_binary_short(MB_YES, mwc->sectorData[i].txSectorNum, &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, mwc->sectorData[i].padding, &buffer[index]);
			index += 2;

			/* print debug statements */
			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
				fprintf(stderr, "dbg5       #MWC transmit sector %d/%d:\n", i + 1, mwc->txInfo.numTxSectors);
				fprintf(stderr, "dbg5       tiltAngleReTx_deg    = %f\n", mwc->sectorData[i].tiltAngleReTx_deg);
				fprintf(stderr, "dbg5       centreFreq_Hz        = %f\n", mwc->sectorData[i].centreFreq_Hz);
				fprintf(stderr, "dbg5       txBeamWidthAlong_deg = %f\n", mwc->sectorData[i].txBeamWidthAlong_deg);
				fprintf(stderr, "dbg5       txSectorNum          = %d\n", mwc->sectorData[i].txSectorNum);
				fprintf(stderr, "dbg5       padding              = %d\n", mwc->sectorData[i].padding);
			}
		}

		/* EMdgmMWCrxInfo - receiver, general info */
		mb_put_binary_short(MB_YES, mwc->rxInfo.numBytesRxInfo, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, mwc->rxInfo.numBeams, &buffer[index]);
		index += 2;
		buffer[index] = mwc->rxInfo.numBytesPerBeamEntry;
		index ++;
		buffer[index] = mwc->rxInfo.phaseFlag;
		index ++;
		buffer[index] = mwc->rxInfo.TVGfunctionApplied;
		index ++;
		buffer[index] = mwc->rxInfo.TVGoffset_dB;
		index ++;
		mb_put_binary_float(MB_YES, mwc->rxInfo.sampleFreq_Hz, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, mwc->rxInfo.soundVelocity_mPerSec, &buffer[index]);
		index += 4;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
			fprintf(stderr, "dbg5       numBytesRxInfo        = %d\n", mwc->rxInfo.numBytesRxInfo);
			fprintf(stderr, "dbg5       numBeams              = %d\n", mwc->rxInfo.numBeams);
			fprintf(stderr, "dbg5       numBytesPerBeamEntry  = %d\n", mwc->rxInfo.numBytesPerBeamEntry);
			fprintf(stderr, "dbg5       phaseFlag             = %d\n", mwc->rxInfo.phaseFlag);
			fprintf(stderr, "dbg5       TVGfunctionApplied    = %d\n", mwc->rxInfo.TVGfunctionApplied);
			fprintf(stderr, "dbg5       TVGoffset_dB          = %d\n", mwc->rxInfo.TVGoffset_dB);
			fprintf(stderr, "dbg5       sampleFreq_Hz         = %f\n", mwc->rxInfo.sampleFreq_Hz);
			fprintf(stderr, "dbg5       soundVelocity_mPerSec = %f\n", mwc->rxInfo.soundVelocity_mPerSec);
		}

		/* EMdgmMWCrxBeamData - receiver, specific info for each beam */
		for (i=0; i<(mwc->rxInfo.numBeams); i++)
		{
			mb_put_binary_float(MB_YES, mwc->beamData_p[i].beamPointAngReVertical_deg, &buffer[index]);
			index += 4;
			mb_put_binary_short(MB_YES, mwc->beamData_p[i].startRangeSampleNum, &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, mwc->beamData_p[i].detectedRangeInSamples, &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, mwc->beamData_p[i].beamTxSectorNum, &buffer[index]);
			index += 2;
			mb_put_binary_short(MB_YES, mwc->beamData_p[i].numSampleData, &buffer[index]);
			index += 2;

			/* now insert the samples */
			memcpy(&buffer[index], &(mwc->beamData_p[i].sampleAmplitude05dB_p), mwc->beamData_p[i].numSampleData);
			index += mwc->beamData_p[i].numSampleData;

			/* Water Column data are followed by struct rxBeamPhase1 or struct rxBeamPhase2 */
			/* if indicated in the field phaseFlag in struct rxInfo */
			switch (mwc->rxInfo.phaseFlag) {
				case 0:
					break;
				case 1:
					/* Rx beam phase in 180/128 degree resolution. */
					buffer[index] = mwc->beamData_p[i].rx_beam_phase1.rxBeamPhase;
					index++;
					break;
				case 2:
					/* Rx beam phase in 0.01 degree resolution */
					mb_put_binary_short(MB_YES, mwc->beamData_p[i].rx_beam_phase2.rxBeamPhase, &buffer[index]);
					index += 2;
					break;
				default:
					break;
			}

			// TODO: don't forget to free(mwc->beamData_p[i].sampleAmplitude05dB_p);!
			// mb_freed(verbose, __FILE__, __LINE__, (void **)&(mwc->beamData_p[i].sampleAmplitude05dB_p), error);
			// free(mwc->beamData_p[i].sampleAmplitude05dB_p);

			/* print debug statements */
			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
				fprintf(stderr, "dbg5       #MWC receiver beam data %d/%d:\n", i + 1, mwc->rxInfo.numBeams);
				fprintf(stderr, "dbg5       tiltAngleReTx_deg       = %f\n", mwc->beamData_p[i].beamPointAngReVertical_deg);
				fprintf(stderr, "dbg5       startRangeSampleNum     = %d\n", mwc->beamData_p[i].startRangeSampleNum);
				fprintf(stderr, "dbg5       detectedRangeInSamples  = %d\n", mwc->beamData_p[i].detectedRangeInSamples);
				fprintf(stderr, "dbg5       beamTxSectorNum         = %d\n", mwc->beamData_p[i].beamTxSectorNum);
				fprintf(stderr, "dbg5       numSampleData           = %d\n", mwc->beamData_p[i].numSampleData);
				fprintf(stderr, "dbg5       sampleAmplitude05dB_p   = [");
				for (int k = 0; k < (mwc->beamData_p[i].numSampleData); k++)
					printf("%d, ", mwc->beamData_p[i].sampleAmplitude05dB_p[k]);
				printf("]\n");
			}
		}

		// TODO: don't forget to free(mwc->beamData_p);!
		// mb_freed(verbose, __FILE__, __LINE__, (void **)&(mwc->beamData_p), error);
		// free(mwc->beamData_p);

		/* Insert closing byte count */
		mb_put_binary_int(MB_YES, mwc->header.numBytesDgm, &buffer[index]);
		// index += 4;
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
};

int mbr_kemkmall_wr_cpo(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_cpo";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_cpo *cpo;
	size_t numBytesRawSensorData;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	cpo = &(store->cpo);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       cpo->header.numBytesDgm:                %u\n", cpo->header.numBytesDgm);
		fprintf(stderr, "dbg5       cpo->header.dgmType:                    %s\n", cpo->header.dgmType);
		fprintf(stderr, "dbg5       cpo->header.dgmVersion:                 %u\n", cpo->header.dgmVersion);
		fprintf(stderr, "dbg5       cpo->header.systemID:                   %u\n", cpo->header.systemID);
		fprintf(stderr, "dbg5       cpo->header.echoSounderID:              %u\n", cpo->header.echoSounderID);
		fprintf(stderr, "dbg5       cpo->header.time_sec:                   %u\n", cpo->header.time_sec);
		fprintf(stderr, "dbg5       cpo->header.time_nanosec:               %u\n", cpo->header.time_nanosec);

		fprintf(stderr, "dbg5       cpo->cmnPart.numBytesCmnPart:           %u\n", cpo->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       cpo->cmnPart.sensorSystem:              %u\n", cpo->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       cpo->cmnPart.sensorStatus:              %u\n", cpo->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       cpo->cmnPart.padding:                   %u\n", cpo->cmnPart.padding);

		fprintf(stderr, "dbg5       cpo->sensorData.timeFromSensor_sec:     %u\n", cpo->sensorData.timeFromSensor_sec);
		fprintf(stderr, "dbg5       cpo->sensorData.timeFromSensor_nanosec: %u\n", cpo->sensorData.timeFromSensor_nanosec);
		fprintf(stderr, "dbg5       cpo->sensorData.posFixQuality_m:        %f\n", cpo->sensorData.posFixQuality_m);
		fprintf(stderr, "dbg5       cpo->sensorData.correctedLat_deg:       %f\n", cpo->sensorData.correctedLat_deg);
		fprintf(stderr, "dbg5       cpo->sensorData.correctedLong_deg:      %f\n", cpo->sensorData.correctedLong_deg);
		fprintf(stderr, "dbg5       cpo->sensorData.speedOverGround_mPerSec:%f\n", cpo->sensorData.speedOverGround_mPerSec);
		fprintf(stderr, "dbg5       cpo->sensorData.correctedLat_deg:       %f\n", cpo->sensorData.courseOverGround_deg);
		fprintf(stderr, "dbg5       cpo->sensorData.correctedLong_deg:      %f\n", cpo->sensorData.ellipsoidHeightReRefPoint_m);
		fprintf(stderr, "dbg5       cpo->sensorData.speedOverGround_mPerSec:%s\n", cpo->sensorData.posDataFromSensor);
	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* common part */
		mb_put_binary_short(MB_YES, cpo->cmnPart.numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, cpo->cmnPart.sensorSystem, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, cpo->cmnPart.sensorStatus, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, cpo->cmnPart.padding, &buffer[index]);
		index += 2;

		/* sensor data block */
		mb_put_binary_int(MB_YES, cpo->sensorData.timeFromSensor_sec, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, cpo->sensorData.timeFromSensor_nanosec, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, cpo->sensorData.posFixQuality_m, &buffer[index]);
		index += 4;
		mb_put_binary_double(MB_YES, cpo->sensorData.correctedLat_deg, &buffer[index]);
		index += 8;
		mb_put_binary_double(MB_YES, cpo->sensorData.correctedLong_deg, &buffer[index]);
		index += 8;
		mb_put_binary_float(MB_YES, cpo->sensorData.speedOverGround_mPerSec, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, cpo->sensorData.courseOverGround_deg, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, cpo->sensorData.ellipsoidHeightReRefPoint_m, &buffer[index]);
		index += 4;

		/* raw data msg from sensor */
		memcpy(&buffer[index], &(cpo->sensorData.posDataFromSensor), numBytesRawSensorData);
		index += numBytesRawSensorData;

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, cpo->header.numBytesDgm, &buffer[index]);
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
};

int mbr_kemkmall_wr_che(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_che";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_che *che;
	size_t numBytesRawSensorData;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	che = &(store->che);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       che->header.numBytesDgm:                %u\n", che->header.numBytesDgm);
		fprintf(stderr, "dbg5       che->header.dgmType:                    %s\n", che->header.dgmType);
		fprintf(stderr, "dbg5       che->header.dgmVersion:                 %u\n", che->header.dgmVersion);
		fprintf(stderr, "dbg5       che->header.systemID:                   %u\n", che->header.systemID);
		fprintf(stderr, "dbg5       che->header.echoSounderID:              %u\n", che->header.echoSounderID);
		fprintf(stderr, "dbg5       che->header.time_sec:                   %u\n", che->header.time_sec);
		fprintf(stderr, "dbg5       che->header.time_nanosec:               %u\n", che->header.time_nanosec);

		fprintf(stderr, "dbg5       che->cmnPart.numBytesCmnPart:           %u\n", che->cmnPart.numBytesCmnPart);
		fprintf(stderr, "dbg5       che->cmnPart.sensorSystem:              %u\n", che->cmnPart.sensorSystem);
		fprintf(stderr, "dbg5       che->cmnPart.sensorStatus:              %u\n", che->cmnPart.sensorStatus);
		fprintf(stderr, "dbg5       che->cmnPart.padding:                   %u\n", che->cmnPart.padding);

		fprintf(stderr, "dbg5       che->data.data.heave_m:                 %f\n", che->data.heave_m);
	}

	/* size of output record */
	*size = dgm_index->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		/* common part */
		mb_put_binary_short(MB_YES, che->cmnPart.numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, che->cmnPart.sensorSystem, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, che->cmnPart.sensorStatus, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, che->cmnPart.padding, &buffer[index]);
		index += 2;

		/* sensor data block */
		mb_put_binary_float(MB_YES, che->data.heave_m, &buffer[index]);
		index += 4;

		/* insert closing byte count */
		mb_put_binary_int(MB_YES, che->header.numBytesDgm, &buffer[index]);
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
};

int mbr_kemkmall_wr_iip(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_iip";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_iip *iip;
	size_t numBytesRawSensorData;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	iip = &(store->iip);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       iip->header.numBytesDgm:                %u\n", iip->header.numBytesDgm);
		fprintf(stderr, "dbg5       iip->header.dgmType:                    %s\n", iip->header.dgmType);
		fprintf(stderr, "dbg5       iip->header.dgmVersion:                 %u\n", iip->header.dgmVersion);
		fprintf(stderr, "dbg5       iip->header.systemID:                   %u\n", iip->header.systemID);
		fprintf(stderr, "dbg5       iip->header.echoSounderID:              %u\n", iip->header.echoSounderID);
		fprintf(stderr, "dbg5       iip->header.time_sec:                   %u\n", iip->header.time_sec);
		fprintf(stderr, "dbg5       iip->header.time_nanosec:               %u\n", iip->header.time_nanosec);

		fprintf(stderr, "dbg5       iip->iip->numBytesCmnPart:              %u\n", iip->numBytesCmnPart);
		fprintf(stderr, "dbg5       iip->info:                              %u\n", iip->info);
		fprintf(stderr, "dbg5       iip->status:                            %u\n", iip->status);
		fprintf(stderr, "dbg5       iip->install_txt:                       %s\n", iip->install_txt);
	}

	/* size of output record */
	*size = iip->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		numBytesRawSensorData = iip->header.numBytesDgm - MBSYS_KMBES_IIP_VAR_OFFSET;

		mb_put_binary_short(MB_YES, iip->numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, iip->info, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, iip->status, &buffer[index]);
		index += 2;
		memcpy(&iip->install_txt, &buffer[index], numBytesRawSensorData);
		index += numBytesRawSensorData;

		/* Insert closing byte count */
		mb_put_binary_int(MB_YES, iip->header.numBytesDgm, &buffer[index]);
		// index += 4;
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
};

int mbr_kemkmall_wr_iop(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
	char *function_name = "mbr_kemkmall_wr_iop";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;
	struct mbsys_kmbes_iop *iop;
	size_t numBytesRawSensorData;
	char *buffer;
	int index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	dgm_index = (struct mbsys_kmbes_index *)&store->dgm_index[store->dgm_count_id];
	iop = &(store->iop);

	/* print debug statements */
	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5       iop->header.numBytesDgm:                %u\n", iop->header.numBytesDgm);
		fprintf(stderr, "dbg5       iop->header.dgmType:                    %s\n", iop->header.dgmType);
		fprintf(stderr, "dbg5       iop->header.dgmVersion:                 %u\n", iop->header.dgmVersion);
		fprintf(stderr, "dbg5       iop->header.systemID:                   %u\n", iop->header.systemID);
		fprintf(stderr, "dbg5       iop->header.echoSounderID:              %u\n", iop->header.echoSounderID);
		fprintf(stderr, "dbg5       iop->header.time_sec:                   %u\n", iop->header.time_sec);
		fprintf(stderr, "dbg5       iop->header.time_nanosec:               %u\n", iop->header.time_nanosec);

		fprintf(stderr, "dbg5       iop->iop->numBytesCmnPart:              %u\n", iop->numBytesCmnPart);
		fprintf(stderr, "dbg5       iop->info:                              %u\n", iop->info);
		fprintf(stderr, "dbg5       iop->status:                            %u\n", iop->status);
		fprintf(stderr, "dbg5       iop->runtime_txt:                       %s\n", iop->runtime_txt);
	}

	/* size of output record */
	*size = iop->header.numBytesDgm;

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
		mbr_kemkmall_wr_header(verbose, bufferptr, store_ptr, error);

		/* insert the data */
		index = MBSYS_KMBES_HEADER_SIZE;

		numBytesRawSensorData = iop->header.numBytesDgm - MBSYS_KMBES_IOP_VAR_OFFSET;

		mb_put_binary_short(MB_YES, iop->numBytesCmnPart, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, iop->info, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, iop->status, &buffer[index]);
		index += 2;
		memcpy(&iop->runtime_txt,  &buffer[index], numBytesRawSensorData);
		index += numBytesRawSensorData;

		/* Insert closing byte count */
		mb_put_binary_int(MB_YES, iop->header.numBytesDgm, &buffer[index]);
		// index += 4;
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
};


int mbr_kemkmall_wr_unknown(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error);

/*--------------------------------------------------------------------*/