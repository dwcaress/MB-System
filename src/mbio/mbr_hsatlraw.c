/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsatlraw.c	2/11/93
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
 * mbr_hsatlraw.c contains the functions for reading and writing
 * multibeam data in the HSATLRAW format.
 * These functions include:
 *   mbr_alm_hsatlraw	- allocate read/write memory
 *   mbr_dem_hsatlraw	- deallocate read/write memory
 *   mbr_rt_hsatlraw	- read and translate data
 *   mbr_wt_hsatlraw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 11, 1993
 *
 */

#include "mbf_hsatlraw.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_hsds.h"

static const int ZERO_ALL = 0;
static const int ZERO_SOME = 1;

/*--------------------------------------------------------------------*/
int mbr_register_hsatlraw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_hsatlraw(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hsatlraw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hsatlraw;
	mb_io_ptr->mb_io_store_alloc = &mbsys_hsds_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_hsds_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hsatlraw;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hsatlraw;
	mb_io_ptr->mb_io_dimensions = &mbsys_hsds_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_hsds_extract;
	mb_io_ptr->mb_io_insert = &mbsys_hsds_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_hsds_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_hsds_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_hsds_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_hsds_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_hsds_insert_svp;
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
int mbr_info_hsatlraw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*beams_amp_max = 59;
	*pixels_ss_max = 0;
	strncpy(format_name, "HSATLRAW", MB_NAME_LENGTH);
	strncpy(system_name, "HSDS", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_HSATLRAW\nInformal Description: Raw Hydrosweep\nAttributes:           Hydrosweep DS, "
	        "bathymetry and amplitude, 59 beams,\n                      ascii, Atlas Electronik.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = false;
	*traveltime = true;
	*beam_flagging = false;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
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
int mbr_alm_hsatlraw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_hsatlraw_struct);
	mb_io_ptr->data_structure_size = 0;
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status &= mbsys_hsds_alloc(verbose, mbio_ptr, (void **)(&mb_io_ptr->store_data), error);
	status &= mb_mallocd(verbose, __FILE__, __LINE__, MBF_HSATLRAW_MAXLINE, &mb_io_ptr->saveptr1, error);

	/* get pointer to mbio descriptor */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)mb_io_ptr->raw_data;
	char *data_ptr = (char *)data;

	/* initialize everything to zeros */
	mbr_zero_hsatlraw(verbose, data_ptr, ZERO_ALL, error);

	char *raw_line = (char *) mb_io_ptr->saveptr1;
	bool *line_save_flag = &mb_io_ptr->saveb1;
	int *type = &mb_io_ptr->save1;
	int *shift = &mb_io_ptr->save2;
  memset((char *)raw_line, 0, MBF_HSATLRAW_MAXLINE);
	*line_save_flag = false;
	*type = MBF_HSATLRAW_NONE;
	*shift = 0;

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
int mbr_dem_hsatlraw(int verbose, void *mbio_ptr, int *error) {
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
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->saveptr1, error);

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
int mbr_zero_hsatlraw(int verbose, void *data_ptr, int mode, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
		fprintf(stderr, "dbg2       mode:       %d\n", mode);
	}

	/* get pointer to data descriptor */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	/* initialize everything to zeros */
	if (data != NULL) {
		/* type of data record */
		data->kind = MB_DATA_NONE;

		/* position (all records ) */
		data->lon = 0.0;
		data->lat = 0.0;

		/* time stamp (all records ) */
		data->year = 0;
		data->month = 0;
		data->day = 0;
		data->hour = 0;
		data->minute = 0;
		data->second = 0;
		data->alt_minute = 0;
		data->alt_second = 0;

		/* additional navigation and depths (ERGNMESS and ERGNEICH) */
		data->course_true = 0.0;
		data->speed_transverse = 0.0;
		data->speed = 0.0;
		data->speed_reference[0] = '\0';
		data->pitch = 0.0;
		data->track = 0;
		data->depth_center = 0.0;
		data->depth_scale = 0.0;
		data->spare = 0;
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++) {
			data->distance[i] = 0;
			data->depth[i] = 0;
		}

		/* travel time data (ERGNSLZT) */
		data->course_ground = 0.0;
		data->speed_ground = 0.0;
		data->heave = 0.0;
		data->roll = 0.0;
		data->time_center = 0.0;
		data->time_scale = 0.0;
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
			data->time[i] = 0;
		for (int i = 0; i < 11; i++)
			data->gyro[i] = 0.0;

		/* amplitude data (ERGNAMPL) */
		data->mode[0] = '\0';
		data->trans_strbd = 0;
		data->trans_vert = 0;
		data->trans_port = 0;
		data->pulse_len_strbd = 0;
		data->pulse_len_vert = 0;
		data->pulse_len_port = 0;
		data->gain_start = 0;
		data->r_compensation_factor = 0;
		data->compensation_start = 0;
		data->increase_start = 0;
		data->tvc_near = 0;
		data->tvc_far = 0;
		data->increase_int_near = 0;
		data->increase_int_far = 0;
		data->gain_center = 0;
		data->filter_gain = 0.0;
		data->amplitude_center = 0;
		data->echo_duration_center = 0;
		data->echo_scale_center = 0;
		for (int i = 0; i < 16; i++) {
			data->gain[i] = 0;
			data->echo_scale[i] = 0;
		}
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++) {
			data->amplitude[i] = 0;
			data->echo_duration[i] = 0;
		}

		/* these values zeroed only when structure
		    is first allocated - this allows
		    these values to be remembered internally
		    once one of these occasional data
		    records is encountered */
		if (mode == ZERO_ALL) {
			/* mean velocity (ERGNHYDI) */
			data->draught = 0.0;
			data->vel_mean = 0.0;
			data->vel_keel = 0.0;
			data->tide = 0.0;

			/* water velocity profile */
			data->num_vel = 0;
			for (int i = 0; i < MBF_HSATLRAW_MAXVEL; i++) {
				data->depth[i] = 0;
				data->velocity[i] = 0;
			}

			/* navigation source (ERGNPOSI) */
			data->pos_corr_x = 0.0;
			data->pos_corr_y = 0.0;
			strncpy(data->sensors, "POS", 9);
		}

		/* comment (LDEOCOMM) */
		strncpy(data->comment,  "", MBF_HSATLRAW_MAXLINE);
	}

	/* assume success */
	const int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

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
int mbr_rt_hsatlraw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	double xx, rr, zz, tt;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)mb_io_ptr->raw_data;
	struct mbsys_hsds_struct *store = (struct mbsys_hsds_struct *)store_ptr;

	/* read next data from file */
	const int status = mbr_hsatlraw_rd_data(verbose, mbio_ptr, error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate values to hydrosweep data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		/* type of data record */
		store->kind = data->kind;

		/* position (all records ) */
		store->lon = data->lon;
		store->lat = data->lat;

		/* time stamp (all records ) */
		store->year = data->year;
		store->month = data->month;
		store->day = data->day;
		store->hour = data->hour;
		store->minute = data->minute;
		store->second = data->second;
		store->alt_minute = data->alt_minute;
		store->alt_second = data->alt_second;

		/* additional navigation and depths (ERGNMESS and ERGNEICH) */
		store->course_true = data->course_true;
		store->speed_transverse = data->speed_transverse;
		store->speed = data->speed;
		store->speed_reference[0] = data->speed_reference[0];
		store->pitch = data->pitch;
		store->track = data->track;
		store->depth_center = data->depth_center;
		store->depth_scale = data->depth_scale;
		store->spare = data->spare;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
			store->distance[i] = data->distance[i];
			store->depth[i] = data->depth[i];
		}

		/* travel time data (ERGNSLZT) */
		store->course_ground = data->course_ground;
		store->speed_ground = data->speed_ground;
		store->heave = data->heave;
		store->roll = data->roll;
		store->time_center = data->time_center;
		store->time_scale = data->time_scale;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++)
			store->time[i] = data->time[i];
		for (int i = 0; i < 11; i++)
			store->gyro[i] = data->gyro[i];

		/* amplitude data (ERGNAMPL) */
		store->mode[0] = data->mode[0];
		store->trans_strbd = data->trans_strbd;
		store->trans_vert = data->trans_vert;
		store->trans_port = data->trans_port;
		store->pulse_len_strbd = data->pulse_len_strbd;
		store->pulse_len_vert = data->pulse_len_vert;
		store->pulse_len_port = data->pulse_len_port;
		store->gain_start = data->gain_start;
		store->r_compensation_factor = data->r_compensation_factor;
		store->compensation_start = data->compensation_start;
		store->increase_start = data->increase_start;
		store->tvc_near = data->tvc_near;
		store->tvc_far = data->tvc_far;
		store->increase_int_near = data->increase_int_near;
		store->increase_int_far = data->increase_int_far;
		store->gain_center = data->gain_center;
		store->filter_gain = data->filter_gain;
		store->amplitude_center = data->amplitude_center;
		store->echo_duration_center = data->echo_duration_center;
		store->echo_scale_center = data->echo_scale_center;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
			store->amplitude[i] = data->amplitude[i];
			store->echo_duration[i] = data->echo_duration[i];
		}
		for (int i = 0; i < 16; i++) {
			store->gain[i] = data->gain[i];
			store->echo_scale[i] = data->echo_scale[i];
		}

		/* mean velocity (ERGNHYDI) */
		store->draught = data->draught;
		store->vel_mean = data->vel_mean;
		store->vel_keel = data->vel_keel;
		store->tide = data->tide;

		/* water velocity profile (HS_ERGNCTDS) */
		store->num_vel = data->num_vel;
		for (int i = 0; i < data->num_vel; i++) {
			store->vdepth[i] = data->vdepth[i];
			store->velocity[i] = data->velocity[i];
		}

		/* navigation source (ERGNPOSI) */
		store->pos_corr_x = data->pos_corr_x;
		store->pos_corr_y = data->pos_corr_y;
		strncpy(store->sensors, data->sensors, 8);

		/* comment (LDEOCMNT) */
		strncpy(store->comment, data->comment, MBSYS_HSDS_MAXLINE);

		/* processed backscatter */
		store->back_scale = 1.0;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
			store->back[i] = mb_io_ptr->new_amp[i];
		}

		/* deal with missing travel times if needed */
		if (store->kind == MB_DATA_DATA) {
			if (store->vel_mean <= 0.0)
				store->vel_mean = 1500.0;
			if (store->vel_keel <= 0.0)
				store->vel_keel = 1500.0;
			if (store->time_scale == 0)
				store->time_scale = 0.01;
			if (store->time_center <= 0.0 && store->depth_center != 0.0) {
				rr = fabs(store->depth_center) + store->draught + store->heave;
				store->time_center = rr / store->vel_mean;
			}
			for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
				if (data->time[i] <= 0 && store->depth[i] != 0) {
					zz = store->depth_scale * (fabs((double)store->depth[i]) + store->draught + store->heave);
					xx = store->depth_scale * store->distance[i];
					rr = sqrt(xx * xx + zz * zz);
					tt = 2 * rr / store->vel_mean;
					store->time[i] = tt / store->time_scale;
				}
			}
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
int mbr_wt_hsatlraw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)mb_io_ptr->raw_data;
	char *data_ptr = (char *)data;
	struct mbsys_hsds_struct *store = (struct mbsys_hsds_struct *)store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL) {
		/* type of data record */
		data->kind = store->kind;

		/* position (all records ) */
		data->lon = store->lon;
		data->lat = store->lat;

		/* time stamp (all records ) */
		data->year = store->year;
		data->month = store->month;
		data->day = store->day;
		data->hour = store->hour;
		data->minute = store->minute;
		data->second = store->second;
		data->alt_minute = store->alt_minute;
		data->alt_second = store->alt_second;

		/* additional navigation and depths (ERGNMESS and ERGNEICH) */
		data->course_true = store->course_true;
		data->speed_transverse = store->speed_transverse;
		data->speed = store->speed;
		data->speed_reference[0] = store->speed_reference[0];
		data->pitch = store->pitch;
		data->track = store->track;
		data->depth_center = store->depth_center;
		data->depth_scale = store->depth_scale;
		data->spare = store->spare;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
			data->distance[i] = store->distance[i];
			data->depth[i] = store->depth[i];
		}

		/* travel time data (ERGNSLZT) */
		data->course_ground = store->course_ground;
		data->speed_ground = store->speed_ground;
		data->heave = store->heave;
		data->roll = store->roll;
		data->time_center = store->time_center;
		data->time_scale = store->time_scale;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++)
			data->time[i] = store->time[i];
		for (int i = 0; i < 11; i++)
			data->gyro[i] = store->gyro[i];

		/* amplitude data (ERGNAMPL) */
		data->mode[0] = store->mode[0];
		data->trans_strbd = store->trans_strbd;
		data->trans_vert = store->trans_vert;
		data->trans_port = store->trans_port;
		data->pulse_len_strbd = store->pulse_len_strbd;
		data->pulse_len_vert = store->pulse_len_vert;
		data->pulse_len_port = store->pulse_len_port;
		data->gain_start = store->gain_start;
		data->r_compensation_factor = store->r_compensation_factor;
		data->compensation_start = store->compensation_start;
		data->increase_start = store->increase_start;
		data->tvc_near = store->tvc_near;
		data->tvc_far = store->tvc_far;
		data->increase_int_near = store->increase_int_near;
		data->increase_int_far = store->increase_int_far;
		data->gain_center = store->gain_center;
		data->filter_gain = store->filter_gain;
		data->amplitude_center = store->amplitude_center;
		data->echo_duration_center = store->echo_duration_center;
		data->echo_scale_center = store->echo_scale_center;
		for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
			data->amplitude[i] = store->amplitude[i];
			data->echo_duration[i] = store->echo_duration[i];
		}
		for (int i = 0; i < 16; i++) {
			data->gain[i] = store->gain[i];
			data->echo_scale[i] = store->echo_scale[i];
		}

		/* mean velocity (ERGNHYDI) */
		data->draught = store->draught;
		data->vel_mean = store->vel_mean;
		data->vel_keel = store->vel_keel;
		data->tide = store->tide;

		/* water velocity profile (HS_ERGNCTDS) */
		data->num_vel = store->num_vel;
		for (int i = 0; i < store->num_vel; i++) {
			data->vdepth[i] = store->vdepth[i];
			data->velocity[i] = store->velocity[i];
		}

		/* navigation source (ERGNPOSI) */
		data->pos_corr_x = store->pos_corr_x;
		data->pos_corr_y = store->pos_corr_y;
		strncpy(data->sensors, store->sensors, 8);

		/* comment (LDEOCMNT) */
		strncpy(data->comment, store->comment, MBSYS_HSDS_MAXLINE);
	}

	/* check that no bathymetry values are negative */
	for (int i = 0; i < MBSYS_HSDS_BEAMS; i++) {
		if (data->depth[i] < 0)
			data->depth[i] = 0;
	}

	/* write next data to file */
	const int status = mbr_hsatlraw_wr_data(verbose, mbio_ptr, data_ptr, error);

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
int mbr_hsatlraw_rd_data(int verbose, void *mbio_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)mb_io_ptr->raw_data;
	char *data_ptr = (char *)data;
	FILE *mbfp = mb_io_ptr->mbfp;
	char *raw_line = (char *) mb_io_ptr->saveptr1;
	bool *line_save_flag = &mb_io_ptr->saveb1;
	int *type = &mb_io_ptr->save1;
	int *shift = &mb_io_ptr->save2;

	/* initialize everything to zeros */
	mbr_zero_hsatlraw(verbose, data_ptr, ZERO_SOME, error);

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	int status = MB_SUCCESS;
	int expect = MBF_HSATLRAW_NONE;
	bool done = false;
	while (!done) {

		/* get next record label */
		if (!(*line_save_flag)) {
			mb_io_ptr->file_bytes = ftell(mbfp);
			status = mbr_hsatlraw_rd_label(verbose, mbfp, raw_line, type, shift, error);
		}
		else
			*line_save_flag = false;

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == MBF_HSATLRAW_NONE) {
			mb_io_ptr->file_bytes = ftell(mbfp);
			done = true;
		}
		else if (status == MB_FAILURE && expect != MBF_HSATLRAW_NONE) {
			mb_io_ptr->file_bytes = ftell(mbfp);
			done = true;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else if (*type == MBF_HSATLRAW_RAW_LINE) {
			strcpy(data->comment, raw_line + *shift);
			mb_io_ptr->file_bytes = ftell(mbfp);
			done = true;
			expect = MBF_HSATLRAW_NONE;
			data->kind = MB_DATA_RAW_LINE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			status = MB_FAILURE;
		}
		else if (expect != MBF_HSATLRAW_NONE && expect != *type) {
			done = true;
			*line_save_flag = true;
		}
		else if (*type == MBF_HSATLRAW_ERGNHYDI) {
			status = mbr_hsatlraw_rd_ergnhydi(verbose, mbfp, data, *shift, error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_MEAN_VELOCITY;
			}
		}
		else if (*type == MBF_HSATLRAW_ERGNPARA) {
			status = mbr_hsatlraw_rd_ergnpara(verbose, mbfp, data, *shift, error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_STANDBY;
			}
		}
		else if (*type == MBF_HSATLRAW_ERGNPOSI) {
			status = mbr_hsatlraw_rd_ergnposi(verbose, mbfp, data, *shift, error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_NAV_SOURCE;
			}
		}
		else if (*type == MBF_HSATLRAW_ERGNEICH) {
			status = mbr_hsatlraw_rd_ergneich(verbose, mbfp, data, *shift, error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS) {
				done = false;
				data->kind = MB_DATA_CALIBRATE;
				expect = MBF_HSATLRAW_ERGNSLZT;
			}
		}
		else if (*type == MBF_HSATLRAW_ERGNMESS) {
			status = mbr_hsatlraw_rd_ergnmess(verbose, mbfp, data, *shift, error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS) {
				done = false;
				data->kind = MB_DATA_DATA;
				expect = MBF_HSATLRAW_ERGNSLZT;
			}
		}
		else if (*type == MBF_HSATLRAW_ERGNSLZT) {
			status = mbr_hsatlraw_rd_ergnslzt(verbose, mbfp, data, *shift, error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS && expect == MBF_HSATLRAW_ERGNSLZT) {
				done = false;
				expect = MBF_HSATLRAW_ERGNAMPL;
			}
			else if (status == MB_SUCCESS) {
				done = true;
				expect = MBF_HSATLRAW_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
			}
		}
		else if (*type == MBF_HSATLRAW_ERGNCTDS) {
			status = mbr_hsatlraw_rd_ergnctds(verbose, mbfp, data, *shift, error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_VELOCITY_PROFILE;
			}
		}
		else if (*type == MBF_HSATLRAW_ERGNAMPL) {
			status = mbr_hsatlraw_rd_ergnampl(verbose, mbfp, data, *shift, error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS && expect == MBF_HSATLRAW_ERGNAMPL) {
				done = true;
				expect = MBF_HSATLRAW_NONE;
			}
			else if (status == MB_SUCCESS) {
				done = true;
				expect = MBF_HSATLRAW_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
			}
		}
		else if (*type == MBF_HSATLRAW_LDEOCMNT) {
			status = mbr_hsatlraw_rd_ldeocmnt(verbose, mbfp, data, *shift, error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_COMMENT;
			}
		}
	}

	/* handle Hydrosweep Y2K problem */
	if (status == MB_SUCCESS && data->year < 1962)
		data->year = 2000 + (data->year % 100);

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
int mbr_hsatlraw_rd_label(int verbose, FILE *mbfp, char *line, int *type, int *shift, int *error) {
	int icmp;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
	}

	/* read next line in file */
	const int status = mbr_hsatlraw_read_line(verbose, mbfp, 0, line, error);

	/* see if we just encountered an identifier record */
	if (status == MB_SUCCESS) {
		*type = MBF_HSATLRAW_RAW_LINE;
		*shift = 0;
		for (int i = 1; i < MBF_HSATLRAW_RECORDS; i++) {
			icmp = strncmp(line, mbf_hsatlraw_labels[i], 8);
			if (icmp == 0)
				*type = i;
		}
	}

	/* didn't find one with zero shift, try shift = 4 in case this
	    is tape data */
	if (status == MB_SUCCESS && *type == MBF_HSATLRAW_RAW_LINE) {
		*shift = 4;
		for (int i = 1; i < MBF_HSATLRAW_RECORDS; i++) {
			icmp = strncmp(line + 4, mbf_hsatlraw_labels[i], 8);
			if (icmp == 0)
				*type = i;
		}
		if (*type == MBF_HSATLRAW_RAW_LINE)
			*shift = 0;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       line:       %s\n", line);
		fprintf(stderr, "dbg2       type:       %d\n", *type);
		fprintf(stderr, "dbg2       shift:      %d\n", *shift);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_hsatlraw_read_line(int verbose, FILE *mbfp, int minimum_size, char *line, int *error) {
	int nchars;
	char *result;
	bool blank;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
	}

	int status = MB_SUCCESS;

	/* read next good line in file */
	bool done = false;
	do {
		/* read next line in file */
		strncpy(line, "", MBF_HSATLRAW_MAXLINE);
		result = fgets(line, MBF_HSATLRAW_MAXLINE, mbfp);

		/* check for eof */
		if (result == line) {
			/* check size of line */
			nchars = strlen(line);
			if (nchars >= minimum_size) {
				done = true;

				/* trim trailing blank characters */
				blank = true;
				for (int i = (nchars - 1); i >= 0 && blank; i--) {
					if (line[i] == ' ' || line[i] == '\r' || line[i] == '\n')
						line[i] = '\0';
					else
						blank = false;
				}
				nchars = strlen(line);
			}

			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			done = true;
			*error = MB_ERROR_EOF;
			status = MB_FAILURE;
		}

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  New line read in function <%s>\n", __func__);
			fprintf(stderr, "dbg5       line:       %s\n", line);
			fprintf(stderr, "dbg5       chars:      %d\n", nchars);
		}

	} while (!done);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       line:       %s\n", line);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_hsatlraw_rd_ergnhydi(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
		fprintf(stderr, "dbg2       shift:      %zu\n", shift);
	}

	/* read event record from file */
	int status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error);

	/* make sure line is long enough */
	if (strlen(line) < 69 + shift) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* parse data from first line */
	if (status == MB_SUCCESS) {
		mb_get_double(&(data->lon), line + shift, 12);
		mb_get_double(&(data->lat), line + 12 + shift, 12);
		mb_get_int(&(data->year), line + 24 + shift, 4);
		mb_get_int(&(data->month), line + 28 + shift, 2);
		mb_get_int(&(data->day), line + 30 + shift, 2);
		mb_get_int(&(data->hour), line + 32 + shift, 2);
		mb_get_int(&(data->minute), line + 34 + shift, 2);
		mb_get_int(&(data->second), line + 36 + shift, 2);
		mb_get_int(&(data->alt_minute), line + 38 + shift, 5);
		mb_get_int(&(data->alt_second), line + 43 + shift, 2);
		mb_get_double(&(data->draught), line + 45 + shift, 4);
		mb_get_double(&(data->vel_mean), line + 49 + shift, 7);
		mb_get_double(&(data->vel_keel), line + 56 + shift, 7);
		mb_get_double(&(data->tide), line + 63 + shift, 6);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       draught:          %f\n", data->draught);
		fprintf(stderr, "dbg5       mean velocity:    %f\n", data->vel_mean);
		fprintf(stderr, "dbg5       keel velocity:    %f\n", data->vel_keel);
		fprintf(stderr, "dbg5       tide:             %f\n", data->tide);
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
int mbr_hsatlraw_rd_ergnpara(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
		fprintf(stderr, "dbg2       shift:      %zu\n", shift);
	}

	/* read event record from file */
	int status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error);

	/* make sure line is long enough */
	if (strlen(line) < 84 + shift) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* parse data from first line */
	if (status == MB_SUCCESS) {
		mb_get_double(&(data->lon), line + shift, 12);
		mb_get_double(&(data->lat), line + 12 + shift, 12);
		mb_get_int(&(data->year), line + 24 + shift, 4);
		mb_get_int(&(data->month), line + 28 + shift, 2);
		mb_get_int(&(data->day), line + 30 + shift, 2);
		mb_get_int(&(data->hour), line + 32 + shift, 2);
		mb_get_int(&(data->minute), line + 34 + shift, 2);
		mb_get_int(&(data->second), line + 36 + shift, 2);
		mb_get_int(&(data->alt_minute), line + 38 + shift, 5);
		mb_get_int(&(data->alt_second), line + 43 + shift, 2);
		mb_get_double(&(data->course_true), line + 45 + shift, 5);
		mb_get_double(&(data->speed_transverse), line + 50 + shift, 9);
		mb_get_double(&(data->speed), line + 59 + shift, 9);
		data->speed_reference[0] = line[68 + shift];
		mb_get_double(&(data->pitch), line + 69 + shift, 4);
		mb_get_int(&(data->track), line + 73 + shift, 4);
		mb_get_double(&(data->depth_center), line + 77 + shift, 7);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       course_true:      %f\n", data->course_true);
		fprintf(stderr, "dbg5       speed_transverse: %f\n", data->speed_transverse);
		fprintf(stderr, "dbg5       speed:            %f\n", data->speed);
		fprintf(stderr, "dbg5       speed_reference:  %c\n", data->speed_reference[0]);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       track:            %d\n", data->track);
		fprintf(stderr, "dbg5       depth_center:     %f\n", data->depth_center);
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
int mbr_hsatlraw_rd_ergnposi(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
		fprintf(stderr, "dbg2       shift:      %zu\n", shift);
	}

	/* read event record from file */
	int status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error);

	/* make sure line is long enough */
	if (strlen(line) < 67 + shift) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* parse data from first line */
	if (status == MB_SUCCESS) {
		mb_get_double(&(data->lon), line + shift, 12);
		mb_get_double(&(data->lat), line + 12 + shift, 12);
		mb_get_int(&(data->year), line + 24 + shift, 4);
		mb_get_int(&(data->month), line + 28 + shift, 2);
		mb_get_int(&(data->day), line + 30 + shift, 2);
		mb_get_int(&(data->hour), line + 32 + shift, 2);
		mb_get_int(&(data->minute), line + 34 + shift, 2);
		mb_get_int(&(data->second), line + 36 + shift, 2);
		mb_get_int(&(data->alt_minute), line + 38 + shift, 5);
		mb_get_int(&(data->alt_second), line + 43 + shift, 2);
		mb_get_double(&(data->pos_corr_x), line + 45 + shift, 7);
		mb_get_double(&(data->pos_corr_y), line + 52 + shift, 7);
		strncpy(data->sensors, line + 59 + shift, 8);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       pos_corr_x:       %f\n", data->pos_corr_x);
		fprintf(stderr, "dbg5       pos_corr_y:       %f\n", data->pos_corr_y);
		fprintf(stderr, "dbg5       sensors:          %s\n", data->sensors);
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
int mbr_hsatlraw_rd_ergneich(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];
	int numvals;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
		fprintf(stderr, "dbg2       shift:      %zu\n", shift);
	}

	/* read event record from file */
	int status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error);

	/* make sure line is long enough */
	if (strlen(line) < 90 + shift) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* parse data from event record */
	if (status == MB_SUCCESS) {
		mb_get_double(&(data->lon), line + shift, 12);
		mb_get_double(&(data->lat), line + 12 + shift, 12);
		mb_get_int(&(data->year), line + 24 + shift, 4);
		mb_get_int(&(data->month), line + 28 + shift, 2);
		mb_get_int(&(data->day), line + 30 + shift, 2);
		mb_get_int(&(data->hour), line + 32 + shift, 2);
		mb_get_int(&(data->minute), line + 34 + shift, 2);
		mb_get_int(&(data->second), line + 36 + shift, 2);
		mb_get_int(&(data->alt_minute), line + 38 + shift, 5);
		mb_get_int(&(data->alt_second), line + 43 + shift, 2);
		mb_get_double(&(data->course_true), line + 45 + shift, 5);
		mb_get_double(&(data->speed_transverse), line + 50 + shift, 9);
		mb_get_double(&(data->speed), line + 59 + shift, 9);
		data->speed_reference[0] = line[68 + shift];
		mb_get_double(&(data->pitch), line + 69 + shift, 4);
		mb_get_int(&(data->track), line + 73 + shift, 4);
		mb_get_double(&(data->depth_center), line + 77 + shift, 7);
		mb_get_double(&(data->depth_scale), line + 84 + shift, 4);
		mb_get_int(&(data->spare), line + 88 + shift, 2);
		if (data->depth_scale > 0.0)
			data->depth[29] = (int)(data->depth_center / data->depth_scale);
		else
			data->depth[29] = (int)data->depth_center;
		data->distance[29] = 0;
	}

	/* read and parse data from first data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->distance[i + 30]), line + i * 4 + 2 + shift, 4);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from second data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->depth[i + 30]), line + i * 4 + 2 + shift, 4);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from third data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++) {
				mb_get_int(&(data->distance[28 - i]), line + i * 4 + 2 + shift, 4);
				data->distance[28 - i] = -data->distance[28 - i];
			}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from fourth data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->depth[28 - i]), line + i * 4 + 2 + shift, 4);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       course_true:      %f\n", data->course_true);
		fprintf(stderr, "dbg5       speed_transverse: %f\n", data->speed_transverse);
		fprintf(stderr, "dbg5       speed:            %f\n", data->speed);
		fprintf(stderr, "dbg5       speed_reference:  %c\n", data->speed_reference[0]);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       track:            %d\n", data->track);
		fprintf(stderr, "dbg5       depth_center:     %f\n", data->depth_center);
		fprintf(stderr, "dbg5       depth_scale:      %f\n", data->depth_scale);
		fprintf(stderr, "dbg5       spare:            %d\n", data->spare);
		fprintf(stderr, "dbg5       distances and depths:\n");
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
			fprintf(stderr, "dbg5         %d  %d  %d\n", i, data->distance[i], data->depth[i]);
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
int mbr_hsatlraw_rd_ergnmess(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];
	int numvals;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
		fprintf(stderr, "dbg2       shift:      %zu\n", shift);
	}

	/* read event record from file */
	int status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error);

	/* make sure line is long enough */
	if (status == MB_SUCCESS && (strlen(line) < 90 + shift || strlen(line) > 92 + shift)) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* parse data from event record */
	if (status == MB_SUCCESS) {
		mb_get_double(&(data->lon), line + shift, 12);
		mb_get_double(&(data->lat), line + 12 + shift, 12);
		mb_get_int(&(data->year), line + 24 + shift, 4);
		mb_get_int(&(data->month), line + 28 + shift, 2);
		mb_get_int(&(data->day), line + 30 + shift, 2);
		mb_get_int(&(data->hour), line + 32 + shift, 2);
		mb_get_int(&(data->minute), line + 34 + shift, 2);
		mb_get_int(&(data->second), line + 36 + shift, 2);
		mb_get_int(&(data->alt_minute), line + 38 + shift, 5);
		mb_get_int(&(data->alt_second), line + 43 + shift, 2);
		mb_get_double(&(data->course_true), line + 45 + shift, 5);
		mb_get_double(&(data->speed_transverse), line + 50 + shift, 9);
		mb_get_double(&(data->speed), line + 59 + shift, 9);
		data->speed_reference[0] = line[68 + shift];
		mb_get_double(&(data->pitch), line + 69 + shift, 4);
		mb_get_int(&(data->track), line + 73 + shift, 4);
		mb_get_double(&(data->depth_center), line + 77 + shift, 7);
		mb_get_double(&(data->depth_scale), line + 84 + shift, 4);
		mb_get_int(&(data->spare), line + 88 + shift, 2);
		if (data->depth_scale > 0.0)
			data->depth[29] = (int)(data->depth_center / data->depth_scale);
		else
			data->depth[29] = (int)data->depth_center;
		data->distance[29] = 0;
	}

	/* read and parse data from first data record */
	if (status == MB_SUCCESS && (status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->distance[i + 30]), line + i * 4 + 2 + shift, 4);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from second data record */
	if (status == MB_SUCCESS && (status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++) {
				mb_get_int(&(data->depth[i + 30]), line + i * 4 + 2 + shift, 4);
			}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from third data record */
	if (status == MB_SUCCESS && (status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++) {
				mb_get_int(&(data->distance[28 - i]), line + i * 4 + 2 + shift, 4);
				data->distance[28 - i] = -data->distance[28 - i];
			}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from fourth data record */
	if (status == MB_SUCCESS && (status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++) {
				mb_get_int(&(data->depth[28 - i]), line + i * 4 + 2 + shift, 4);
			}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       course_true:      %f\n", data->course_true);
		fprintf(stderr, "dbg5       speed_transverse: %f\n", data->speed_transverse);
		fprintf(stderr, "dbg5       speed:            %f\n", data->speed);
		fprintf(stderr, "dbg5       speed_reference:  %c\n", data->speed_reference[0]);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       track:            %d\n", data->track);
		fprintf(stderr, "dbg5       depth_center:     %f\n", data->depth_center);
		fprintf(stderr, "dbg5       depth_scale:      %f\n", data->depth_scale);
		fprintf(stderr, "dbg5       spare:            %d\n", data->spare);
		fprintf(stderr, "dbg5       distances and depths:\n");
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
			fprintf(stderr, "dbg5         %d  %d  %d\n", i, data->distance[i], data->depth[i]);
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
int mbr_hsatlraw_rd_ergnslzt(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];
	int numvals;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
		fprintf(stderr, "dbg2       shift:      %zu\n", shift);
	}

	/* read event record from file */
	int status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error);

	/* make sure line is long enough */
	if (strlen(line) < 84 + shift) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* parse data from event record */
	if (status == MB_SUCCESS) {
		mb_get_double(&(data->lon), line + shift, 12);
		mb_get_double(&(data->lat), line + 12 + shift, 12);
		mb_get_int(&(data->year), line + 24 + shift, 4);
		mb_get_int(&(data->month), line + 28 + shift, 2);
		mb_get_int(&(data->day), line + 30 + shift, 2);
		mb_get_int(&(data->hour), line + 32 + shift, 2);
		mb_get_int(&(data->minute), line + 34 + shift, 2);
		mb_get_int(&(data->second), line + 36 + shift, 2);
		mb_get_double(&(data->course_true), line + 38 + shift, 5);
		mb_get_double(&(data->course_ground), line + 43 + shift, 5);
		mb_get_double(&(data->speed_ground), line + 48 + shift, 9);
		mb_get_double(&(data->heave), line + 57 + shift, 6);
		mb_get_double(&(data->pitch), line + 63 + shift, 4);
		mb_get_double(&(data->roll), line + 67 + shift, 5);
		mb_get_double(&(data->time_center), line + 72 + shift, 6);
		mb_get_double(&(data->time_scale), line + 78 + shift, 6);
		data->time[29] = (int)(0.0001 * (data->time_center) / (data->time_scale));
	}

	/* read and parse data from first data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->time[i + 30]), line + i * 4 + 2 + shift, 4);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from second data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		mb_get_int(&numvals, line + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->time[28 - i]), line + i * 4 + 2 + shift, 4);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from third data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		for (int i = 0; i < 11; i++)
			mb_get_double(&(data->gyro[i]), line + i * 5 + shift, 5);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       course_true:      %f\n", data->course_true);
		fprintf(stderr, "dbg5       course_ground:    %f\n", data->course_ground);
		fprintf(stderr, "dbg5       speed_ground:     %f\n", data->speed_ground);
		fprintf(stderr, "dbg5       heave:            %f\n", data->heave);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       roll:             %f\n", data->roll);
		fprintf(stderr, "dbg5       time_center:      %f\n", data->depth_center);
		fprintf(stderr, "dbg5       time_scale:       %f\n", data->time_scale);
		fprintf(stderr, "dbg5       travel times:\n");
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
			fprintf(stderr, "dbg5         %d  %d\n", i, data->time[i]);
		fprintf(stderr, "dbg5       gyro headings:\n");
		for (int i = 0; i < 11; i++)
			fprintf(stderr, "dbg5         %d  %f\n", i, data->gyro[i]);
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
int mbr_hsatlraw_rd_ergnctds(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];
	int nlines;
	int numvals;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
		fprintf(stderr, "dbg2       shift:      %zu\n", shift);
	}

	/* read event record from file */
	int status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error);

	/* make sure line is long enough */
	if (strlen(line) < 40 + shift) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* parse data from event record */
	if (status == MB_SUCCESS) {
		mb_get_double(&(data->lon), line + shift, 12);
		mb_get_double(&(data->lat), line + 12 + shift, 12);
		mb_get_int(&(data->year), line + 24 + shift, 4);
		mb_get_int(&(data->month), line + 28 + shift, 2);
		mb_get_int(&(data->day), line + 30 + shift, 2);
		mb_get_int(&(data->hour), line + 32 + shift, 2);
		mb_get_int(&(data->minute), line + 34 + shift, 2);
		mb_get_int(&(data->second), line + 36 + shift, 2);
		mb_get_int(&(data->num_vel), line + 38 + shift, 2);
	}

	nlines = 0;
	if (status == MB_SUCCESS) {
		/* figure out how many lines to read */
		if (data->num_vel > MBF_HSATLRAW_MAXVEL)
			data->num_vel = MBF_HSATLRAW_MAXVEL;
		nlines = data->num_vel / 10;
		if (data->num_vel % 10 > 0)
			nlines++;
	}

	/* read and parse data records from file */
	for (int i = 0; i < nlines; i++) {
		if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
			numvals = 10;
			if (i == nlines - 1)
				numvals = data->num_vel % 10;
			for (int j = 0; j < numvals; j++) {
				const int k = j + i * 10;
				mb_get_double(&(data->vdepth[k]), line + j * 11 + shift, 5);
				mb_get_double(&(data->velocity[k]), line + j * 11 + 5 + shift, 6);
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       num_vel:          %d\n", data->num_vel);
		fprintf(stderr, "dbg5       water depths and velocities:\n");
		for (int i = 0; i < 11; i++)
			fprintf(stderr, "dbg5         %d  %f  %f\n", i, data->vdepth[i], data->velocity[i]);
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
int mbr_hsatlraw_rd_ergnampl(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];
	int numvals;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
		fprintf(stderr, "dbg2       shift:      %zu\n", shift);
	}

	/* read event record from file */
	int status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error);

	/* make sure line is long enough */
	if (strlen(line) < 90 + shift) {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* parse data from event record */
	if (status == MB_SUCCESS) {
		mb_get_double(&(data->lon), line + shift, 12);
		mb_get_double(&(data->lat), line + 12 + shift, 12);
		mb_get_int(&(data->year), line + 24 + shift, 4);
		mb_get_int(&(data->month), line + 28 + shift, 2);
		mb_get_int(&(data->day), line + 30 + shift, 2);
		mb_get_int(&(data->hour), line + 32 + shift, 2);
		mb_get_int(&(data->minute), line + 34 + shift, 2);
		mb_get_int(&(data->second), line + 36 + shift, 2);
		data->mode[0] = line[38 + shift];
		mb_get_int(&(data->trans_strbd), line + 39 + shift, 3);
		mb_get_int(&(data->trans_vert), line + 42 + shift, 3);
		mb_get_int(&(data->trans_port), line + 45 + shift, 3);
		mb_get_int(&(data->pulse_len_strbd), line + 48 + shift, 2);
		mb_get_int(&(data->pulse_len_vert), line + 50 + shift, 2);
		mb_get_int(&(data->pulse_len_port), line + 52 + shift, 2);
		mb_get_int(&(data->gain_start), line + 54 + shift, 2);
		mb_get_int(&(data->r_compensation_factor), line + 56 + shift, 2);
		mb_get_int(&(data->compensation_start), line + 58 + shift, 4);
		mb_get_int(&(data->increase_start), line + 62 + shift, 5);
		mb_get_int(&(data->tvc_near), line + 67 + shift, 2);
		mb_get_int(&(data->tvc_far), line + 69 + shift, 2);
		mb_get_int(&(data->increase_int_near), line + 71 + shift, 3);
		mb_get_int(&(data->increase_int_far), line + 74 + shift, 3);
		mb_get_int(&(data->gain_center), line + 77 + shift, 1);
		mb_get_double(&(data->filter_gain), line + 78 + shift, 5);
		mb_get_int(&(data->amplitude_center), line + 83 + shift, 3);
		mb_get_int(&(data->echo_duration_center), line + 86 + shift, 3);
		mb_get_int(&(data->echo_scale_center), line + 89 + shift, 1);
		data->amplitude[29] = data->amplitude_center;
		data->echo_duration[29] = (int)data->echo_duration_center;
	}

	/* read and parse data from first data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		for (int i = 0; i < 8; i++)
			mb_get_int(&(data->gain[i + 8]), line + i + shift, 1);
		mb_get_int(&numvals, line + 8 + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->amplitude[i + 30]), line + i * 3 + 10 + shift, 3);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from second data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		for (int i = 0; i < 8; i++)
			mb_get_int(&(data->gain[i]), line + i + shift, 1);
		mb_get_int(&numvals, line + 8 + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->amplitude[28 - i]), line + i * 3 + 10 + shift, 3);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from third data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		for (int i = 0; i < 8; i++)
			mb_get_int(&(data->echo_scale[i + 8]), line + 1 + shift, 1);
		mb_get_int(&numvals, line + 8 + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->echo_duration[i + 30]), line + i * 3 + 10 + shift, 3);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read and parse data from fourth data record */
	if ((status = mbr_hsatlraw_read_line(verbose, mbfp, shift + 9, line, error)) == MB_SUCCESS) {
		for (int i = 0; i < 8; i++)
			mb_get_int(&(data->echo_scale[i]), line + 1 + shift, 1);
		mb_get_int(&numvals, line + 8 + shift, 2);
		if (numvals == 29)
			for (int i = 0; i < numvals; i++)
				mb_get_int(&(data->echo_duration[28 - i]), line + i * 3 + 10 + shift, 3);
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       mode:             %c\n", data->mode[0]);
		fprintf(stderr, "dbg5       trans_strbd:      %d\n", data->trans_strbd);
		fprintf(stderr, "dbg5       trans_vert:       %d\n", data->trans_vert);
		fprintf(stderr, "dbg5       trans_port:       %d\n", data->trans_port);
		fprintf(stderr, "dbg5       pulse_len_strbd:  %d\n", data->pulse_len_strbd);
		fprintf(stderr, "dbg5       pulse_len_vert:   %d\n", data->pulse_len_vert);
		fprintf(stderr, "dbg5       pulse_len_port:   %d\n", data->pulse_len_port);
		fprintf(stderr, "dbg5       gain_start:       %d\n", data->gain_start);
		fprintf(stderr, "dbg5       r_comp_factor:    %d\n", data->r_compensation_factor);
		fprintf(stderr, "dbg5       comp_start:       %d\n", data->compensation_start);
		fprintf(stderr, "dbg5       increase_start:   %d\n", data->increase_start);
		fprintf(stderr, "dbg5       tvc_near:         %d\n", data->tvc_near);
		fprintf(stderr, "dbg5       increase_start:   %d\n", data->increase_start);
		fprintf(stderr, "dbg5       tvc_far:          %d\n", data->tvc_far);
		fprintf(stderr, "dbg5       increase_int_near:%d\n", data->increase_int_near);
		fprintf(stderr, "dbg5       increase_int_far: %d\n", data->increase_int_far);
		fprintf(stderr, "dbg5       gain_center:      %d\n", data->gain_center);
		fprintf(stderr, "dbg5       filter_gain:      %f\n", data->filter_gain);
		fprintf(stderr, "dbg5       amplitude_center: %d\n", data->amplitude_center);
		fprintf(stderr, "dbg5       echo_dur_center:  %d\n", data->echo_duration_center);
		fprintf(stderr, "dbg5       echo_scal_center: %d\n", data->echo_scale_center);
		fprintf(stderr, "dbg5       amplitudes and echo durations:\n");
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
			fprintf(stderr, "dbg5         %d  %d  %d\n", i, data->amplitude[i], data->echo_duration[i]);
		fprintf(stderr, "dbg5       gains and echo scales:\n");
		for (int i = 0; i < 16; i++)
			fprintf(stderr, "dbg5         %d  %d  %d\n", i, data->gain[i], data->echo_scale[i]);
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
int mbr_hsatlraw_rd_ldeocmnt(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
		fprintf(stderr, "dbg2       shift:      %zu\n", shift);
	}

	/* read comment record from file */
	int status = mbr_hsatlraw_read_line(verbose, mbfp, shift, line, error);

	/* copy comment into data structure */
	if (status == MB_SUCCESS) {
    memset(data->comment, 0 , MBF_HSATLRAW_MAXLINE);
		if (strlen(line + shift) > 0)
			strncpy(data->comment, line + shift, MBF_HSATLRAW_MAXLINE - 1);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Value read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       comment:          %s\n", data->comment);
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
int mbr_hsatlraw_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;
	FILE *mbfp = mb_io_ptr->mbfp;

	int status = MB_SUCCESS;

	if (data->kind == MB_DATA_RAW_LINE) {
		status = mbr_hsatlraw_wr_rawline(verbose, mbfp, data, error);
	}
	else if (data->kind == MB_DATA_DATA) {
		/* status = */ mbr_hsatlraw_wr_ergnmess(verbose, mbfp, data_ptr, error);
		/* status = */ mbr_hsatlraw_wr_ergnslzt(verbose, mbfp, data_ptr, error);
		status = mbr_hsatlraw_wr_ergnampl(verbose, mbfp, data_ptr, error);
	}
	else if (data->kind == MB_DATA_CALIBRATE) {
		/* status = */ mbr_hsatlraw_wr_ergneich(verbose, mbfp, data_ptr, error);
		/* status = */ mbr_hsatlraw_wr_ergnslzt(verbose, mbfp, data_ptr, error);
		status = mbr_hsatlraw_wr_ergnampl(verbose, mbfp, data_ptr, error);
	}
	else if (data->kind == MB_DATA_MEAN_VELOCITY) {
		status = mbr_hsatlraw_wr_ergnhydi(verbose, mbfp, data_ptr, error);
	}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE) {
		status = mbr_hsatlraw_wr_ergnctds(verbose, mbfp, data_ptr, error);
	}
	else if (data->kind == MB_DATA_STANDBY) {
		status = mbr_hsatlraw_wr_ergnpara(verbose, mbfp, data_ptr, error);
	}
	else if (data->kind == MB_DATA_NAV_SOURCE) {
		status = mbr_hsatlraw_wr_ergnposi(verbose, mbfp, data_ptr, error);
	}
	else if (data->kind == MB_DATA_COMMENT) {
		status = mbr_hsatlraw_wr_ldeocmnt(verbose, mbfp, data_ptr, error);
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Data record kind in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:       %d\n", data->kind);
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
int mbr_hsatlraw_wr_label(int verbose, FILE *mbfp, char type, int *error) {
	char line[MBF_HSATLRAW_MAXLINE];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       type:       %d\n", type);
	}

	/* write label in file */
	sprintf(line, "%8s\n", mbf_hsatlraw_labels[(int)type]);
	int status = mbr_hsatlraw_write_line(verbose, mbfp, line, error);

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
int mbr_hsatlraw_write_line(int verbose, FILE *mbfp, char *line, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       line:       %s\n", line);
	}

	int status = MB_SUCCESS;

	/* write next line in file */
	if ((status = fputs(line, mbfp)) != EOF) {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}
	else {
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
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
int mbr_hsatlraw_wr_rawline(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       raw line:         %s\n", data->comment);
	}

	int status = MB_SUCCESS;

	/* write out the data */
	const int count = fprintf(mbfp, "%s\n", data->comment);
	if (count >= 0) {
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
	}
	else {
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnhydi(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       draught:          %f\n", data->draught);
		fprintf(stderr, "dbg5       mean velocity:    %f\n", data->vel_mean);
		fprintf(stderr, "dbg5       keel velocity:    %f\n", data->vel_keel);
		fprintf(stderr, "dbg5       tide:             %f\n", data->tide);
	}

	/* write the record label */
	int status = mbr_hsatlraw_wr_label(verbose, mbfp, MBF_HSATLRAW_ERGNHYDI, error);

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* output the event line */
		fprintf(mbfp, "%+12.7f", data->lon);
		fprintf(mbfp, "%+12.7f", data->lat);
		fprintf(mbfp, "%4.4d", data->year);
		fprintf(mbfp, "%2.2d", data->month);
		fprintf(mbfp, "%2.2d", data->day);
		fprintf(mbfp, "%2.2d", data->hour);
		fprintf(mbfp, "%2.2d", data->minute);
		fprintf(mbfp, "%2.2d", data->second);
		fprintf(mbfp, "%5d", data->alt_minute);
		fprintf(mbfp, "%2d", data->alt_second);
		fprintf(mbfp, "%4.1f", data->draught);
		fprintf(mbfp, "%7.2f", data->vel_mean);
		fprintf(mbfp, "%7.2f", data->vel_keel);
		const int count = fprintf(mbfp, "%+06.2f\n", data->tide);

		/* check for an error */
		if (count > 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnpara(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       course_true:      %f\n", data->course_true);
		fprintf(stderr, "dbg5       speed_transverse: %f\n", data->speed_transverse);
		fprintf(stderr, "dbg5       speed:            %f\n", data->speed);
		fprintf(stderr, "dbg5       speed_reference:  %c\n", data->speed_reference[0]);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       track:            %d\n", data->track);
		fprintf(stderr, "dbg5       depth_center:     %f\n", data->depth_center);
	}

	/* write the record label */
	int status = mbr_hsatlraw_wr_label(verbose, mbfp, MBF_HSATLRAW_ERGNPARA, error);

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* output the event line */
		fprintf(mbfp, "%+12.7f", data->lon);
		fprintf(mbfp, "%+12.7f", data->lat);
		fprintf(mbfp, "%4.4d", data->year);
		fprintf(mbfp, "%2.2d", data->month);
		fprintf(mbfp, "%2.2d", data->day);
		fprintf(mbfp, "%2.2d", data->hour);
		fprintf(mbfp, "%2.2d", data->minute);
		fprintf(mbfp, "%2.2d", data->second);
		fprintf(mbfp, "%5d", data->alt_minute);
		fprintf(mbfp, "%2d", data->alt_second);
		fprintf(mbfp, "%5.1f", data->course_true);
		fprintf(mbfp, "%+9.1f", data->speed_transverse);
		fprintf(mbfp, "%+9.1f", data->speed);
		fprintf(mbfp, "%c", data->speed_reference[0]);
		fprintf(mbfp, "%+4.1f", data->pitch);
		fprintf(mbfp, "%4.4d", data->track);
		const int count = fprintf(mbfp, "%7.1f\n", data->depth_center);

		/* check for an error */
		if (count > 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnposi(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       pos_corr_x:       %f\n", data->pos_corr_x);
		fprintf(stderr, "dbg5       pos_corr_y:       %f\n", data->pos_corr_y);
		fprintf(stderr, "dbg5       sensors:          %s\n", data->sensors);
	}

	/* write the record label */
	int status = mbr_hsatlraw_wr_label(verbose, mbfp, MBF_HSATLRAW_ERGNPOSI, error);

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* output the event line */
		fprintf(mbfp, "%+12.7f", data->lon);
		fprintf(mbfp, "%+12.7f", data->lat);
		fprintf(mbfp, "%4.4d", data->year);
		fprintf(mbfp, "%2.2d", data->month);
		fprintf(mbfp, "%2.2d", data->day);
		fprintf(mbfp, "%2.2d", data->hour);
		fprintf(mbfp, "%2.2d", data->minute);
		fprintf(mbfp, "%2.2d", data->second);
		fprintf(mbfp, "%5d", data->alt_minute);
		fprintf(mbfp, "%2d", data->alt_second);
		fprintf(mbfp, "%7.0f", data->pos_corr_x);
		fprintf(mbfp, "%7.0f", data->pos_corr_y);
		const int count = fprintf(mbfp, "%8s\n", data->sensors);

		/* check for an error */
		if (count > 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergneich(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       course_true:      %f\n", data->course_true);
		fprintf(stderr, "dbg5       speed_transverse: %f\n", data->speed_transverse);
		fprintf(stderr, "dbg5       speed:            %f\n", data->speed);
		fprintf(stderr, "dbg5       speed_reference:  %c\n", data->speed_reference[0]);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       track:            %d\n", data->track);
		fprintf(stderr, "dbg5       depth_center:     %f\n", data->depth_center);
		fprintf(stderr, "dbg5       depth_scale:      %f\n", data->depth_scale);
		fprintf(stderr, "dbg5       spare:            %d\n", data->spare);
		fprintf(stderr, "dbg5       distances and depths:\n");
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
			fprintf(stderr, "dbg5         %d  %d  %d\n", i, data->distance[i], data->depth[i]);
	}

	/* write the record label */
	int status = mbr_hsatlraw_wr_label(verbose, mbfp, MBF_HSATLRAW_ERGNEICH, error);

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* output the event line */
		fprintf(mbfp, "%+12.7f", data->lon);
		fprintf(mbfp, "%+12.7f", data->lat);
		fprintf(mbfp, "%4.4d", data->year);
		fprintf(mbfp, "%2.2d", data->month);
		fprintf(mbfp, "%2.2d", data->day);
		fprintf(mbfp, "%2.2d", data->hour);
		fprintf(mbfp, "%2.2d", data->minute);
		fprintf(mbfp, "%2.2d", data->second);
		fprintf(mbfp, "%5d", data->alt_minute);
		fprintf(mbfp, "%2d", data->alt_second);
		fprintf(mbfp, "%5.1f", data->course_true);
		fprintf(mbfp, "%+9.1f", data->speed_transverse);
		fprintf(mbfp, "%+9.1f", data->speed);
		fprintf(mbfp, "%c", data->speed_reference[0]);
		fprintf(mbfp, "%+4.1f", data->pitch);
		fprintf(mbfp, "%4.4d", data->track);
		fprintf(mbfp, "%7.1f", data->depth_center);
		fprintf(mbfp, "%4.2f", data->depth_scale);
		fprintf(mbfp, "%2d\n", data->spare);

		/* output forward distances */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", data->distance[i + 30]);
		fprintf(mbfp, "\n");

		/* output forward depths */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", data->depth[i + 30]);
		fprintf(mbfp, "\n");

		/* output aft distances */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", (-data->distance[28 - i]));
		fprintf(mbfp, "\n");

		/* output aft depths */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", data->depth[28 - i]);
		const int count = fprintf(mbfp, "\n");

		/* check for an error */
		if (count > 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnmess(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       alt minute:       %d\n", data->alt_minute);
		fprintf(stderr, "dbg5       alt second:       %d\n", data->alt_second);
		fprintf(stderr, "dbg5       course_true:      %f\n", data->course_true);
		fprintf(stderr, "dbg5       speed_transverse: %f\n", data->speed_transverse);
		fprintf(stderr, "dbg5       speed:            %f\n", data->speed);
		fprintf(stderr, "dbg5       speed_reference:  %c\n", data->speed_reference[0]);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       track:            %d\n", data->track);
		fprintf(stderr, "dbg5       depth_center:     %f\n", data->depth_center);
		fprintf(stderr, "dbg5       depth_scale:      %f\n", data->depth_scale);
		fprintf(stderr, "dbg5       spare:            %d\n", data->spare);
		fprintf(stderr, "dbg5       distances and depths:\n");
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
			fprintf(stderr, "dbg5         %d  %d  %d\n", i, data->distance[i], data->depth[i]);
	}

	/* write the record label */
	int status = mbr_hsatlraw_wr_label(verbose, mbfp, MBF_HSATLRAW_ERGNMESS, error);

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* output the event line */
		fprintf(mbfp, "%+12.7f", data->lon);
		fprintf(mbfp, "%+12.7f", data->lat);
		fprintf(mbfp, "%4.4d", data->year);
		fprintf(mbfp, "%2.2d", data->month);
		fprintf(mbfp, "%2.2d", data->day);
		fprintf(mbfp, "%2.2d", data->hour);
		fprintf(mbfp, "%2.2d", data->minute);
		fprintf(mbfp, "%2.2d", data->second);
		fprintf(mbfp, "%5d", data->alt_minute);
		fprintf(mbfp, "%2d", data->alt_second);
		fprintf(mbfp, "%5.1f", data->course_true);
		fprintf(mbfp, "%+9.1f", data->speed_transverse);
		fprintf(mbfp, "%+9.1f", data->speed);
		fprintf(mbfp, "%c", data->speed_reference[0]);
		fprintf(mbfp, "%+4.1f", data->pitch);
		fprintf(mbfp, "%4.4d", data->track);
		fprintf(mbfp, "%7.1f", data->depth_center);
		fprintf(mbfp, "%4.2f", data->depth_scale);
		fprintf(mbfp, "%2d\n", data->spare);

		/* output starboard crosstrack distances */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", data->distance[i + 30]);
		fprintf(mbfp, "\n");

		/* output starboard crosstrack depths */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", data->depth[i + 30]);
		fprintf(mbfp, "\n");

		/* output port crosstrack distances */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", (-data->distance[28 - i]));
		fprintf(mbfp, "\n");

		/* output port crosstrack depths */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", data->depth[28 - i]);
		const int count = fprintf(mbfp, "\n");

		/* check for an error */
		if (count > 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnslzt(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	/* check if there are data to output */
	bool datacheck = false;
	for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
		if (data->time[i] > 0)
			datacheck = true;

	if (verbose >= 5 && !datacheck) {
		fprintf(stderr, "\ndbg5  No values to be written in MBIO function <%s>\n", __func__);
	}

	if (verbose >= 5 && datacheck) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       course_true:      %f\n", data->course_true);
		fprintf(stderr, "dbg5       course_ground:    %f\n", data->course_ground);
		fprintf(stderr, "dbg5       speed_ground:     %f\n", data->speed_ground);
		fprintf(stderr, "dbg5       heave:            %f\n", data->heave);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       roll:             %f\n", data->roll);
		fprintf(stderr, "dbg5       time_center:      %f\n", data->depth_center);
		fprintf(stderr, "dbg5       time_scale:       %f\n", data->time_scale);
		fprintf(stderr, "dbg5       travel times:\n");
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
			fprintf(stderr, "dbg5         %d  %d\n", i, data->time[i]);
		fprintf(stderr, "dbg5       gyro headings:\n");
		for (int i = 0; i < 11; i++)
			fprintf(stderr, "dbg5         %d  %f\n", i, data->gyro[i]);
	}

	int status = MB_SUCCESS;

	/* write the record label */
	if (datacheck)
		status = mbr_hsatlraw_wr_label(verbose, mbfp, MBF_HSATLRAW_ERGNSLZT, error);

	/* write out the data */
	if (status == MB_SUCCESS && datacheck) {
		/* output the event line */
		fprintf(mbfp, "%+12.7f", data->lon);
		fprintf(mbfp, "%+12.7f", data->lat);
		fprintf(mbfp, "%4.4d", data->year);
		fprintf(mbfp, "%2.2d", data->month);
		fprintf(mbfp, "%2.2d", data->day);
		fprintf(mbfp, "%2.2d", data->hour);
		fprintf(mbfp, "%2.2d", data->minute);
		fprintf(mbfp, "%2.2d", data->second);
		fprintf(mbfp, "%5.1f", data->course_true);
		fprintf(mbfp, "%5.1f", data->course_ground);
		fprintf(mbfp, "%+9.1f", data->speed_ground);
		fprintf(mbfp, "%+6.2f", data->heave);
		fprintf(mbfp, "%+4.1f", data->pitch);
		fprintf(mbfp, "%+5.1f", data->roll);
		fprintf(mbfp, "%06.0f", data->time_center);
		fprintf(mbfp, "%6.4f\n", data->time_scale);

		/* output starboard crosstrack travel times */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", data->time[i + 30]);
		fprintf(mbfp, "\n");

		/* output port crosstrack travel times */
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%4.4d", data->time[28 - i]);
		fprintf(mbfp, "\n");

		/* output gyro headings */
		for (int i = 0; i < 11; i++)
			fprintf(mbfp, "%05.1f", data->gyro[i]);
		const int count = fprintf(mbfp, "\n");

		/* check for an error */
		if (count > 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnctds(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	int nline, nrem;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       num_vel:          %d\n", data->num_vel);
		fprintf(stderr, "dbg5       water depths and velocities:\n");
		for (int i = 0; i < 11; i++)
			fprintf(stderr, "dbg5         %d  %f  %f\n", i, data->vdepth[i], data->velocity[i]);
	}

	/* write the record label */
	int status = mbr_hsatlraw_wr_label(verbose, mbfp, MBF_HSATLRAW_ERGNCTDS, error);

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* output the event line */
		fprintf(mbfp, "%+12.7f", data->lon);
		fprintf(mbfp, "%+12.7f", data->lat);
		fprintf(mbfp, "%4.4d", data->year);
		fprintf(mbfp, "%2.2d", data->month);
		fprintf(mbfp, "%2.2d", data->day);
		fprintf(mbfp, "%2.2d", data->hour);
		fprintf(mbfp, "%2.2d", data->minute);
		fprintf(mbfp, "%2.2d", data->second);
		int count = fprintf(mbfp, "%2d\n", data->num_vel);

		/* figure number of velocity lines to write */
		nline = data->num_vel / 10;
		nrem = data->num_vel % 10;

		/* write all of the full lines */
		for (int i = 0; i < nline; i++) {
			for (int i = 0; i < 10; i++)
				fprintf(mbfp, "%5.0f%6.1f", data->vdepth[i], data->velocity[i]);
			count = fprintf(mbfp, "\n");
		}

		/* write the last line as needed */
		if (nrem > 0) {
			for (int i = 0; i < nrem; i++)
				fprintf(mbfp, "%5.0f%6.1f", data->vdepth[i], data->velocity[i]);
			for (int i = 0; i < (10 - nrem); i++)
				fprintf(mbfp, "           ");
			count = fprintf(mbfp, "\n");
		}

		/* check for an error */
		if (count > 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ergnampl(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	/* check if there are data to output */
	bool datacheck = false;
	for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
		if (data->amplitude[i] > 0)
			datacheck = true;

	if (verbose >= 5 && !datacheck) {
		fprintf(stderr, "\ndbg5  No values to be written in MBIO function <%s>\n", __func__);
	}

	if (verbose >= 5 && datacheck) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       longitude:        %f\n", data->lon);
		fprintf(stderr, "dbg5       latitude:         %f\n", data->lat);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       second:           %d\n", data->second);
		fprintf(stderr, "dbg5       mode:             %c\n", data->mode[0]);
		fprintf(stderr, "dbg5       trans_strbd:      %d\n", data->trans_strbd);
		fprintf(stderr, "dbg5       trans_vert:       %d\n", data->trans_vert);
		fprintf(stderr, "dbg5       trans_port:       %d\n", data->trans_port);
		fprintf(stderr, "dbg5       pulse_len_strbd:  %d\n", data->pulse_len_strbd);
		fprintf(stderr, "dbg5       pulse_len_vert:   %d\n", data->pulse_len_vert);
		fprintf(stderr, "dbg5       pulse_len_port:   %d\n", data->pulse_len_port);
		fprintf(stderr, "dbg5       gain_start:       %d\n", data->gain_start);
		fprintf(stderr, "dbg5       r_comp_factor:    %d\n", data->r_compensation_factor);
		fprintf(stderr, "dbg5       comp_start:       %d\n", data->compensation_start);
		fprintf(stderr, "dbg5       increase_start:   %d\n", data->increase_start);
		fprintf(stderr, "dbg5       tvc_near:         %d\n", data->tvc_near);
		fprintf(stderr, "dbg5       increase_start:   %d\n", data->increase_start);
		fprintf(stderr, "dbg5       tvc_far:          %d\n", data->tvc_far);
		fprintf(stderr, "dbg5       increase_int_near:%d\n", data->increase_int_near);
		fprintf(stderr, "dbg5       increase_int_far: %d\n", data->increase_int_far);
		fprintf(stderr, "dbg5       gain_center:      %d\n", data->gain_center);
		fprintf(stderr, "dbg5       filter_gain:      %f\n", data->filter_gain);
		fprintf(stderr, "dbg5       amplitude_center: %d\n", data->amplitude_center);
		fprintf(stderr, "dbg5       echo_dur_center:  %d\n", data->echo_duration_center);
		fprintf(stderr, "dbg5       echo_scal_center: %d\n", data->echo_scale_center);
		fprintf(stderr, "dbg5       amplitudes and echo durations:\n");
		for (int i = 0; i < MBF_HSATLRAW_BEAMS; i++)
			fprintf(stderr, "dbg5         %d  %d  %d\n", i, data->amplitude[i], data->echo_duration[i]);
		fprintf(stderr, "dbg5       gains and echo scales:\n");
		for (int i = 0; i < 16; i++)
			fprintf(stderr, "dbg5         %d  %d  %d\n", i, data->gain[i], data->echo_scale[i]);
	}

	int status = MB_SUCCESS;

	/* write the record label */
	if (datacheck)
		status = mbr_hsatlraw_wr_label(verbose, mbfp, MBF_HSATLRAW_ERGNAMPL, error);

	/* write out the data */
	if (status == MB_SUCCESS && datacheck) {
		/* output the event line */
		fprintf(mbfp, "%+12.7f", data->lon);
		fprintf(mbfp, "%+12.7f", data->lat);
		fprintf(mbfp, "%4.4d", data->year);
		fprintf(mbfp, "%2.2d", data->month);
		fprintf(mbfp, "%2.2d", data->day);
		fprintf(mbfp, "%2.2d", data->hour);
		fprintf(mbfp, "%2.2d", data->minute);
		fprintf(mbfp, "%2.2d", data->second);
		fprintf(mbfp, "%c", data->mode[0]);
		fprintf(mbfp, "%3.3d", data->trans_strbd);
		fprintf(mbfp, "%3.3d", data->trans_vert);
		fprintf(mbfp, "%3.3d", data->trans_port);
		fprintf(mbfp, "%2.2d", data->pulse_len_strbd);
		fprintf(mbfp, "%2.2d", data->pulse_len_vert);
		fprintf(mbfp, "%2.2d", data->pulse_len_port);
		fprintf(mbfp, "%2.2d", data->gain_start);
		fprintf(mbfp, "%2.2d", data->r_compensation_factor);
		fprintf(mbfp, "%4.4d", data->compensation_start);
		fprintf(mbfp, "%5.5d", data->increase_start);
		fprintf(mbfp, "%2.2d", data->tvc_near);
		fprintf(mbfp, "%2.2d", data->tvc_far);
		fprintf(mbfp, "%3.3d", data->increase_int_near);
		fprintf(mbfp, "%3.3d", data->increase_int_far);
		fprintf(mbfp, "%1d", data->gain_center);
		fprintf(mbfp, "%+5.1f", data->filter_gain);
		fprintf(mbfp, "%3.3d", data->amplitude_center);
		fprintf(mbfp, "%3.3d", data->echo_duration_center);
		fprintf(mbfp, "%1d\n", data->echo_scale_center);

		/* output starboard amplitudes */
		for (int i = 0; i < 8; i++)
			fprintf(mbfp, "%1.1d", data->gain[i + 8]);
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%3.3d", data->amplitude[i + 30]);
		fprintf(mbfp, "\n");

		/* output port amplitudes */
		for (int i = 0; i < 8; i++)
			fprintf(mbfp, "%1.1d", data->gain[i]);
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%3.3d", data->amplitude[28 - i]);
		fprintf(mbfp, "\n");

		/* output starboard echo durations */
		for (int i = 0; i < 8; i++)
			fprintf(mbfp, "%1.1d", data->echo_scale[i + 8]);
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%3.3d", data->echo_duration[i + 30]);
		fprintf(mbfp, "\n");

		/* output port echo durations */
		for (int i = 0; i < 8; i++)
			fprintf(mbfp, "%1.1d", data->echo_scale[i]);
		fprintf(mbfp, "29");
		for (int i = 0; i < 29; i++)
			fprintf(mbfp, "%3.3d", data->echo_duration[28 - i]);
		int count = fprintf(mbfp, "\n");

		/* check for an error */
		if (count > 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
int mbr_hsatlraw_wr_ldeocmnt(int verbose, FILE *mbfp, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_hsatlraw_struct *data = (struct mbf_hsatlraw_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       comment:          %s\n", data->comment);
	}

	int status = MB_SUCCESS;

	/* write the record label */
	status = mbr_hsatlraw_wr_label(verbose, mbfp, MBF_HSATLRAW_LDEOCMNT, error);

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* output the event line */
		int count = fprintf(mbfp, "%s\n", data->comment);

		/* check for an error */
		if (count >= 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
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
