/*--------------------------------------------------------------------
 *    The MB-system:    mbr_swplssxp.c  5/6/2013
 *
 *    Copyright (c) 2013-2023 by
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
 * mbr_swplssxp.c contains the functions for reading and writing
 * interferometric sonar data in the MBF_SWPLSSXP format.
 * These functions include:
 *   mbr_alm_swplssxp   - allocate read/write memory
 *   mbr_dem_swplssxp   - deallocate read/write memory
 *   mbr_rt_swplssxp    - read and translate data
 *   mbr_wt_swplssxp    - translate and write data
 *
 * Author:  D. P. Finlayson and D. W. Caress
 * Date:    February 22, 2014
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_swathplus.h"

/* turn on debug statements here */
// #define MBF_SWPLSSXP_DEBUG 1

/*--------------------------------------------------------------------*/
int mbr_info_swplssxp(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_SWATHPLUS;
	*beams_bath_max = SWPLS_MAX_BEAMS;
	*beams_amp_max = SWPLS_MAX_BEAMS;
	*pixels_ss_max = SWPLS_MAX_PIXELS;
	strncpy(format_name, "SWPLSSXP", MB_NAME_LENGTH);
	strncpy(system_name, "SWATHPLUS", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_SWPLSSXP\nInformal Description: SEA interferometric sonar vendor processed data format\n"
	        "Attributes:           SEA SWATHplus,\n"
	        "                      bathymetry and amplitude,\n"
	        "                      variable beams, binary, SEA.\n",
	        MB_DESCRIPTION_LENGTH);
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
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = SWPLS_TYPE_M_BEAM_WIDTH;
	*beamwidth_ltrack = SWPLS_TYPE_M_BEAM_WIDTH;

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
		fprintf(stderr, "dbg2       attitude_source:    %d\n", *attitude_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", *svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", *beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", *beamwidth_ltrack);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
} /* mbr_info_swplssxp */
/*--------------------------------------------------------------------*/
int mbr_alm_swplssxp(int verbose, void *mbio_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	int status = mbsys_swathplus_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);
	int *recordid = (int *)&mb_io_ptr->save3;
	int *recordidlast = (int *)&mb_io_ptr->save4;
	char **bufferptr = (char **)&mb_io_ptr->saveptr1;
	int *bufferalloc = (int *)&mb_io_ptr->save6;
	int *size = (int *)&mb_io_ptr->save8;
	int *nbadrec = (int *)&mb_io_ptr->save9;
	int *header_rec_written = (int *)&(mb_io_ptr->save1);
	int *projection_rec_written = (int *)&(mb_io_ptr->save2);

	*recordid = SWPLS_ID_NONE;
	*recordidlast = SWPLS_ID_NONE;
	*bufferptr = NULL;
	*bufferalloc = 0;
	*size = 0;
	*nbadrec = 0;
	*header_rec_written = false;
	*projection_rec_written = false;

	/* allocate memory if necessary */
	if (status == MB_SUCCESS) {
		status = mb_reallocd(verbose, __FILE__, __LINE__, SWPLS_BUFFER_STARTSIZE, (void **)bufferptr, error);
		if (status == MB_SUCCESS) {
			*bufferalloc = SWPLS_BUFFER_STARTSIZE;
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
} /* mbr_alm_swplssxp */
/*--------------------------------------------------------------------*/
int mbr_dem_swplssxp(int verbose, void *mbio_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	int status = mbsys_swathplus_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* deallocate memory for reading/writing buffer */
	char **bufferptr = (char **)&mb_io_ptr->saveptr1;
	int *bufferalloc = (int *)&mb_io_ptr->save6;
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)bufferptr, error);
	*bufferalloc = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:   %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
} /* mbr_dem_swplssxp */
/*--------------------------------------------------------------------*/
int mbr_swplssxp_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbsys_swathplus_struct *store = (struct mbsys_swathplus_struct *)store_ptr;

	/* get saved values */
	int *recordid = (int *)&mb_io_ptr->save3;
	int *recordidlast = (int *)&mb_io_ptr->save4;
	char **bufferptr = (char **)&mb_io_ptr->saveptr1;
	char *buffer = (char *)*bufferptr;
	int *bufferalloc = (int *)&mb_io_ptr->save6;
	int *size = (int *)&mb_io_ptr->save8;
	int *nbadrec = (int *)&mb_io_ptr->save9;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* loop over reading data until a record is ready for return */
	int status = MB_SUCCESS;
	bool done = false;
	*error = MB_ERROR_NO_ERROR;
	while (!done) {
		/* read next record header into buffer */
		size_t read_len = (size_t)SWPLS_SIZE_BLOCKHEADER;
		status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);

		/* check header - if not a good header read a byte
		    at a time until a good header is found */
		int skip = 0;
		while (status == MB_SUCCESS && swpls_chk_header(verbose, mbio_ptr, buffer, recordid, size, error) != MB_SUCCESS) {
			/* get next byte */
			for (int i = 0; i < SWPLS_SIZE_BLOCKHEADER - 1; i++) {
				buffer[i] = buffer[i + 1];
			}
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, &buffer[SWPLS_SIZE_BLOCKHEADER - 1], &read_len, error);
			skip++;
		}

		/* report problem */
		if ((skip > 0) && (verbose >= 0)) {
			if (*nbadrec == 0) {
				fprintf(stderr, "The MBR_SWPLSSXP module skipped data between identified\n"
				                "data records. Something is broken, most probably the data...\n"
				                "However, the data may include a data record type that we\n"
				                "haven't seen yet, or there could be an error in the code.\n"
				                "If skipped data are reported multiple times, \n"
				                "we recommend you send a data sample and problem \n"
				                "description to the MB-System team \n"
				                "(caress@mbari.org and dale@ldeo.columbia.edu)\n"
				                "Have a nice day...\n");
			}
			fprintf(stderr, "MBR_SWPLSSXP skipped %d bytes between records %4.4X:%d and %4.4X:%d\n", skip, *recordidlast,
			        *recordidlast, *recordid, *recordid);
			(*nbadrec)++;
		}

		*recordidlast = *recordid;
		store->type = *recordid;

		/* allocate memory to read rest of record if necessary */
		if (*bufferalloc < *size + SWPLS_SIZE_BLOCKHEADER) {
			status = mb_reallocd(verbose, __FILE__, __LINE__, *size + SWPLS_SIZE_BLOCKHEADER, (void **)bufferptr, error);
			if (status != MB_SUCCESS) {
				*bufferalloc = 0;
				done = true;
			}
			else {
				*bufferalloc = *size + SWPLS_SIZE_BLOCKHEADER;
				buffer = (char *)*bufferptr;
			}
		}

		/* read the rest of the record */
		if (status == MB_SUCCESS) {
			read_len = (size_t)*size;
			status = mb_fileio_get(verbose, mbio_ptr, &buffer[SWPLS_SIZE_BLOCKHEADER], &read_len, error);
		}

		/* parse the data record */
		if (status == MB_SUCCESS && !done) {
			if (*recordid == SWPLS_ID_SXP_HEADER_DATA) {
				status = swpls_rd_sxpheader(verbose, buffer, store_ptr, error);
				done = true;
			}
			else if (*recordid == SWPLS_ID_PROCESSED_PING) {
				status = swpls_rd_sxpping(verbose, buffer, store_ptr, SWPLS_ID_PROCESSED_PING, error);
				done = true;
			}
			else if (*recordid == SWPLS_ID_PROCESSED_PING2) {
				status = swpls_rd_sxpping(verbose, buffer, store_ptr, SWPLS_ID_PROCESSED_PING2, error);
				done = true;
			}
			else if (*recordid == SWPLS_ID_COMMENT) {
				status = swpls_rd_comment(verbose, buffer, store_ptr, error);
				done = true;
			}
			else if (*recordid == SWPLS_ID_PROJECTION) {
				status = swpls_rd_projection(verbose, buffer, store_ptr, error);
				done = true;
			}
			else {
				done = false;
			}
		}

		if (status == MB_FAILURE) {
			done = true;
		}
	}

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
} /* mbr_swplssxp_rd_data */
/*--------------------------------------------------------------------*/
int mbr_rt_swplssxp(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* read next data from file */
	int status = mbr_swplssxp_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* get pointer to data structure */
	struct mbsys_swathplus_struct *store = (struct mbsys_swathplus_struct *)store_ptr;
	swpls_projection *projection = &store->projection;

	/* check if projection has been set from *.prj file, if so, copy into projection structure */
	if (!store->projection_set && mb_io_ptr->projection_initialized) {
		projection->time_d = time(NULL);
		projection->microsec = 0;
		projection->nchars = strnlen(mb_io_ptr->projection_id, MB_NAME_LENGTH);
		if (projection->projection_alloc < projection->nchars) {
			status = mb_reallocd(verbose, __FILE__, __LINE__, (size_t)projection->nchars, (void **)&(projection->projection_id),
			                     error);
			if (status != MB_SUCCESS) {
				projection->projection_alloc = 0;
			}
			else {
				projection->projection_alloc = projection->nchars;
			}
		}

		if (status == MB_SUCCESS) {
			strncpy(projection->projection_id, mb_io_ptr->projection_id, (size_t)projection->nchars);
			store->projection_set = true;
		}
	}
	/* check if projection has been read from *mb222 file, if so, tell mb system */
	else if (store->projection_set && !mb_io_ptr->projection_initialized) {
		mb_proj_init(verbose, projection->projection_id, &(mb_io_ptr->pjptr), error);
		strncpy(mb_io_ptr->projection_id, projection->projection_id, MB_NAME_LENGTH);
		mb_io_ptr->projection_initialized = true;
	}

	/* throw away data if the time stamp makes no sense */
	if ((status == MB_SUCCESS) && (store->kind == MB_DATA_DATA) && (store->time_i[0] < 2003)) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* save fix data, used to calculate vessel speed */
	if ((status == MB_SUCCESS) && (store->kind == MB_DATA_DATA)) {
		/* add latest fix */
		mb_navint_add(verbose, mbio_ptr, store->time_d, store->sxp_ping.txer_e, store->sxp_ping.txer_n, error);
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
} /* mbr_rt_swplssxp */
/*--------------------------------------------------------------------*/
int mbr_wt_swplssxp(int verbose, void *mbio_ptr, void *store_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbsys_swathplus_struct *store = (struct mbsys_swathplus_struct *)store_ptr;

	/* get pointers to saved data */
	int *header_rec_written = &(mb_io_ptr->save1);
	int *projection_rec_written = &(mb_io_ptr->save2);

	int status = MB_SUCCESS;

	/* write header record if needed (just once, here at top of file) */
	if (store->sxp_header_set && !*header_rec_written) {
		const int origkind = store->kind;
		const int origtype = store->type;
		store->kind = MB_DATA_HEADER;
		store->type = SWPLS_ID_SXP_HEADER_DATA;
		status = swpls_wr_data(verbose, mbio_ptr, store_ptr, error);
		if (status == MB_SUCCESS) {
			*header_rec_written = true;
		}
		store->kind = origkind;
		store->type = origtype;
	}

	/* write projection record if needed (just once, here at top of file) */
	if (store->projection_set && !*projection_rec_written) {
		const int origkind = store->kind;
		const int origtype = store->type;
		store->kind = MB_DATA_PARAMETER;
		store->type = SWPLS_ID_PROJECTION;
		status = swpls_wr_data(verbose, mbio_ptr, store_ptr, error);
		if (status == MB_SUCCESS) {
			*projection_rec_written = true;
		}
		store->kind = origkind;
		store->type = origtype;
	}

	/* write the record to file EXCEPT headers and projections */
	if ((store->type != SWPLS_ID_SXP_HEADER_DATA) && (store->type != SWPLS_ID_PROJECTION)) {
		status = swpls_wr_data(verbose, mbio_ptr, store_ptr, error);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
} /* mbr_wt_swplssxp */

/*--------------------------------------------------------------------*/
int mbr_register_swplssxp(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_swplssxp(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_swplssxp;
	mb_io_ptr->mb_io_format_free = &mbr_dem_swplssxp;
	mb_io_ptr->mb_io_store_alloc = &mbsys_swathplus_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_swathplus_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_swplssxp;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_swplssxp;
	mb_io_ptr->mb_io_dimensions = &mbsys_swathplus_dimensions;
	mb_io_ptr->mb_io_pingnumber = &mbsys_swathplus_pingnumber;
	mb_io_ptr->mb_io_sonartype = &mbsys_swathplus_sonartype;
	mb_io_ptr->mb_io_sidescantype = &mbsys_swathplus_sidescantype;
	mb_io_ptr->mb_io_extract = &mbsys_swathplus_extract;
	mb_io_ptr->mb_io_insert = &mbsys_swathplus_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_swathplus_extract_nav;
	mb_io_ptr->mb_io_extract_nnav = NULL;
	mb_io_ptr->mb_io_insert_nav = &mbsys_swathplus_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_swathplus_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_swathplus_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_swathplus_detects;
	mb_io_ptr->mb_io_gains = &mbsys_swathplus_gains;
	mb_io_ptr->mb_io_copyrecord = &mbsys_swathplus_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;
	mb_io_ptr->mb_io_pulses = NULL;
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
		fprintf(stderr, "dbg2       heading_source:     %d\n", mb_io_ptr->heading_source);
		fprintf(stderr, "dbg2       attitude_source:         %d\n", mb_io_ptr->attitude_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", mb_io_ptr->svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr, "dbg2       format_alloc:       %p\n", mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr, "dbg2       format_free:        %p\n", mb_io_ptr->mb_io_format_free);
		fprintf(stderr, "dbg2       store_alloc:        %p\n", mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr, "dbg2       store_free:         %p\n", mb_io_ptr->mb_io_store_free);
		fprintf(stderr, "dbg2       read_ping:          %p\n", mb_io_ptr->mb_io_read_ping);
		fprintf(stderr, "dbg2       write_ping:         %p\n", mb_io_ptr->mb_io_write_ping);
		fprintf(stderr, "dbg2       extract:            %p\n", mb_io_ptr->mb_io_extract);
		fprintf(stderr, "dbg2       insert:             %p\n", mb_io_ptr->mb_io_insert);
		fprintf(stderr, "dbg2       extract_nav:        %p\n", mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr, "dbg2       insert_nav:         %p\n", mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr, "dbg2       extract_altitude:   %p\n", mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr, "dbg2       insert_altitude:    %p\n", mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr, "dbg2       extract_svp:        %p\n", mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr, "dbg2       insert_svp:         %p\n", mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr, "dbg2       ttimes:             %p\n", mb_io_ptr->mb_io_ttimes);
		fprintf(stderr, "dbg2       detects:            %p\n", mb_io_ptr->mb_io_detects);
		fprintf(stderr, "dbg2       pulses:             %p\n", mb_io_ptr->mb_io_pulses);
		fprintf(stderr, "dbg2       extract_rawss:      %p\n", mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr, "dbg2       insert_rawss:       %p\n", mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr, "dbg2       extract_segytraceheader: %p\n", mb_io_ptr->mb_io_extract_segytraceheader);
		fprintf(stderr, "dbg2       extract_segy:       %p\n", mb_io_ptr->mb_io_extract_segy);
		fprintf(stderr, "dbg2       insert_segy:        %p\n", mb_io_ptr->mb_io_insert_segy);
		fprintf(stderr, "dbg2       copyrecord:         %p\n", mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}
