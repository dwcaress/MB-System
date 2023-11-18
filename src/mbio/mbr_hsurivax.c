/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsurivax.c	2/2/93
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
 * mbr_hsurivax.c contains the functions for reading and writing
 * multibeam data in the HSURICEN format.   The only difference between
 * the HSURIVAX format and the HSURICEN format is that the data files
 * are stored in VAX byte order ("little endian"). This is why the
 * format structure definitions are taken from the include file
 * mbf_hsuricen.h.
 * These functions include:
 *   mbr_alm_hsurivax	- allocate read/write memory
 *   mbr_dem_hsurivax	- deallocate read/write memory
 *   mbr_rt_hsurivax	- read and translate data
 *   mbr_wt_hsurivax	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mb_swap.h"
#include "mbf_hsuricen.h"
#include "mbsys_hsds.h"

/*--------------------------------------------------------------------*/
int mbr_info_hsurivax(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_HSDS;
	*beams_bath_max = 59;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "HSURIVAX", MB_NAME_LENGTH);
	strncpy(system_name, "HSDS", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_HSURIVAX\nInformal Description: URI Hydrosweep from VAX\nAttributes:           Hydrosweep "
	        "DS, 59 beams, bathymetry, binary,\n                      VAX byte order, URI.\n",
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
	*beamwidth_xtrack = 2.0;
	*beamwidth_ltrack = 2.0;

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
int mbr_alm_hsurivax(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_hsuricen_struct);
	mb_io_ptr->data_structure_size = sizeof(struct mbf_hsuricen_data_struct);
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status &= mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_hsds_struct), &mb_io_ptr->store_data, error);

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
int mbr_dem_hsurivax(int verbose, void *mbio_ptr, int *error) {
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
int mbr_rt_hsurivax(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_hsuricen_struct *dataplus = (struct mbf_hsuricen_struct *)mb_io_ptr->raw_data;
	struct mbf_hsuricen_data_struct *data = &(dataplus->data);
	char *datacomment = (char *)data;
	dataplus->kind = MB_DATA_DATA;
	struct mbsys_hsds_struct *store = (struct mbsys_hsds_struct *)store_ptr;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record from file */
	int status = MB_SUCCESS;
	const size_t num_bytes = fread(data, 1, mb_io_ptr->data_structure_size, mb_io_ptr->mbfp);
	if (num_bytes == mb_io_ptr->data_structure_size) {
		mb_io_ptr->file_bytes += status;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	else {
		mb_io_ptr->file_bytes += status;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

/* byte swap the data if necessary */
#ifndef BYTESWAPPED
	if (status == MB_SUCCESS && data->sec != 25443) {
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->hdg = mb_swap_short(data->hdg);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->pitch = mb_swap_short(data->pitch);
		data->scale = mb_swap_short(data->scale);
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
		}
	}
#endif

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS) {
		if (data->sec == 25443) {
			dataplus->kind = MB_DATA_COMMENT;
		}
		else if (data->year == 0) {
			dataplus->kind = MB_DATA_NONE;
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

	/* translate values to hydrosweep data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		/* type of data record */
		store->kind = dataplus->kind;

		/* time stamp (all records ) */
		int time_j[5];
		time_j[0] = data->year;
		time_j[1] = data->day;
		time_j[2] = data->min;
		time_j[3] = data->sec / 100;
		time_j[4] = 10000 * (data->sec - 100 * time_j[3]);
		mb_get_itime(verbose, time_j, mb_io_ptr->new_time_i);
		mb_get_time(verbose, mb_io_ptr->new_time_i, &(mb_io_ptr->new_time_d));
		store->year = mb_io_ptr->new_time_i[0];
		store->month = mb_io_ptr->new_time_i[1];
		store->day = mb_io_ptr->new_time_i[2];
		store->hour = mb_io_ptr->new_time_i[3];
		store->minute = mb_io_ptr->new_time_i[4];
		store->second = mb_io_ptr->new_time_i[5];
		store->alt_minute = 0;
		store->alt_second = 0;

		/* position (all records ) */
		store->lon = 0.0000001 * data->lon;
		store->lat = 0.0000001 * data->lat;
		if (store->lon > 180.)
			store->lon = store->lon - 360.;
		else if (store->lon < -180.)
			store->lon = store->lon + 360.;

		/* additional navigation and depths (ERGNMESS and ERGNEICH) */
		store->course_true = 0.1 * data->hdg;
		store->speed_transverse = 0.0;
		store->speed = 0.005092593 * data->speed;
		store->speed_reference[0] = data->speed_ref;
		store->pitch = 0.1 * data->pitch;
		store->track = 0;
		store->depth_scale = 0.01 * data->scale;
		store->depth_center = store->depth_scale * data->deph[MBSYS_HSDS_BEAMS / 2];
		store->spare = 1;
		/* const int id = MBSYS_HSDS_BEAMS - 1; */
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
			store->distance[i] = data->dist[i];
			store->depth[i] = data->deph[i];
		}
		/*		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
		            {
		            store->distance[id-i] = data->dist[i];
		            store->depth[id-i] = data->deph[i];
		            }*/

		/* travel time data (ERGNSLZT) */
		store->course_ground = 0.1 * data->course;
		store->speed_ground = 0.0;
		store->heave = 0.0;
		store->roll = 0.0;
		store->time_center = 0.0;
		store->time_scale = 0.0;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++)
			store->time[i] = 0;
		for (int i = 0; i < 11; i++)
			store->gyro[i] = 0.0;

		/* amplitude data (ERGNAMPL) */
		store->mode[0] = '\0';
		store->trans_strbd = 0;
		store->trans_vert = 0;
		store->trans_port = 0;
		store->pulse_len_strbd = 0;
		store->pulse_len_vert = 0;
		store->pulse_len_port = 0;
		store->gain_start = 0;
		store->r_compensation_factor = 0;
		store->compensation_start = 0;
		store->increase_start = 0;
		store->tvc_near = 0;
		store->tvc_far = 0;
		store->increase_int_near = 0;
		store->increase_int_far = 0;
		store->gain_center = 0;
		store->filter_gain = 0.0;
		store->amplitude_center = 0;
		store->echo_duration_center = 0;
		store->echo_scale_center = 0;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
			store->amplitude[i] = 0;
			store->echo_duration[i] = 0;
		}
		for (int i = 0; i < 16; i++) {
			store->gain[i] = 0;
			store->echo_scale[i] = 0;
		}

		/* mean velocity (ERGNHYDI) */
		store->draught = 0.0;
		store->vel_mean = 0.0;
		store->vel_keel = 0.0;
		store->tide = 0.0;

		/* water velocity profile (HS_ERGNCTDS) */
		store->num_vel = 0.0;

		/* navigation source (ERGNPOSI) */
		store->pos_corr_x = 0.0;
		store->pos_corr_y = 0.0;
		strncpy(store->sensors, "", 8);

		/* comment (LDEOCMNT) */
		strncpy(store->comment, &datacomment[2], MBSYS_HSDS_MAXLINE);

		/* processed backscatter */
		store->back_scale = 0.0;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++)
			store->back[i] = 0;
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
int mbr_wt_hsurivax(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_hsuricen_struct *dataplus = (struct mbf_hsuricen_struct *)mb_io_ptr->raw_data;
	struct mbf_hsuricen_data_struct *data = &(dataplus->data);
	char *datacomment = (char *)data;
	struct mbsys_hsds_struct *store = (struct mbsys_hsds_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Status at beginning of MBIO function <%s>\n", __func__);
		if (store != NULL)
			fprintf(stderr, "dbg5       store->kind:    %d\n", store->kind);
		fprintf(stderr, "dbg5       new_kind:       %d\n", mb_io_ptr->new_kind);
		fprintf(stderr, "dbg5       new_error:      %d\n", mb_io_ptr->new_error);
		fprintf(stderr, "dbg5       error:          %d\n", *error);
	}

	/* first set some plausible amounts for some of the
	    variables in the HSURICEN record */
	data->course = 0;
	data->pitch = 0;
	data->scale = 100;     /* this is a unit scale factor */
	data->speed_ref = 'B'; /* assume speed is over the ground */
	data->quality = 0;

	/* second translate values from hydrosweep data storage structure */
	if (store != NULL) {
		dataplus->kind = store->kind;
		if (store->kind == MB_DATA_DATA) {
			/* position */
			if (store->lon < -180.0)
				store->lon = store->lon + 360.0;
			if (store->lon > 180.0)
				store->lon = store->lon - 360.0;
			data->lon = (int)(0.5 + 10000000.0 * store->lon);
			data->lat = (int)(0.5 + 10000000.0 * store->lat);

			/* time stamp */
			int time_i[7];
			time_i[0] = store->year;
			time_i[1] = store->month;
			time_i[2] = store->day;
			time_i[3] = store->hour;
			time_i[4] = store->minute;
			time_i[5] = store->second;
			time_i[6] = 0;
			int time_j[5];
			mb_get_jtime(verbose, time_i, time_j);
			data->year = time_j[0];
			data->day = time_j[1];
			data->min = time_j[2];
			data->sec = 100 * time_j[3] + 0.0001 * time_j[4];

			/* additional navigation and depths */
			data->hdg = 10.0 * store->course_true;
			data->course = 10.0 * store->course_ground;
			data->speed = 196.36363636363 * store->speed;
			data->speed_ref = store->speed_reference[0];
			data->pitch = 10.0 * store->pitch;
			data->scale = 100 * store->depth_scale;
			/* const int id = MBSYS_HSDS_BEAMS - 1; */
			for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
				data->dist[i] = store->distance[i];
				data->deph[i] = store->depth[i];
			}
			/*			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			                {
			                data->dist[i] = store->distance[id-i];
			                data->deph[i] = store->depth[id-i];
			                }*/
		}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT) {
			strcpy(datacomment, "cc");
			strncat(datacomment, store->comment, MBSYS_HSDS_MAXLINE);
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Ready to write data in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:       %d\n", dataplus->kind);
		fprintf(stderr, "dbg5       error:      %d\n", *error);
	}

/* byte swap the data if necessary */
#ifndef BYTESWAPPED
	if (dataplus->kind == MB_DATA_DATA) {
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->hdg = mb_swap_short(data->hdg);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->pitch = mb_swap_short(data->pitch);
		data->scale = mb_swap_short(data->scale);
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
		}
	}
#endif

	int status = MB_SUCCESS;

	/* write next record to file */
	if (dataplus->kind == MB_DATA_DATA || dataplus->kind == MB_DATA_COMMENT) {
		const size_t num_bytes = fwrite(data, 1, mb_io_ptr->data_structure_size, mb_io_ptr->mbfp);
		if (num_bytes == mb_io_ptr->data_structure_size) {
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
int mbr_register_hsurivax(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_hsurivax(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hsurivax;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hsurivax;
	mb_io_ptr->mb_io_store_alloc = &mbsys_hsds_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_hsds_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hsurivax;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hsurivax;
	mb_io_ptr->mb_io_dimensions = &mbsys_hsds_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_hsds_extract;
	mb_io_ptr->mb_io_insert = &mbsys_hsds_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_hsds_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_hsds_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_hsds_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_hsds_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_hsds_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_hsds_copy;
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
