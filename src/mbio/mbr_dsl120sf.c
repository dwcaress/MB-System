/*--------------------------------------------------------------------
 *    The MB-system:	mbr_dsl120sf.c	8/6/96
 *
 *    Copyright (c) 1996-2023 by
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
 * mbr_dsl120sf.c contains the functions for reading and writing
 * multibeam data in the DSL120SF format.
 * These functions include:
 *   mbr_alm_dsl120sf	- allocate read/write memory
 *   mbr_dem_dsl120sf	- deallocate read/write memory
 *   mbr_rt_dsl120sf	- read and translate data
 *   mbr_wt_dsl120sf	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	August 6, 1996
 *
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbf_dsl120sf.h"
#include "mbsys_dsl.h"

/*--------------------------------------------------------------------*/
int mbr_info_dsl120sf(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_DSL;
	*beams_bath_max = 2048;
	*beams_amp_max = 0;
	*pixels_ss_max = 8192;
	strncpy(format_name, "DSL120SF", MB_NAME_LENGTH);
	strncpy(system_name, "DSL", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_DSL120SF\nInformal Description: WHOI DSL AMS-120 processed format\nAttributes:           "
	        "2048 beam bathymetry, 8192 pixel sidescan,\n                      binary, single files, WHOI DSL.\n",
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
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

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
int mbr_zero_dsl120sf(int verbose, char *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to data descriptor */
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)data_ptr;

	/* initialize everything to zeros */
	if (data != NULL) {

		/* initialize everything */
		data->kind = MB_DATA_NONE;
		data->rec_type = DSL_NONE;
		data->rec_len = 0;
		data->rec_hdr_len = 0;
		data->p_flags = 0;
		data->num_data_types = 0;
		data->ping = 0;
		for (int i = 0; i < 4; i++)
			data->sonar_cmd[i] = '\0';
		for (int i = 0; i < 24; i++)
			data->time_stamp[i] = '\0';
		data->nav_x = 0.0;
		data->nav_y = 0.0;
		data->depth = 0.0;
		data->heading = 0.0;
		data->pitch = 0.0;
		data->roll = 0.0;
		data->alt = 0.0;
		data->ang_offset = 0.0;
		data->transmit_pwr = 0;
		data->gain_port = 0;
		data->gain_starbd = 0;
		data->pulse_width = 0.0;
		data->swath_width = 0;
		data->side = 0;
		data->swapped = 3;
		data->tv_sec = 0;
		data->tv_usec = 0;
		data->digitalinterface = 0;
		for (int i = 0; i < 5; i++)
			data->reserved[i] = 0;
		data->bat_type = DSL_BATH;
		data->bat_len = 0;
		data->bat_hdr_len = 0;
		data->bat_num_bins = 0;
		data->bat_sampleSize = 0.0;
		data->bat_p_flags = 0;
		data->bat_max_range = 0.0;
		for (int i = 0; i < 9; i++)
			data->bat_future[i] = 0;
		for (int i = 0; i < MBSYS_DSL_MAXBEAMS_SIDE; i++) {
			data->bat_port[i] = 0.0;
			data->bat_stbd[i] = 0.0;
		}
		data->amp_type = DSL_AMP;
		data->amp_len = 0;
		data->amp_hdr_len = 0;
		data->amp_num_samp = 0;
		data->amp_sampleSize = 0.0;
		data->amp_p_flags = 0;
		data->amp_max_range = 0.0;
		data->amp_channel = 0.0;
		for (int i = 0; i < 8; i++)
			data->amp_future[i] = 0;
		for (int i = 0; i < MBSYS_DSL_MAXBEAMS_SIDE; i++) {
			data->amp_port[i] = 0.0;
			data->amp_stbd[i] = 0.0;
		}
		for (int i = 0; i < MBF_DSL120SF_COMMENT_LENGTH; i++)
			data->comment[i] = '\0';
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
int mbr_alm_dsl120sf(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_dsl120sf_struct);
	mb_io_ptr->data_structure_size = 0;
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status &= mbsys_dsl_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* initialize everything to zeros */
	mbr_zero_dsl120sf(verbose, mb_io_ptr->raw_data, error);

	/* get pointer to data descriptor */
	/* struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data; */

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
int mbr_dem_dsl120sf(int verbose, void *mbio_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->raw_data, error);
	status &= mbsys_dsl_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_dsl120sf_rd_header(int verbose, void *mbio_ptr, FILE *mbfp, int *error) {
	char buffer[124];
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data;

	/* read header */
	int status = fread(buffer, 1, 124, mbfp);
	if (status == 124) {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* translate header */
	if (status == MB_SUCCESS) {
		data->rec_type = DSL_HEADER;

		index = 0;
		mb_get_binary_int(false, &buffer[index], (int *)&data->rec_len);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->rec_hdr_len);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->p_flags);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->num_data_types);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->ping);
		index += 4;
		for (int i = 0; i < 4; i++) {
			data->sonar_cmd[i] = buffer[index];
			index++;
		}
		for (int i = 0; i < 24; i++) {
			data->time_stamp[i] = buffer[index];
			index++;
		}
		mb_get_binary_float(false, &buffer[index], &data->nav_x);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->nav_y);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->depth);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->heading);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->pitch);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->roll);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->alt);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->ang_offset);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->transmit_pwr);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->gain_port);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->gain_starbd);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->pulse_width);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->swath_width);
		index += 4;
		data->side = buffer[index];
		index++;
		data->swapped = buffer[index];
		index++;
		index += 2;
		mb_get_binary_int(false, &buffer[index], (int *)&data->tv_sec);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->tv_usec);
		index += 4;
		mb_get_binary_short(false, &buffer[index], &data->digitalinterface);
		index += 2;
		for (int i = 0; i < 5; i++) {
			mb_get_binary_short(false, &buffer[index], &data->reserved[i]);
			index += 2;
		}
	}

	if (verbose >= 5 && status == MB_SUCCESS) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       rec_type:         %d\n", data->rec_type);
		fprintf(stderr, "dbg5       rec_len:          %d\n", data->rec_len);
		fprintf(stderr, "dbg5       rec_hdr_len:      %d\n", data->rec_hdr_len);
		fprintf(stderr, "dbg5       p_flags:          %d\n", data->p_flags);
		fprintf(stderr, "dbg5       num_data_types:   %d\n", data->num_data_types);
		fprintf(stderr, "dbg5       ping:             %d\n", data->ping);
		fprintf(stderr, "dbg5       sonar_cmd:        %c%c%c%c\n", data->sonar_cmd[0], data->sonar_cmd[1], data->sonar_cmd[2],
		        data->sonar_cmd[3]);
		fprintf(stderr, "dbg5       time_stamp:       ");
		for (int i = 0; i < 24; i++)
			fprintf(stderr, "%c", data->time_stamp[i]);
		fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       nav_x:            %f\n", data->nav_x);
		fprintf(stderr, "dbg5       nav_y:            %f\n", data->nav_y);
		fprintf(stderr, "dbg5       depth:            %f\n", data->depth);
		fprintf(stderr, "dbg5       heading:          %f\n", data->heading);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       roll:             %f\n", data->roll);
		fprintf(stderr, "dbg5       alt:              %f\n", data->alt);
		fprintf(stderr, "dbg5       ang_offset:       %f\n", data->ang_offset);
		fprintf(stderr, "dbg5       transmit_pwr:     %d\n", data->transmit_pwr);
		fprintf(stderr, "dbg5       gain_port:        %d\n", data->gain_port);
		fprintf(stderr, "dbg5       gain_starbd:      %d\n", data->gain_starbd);
		fprintf(stderr, "dbg5       pulse_width:      %f\n", data->pulse_width);
		fprintf(stderr, "dbg5       swath_width:      %d\n", data->swath_width);
		fprintf(stderr, "dbg5       side:             %c\n", data->side);
		fprintf(stderr, "dbg5       swapped:          %c\n", data->swapped);
		fprintf(stderr, "dbg5       tv_sec:           %d\n", data->tv_sec);
		fprintf(stderr, "dbg5       tv_usec:          %d\n", data->tv_usec);
		fprintf(stderr, "dbg5       digitalinterface: %d\n", data->digitalinterface);
		for (int i = 0; i < 5; i++)
			fprintf(stderr, "dbg5       reserved:         %d\n", data->reserved[i]);
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
int mbr_dsl120sf_rd_dataheader(int verbose, void *mbio_ptr, FILE *mbfp, char *type, int *len, int *hdr_len, int *error) {
	char buffer[12];
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
	}

	/* read header */
	int status = fread(buffer, 1, 12, mbfp);
	if (status == 12) {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* translate header */
	if (status == MB_SUCCESS) {
		strncpy(type, buffer, 4);
		index = 4;
		mb_get_binary_int(false, &buffer[index], len);
		index += 4;
		mb_get_binary_int(false, &buffer[index], hdr_len);
		index += 4;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       type:      %s\n", type);
		fprintf(stderr, "dbg2       len:       %d\n", *len);
		fprintf(stderr, "dbg2       hdr_len:   %d\n", *hdr_len);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_dsl120sf_rd_bath(int verbose, void *mbio_ptr, FILE *mbfp, int *error) {
	int read_bytes;
	char buffer[10000];
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data;

	/* read bath record */
	read_bytes = data->bat_len - 12;
	int status = fread(buffer, 1, read_bytes, mbfp);
	if (status == read_bytes) {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* translate header and data */
	if (status == MB_SUCCESS) {
		index = 0;
		mb_get_binary_int(false, &buffer[index], (int *)&data->bat_num_bins);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->bat_sampleSize);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->bat_p_flags);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->bat_max_range);
		index += 4;
		for (int i = 0; i < 9; i++) {
			mb_get_binary_int(false, &buffer[index], &data->bat_future[i]);
			index += 4;
		}
		for (int i = 0; i < data->bat_num_bins; i++) {
			mb_get_binary_float(false, &buffer[index], &data->bat_port[i]);
			index += 4;
			mb_get_binary_float(false, &buffer[index], &data->bat_stbd[i]);
			index += 4;
		}
	}

	if (verbose >= 5 && status == MB_SUCCESS) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       bat_type:         %d\n", data->bat_type);
		fprintf(stderr, "dbg5       bat_len:          %d\n", data->bat_len);
		fprintf(stderr, "dbg5       bat_hdr_len:      %d\n", data->bat_hdr_len);
		fprintf(stderr, "dbg5       bat_num_bins:     %d\n", data->bat_num_bins);
		fprintf(stderr, "dbg5       bat_sampleSize:   %f\n", data->bat_sampleSize);
		fprintf(stderr, "dbg5       bat_p_flags:      %d\n", data->bat_p_flags);
		fprintf(stderr, "dbg5       bat_max_range:    %f\n", data->bat_max_range);
		for (int i = 0; i < 9; i++)
			fprintf(stderr, "dbg5       bat_future:       %d\n", data->bat_future[i]);
		for (int i = 0; i < data->bat_num_bins; i++)
			fprintf(stderr, "dbg5       bath[%d]:         %f\t%f\n", i, data->bat_port[i], data->bat_stbd[i]);
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
int mbr_dsl120sf_rd_amp(int verbose, void *mbio_ptr, FILE *mbfp, int *error) {
	int read_bytes;
	char buffer[10000];
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data;

	/* read amp record */
	read_bytes = data->amp_len - 12;
	int status = fread(buffer, 1, read_bytes, mbfp);
	if (status == read_bytes) {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* translate header and data */
	if (status == MB_SUCCESS) {
		index = 0;
		mb_get_binary_int(false, &buffer[index], (int *)&data->amp_num_samp);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->amp_sampleSize);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->amp_p_flags);
		index += 4;
		mb_get_binary_float(false, &buffer[index], &data->amp_max_range);
		index += 4;
		mb_get_binary_int(false, &buffer[index], (int *)&data->amp_channel);
		index += 4;
		for (int i = 0; i < 8; i++) {
			mb_get_binary_int(false, &buffer[index], &data->amp_future[i]);
			index += 4;
		}
		for (int i = 0; i < data->bat_num_bins; i++) {
			mb_get_binary_float(false, &buffer[index], &data->amp_port[i]);
			index += 4;
			mb_get_binary_float(false, &buffer[index], &data->amp_stbd[i]);
			index += 4;
		}
	}

	if (verbose >= 5 && status == MB_SUCCESS) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       amp_type:         %d\n", data->amp_type);
		fprintf(stderr, "dbg5       amp_len:          %d\n", data->amp_len);
		fprintf(stderr, "dbg5       amp_hdr_len:      %d\n", data->amp_hdr_len);
		fprintf(stderr, "dbg5       amp_num_samp:     %d\n", data->amp_num_samp);
		fprintf(stderr, "dbg5       amp_sampleSize:   %f\n", data->amp_sampleSize);
		fprintf(stderr, "dbg5       amp_p_flags:      %d\n", data->amp_p_flags);
		fprintf(stderr, "dbg5       amp_max_range:    %f\n", data->amp_max_range);
		fprintf(stderr, "dbg5       amp_channel:      %d\n", data->amp_channel);
		for (int i = 0; i < 8; i++)
			fprintf(stderr, "dbg5       amp_future:       %d\n", data->amp_future[i]);
		for (int i = 0; i < data->amp_num_samp; i++)
			fprintf(stderr, "dbg5       amp[%d]:          %f\t%f\n", i, data->amp_port[i], data->amp_stbd[i]);
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
int mbr_dsl120sf_rd_comment(int verbose, void *mbio_ptr, FILE *mbfp, int *error) {
	int read_bytes;
	char buffer[80];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data;

	/* read comment record */
	read_bytes = 80;
	int status = fread(buffer, 1, read_bytes, mbfp);
	if (status == read_bytes) {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* copy comment */
	if (status == MB_SUCCESS) {
		strncpy(data->comment, buffer, 79);
		data->comment[79] = '\0';
	}

	if (verbose >= 5 && status == MB_SUCCESS) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
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
int mbr_dsl120sf_rd_data(int verbose, void *mbio_ptr, int *error) {
	char tag[5];
	char type[5];
	int len;
	int hdr_len;
	int found;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data;

	int status = MB_SUCCESS;

	/* read data */
	if (mb_io_ptr->mbfp != NULL) {
		/* read next four bytes */
		found = false;
		status = fread(tag, 1, 4, mb_io_ptr->mbfp);
		if (status == 4)
			status = MB_SUCCESS;
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}

		/* if tag not found read single bytes until found
		    or end of file */
		while (!found && status == MB_SUCCESS) {
			/* look for "DSL " tag at start of record */
			if (strncmp(tag, "DSL ", 4) == 0)
				found = true;

			/* read next byte */
			if (!found) {
				for (int i = 0; i < 3; i++)
					tag[i] = tag[i + 1];
				status = fread(&tag[3], 1, 1, mb_io_ptr->mbfp);
				if (status == 1)
					status = MB_SUCCESS;
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
			}
		}

		/* now read the rest of the header */
		if (status == MB_SUCCESS)
			status = mbr_dsl120sf_rd_header(verbose, mbio_ptr, mb_io_ptr->mbfp, error);

		/* now read each of the data records */
		if (status == MB_SUCCESS)
			for (int i = 0; i < data->num_data_types; i++) {
				status = mbr_dsl120sf_rd_dataheader(verbose, mbio_ptr, mb_io_ptr->mbfp, type, &len, &hdr_len, error);

				if (status == MB_SUCCESS && strncmp(type, "BATH", 4) == 0) {
					data->bat_len = len;
					data->bat_hdr_len = hdr_len;
					status = mbr_dsl120sf_rd_bath(verbose, mbio_ptr, mb_io_ptr->mbfp, error);
					if (status == MB_SUCCESS)
						data->kind = MB_DATA_DATA;
				}
				else if (status == MB_SUCCESS && strncmp(type, "AMP ", 4) == 0) {
					data->amp_len = len;
					data->amp_hdr_len = hdr_len;
					status = mbr_dsl120sf_rd_amp(verbose, mbio_ptr, mb_io_ptr->mbfp, error);
					if (status == MB_SUCCESS)
						data->kind = MB_DATA_DATA;
				}
				else if (status == MB_SUCCESS && strncmp(type, "COMM", 4) == 0) {
					status = mbr_dsl120sf_rd_comment(verbose, mbio_ptr, mb_io_ptr->mbfp, error);
					if (status == MB_SUCCESS)
						data->kind = MB_DATA_COMMENT;
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
int mbr_rt_dsl120sf(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data;
	struct mbsys_dsl_struct *store = (struct mbsys_dsl_struct *)store_ptr;

	/* read next data from file */
	const int status = mbr_dsl120sf_rd_data(verbose, mbio_ptr, error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate values to dsl data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		store->kind = data->kind;
		store->rec_type = data->rec_type;
		store->rec_len = data->rec_len;
		store->rec_hdr_len = data->rec_hdr_len;
		store->p_flags = data->p_flags;
		store->num_data_types = data->num_data_types;
		store->ping = data->ping;
		for (int i = 0; i < 4; i++)
			store->sonar_cmd[i] = data->sonar_cmd[i];
		for (int i = 0; i < 24; i++)
			store->time_stamp[i] = data->time_stamp[i];
		store->nav_x = data->nav_x;
		store->nav_y = data->nav_y;
		store->depth = data->depth;
		store->heading = data->heading;
		store->pitch = data->pitch;
		store->roll = data->roll;
		store->alt = data->alt;
		store->ang_offset = data->ang_offset;
		store->transmit_pwr = data->transmit_pwr;
		store->gain_port = data->gain_port;
		store->gain_starbd = data->gain_starbd;
		store->pulse_width = data->pulse_width;
		store->swath_width = data->swath_width;
		store->side = data->side;
		store->swapped = data->swapped;
		store->tv_sec = data->tv_sec;
		store->tv_usec = data->tv_usec;
		store->digitalinterface = data->digitalinterface;
		for (int i = 0; i < 5; i++)
			store->reserved[i] = data->reserved[i];
		store->bat_type = data->bat_type;
		store->bat_len = data->bat_len;
		store->bat_hdr_len = data->bat_hdr_len;
		store->bat_num_bins = data->bat_num_bins;
		store->bat_sampleSize = data->bat_sampleSize;
		store->bat_p_flags = data->bat_p_flags;
		store->bat_max_range = data->bat_max_range;
		for (int i = 0; i < 9; i++)
			store->bat_future[i] = data->bat_future[i];
		for (int i = 0; i < MBSYS_DSL_MAXBEAMS_SIDE; i++) {
			store->bat_port[i] = data->bat_port[i];
			store->bat_stbd[i] = data->bat_stbd[i];
		}
		store->amp_type = data->amp_type;
		store->amp_len = data->amp_len;
		store->amp_hdr_len = data->amp_hdr_len;
		store->amp_num_samp = data->amp_num_samp;
		store->amp_sampleSize = data->amp_sampleSize;
		store->amp_p_flags = data->amp_p_flags;
		store->amp_max_range = data->amp_max_range;
		store->amp_channel = data->amp_channel;
		for (int i = 0; i < 8; i++)
			store->amp_future[i] = data->amp_future[i];
		for (int i = 0; i < MBSYS_DSL_MAXBEAMS_SIDE; i++) {
			store->amp_port[i] = data->amp_port[i];
			store->amp_stbd[i] = data->amp_stbd[i];
		}
		strncpy(store->comment, data->comment, MBSYS_DSL_COMMENT_LENGTH - 1);
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
int mbr_dsl120sf_wr_bathamp(int verbose, void *mbio_ptr, FILE *mbfp, int *error) {
	char buffer[17000];
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to write in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       rec_type:         %d\n", data->rec_type);
		fprintf(stderr, "dbg5       rec_len:          %d\n", data->rec_len);
		fprintf(stderr, "dbg5       rec_hdr_len:      %d\n", data->rec_hdr_len);
		fprintf(stderr, "dbg5       p_flags:          %d\n", data->p_flags);
		fprintf(stderr, "dbg5       num_data_types:   %d\n", data->num_data_types);
		fprintf(stderr, "dbg5       ping:             %d\n", data->ping);
		fprintf(stderr, "dbg5       sonar_cmd:        %c%c%c%c\n", data->sonar_cmd[0], data->sonar_cmd[1], data->sonar_cmd[2],
		        data->sonar_cmd[3]);
		fprintf(stderr, "dbg5       time_stamp:       ");
		for (int i = 0; i < 24; i++)
			fprintf(stderr, "%c", data->time_stamp[i]);
		fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       nav_x:            %f\n", data->nav_x);
		fprintf(stderr, "dbg5       nav_y:            %f\n", data->nav_y);
		fprintf(stderr, "dbg5       depth:            %f\n", data->depth);
		fprintf(stderr, "dbg5       heading:          %f\n", data->heading);
		fprintf(stderr, "dbg5       pitch:            %f\n", data->pitch);
		fprintf(stderr, "dbg5       roll:             %f\n", data->roll);
		fprintf(stderr, "dbg5       alt:              %f\n", data->alt);
		fprintf(stderr, "dbg5       ang_offset:       %f\n", data->ang_offset);
		fprintf(stderr, "dbg5       transmit_pwr:     %d\n", data->transmit_pwr);
		fprintf(stderr, "dbg5       gain_port:        %d\n", data->gain_port);
		fprintf(stderr, "dbg5       gain_starbd:      %d\n", data->gain_starbd);
		fprintf(stderr, "dbg5       pulse_width:      %f\n", data->pulse_width);
		fprintf(stderr, "dbg5       swath_width:      %d\n", data->swath_width);
		fprintf(stderr, "dbg5       side:             %c\n", data->side);
		fprintf(stderr, "dbg5       swapped:          %c\n", data->swapped);
		fprintf(stderr, "dbg5       tv_sec:           %d\n", data->tv_sec);
		fprintf(stderr, "dbg5       tv_usec:          %d\n", data->tv_usec);
		fprintf(stderr, "dbg5       digitalinterface: %d\n", data->digitalinterface);
		for (int i = 0; i < 5; i++)
			fprintf(stderr, "dbg5       reserved:         %d\n", data->reserved[i]);
		fprintf(stderr, "dbg5       bat_type:         %d\n", data->bat_type);
		fprintf(stderr, "dbg5       bat_len:          %d\n", data->bat_len);
		fprintf(stderr, "dbg5       bat_hdr_len:      %d\n", data->bat_hdr_len);
		fprintf(stderr, "dbg5       bat_num_bins:     %d\n", data->bat_num_bins);
		fprintf(stderr, "dbg5       bat_sampleSize:   %f\n", data->bat_sampleSize);
		fprintf(stderr, "dbg5       bat_p_flags:      %d\n", data->bat_p_flags);
		fprintf(stderr, "dbg5       bat_max_range:    %f\n", data->bat_max_range);
		for (int i = 0; i < 9; i++)
			fprintf(stderr, "dbg5       bat_future:       %d\n", data->bat_future[i]);
		for (int i = 0; i < data->bat_num_bins; i++)
			fprintf(stderr, "dbg5       bath[%d]:         %f\t%f\n", i, data->bat_port[i], data->bat_stbd[i]);
		fprintf(stderr, "dbg5       amp_type:         %d\n", data->amp_type);
		fprintf(stderr, "dbg5       amp_len:          %d\n", data->amp_len);
		fprintf(stderr, "dbg5       amp_hdr_len:      %d\n", data->amp_hdr_len);
		fprintf(stderr, "dbg5       amp_num_samp:     %d\n", data->amp_num_samp);
		fprintf(stderr, "dbg5       amp_sampleSize:   %f\n", data->amp_sampleSize);
		fprintf(stderr, "dbg5       amp_p_flags:      %d\n", data->amp_p_flags);
		fprintf(stderr, "dbg5       amp_max_range:    %f\n", data->amp_max_range);
		fprintf(stderr, "dbg5       amp_channel:      %d\n", data->amp_channel);
		for (int i = 0; i < 8; i++)
			fprintf(stderr, "dbg5       amp_future:       %d\n", data->amp_future[i]);
		for (int i = 0; i < data->amp_num_samp; i++)
			fprintf(stderr, "dbg5       amp[%d]:          %f\t%f\n", i, data->amp_port[i], data->amp_stbd[i]);
	}

	/* make sure both bath and amp are included */
	data->num_data_types = 2;
	data->rec_len = data->rec_hdr_len + data->bat_len + data->bat_hdr_len + data->amp_len + data->amp_hdr_len;

	/* construct header record */
	index = 0;
	mb_put_binary_int(false, DSL_HEADER, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->rec_len, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->rec_hdr_len, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->p_flags, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->num_data_types, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->ping, &buffer[index]);
	index += 4;
	for (int i = 0; i < 4; i++) {
		buffer[index] = data->sonar_cmd[i];
		index++;
	}
	for (int i = 0; i < 24; i++) {
		buffer[index] = data->time_stamp[i];
		index++;
	}
	mb_put_binary_float(false, data->nav_x, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->nav_y, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->depth, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->heading, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->pitch, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->roll, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->alt, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->ang_offset, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->transmit_pwr, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->gain_port, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->gain_starbd, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->pulse_width, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->swath_width, &buffer[index]);
	index += 4;
	buffer[index] = data->side;
	index++;
	buffer[index] = data->swapped;
	index++;
	index += 2;
	mb_put_binary_int(false, data->tv_sec, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->tv_usec, &buffer[index]);
	index += 4;
	mb_put_binary_short(false, data->digitalinterface, &buffer[index]);
	index += 2;
	for (int i = 0; i < 5; i++) {
		mb_put_binary_short(false, data->reserved[i], &buffer[index]);
		index += 2;
	}

	/* construct bathymetry record */
	mb_put_binary_int(false, data->bat_type, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->bat_len, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->bat_hdr_len, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->bat_num_bins, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->bat_sampleSize, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->bat_p_flags, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->bat_max_range, &buffer[index]);
	index += 4;
	for (int i = 0; i < 9; i++) {
		mb_put_binary_int(false, data->bat_future[i], &buffer[index]);
		index += 4;
	}
	for (int i = 0; i < data->bat_num_bins; i++) {
		mb_put_binary_float(false, data->bat_port[i], &buffer[index]);
		index += 4;
		mb_put_binary_float(false, data->bat_stbd[i], &buffer[index]);
		index += 4;
	}

	/* construct amplitude record */
	mb_put_binary_int(false, data->amp_type, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->amp_len, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->amp_hdr_len, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->amp_num_samp, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->amp_sampleSize, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->amp_p_flags, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->amp_max_range, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->amp_channel, &buffer[index]);
	index += 4;
	for (int i = 0; i < 8; i++) {
		mb_put_binary_int(false, data->amp_future[i], &buffer[index]);
		index += 4;
	}
	for (int i = 0; i < data->bat_num_bins; i++) {
		mb_put_binary_float(false, data->amp_port[i], &buffer[index]);
		index += 4;
		mb_put_binary_float(false, data->amp_stbd[i], &buffer[index]);
		index += 4;
	}

	/* write the record */
	int status = fwrite(buffer, 1, data->rec_len, mbfp);
	if (status != data->rec_len) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

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
int mbr_dsl120sf_wr_comment(int verbose, void *mbio_ptr, FILE *mbfp, int *error) {
	char buffer[10000];
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       comment:          %s\n", data->comment);
	}

	/* set record and header sizes */
	data->num_data_types = 1;
	data->rec_len = 128 + 12 + 80;
	data->rec_hdr_len = 128;

	index = 0;
	mb_put_binary_int(false, DSL_HEADER, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->rec_len, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->rec_hdr_len, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->p_flags, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->num_data_types, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->ping, &buffer[index]);
	index += 4;
	for (int i = 0; i < 4; i++) {
		buffer[index] = data->sonar_cmd[i];
		index++;
	}
	for (int i = 0; i < 24; i++) {
		buffer[index] = data->time_stamp[i];
		index++;
	}
	mb_put_binary_float(false, data->nav_x, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->nav_y, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->depth, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->heading, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->pitch, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->roll, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->alt, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->ang_offset, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->transmit_pwr, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->gain_port, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->gain_starbd, &buffer[index]);
	index += 4;
	mb_put_binary_float(false, data->pulse_width, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->swath_width, &buffer[index]);
	index += 4;
	buffer[index] = data->side;
	index++;
	buffer[index] = data->swapped;
	index++;
	index += 2;
	mb_put_binary_int(false, data->tv_sec, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, data->tv_usec, &buffer[index]);
	index += 4;
	mb_put_binary_short(false, data->digitalinterface, &buffer[index]);
	index += 2;
	for (int i = 0; i < 5; i++) {
		mb_put_binary_short(false, data->reserved[i], &buffer[index]);
		index += 2;
	}

	/* construct comment record */
	mb_put_binary_int(false, DSL_COMMENT, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, 12 + 80, &buffer[index]);
	index += 4;
	mb_put_binary_int(false, 12, &buffer[index]);
	index += 4;
	strncpy(&buffer[index], data->comment, 79);
	index += 79;
	buffer[index] = '\0';
	index++;

	/* write the record */
	int status = fwrite(buffer, 1, data->rec_len, mbfp);
	if (status != data->rec_len) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
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
int mbr_dsl120sf_wr_data(int verbose, void *mbio_ptr, char *data_ptr, int *error) {
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
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)data_ptr;

	int status = MB_SUCCESS;

	if (data->kind == MB_DATA_COMMENT) {
		status = mbr_dsl120sf_wr_comment(verbose, mbio_ptr, mb_io_ptr->mbfp, error);
	}
	else if (data->kind == MB_DATA_DATA) {
		status = mbr_dsl120sf_wr_bathamp(verbose, mbio_ptr, mb_io_ptr->mbfp, error);
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
int mbr_wt_dsl120sf(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_dsl120sf_struct *data = (struct mbf_dsl120sf_struct *)mb_io_ptr->raw_data;
	char *data_ptr = (char *)data;
	struct mbsys_dsl_struct *store = (struct mbsys_dsl_struct *)store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL) {
		data->kind = store->kind;
		data->rec_type = store->rec_type;
		data->rec_len = store->rec_len;
		data->rec_hdr_len = store->rec_hdr_len;
		data->p_flags = store->p_flags;
		data->num_data_types = store->num_data_types;
		data->ping = store->ping;
		for (int i = 0; i < 4; i++)
			data->sonar_cmd[i] = store->sonar_cmd[i];
		for (int i = 0; i < 24; i++)
			data->time_stamp[i] = store->time_stamp[i];
		data->nav_x = store->nav_x;
		data->nav_y = store->nav_y;
		data->depth = store->depth;
		data->heading = store->heading;
		data->pitch = store->pitch;
		data->roll = store->roll;
		data->alt = store->alt;
		data->ang_offset = store->ang_offset;
		data->transmit_pwr = store->transmit_pwr;
		data->gain_port = store->gain_port;
		data->gain_starbd = store->gain_starbd;
		data->pulse_width = store->pulse_width;
		data->swath_width = store->swath_width;
		data->side = store->side;
		data->swapped = store->swapped;
		data->tv_sec = store->tv_sec;
		data->tv_usec = store->tv_usec;
		data->digitalinterface = store->digitalinterface;
		for (int i = 0; i < 5; i++)
			data->reserved[i] = store->reserved[i];
		data->bat_type = store->bat_type;
		data->bat_len = store->bat_len;
		data->bat_hdr_len = store->bat_hdr_len;
		data->bat_num_bins = store->bat_num_bins;
		data->bat_sampleSize = store->bat_sampleSize;
		data->bat_p_flags = store->bat_p_flags;
		data->bat_max_range = store->bat_max_range;
		for (int i = 0; i < 9; i++)
			data->bat_future[i] = store->bat_future[i];
		for (int i = 0; i < MBSYS_DSL_MAXBEAMS_SIDE; i++) {
			data->bat_port[i] = store->bat_port[i];
			data->bat_stbd[i] = store->bat_stbd[i];
		}
		data->amp_type = store->amp_type;
		data->amp_len = store->amp_len;
		data->amp_hdr_len = store->amp_hdr_len;
		data->amp_num_samp = store->amp_num_samp;
		data->amp_sampleSize = store->amp_sampleSize;
		data->amp_p_flags = store->amp_p_flags;
		data->amp_max_range = store->amp_max_range;
		data->amp_channel = store->amp_channel;
		for (int i = 0; i < 8; i++)
			data->amp_future[i] = store->amp_future[i];
		for (int i = 0; i < MBSYS_DSL_MAXBEAMS_SIDE; i++) {
			data->amp_port[i] = store->amp_port[i];
			data->amp_stbd[i] = store->amp_stbd[i];
		}
		strncpy(data->comment, store->comment, MBF_DSL120SF_COMMENT_LENGTH - 1);
	}

	/* write next data to file */
	const int status = mbr_dsl120sf_wr_data(verbose, mbio_ptr, data_ptr, error);

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
int mbr_register_dsl120sf(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_dsl120sf(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_dsl120sf;
	mb_io_ptr->mb_io_format_free = &mbr_dem_dsl120sf;
	mb_io_ptr->mb_io_store_alloc = &mbsys_dsl_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_dsl_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_dsl120sf;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_dsl120sf;
	mb_io_ptr->mb_io_dimensions = &mbsys_dsl_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_dsl_extract;
	mb_io_ptr->mb_io_insert = &mbsys_dsl_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_dsl_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_dsl_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_dsl_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_dsl_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_dsl_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_dsl_copy;
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
