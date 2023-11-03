/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sburicen.c	2/2/93
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
 * mbr_sburicen.c contains the functions for reading and writing
 * multibeam data in the SBURICEN format.
 * These functions include:
 *   mbr_alm_sburicen	- allocate read/write memory
 *   mbr_dem_sburicen	- deallocate read/write memory
 *   mbr_rt_sburicen	- read and translate data
 *   mbr_wt_sburicen	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
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
#include "mbf_sburicen.h"
#include "mbsys_sb.h"

/*--------------------------------------------------------------------*/
int mbr_info_sburicen(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_SB;
	*beams_bath_max = 19;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "SBURICEN", MB_NAME_LENGTH);
	strncpy(system_name, "SB", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_SBURICEN\nInformal Description: URI Sea Beam\nAttributes:           Sea Beam, bathymetry, "
	        "19 beams, binary, centered,\n                      URI.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = false;
	*traveltime = false;
	*beam_flagging = true;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 2.67;
	*beamwidth_ltrack = 2.67;

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
int mbr_alm_sburicen(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_sburicen_struct);
	mb_io_ptr->data_structure_size = sizeof(struct mbf_sburicen_data_struct);
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status &= mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_sb_struct), &mb_io_ptr->store_data, error);

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
int mbr_dem_sburicen(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->raw_data, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->store_data, error);

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
int mbr_rt_sburicen(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_sburicen_struct *dataplus = (struct mbf_sburicen_struct *)mb_io_ptr->raw_data;
	struct mbf_sburicen_data_struct *data = &(dataplus->data);
	char *datacomment = (char *)data;
	dataplus->kind = MB_DATA_DATA;
	struct mbsys_sb_struct *store = (struct mbsys_sb_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* read next record from file */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
  size_t read_len = 0;
	if ((read_len = fread(data, 1, mb_io_ptr->data_structure_size, mb_io_ptr->mbfp)) == mb_io_ptr->data_structure_size) {
		mb_io_ptr->file_bytes += read_len;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	else {
		mb_io_ptr->file_bytes += read_len;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS) {
		for (int i = 0; i < MBSYS_SB_BEAMS; i++) {
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
		}
		data->axis = mb_swap_short(data->axis);
		data->major = mb_swap_short(data->major);
		data->minor = mb_swap_short(data->minor);
		data->sbhdg = mb_swap_short(data->sbhdg);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lon2b = mb_swap_short(data->lon2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->sbtim = mb_swap_short(data->sbtim);
	}
#endif

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS) {
		if (data->deph[0] > 15000) {
			dataplus->kind = MB_DATA_COMMENT;
		}
		else if (data->year == 0) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
		else {
			dataplus->kind = MB_DATA_DATA;
		}
	}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to seabeam data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		/* type of data record */
		store->kind = dataplus->kind;

		if (store->kind == MB_DATA_DATA) {
			/* position */
			store->lon2u = data->lon2u;
			store->lon2b = data->lon2b;
			store->lat2u = data->lat2u;
			store->lat2b = data->lat2b;

			/* time stamp */
			store->year = data->year;
			store->day = data->day;
			store->min = data->min;
			store->sec = data->sec;

			/* depths and distances */
			/* switch order of data as it is read into the global arrays */
			const int id = MBSYS_SB_BEAMS - 1;
			for (int i = 0; i < MBSYS_SB_BEAMS; i++) {
				store->dist[id - i] = data->dist[i];
				store->deph[id - i] = data->deph[i];
			}

			/* additional values */
			store->sbtim = data->sbtim;
			store->sbhdg = data->sbhdg;
			store->axis = data->axis;
			store->major = data->major;
			store->minor = data->minor;
		}

		else if (store->kind == MB_DATA_COMMENT) {
			/* comment */
			strncpy(store->comment, &datacomment[2], MBSYS_SB_MAXLINE);
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
int mbr_wt_sburicen(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	/* get pointer to raw data structure */
	struct mbf_sburicen_struct *dataplus = (struct mbf_sburicen_struct *)mb_io_ptr->raw_data;
	struct mbf_sburicen_data_struct *data = &(dataplus->data);
	char *datacomment = (char *)data;
	struct mbsys_sb_struct *store = (struct mbsys_sb_struct *)store_ptr;

	/* first set some plausible amounts for some of the
	    variables in the MBURICEN record */
	data->sbtim = 0;
	data->axis = 0;
	data->major = 0;
	data->minor = 0;

	/* translate values from seabeam data storage structure */
	dataplus->kind = store->kind;
	if (store->kind == MB_DATA_DATA) {
		/* position */
		data->lon2u = store->lon2u;
		data->lon2b = store->lon2b;
		data->lat2u = store->lat2u;
		data->lat2b = store->lat2b;

		/* time stamp */
		data->year = store->year;
		data->day = store->day;
		data->min = store->min;
		data->sec = store->sec;

		/* depths and distances */
		/* switch order of data as it is read
		    into the output arrays */
		const int id = MBSYS_SB_BEAMS - 1;
		for (int i = 0; i < MBSYS_SB_BEAMS; i++) {
			data->dist[i] = store->dist[id - i];
			data->deph[i] = store->deph[id - i];
		}

		/* additional values */
		data->sbtim = store->sbtim;
		data->sbhdg = store->sbhdg;
		data->axis = store->axis;
		data->major = store->major;
		data->minor = store->minor;
	}

	/* comment */
	else if (store->kind == MB_DATA_COMMENT) {
		strcpy(datacomment, "cc");
		strncat(datacomment, store->comment, mb_io_ptr->data_structure_size - 3);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Ready to write data in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:       %d\n", dataplus->kind);
		fprintf(stderr, "dbg5       error:      %d\n", *error);
		fprintf(stderr, "dbg5       status:     %d\n", status);
	}

/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (dataplus->kind == MB_DATA_DATA || dataplus->kind == MB_DATA_COMMENT) {
		for (int i = 0; i < MBSYS_SB_BEAMS; i++) {
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
		}
		data->axis = mb_swap_short(data->axis);
		data->major = mb_swap_short(data->major);
		data->minor = mb_swap_short(data->minor);
		data->sbhdg = mb_swap_short(data->sbhdg);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lon2b = mb_swap_short(data->lon2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->sbtim = mb_swap_short(data->sbtim);
	}
#endif

	/* write next record to file */
	if (dataplus->kind == MB_DATA_DATA || dataplus->kind == MB_DATA_COMMENT) {
    size_t write_len = 0;
		if ((write_len = fwrite(data, 1, mb_io_ptr->data_structure_size, mb_io_ptr->mbfp)) == mb_io_ptr->data_structure_size) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}
	else {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (verbose >= 5)
			fprintf(stderr, "\ndbg5  No data written in MBIO function <%s>\n", __func__);
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
int mbr_register_sburicen(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_sburicen(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sburicen;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sburicen;
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_sb_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sburicen;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sburicen;
	mb_io_ptr->mb_io_dimensions = &mbsys_sb_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_sb_extract;
	mb_io_ptr->mb_io_insert = &mbsys_sb_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_sb_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_sb_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb_copy;
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
