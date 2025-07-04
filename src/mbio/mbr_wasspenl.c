/*--------------------------------------------------------------------
 *    The MB-system:	mbr_wasspenl.c	1/27/2014
 *
 *    Copyright (c) 2014-2025 by
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
 * mbr_wasspenl.c contains the functions for reading and writing
 * multibeam data in the WASSPENL format.
 * These functions include:
 *   mbr_alm_wasspenl	- allocate read/write memory
 *   mbr_dem_wasspenl	- deallocate read/write memory
 *   mbr_rt_wasspenl	- read and translate data
 *   mbr_wt_wasspenl	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	January 27, 2014
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
#include "mbsys_wassp.h"

/* turn on debug statements here */
/* #define MBR_WASSPENLDEBUG 1 */

/*--------------------------------------------------------------------*/
int mbr_info_wasspenl(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_WASSP;
	*beams_bath_max = MBSYS_WASSP_MAX_BEAMS;
	*beams_amp_max = MBSYS_WASSP_MAX_BEAMS;
	*pixels_ss_max = MBSYS_WASSP_MAX_PIXELS;
	strncpy(format_name, "WASSPENL", MB_NAME_LENGTH);
	strncpy(system_name, "WASSP", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_WASSPENL\nInformal Description: WASSP Multibeam Vendor Format\nAttributes:           "
	        "WASSP multibeams, \n                      bathymetry and amplitude,\n		      122 or 244 beams, binary, "
	        "Electronic Navigation Ltd.\n",
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
	*beamwidth_xtrack = 4.0;
	*beamwidth_ltrack = 4.0;

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
int mbr_alm_wasspenl(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	int status = mbsys_wassp_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* allocate starting memory for data record buffer */
	char **bufferptr = (char **)&mb_io_ptr->saveptr1;
	// char *buffer = (char *)*bufferptr;
	unsigned int *bufferalloc = (unsigned int *)&mb_io_ptr->save6;
	*bufferptr = NULL;
	*bufferalloc = 0;
	if (status == MB_SUCCESS) {
		status = mb_reallocd(verbose, __FILE__, __LINE__, MBSYS_WASSP_BUFFER_STARTSIZE, (void **)bufferptr, error);
		if (status == MB_SUCCESS)
			*bufferalloc = MBSYS_WASSP_BUFFER_STARTSIZE;
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
int mbr_dem_wasspenl(int verbose, void *mbio_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for reading/writing buffer */
	char **bufferptr = (char **)&mb_io_ptr->saveptr1;
	// char *buffer = (char *)*bufferptr;
	unsigned int *bufferalloc = (unsigned int *)&mb_io_ptr->save6;
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)bufferptr, error);
	*bufferalloc = 0;

	/* deallocate memory for data descriptor */
	status &= mbsys_wassp_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_wasspenl_rd_genbathy(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_genbathy_struct *genbathy = &(store->genbathy);

	/* extract the data */
	int index = 16;
	mb_get_binary_int(true, &buffer[index], &(genbathy->version));
	index += 4;
	mb_get_binary_double(true, &buffer[index], &(genbathy->msec));
	index += 8;
	genbathy->day = buffer[index];
	index++;
	genbathy->month = buffer[index];
	index++;
	mb_get_binary_short(true, &buffer[index], &(genbathy->year));
	index += 2;
	mb_get_binary_int(true, &buffer[index], &(genbathy->ping_number));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(genbathy->sonar_model));
	index += 4;
	mb_get_binary_long(true, &buffer[index], &(genbathy->transducer_serial_number));
	index += 8;
	mb_get_binary_int(true, &buffer[index], &(genbathy->number_beams));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(genbathy->modeflags));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(genbathy->sampling_frequency));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(genbathy->acoustic_frequency));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(genbathy->tx_power));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(genbathy->pulse_width));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(genbathy->absorption_loss));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(genbathy->spreading_loss));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(genbathy->sample_type));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(genbathy->sound_velocity));
	index += 4;
	for (unsigned int i = 0; i < genbathy->number_beams; i++) {
		mb_get_binary_float(true, &buffer[index], &(genbathy->detection_point[i]));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(genbathy->rx_angle[i]));
		index += 4;
		mb_get_binary_int(true, &buffer[index], &(genbathy->flags[i]));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(genbathy->backscatter[i]));
		index += 4;
	}
	mb_get_binary_int(true, &buffer[index], &(genbathy->checksum));
	index += 4;

	int status = MB_SUCCESS;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_DATA;

		/* get the time */
		store->time_i[0] = genbathy->year;
		store->time_i[1] = genbathy->month;
		store->time_i[2] = genbathy->day;
		store->time_i[3] = (int)floor(genbathy->msec / 3600000.0);
		store->time_i[4] = (int)floor((genbathy->msec - 3600000.0 * store->time_i[3]) / 60000.0);
		store->time_i[5] = (int)floor((genbathy->msec - 3600000.0 * store->time_i[3] - 60000.0 * store->time_i[4]) / 1000.0);
		;
		store->time_i[6] =
		    (int)((genbathy->msec - 3600000.0 * store->time_i[3] - 60000.0 * store->time_i[4] - 1000.0 * store->time_i[5]) *
		          1000.0);
		;
		mb_get_time(verbose, store->time_i, &(store->time_d));
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       genbathy->version:                    %u\n", genbathy->version);
		fprintf(stderr, "dbg5       genbathy->msec:                       %f\n", genbathy->msec);
		fprintf(stderr, "dbg5       genbathy->day:                        %u\n", genbathy->day);
		fprintf(stderr, "dbg5       genbathy->month:                      %u\n", genbathy->month);
		fprintf(stderr, "dbg5       genbathy->year:                       %u\n", genbathy->year);
		fprintf(stderr, "dbg5       genbathy->ping_number:                %u\n", genbathy->ping_number);
		fprintf(stderr, "dbg5       genbathy->sonar_model:                %u\n", genbathy->sonar_model);
		fprintf(stderr, "dbg5       genbathy->transducer_serial_number:   %lu\n", genbathy->transducer_serial_number);
		fprintf(stderr, "dbg5       genbathy->number_beams:               %u\n", genbathy->number_beams);
		fprintf(stderr, "dbg5       genbathy->modeflags:                  %u\n", genbathy->modeflags);
		fprintf(stderr, "dbg5       genbathy->sampling_frequency:         %f\n", genbathy->sampling_frequency);
		fprintf(stderr, "dbg5       genbathy->acoustic_frequency:         %f\n", genbathy->acoustic_frequency);
		fprintf(stderr, "dbg5       genbathy->tx_power:                   %f\n", genbathy->tx_power);
		fprintf(stderr, "dbg5       genbathy->pulse_width:                %f\n", genbathy->pulse_width);
		fprintf(stderr, "dbg5       genbathy->absorption_loss:            %f\n", genbathy->absorption_loss);
		fprintf(stderr, "dbg5       genbathy->spreading_loss:             %f\n", genbathy->spreading_loss);
		fprintf(stderr, "dbg5       genbathy->sample_type:                %u\n", genbathy->sample_type);
		fprintf(stderr, "dbg5       genbathy->sound_velocity:             %f\n", genbathy->sound_velocity);
		for (unsigned int i = 0; i < genbathy->number_beams; i++) {
			fprintf(stderr, "dbg5       genbathy->detection_point[%3d]:       %f\n", i, genbathy->detection_point[i]);
			fprintf(stderr, "dbg5       genbathy->rx_angle[%3d]:              %f\n", i, genbathy->rx_angle[i]);
			fprintf(stderr, "dbg5       genbathy->flags[%3d]:                 %u\n", i, genbathy->flags[i]);
			fprintf(stderr, "dbg5       genbathy->backscatter[%3d]:           %f\n", i, genbathy->backscatter[i]);
		}
		fprintf(stderr, "dbg5       genbathy->checksum:                   %u\n", genbathy->checksum);
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
int mbr_wasspenl_rd_corbathy(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_corbathy_struct *corbathy = &(store->corbathy);

	/* extract the data */
	int index = 16;
	mb_get_binary_int(true, &buffer[index], &(corbathy->version));
	index += 4;
	mb_get_binary_double(true, &buffer[index], &(corbathy->msec));
	index += 8;
	mb_get_binary_int(true, &buffer[index], &(corbathy->num_beams));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(corbathy->ping_number));
	index += 4;
	mb_get_binary_double(true, &buffer[index], &(corbathy->latitude));
	index += 8;
	mb_get_binary_double(true, &buffer[index], &(corbathy->longitude));
	index += 8;
	mb_get_binary_float(true, &buffer[index], &(corbathy->bearing));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(corbathy->roll));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(corbathy->pitch));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(corbathy->heave));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(corbathy->sample_type));
	index += 4;
	for (int i = 0; i < 5; i++) {
		mb_get_binary_int(true, &buffer[index], &(corbathy->spare[i]));
		index += 4;
	}
	for (unsigned int i = 0; i < corbathy->num_beams; i++) {
		mb_get_binary_int(true, &buffer[index], &(corbathy->beam_index[i]));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(corbathy->x[i]));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(corbathy->y[i]));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(corbathy->z[i]));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(corbathy->beam_angle[i]));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(corbathy->backscatter[i]));
		index += 4;
		corbathy->quality[i] = buffer[index];
		index++;
		corbathy->fish[i] = buffer[index];
		index++;
		corbathy->roughness[i] = buffer[index];
		index++;
		corbathy->empty[i] = buffer[index];
		index++;
		mb_get_binary_int(true, &buffer[index], &(corbathy->pad[i]));
		index += 4;
	}
	mb_get_binary_int(true, &buffer[index], &(corbathy->checksum));
	index += 4;

	int status = MB_SUCCESS;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_DATA;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       corbathy->version:                    %u\n", corbathy->version);
		fprintf(stderr, "dbg5       corbathy->msec:                       %f\n", corbathy->msec);
		fprintf(stderr, "dbg5       corbathy->num_beams:                  %u\n", corbathy->num_beams);
		fprintf(stderr, "dbg5       corbathy->ping_number:                %u\n", corbathy->ping_number);
		fprintf(stderr, "dbg5       corbathy->latitude:                   %f\n", corbathy->latitude);
		fprintf(stderr, "dbg5       corbathy->longitude:                  %f\n", corbathy->longitude);
		fprintf(stderr, "dbg5       corbathy->bearing:                    %f\n", corbathy->bearing);
		fprintf(stderr, "dbg5       corbathy->roll:                       %f\n", corbathy->roll);
		fprintf(stderr, "dbg5       corbathy->pitch:                      %f\n", corbathy->pitch);
		fprintf(stderr, "dbg5       corbathy->heave:                      %f\n", corbathy->heave);
		fprintf(stderr, "dbg5       corbathy->sample_type:                %u\n", corbathy->sample_type);
		for (int i = 0; i < 5; i++) {
			fprintf(stderr, "dbg5       corbathy->spare[%3d]:                 %u\n", i, corbathy->spare[i]);
		}
		for (unsigned int i = 0; i < corbathy->num_beams; i++) {
			fprintf(stderr, "dbg5       corbathy->beam_index[%3d]:            %u\n", i, corbathy->beam_index[i]);
			fprintf(stderr, "dbg5       corbathy->x[%3d]:                     %f\n", i, corbathy->x[i]);
			fprintf(stderr, "dbg5       corbathy->y[%3d]:                     %f\n", i, corbathy->y[i]);
			fprintf(stderr, "dbg5       corbathy->z[%3d]:                     %f\n", i, corbathy->z[i]);
			fprintf(stderr, "dbg5       corbathy->beam_angle[%3d]:            %f\n", i, corbathy->beam_angle[i]);
			fprintf(stderr, "dbg5       corbathy->backscatter[%3d]:           %f\n", i, corbathy->backscatter[i]);
			fprintf(stderr, "dbg5       corbathy->quality[%3d]:               %u\n", i, corbathy->quality[i]);
			fprintf(stderr, "dbg5       corbathy->fish[%3d]:                  %u\n", i, corbathy->fish[i]);
			fprintf(stderr, "dbg5       corbathy->roughness[%3d]:             %u\n", i, corbathy->roughness[i]);
			fprintf(stderr, "dbg5       corbathy->empty[%3d]:                 %u\n", i, corbathy->empty[i]);
			fprintf(stderr, "dbg5       corbathy->pad[%3d]:                   %u\n", i, corbathy->pad[i]);
		}
		fprintf(stderr, "dbg5       corbathy->checksum:                   %u\n", corbathy->checksum);
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
int mbr_wasspenl_rd_rawsonar(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_rawsonar_struct *rawsonar = &(store->rawsonar);

	/* extract the data */
	int index = 16;
	mb_get_binary_int(true, &buffer[index], &(rawsonar->version));
	index += 4;
	mb_get_binary_double(true, &buffer[index], &(rawsonar->msec));
	index += 8;
	mb_get_binary_int(true, &buffer[index], &(rawsonar->ping_number));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(rawsonar->sample_rate));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(rawsonar->n));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(rawsonar->m));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(rawsonar->tx_power));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(rawsonar->pulse_width));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(rawsonar->sample_type));
	index += 4;
	for (unsigned int i = 0; i < rawsonar->n; i++) {
		mb_get_binary_int(true, &buffer[index], &(rawsonar->spare[i]));
		index += 4;
	}
	for (unsigned int i = 0; i < rawsonar->n; i++) {
		mb_get_binary_int(true, &buffer[index], &(rawsonar->beam_index[i]));
		index += 4;
	}
	for (unsigned int i = 0; i < rawsonar->n; i++) {
		mb_get_binary_int(true, &buffer[index], &(rawsonar->detection_point[i]));
		index += 4;
	}
	for (unsigned int i = 0; i < rawsonar->n; i++) {
		mb_get_binary_float(true, &buffer[index], &(rawsonar->beam_angle[i]));
		index += 4;
	}
	const size_t rawdata_len = (size_t)(rawsonar->n * rawsonar->m);
	int status = MB_SUCCESS;
	if (rawsonar->rawdata_alloc < rawdata_len) {
		status = mb_reallocd(verbose, __FILE__, __LINE__, rawdata_len, (void **)&(rawsonar->rawdata), error);
		if (status != MB_SUCCESS)
			rawsonar->rawdata_alloc = 0;
		else
			rawsonar->rawdata_alloc = rawdata_len;
	}
	memcpy(rawsonar->rawdata, &buffer[index], rawdata_len);
	index += rawdata_len;
	mb_get_binary_int(true, &buffer[index], &(rawsonar->checksum));
	index += 4;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_WATER_COLUMN;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       rawsonar->version:                    %u\n", rawsonar->version);
		fprintf(stderr, "dbg5       rawsonar->msec:                       %f\n", rawsonar->msec);
		fprintf(stderr, "dbg5       rawsonar->ping_number:                %u\n", rawsonar->ping_number);
		fprintf(stderr, "dbg5       rawsonar->sample_rate:                %f\n", rawsonar->sample_rate);
		fprintf(stderr, "dbg5       rawsonar->n:                          %u\n", rawsonar->n);
		fprintf(stderr, "dbg5       rawsonar->m:                          %u\n", rawsonar->m);
		fprintf(stderr, "dbg5       rawsonar->tx_power:                   %f\n", rawsonar->tx_power);
		fprintf(stderr, "dbg5       rawsonar->pulse_width:                %f\n", rawsonar->pulse_width);
		fprintf(stderr, "dbg5       rawsonar->sample_type:                %u\n", rawsonar->sample_type);
		for (unsigned int i = 0; i < rawsonar->n; i++) {
			fprintf(stderr, "dbg5       rawsonar->spare[%3d]:                 %u\n", i, rawsonar->spare[i]);
			fprintf(stderr, "dbg5       rawsonar->beam_index[%3d]:            %u\n", i, rawsonar->beam_index[i]);
			fprintf(stderr, "dbg5       rawsonar->detection_point[%3d]:       %u\n", i, rawsonar->detection_point[i]);
			fprintf(stderr, "dbg5       rawsonar->beam_angle[%3d]:            %f\n", i, rawsonar->beam_angle[i]);
		}
		fprintf(stderr, "dbg5       rawsonar->rawdata_alloc:              %zu\n", rawsonar->rawdata_alloc);
		for (unsigned int i = 0; i < rawsonar->m; i++)
			for (unsigned int j = 0; j < rawsonar->n; j++) {
				const int k = i * rawsonar->n + j;
				fprintf(stderr, "dbg5       rawsonar->rawdata[%4d][%4d]:          %u\n", i, j, rawsonar->rawdata[k]);
			}
		fprintf(stderr, "dbg5       rawsonar->checksum:                   %u\n", rawsonar->checksum);
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
int mbr_wasspenl_rd_gen_sens(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_gen_sens_struct *gen_sens = &(store->gen_sens);

	/* extract the data */
	int index = 16;
	mb_get_binary_int(true, &buffer[index], &(gen_sens->version));
	index += 4;
	mb_get_binary_double(true, &buffer[index], &(gen_sens->msec));
	index += 8;
	mb_get_binary_int(true, &buffer[index], &(gen_sens->port_number));
	index += 4;
	gen_sens->message_length = buffer[index];
	index++;
	memcpy(gen_sens->message, &buffer[index], (size_t)gen_sens->message_length);
	index += gen_sens->message_length;
	mb_get_binary_int(true, &buffer[index], &(gen_sens->checksum));
	index += 4;

	int status = MB_SUCCESS;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_GEN_SENS;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       gen_sens->version:                    %u\n", gen_sens->version);
		fprintf(stderr, "dbg5       gen_sens->msec:                       %f\n", gen_sens->msec);
		fprintf(stderr, "dbg5       gen_sens->port_number:                %u\n", gen_sens->port_number);
		fprintf(stderr, "dbg5       gen_sens->message_length:             %u\n", gen_sens->message_length);
		fprintf(stderr, "dbg5       gen_sens->message:                    %s\n", gen_sens->message);
		fprintf(stderr, "dbg5       gen_sens->checksum:                   %u\n", gen_sens->checksum);
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

int mbr_wasspenl_rd_nvupdate(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_nvupdate_struct *nvupdate = &(store->nvupdate);

	/* extract the data */
	int index = 16;
	mb_get_binary_int(true, &buffer[index], &(nvupdate->version));
	index += 4;
	mb_get_binary_double(true, &buffer[index], &(nvupdate->latitude));
	index += 8;
	mb_get_binary_double(true, &buffer[index], &(nvupdate->longitude));
	index += 8;
	mb_get_binary_float(true, &buffer[index], &(nvupdate->sog));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(nvupdate->cog));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(nvupdate->heading));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(nvupdate->roll));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(nvupdate->pitch));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(nvupdate->heave));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(nvupdate->nadir_depth));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(nvupdate->checksum));
	index += 4;

	int status = MB_SUCCESS;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_NAV;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       nvupdate->version:                    %u\n", nvupdate->version);
		fprintf(stderr, "dbg5       nvupdate->latitude:                   %f\n", nvupdate->latitude);
		fprintf(stderr, "dbg5       nvupdate->longitude:                  %f\n", nvupdate->longitude);
		fprintf(stderr, "dbg5       nvupdate->sog:                        %f\n", nvupdate->sog);
		fprintf(stderr, "dbg5       nvupdate->cog:                        %f\n", nvupdate->cog);
		fprintf(stderr, "dbg5       nvupdate->heading:                    %f\n", nvupdate->heading);
		fprintf(stderr, "dbg5       nvupdate->roll:                       %f\n", nvupdate->roll);
		fprintf(stderr, "dbg5       nvupdate->pitch:                      %f\n", nvupdate->pitch);
		fprintf(stderr, "dbg5       nvupdate->heave:                      %f\n", nvupdate->heave);
		fprintf(stderr, "dbg5       nvupdate->nadir_depth:                %f\n", nvupdate->nadir_depth);
		fprintf(stderr, "dbg5       nvupdate->checksum:                   %u\n", nvupdate->checksum);
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

int mbr_wasspenl_rd_wcd_navi(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_wcd_navi_struct *wcd_navi = &(store->wcd_navi);

	/* extract the data */
	int index = 16;
	mb_get_binary_int(true, &buffer[index], &(wcd_navi->version));
	index += 4;
	mb_get_binary_double(true, &buffer[index], &(wcd_navi->latitude));
	index += 8;
	mb_get_binary_double(true, &buffer[index], &(wcd_navi->longitude));
	index += 8;
	mb_get_binary_int(true, &buffer[index], &(wcd_navi->num_points));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(wcd_navi->bearing));
	index += 4;
	mb_get_binary_double(true, &buffer[index], &(wcd_navi->msec));
	index += 8;
	mb_get_binary_int(true, &buffer[index], &(wcd_navi->ping_number));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(wcd_navi->sample_rate));
	index += 4;
	int status = MB_SUCCESS;
	if (wcd_navi->wcdata_alloc < wcd_navi->num_points) {
		status =
		    mb_reallocd(verbose, __FILE__, __LINE__, wcd_navi->num_points * sizeof(float), (void **)&(wcd_navi->wcdata_x), error);
		status =
		    mb_reallocd(verbose, __FILE__, __LINE__, wcd_navi->num_points * sizeof(float), (void **)&(wcd_navi->wcdata_y), error);
		status = mb_reallocd(verbose, __FILE__, __LINE__, wcd_navi->num_points * sizeof(float), (void **)&(wcd_navi->wcdata_mag),
		                     error);
		if (status != MB_SUCCESS)
			wcd_navi->wcdata_alloc = 0;
		else
			wcd_navi->wcdata_alloc = wcd_navi->num_points;
	}
	for (unsigned int i = 0; i < wcd_navi->num_points; i++) {
		mb_get_binary_float(true, &buffer[index], &(wcd_navi->wcdata_x[i]));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(wcd_navi->wcdata_y[i]));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(wcd_navi->wcdata_mag[i]));
		index += 4;
	}
	mb_get_binary_int(true, &buffer[index], &(wcd_navi->checksum));
	index += 4;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_WC_PICKS;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       wcd_navi->version:                    %u\n", wcd_navi->version);
		fprintf(stderr, "dbg5       wcd_navi->latitude:                   %f\n", wcd_navi->latitude);
		fprintf(stderr, "dbg5       wcd_navi->longitude:                  %f\n", wcd_navi->longitude);
		fprintf(stderr, "dbg5       wcd_navi->num_points:                 %u\n", wcd_navi->num_points);
		fprintf(stderr, "dbg5       wcd_navi->bearing:                    %f\n", wcd_navi->bearing);
		fprintf(stderr, "dbg5       wcd_navi->msec:                       %f\n", wcd_navi->msec);
		fprintf(stderr, "dbg5       wcd_navi->ping_number:                %u\n", wcd_navi->ping_number);
		fprintf(stderr, "dbg5       wcd_navi->sample_type:                %f\n", wcd_navi->sample_rate);
		for (unsigned int i = 0; i < wcd_navi->num_points; i++) {
			fprintf(stderr, "dbg5       wcd_navi->wcdata_x[%3d]:              %f\n", i, wcd_navi->wcdata_x[i]);
			fprintf(stderr, "dbg5       wcd_navi->wcdata_y[%3d]:              %f\n", i, wcd_navi->wcdata_y[i]);
			fprintf(stderr, "dbg5       wcd_navi->wcdata_mag[%3d]:            %f\n", i, wcd_navi->wcdata_mag[i]);
		}
		fprintf(stderr, "dbg5       wcd_navi->checksum:                   %u\n", wcd_navi->checksum);
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

int mbr_wasspenl_rd_sensprop(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_sensprop_struct *sensprop = &(store->sensprop);

	/* extract the data */
	int index = 16;
	mb_get_binary_int(true, &buffer[index], &(sensprop->version));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(sensprop->flags));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(sensprop->sea_level_reference));
	index += 4;
	mb_get_binary_float(true, &buffer[index], &(sensprop->element_spacing));
	index += 4;
	for (int i = 0; i < 8; i++) {
		mb_get_binary_int(true, &buffer[index], &(sensprop->spare[i]));
		index += 4;
	}
	mb_get_binary_int(true, &buffer[index], &(sensprop->n));
	index += 4;

	int status = MB_SUCCESS;

	if (sensprop->n_alloc < sensprop->n) {
		status = mb_reallocd(verbose, __FILE__, __LINE__, sensprop->n * sizeof(struct mbsys_wassp_sensor_struct),
		                     (void **)&(sensprop->sensors), error);
		if (status != MB_SUCCESS)
			sensprop->n_alloc = 0;
		else
			sensprop->n_alloc = sensprop->n;
	}
	for (unsigned int i = 0; i < sensprop->n; i++) {
		mb_get_binary_int(true, &buffer[index], &(sensprop->sensors[i].sensor_type));
		index += 4;
		mb_get_binary_int(true, &buffer[index], &(sensprop->sensors[i].flags));
		index += 4;
		sensprop->sensors[i].port_number = buffer[index];
		index++;
		sensprop->sensors[i].device = buffer[index];
		index++;
		sensprop->sensors[i].sentence = buffer[index];
		index++;
		sensprop->sensors[i].sensor_model = buffer[index];
		index++;
		mb_get_binary_float(true, &buffer[index], &(sensprop->sensors[i].latency));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(sensprop->sensors[i].roll_bias));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(sensprop->sensors[i].pitch_bias));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(sensprop->sensors[i].yaw_bias));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(sensprop->sensors[i].offset_x));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(sensprop->sensors[i].offset_y));
		index += 4;
		mb_get_binary_float(true, &buffer[index], &(sensprop->sensors[i].offset_z));
		index += 4;
	}
	mb_get_binary_int(true, &buffer[index], &(sensprop->checksum));
	index += 4;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_SENSOR_PARAMETERS;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sensprop->version:                    %u\n", sensprop->version);
		fprintf(stderr, "dbg5       sensprop->flags:                      %u\n", sensprop->flags);
		fprintf(stderr, "dbg5       sensprop->sea_level_reference:        %f\n", sensprop->sea_level_reference);
		fprintf(stderr, "dbg5       sensprop->element_spacing:            %f\n", sensprop->element_spacing);
		for (int i = 0; i < 8; i++)
			fprintf(stderr, "dbg5       sensprop->spare[%d]:                   %d\n", i, sensprop->spare[i]);
		fprintf(stderr, "dbg5       sensprop->n:                          %d\n", sensprop->n);
		for (unsigned int i = 0; i < sensprop->n; i++) {
			fprintf(stderr, "dbg5       sensprop->sensors[%d].sensor_type:    %u\n", i, sensprop->sensors[i].sensor_type);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].flags:          %u\n", i, sensprop->sensors[i].flags);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].port_number:    %u\n", i, sensprop->sensors[i].port_number);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].device:         %u\n", i, sensprop->sensors[i].device);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].sentence:       %u\n", i, sensprop->sensors[i].sentence);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].sensor_model:   %u\n", i, sensprop->sensors[i].sensor_model);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].latency:        %f\n", i, sensprop->sensors[i].latency);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].roll_bias:      %f\n", i, sensprop->sensors[i].roll_bias);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].pitch_bias:     %f\n", i, sensprop->sensors[i].pitch_bias);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].yaw_bias:       %f\n", i, sensprop->sensors[i].yaw_bias);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].offset_x:       %f\n", i, sensprop->sensors[i].offset_x);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].offset_y:       %f\n", i, sensprop->sensors[i].offset_y);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].offset_z:       %f\n", i, sensprop->sensors[i].offset_z);
		}
		fprintf(stderr, "dbg5       sensprop->checksum:                   %u\n", sensprop->checksum);
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

int mbr_wasspenl_rd_sys_prop(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_sys_prop_struct *sys_prop = &(store->sys_prop);

	/* extract the data */
	int index = 16;
	mb_get_binary_int(true, &buffer[index], &(sys_prop->version));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(sys_prop->product_type));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(sys_prop->protocol_version));
	index += 4;
	for (int i = 0; i < 4; i++) {
		mb_get_binary_int(true, &buffer[index], &(sys_prop->sw_version[i]));
		index += 4;
	}
	mb_get_binary_int(true, &buffer[index], &(sys_prop->fw_version));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(sys_prop->hw_version));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(sys_prop->transducer_sn));
	index += 4;
	mb_get_binary_int(true, &buffer[index], &(sys_prop->transceiver_sn));
	index += 4;
	for (int i = 0; i < 8; i++) {
		mb_get_binary_int(true, &buffer[index], &(sys_prop->spare[i]));
		index += 4;
	}
	mb_get_binary_int(true, &buffer[index], &(sys_prop->checksum));
	index += 4;

	int status = MB_SUCCESS;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_INSTALLATION;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sys_prop->version:                    %u\n", sys_prop->version);
		fprintf(stderr, "dbg5       sys_prop->product_type:               %u\n", sys_prop->product_type);
		fprintf(stderr, "dbg5       sys_prop->protocol_version:           %u\n", sys_prop->protocol_version);
		for (int i = 0; i < 4; i++)
			fprintf(stderr, "dbg5       sys_prop->sw_version[%d]:             %u\n", i, sys_prop->sw_version[i]);
		fprintf(stderr, "dbg5       sys_prop->fw_version:                 %u\n", sys_prop->fw_version);
		fprintf(stderr, "dbg5       sys_prop->hw_version:                 %u\n", sys_prop->hw_version);
		fprintf(stderr, "dbg5       sys_prop->transducer_sn:              %u\n", sys_prop->transducer_sn);
		fprintf(stderr, "dbg5       sys_prop->transceiver_sn:             %u\n", sys_prop->transceiver_sn);
		for (int i = 0; i < 8; i++)
			fprintf(stderr, "dbg5       sys_prop->spare[%d]:                  %u\n", i, sys_prop->spare[i]);
		fprintf(stderr, "dbg5       sys_prop->checksum:                   %u\n", sys_prop->checksum);
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

int mbr_wasspenl_rd_sys_cfg1(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = &(store->sys_cfg1);

	/* get the size */
	int index = 4;
	unsigned int size = 0;
	mb_get_binary_int(true, &buffer[index], &size);
	sys_cfg1->sys_cfg1_len = size;

	int status = MB_SUCCESS;

	/* extract the data */
	if (sys_cfg1->sys_cfg1_data_alloc < size) {
		status = mb_reallocd(verbose, __FILE__, __LINE__, (size_t)size, (void **)&(sys_cfg1->sys_cfg1_data), error);
		if (status != MB_SUCCESS)
			sys_cfg1->sys_cfg1_data_alloc = 0;
		else
			sys_cfg1->sys_cfg1_data_alloc = size;
	}
	memcpy(sys_cfg1->sys_cfg1_data, buffer, (size_t)sys_cfg1->sys_cfg1_len);

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_PARAMETER;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		for (unsigned int i = 0; i < sys_cfg1->sys_cfg1_len; i++) {
			fprintf(stderr, "dbg5       sys_cfg1->sys_cfg1_data[%3d]:           %u\n", i, sys_cfg1->sys_cfg1_data[i]);
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

int mbr_wasspenl_rd_mcomment(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_mcomment_struct *mcomment = &(store->mcomment);

	/* extract the data */
	int index = 16;
	mb_get_binary_int(true, &buffer[index], &(mcomment->comment_length));
	index += 4;
	memcpy(mcomment->comment_message, &buffer[index], (size_t)mcomment->comment_length);

	int status = MB_SUCCESS;

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_COMMENT;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       mcomment->comment_length:             %u\n", mcomment->comment_length);
		fprintf(stderr, "dbg5       mcomment->comment_message:            %s\n", mcomment->comment_message);
		fprintf(stderr, "dbg5       mcomment->checksum:                   %u\n", mcomment->checksum);
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

int mbr_wasspenl_rd_unknown1(int verbose, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_unknown1_struct *unknown1 = &(store->unknown1);

	/* get the size */
	int index = 4;
	unsigned int size = 0;
	mb_get_binary_int(true, &buffer[index], &size);
	unknown1->unknown1_len = size;

	int status = MB_SUCCESS;

	/* extract the data */
	if (unknown1->unknown1_data_alloc < size) {
		status = mb_reallocd(verbose, __FILE__, __LINE__, (size_t)size, (void **)&(unknown1->unknown1_data), error);
		if (status != MB_SUCCESS)
			unknown1->unknown1_data_alloc = 0;
		else
			unknown1->unknown1_data_alloc = size;
	}
	memcpy(unknown1->unknown1_data, buffer, (size_t)unknown1->unknown1_len);

	/* set kind */
	if (status == MB_SUCCESS) {
		/* set kind */
		store->kind = MB_DATA_RAW_LINE;
	}
	else {
		store->kind = MB_DATA_NONE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		for (unsigned int i = 0; i < unknown1->unknown1_len; i++) {
			fprintf(stderr, "dbg5       unknown1->unknown1_data[%3d]:           %u\n", i, unknown1->unknown1_data[i]);
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
int mbr_wasspenl_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int status = MB_SUCCESS;
	unsigned int syncvalue = 0;
	char recordid[12];
	size_t read_len;
	int skip;
	bool reset_beamflags;

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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);

	char **bufferptr = (char **)&mb_io_ptr->saveptr1;
	char *buffer = (char *)*bufferptr;
	unsigned int *bufferalloc = (unsigned int *)&mb_io_ptr->save6;
	unsigned int *record_size = (unsigned int *)&buffer[4];

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* loop over reading data until a record is ready for return */
	*error = MB_ERROR_NO_ERROR;
	memset((void *)recordid, 0, (size_t)12);
	bool done = false;
	while (!done) {
		/* read next record header into buffer */
		read_len = (size_t)16;
		status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);

		/* check header - if not a good header read a byte
		    at a time until a good header is found */
		skip = 0;
		mb_get_binary_int(true, buffer, &syncvalue);
		while (status == MB_SUCCESS && syncvalue != MBSYS_WASSP_SYNC) {
			/* get next byte */
			for (int i = 0; i < 15; i++)
				buffer[i] = buffer[i + 1];
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, &buffer[15], &read_len, error);
			mb_get_binary_int(true, buffer, &syncvalue);
			skip++;
		}

		/* get record id string */
		memcpy((void *)recordid, (void *)&buffer[8], (size_t)8);
#ifdef MBR_WASSPENLDEBUG
		fprintf(stderr, "Found sync - skip:%d record:%s\n", skip, recordid);
#endif

		/* report problem */
		if (skip > 0 && verbose >= 0) {
			fprintf(stderr, "\nThe MBF_WASSPENL module skipped data between identified\n"
			                "data records. Something is broken, most probably the data...\n"
			                "However, the data may include a data record type that we\n"
			                "haven't seen yet, or there could be an error in the code.\n"
			                "If skipped data are reported multiple times, \n"
			                "we recommend you send a data sample and problem \n"
			                "description to the MB-System team \n"
			                "(caress@mbari.org and dale@ldeo.columbia.edu)\n"
			                "Have a nice day...\n");
			fprintf(stderr, "MBF_WASSPENL skipped %d bytes before record %s\n", skip, recordid);
		}

		/* allocate memory to read rest of record if necessary */
		if (*bufferalloc < *record_size) {
			status = mb_reallocd(verbose, __FILE__, __LINE__, *record_size, (void **)bufferptr, error);
			if (status != MB_SUCCESS) {
				*bufferalloc = 0;
				done = true;
			}
			else {
				*bufferalloc = *record_size;
				buffer = (char *)*bufferptr;
			}
		}

		/* read the rest of the record */
		if (status == MB_SUCCESS) {
			read_len = (size_t)(*record_size - 16);
			status = mb_fileio_get(verbose, mbio_ptr, &buffer[16], &read_len, error);
		}

		/* if valid parse the record */
		if (status == MB_SUCCESS) {
			/* read GENBATHY record */
			if (strncmp(recordid, "GENBATHY", 8) == 0) {
				status = mbr_wasspenl_rd_genbathy(verbose, buffer, store_ptr, error);
			}

			/* read CORBATHY record */
			else if (strncmp(recordid, "CORBATHY", 8) == 0) {
				status = mbr_wasspenl_rd_corbathy(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS) {
					if (genbathy->ping_number == corbathy->ping_number) {
						done = true;

						/* reset beam flags if necessary */
						reset_beamflags = false;
						for (unsigned int i = 0; i < corbathy->num_beams; i++) {
							if (corbathy->z[i] == 0 && corbathy->empty[i] != MB_FLAG_NULL)
								reset_beamflags = true;
						}
						if (reset_beamflags)
							for (unsigned int i = 0; i < corbathy->num_beams; i++) {
								const int j = corbathy->beam_index[i];
								if (corbathy->z[i] == 0)
									corbathy->empty[i] = MB_FLAG_NULL;
								else if (genbathy->flags[j] & 0x01)
									corbathy->empty[i] = MB_FLAG_NONE;
								else
									corbathy->empty[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
							}
					}
					else {
						status = MB_FAILURE;
						*error = MB_ERROR_UNINTELLIGIBLE;
						done = true;
					}
				}
			}

			/* read RAWSONAR record */
			else if (strncmp(recordid, "RAWSONAR", 8) == 0) {
				status = mbr_wasspenl_rd_rawsonar(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = true;
			}

			/* read GEN_SENS record */
			else if (strncmp(recordid, "GEN_SENS", 8) == 0) {
				status = mbr_wasspenl_rd_gen_sens(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = true;
			}

			/* read NVUPDATE record */
			else if (strncmp(recordid, "NVUPDATE", 8) == 0) {
				status = mbr_wasspenl_rd_nvupdate(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = true;
			}

			/* read WCD_NAVI record */
			else if (strncmp(recordid, "WCD_NAVI", 8) == 0) {
				status = mbr_wasspenl_rd_wcd_navi(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = true;
			}

			/* read SENSPROP record */
			else if (strncmp(recordid, "SENSPROP", 8) == 0) {
				status = mbr_wasspenl_rd_sensprop(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = true;
			}

			/* read SYS_PROP record */
			else if (strncmp(recordid, "SYS_PROP", 8) == 0) {
				status = mbr_wasspenl_rd_sys_prop(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = true;
			}

			/* read SYS_CFG1 record */
			else if (strncmp(recordid, "SYS_CFG1", 8) == 0) {
				status = mbr_wasspenl_rd_sys_cfg1(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = true;
			}

			/* read MCOMMENT_ record */
			else if (strncmp(recordid, "MCOMMENT", 8) == 0) {
				status = mbr_wasspenl_rd_mcomment(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = true;
			}

			/* read an unknown1 record */
			else {
				status = mbr_wasspenl_rd_unknown1(verbose, buffer, store_ptr, error);
				if (status == MB_SUCCESS)
					done = true;
			}
		}

		/* set done if read failure */
		else {
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
}
/*--------------------------------------------------------------------*/
int mbr_rt_wasspenl(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr, "About to call mbr_wasspenl_rd_data...\n");
#endif

	/* read next data from file */
	const int status = mbr_wasspenl_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* get pointers to data structures */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr, "Done with mbr_wasspenl_rd_data: status:%d error:%d record kind:%d\n\n", status, *error, store->kind);
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
int mbr_wasspenl_wr_genbathy(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
	char *buffer;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_genbathy_struct *genbathy = &(store->genbathy);
	genbathy->version = 3;
	genbathy->checksum = 0x8806CBA5;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       genbathy->version:                    %u\n", genbathy->version);
		fprintf(stderr, "dbg5       genbathy->msec:                       %f\n", genbathy->msec);
		fprintf(stderr, "dbg5       genbathy->day:                        %u\n", genbathy->day);
		fprintf(stderr, "dbg5       genbathy->month:                      %u\n", genbathy->month);
		fprintf(stderr, "dbg5       genbathy->year:                       %u\n", genbathy->year);
		fprintf(stderr, "dbg5       genbathy->ping_number:                %u\n", genbathy->ping_number);
		fprintf(stderr, "dbg5       genbathy->sonar_model:                %u\n", genbathy->sonar_model);
		fprintf(stderr, "dbg5       genbathy->transducer_serial_number:   %lu\n", genbathy->transducer_serial_number);
		fprintf(stderr, "dbg5       genbathy->number_beams:               %u\n", genbathy->number_beams);
		fprintf(stderr, "dbg5       genbathy->modeflags:                  %u\n", genbathy->modeflags);
		fprintf(stderr, "dbg5       genbathy->sampling_frequency:         %f\n", genbathy->sampling_frequency);
		fprintf(stderr, "dbg5       genbathy->acoustic_frequency:         %f\n", genbathy->acoustic_frequency);
		fprintf(stderr, "dbg5       genbathy->tx_power:                   %f\n", genbathy->tx_power);
		fprintf(stderr, "dbg5       genbathy->pulse_width:                %f\n", genbathy->pulse_width);
		fprintf(stderr, "dbg5       genbathy->absorption_loss:            %f\n", genbathy->absorption_loss);
		fprintf(stderr, "dbg5       genbathy->spreading_loss:             %f\n", genbathy->spreading_loss);
		fprintf(stderr, "dbg5       genbathy->sample_type:                %u\n", genbathy->sample_type);
		fprintf(stderr, "dbg5       genbathy->sound_velocity:             %f\n", genbathy->sound_velocity);
		for (unsigned int i = 0; i < genbathy->number_beams; i++) {
			fprintf(stderr, "dbg5       genbathy->detection_point[%3d]:       %f\n", i, genbathy->detection_point[i]);
			fprintf(stderr, "dbg5       genbathy->rx_angle[%3d]:              %f\n", i, genbathy->rx_angle[i]);
			fprintf(stderr, "dbg5       genbathy->flags[%3d]:                 %u\n", i, genbathy->flags[i]);
			fprintf(stderr, "dbg5       genbathy->backscatter[%3d]:           %f\n", i, genbathy->backscatter[i]);
		}
		fprintf(stderr, "dbg5       genbathy->checksum:                   %u\n", genbathy->checksum);
	}

	/* figure out size of output record */
	*size = 92 + 16 * genbathy->number_beams;

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

		/* insert the data */
		index = 0;
		mb_put_binary_int(true, MBSYS_WASSP_SYNC, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, *size, &buffer[index]);
		index += 4;
		strncpy(&buffer[index], "GENBATHY", 8);
		index += 8;
		mb_put_binary_int(true, genbathy->version, &buffer[index]);
		index += 4;
		mb_put_binary_double(true, genbathy->msec, &buffer[index]);
		index += 8;
		buffer[index] = genbathy->day;
		index++;
		buffer[index] = genbathy->month;
		index++;
		mb_put_binary_short(true, genbathy->year, &buffer[index]);
		index += 2;
		mb_put_binary_int(true, genbathy->ping_number, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, genbathy->sonar_model, &buffer[index]);
		index += 4;
		mb_put_binary_long(true, genbathy->transducer_serial_number, &buffer[index]);
		index += 8;
		mb_put_binary_int(true, genbathy->number_beams, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, genbathy->modeflags, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, genbathy->sampling_frequency, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, genbathy->acoustic_frequency, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, genbathy->tx_power, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, genbathy->pulse_width, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, genbathy->absorption_loss, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, genbathy->spreading_loss, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, genbathy->sample_type, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, genbathy->sound_velocity, &buffer[index]);
		index += 4;
		for (unsigned int i = 0; i < genbathy->number_beams; i++) {
			mb_put_binary_float(true, genbathy->detection_point[i], &buffer[index]);
			index += 4;
			mb_put_binary_float(true, genbathy->rx_angle[i], &buffer[index]);
			index += 4;
			mb_put_binary_int(true, genbathy->flags[i], &buffer[index]);
			index += 4;
			mb_put_binary_float(true, genbathy->backscatter[i], &buffer[index]);
			index += 4;
		}

		/* now add the checksum */
		mb_put_binary_int(true, genbathy->checksum, &buffer[index]);
		index += 4;
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
int mbr_wasspenl_wr_corbathy(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
	char *buffer;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_corbathy_struct *corbathy = &(store->corbathy);
	corbathy->version = 4;
	corbathy->checksum = 0x8806CBA5;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       corbathy->version:                    %u\n", corbathy->version);
		fprintf(stderr, "dbg5       corbathy->msec:                       %f\n", corbathy->msec);
		fprintf(stderr, "dbg5       corbathy->num_beams:                  %u\n", corbathy->num_beams);
		fprintf(stderr, "dbg5       corbathy->ping_number:                %u\n", corbathy->ping_number);
		fprintf(stderr, "dbg5       corbathy->latitude:                   %f\n", corbathy->latitude);
		fprintf(stderr, "dbg5       corbathy->longitude:                  %f\n", corbathy->longitude);
		fprintf(stderr, "dbg5       corbathy->bearing:                    %f\n", corbathy->bearing);
		fprintf(stderr, "dbg5       corbathy->roll:                       %f\n", corbathy->roll);
		fprintf(stderr, "dbg5       corbathy->pitch:                      %f\n", corbathy->pitch);
		fprintf(stderr, "dbg5       corbathy->heave:                      %f\n", corbathy->heave);
		fprintf(stderr, "dbg5       corbathy->sample_type:                %u\n", corbathy->sample_type);
		for (int i = 0; i < 5; i++) {
			fprintf(stderr, "dbg5       corbathy->spare[%3d]:                 %u\n", i, corbathy->spare[i]);
		}
		for (unsigned int i = 0; i < corbathy->num_beams; i++) {
			fprintf(stderr, "dbg5       corbathy->beam_index[%3d]:            %u\n", i, corbathy->beam_index[i]);
			fprintf(stderr, "dbg5       corbathy->x[%3d]:                     %f\n", i, corbathy->x[i]);
			fprintf(stderr, "dbg5       corbathy->y[%3d]:                     %f\n", i, corbathy->y[i]);
			fprintf(stderr, "dbg5       corbathy->z[%3d]:                     %f\n", i, corbathy->z[i]);
			fprintf(stderr, "dbg5       corbathy->beam_angle[%3d]:            %f\n", i, corbathy->beam_angle[i]);
			fprintf(stderr, "dbg5       corbathy->backscatter[%3d]:           %f\n", i, corbathy->backscatter[i]);
			fprintf(stderr, "dbg5       corbathy->quality[%3d]:               %u\n", i, corbathy->quality[i]);
			fprintf(stderr, "dbg5       corbathy->fish[%3d]:                  %u\n", i, corbathy->fish[i]);
			fprintf(stderr, "dbg5       corbathy->roughness[%3d]:             %u\n", i, corbathy->roughness[i]);
			fprintf(stderr, "dbg5       corbathy->empty[%3d]:                 %u\n", i, corbathy->empty[i]);
			fprintf(stderr, "dbg5       corbathy->pad[%3d]:                   %u\n", i, corbathy->pad[i]);
		}
		fprintf(stderr, "dbg5       corbathy->checksum:                   %u\n", corbathy->checksum);
	}

	/* figure out size of output record */
	*size = 100 + 32 * corbathy->num_beams;

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

		/* insert the data */
		index = 0;
		mb_put_binary_int(true, MBSYS_WASSP_SYNC, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, *size, &buffer[index]);
		index += 4;
		strncpy(&buffer[index], "CORBATHY", 8);
		index += 8;
		mb_put_binary_int(true, corbathy->version, &buffer[index]);
		index += 4;
		mb_put_binary_double(true, corbathy->msec, &buffer[index]);
		index += 8;
		mb_put_binary_int(true, corbathy->num_beams, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, corbathy->ping_number, &buffer[index]);
		index += 4;
		mb_put_binary_double(true, corbathy->latitude, &buffer[index]);
		index += 8;
		mb_put_binary_double(true, corbathy->longitude, &buffer[index]);
		index += 8;
		mb_put_binary_float(true, corbathy->bearing, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, corbathy->roll, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, corbathy->pitch, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, corbathy->heave, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, corbathy->sample_type, &buffer[index]);
		index += 4;
		for (int i = 0; i < 5; i++) {
			mb_put_binary_int(true, corbathy->spare[i], &buffer[index]);
			index += 4;
		}
		for (unsigned int i = 0; i < corbathy->num_beams; i++) {
			mb_put_binary_int(true, corbathy->beam_index[i], &buffer[index]);
			index += 4;
			mb_put_binary_float(true, corbathy->x[i], &buffer[index]);
			index += 4;
			mb_put_binary_float(true, corbathy->y[i], &buffer[index]);
			index += 4;
			mb_put_binary_float(true, corbathy->z[i], &buffer[index]);
			index += 4;
			mb_put_binary_float(true, corbathy->beam_angle[i], &buffer[index]);
			index += 4;
			mb_put_binary_float(true, corbathy->backscatter[i], &buffer[index]);
			index += 4;
			buffer[index] = corbathy->quality[i];
			index++;
			buffer[index] = corbathy->fish[i];
			index++;
			buffer[index] = corbathy->roughness[i];
			index++;
			buffer[index] = corbathy->empty[i];
			index++;
			mb_put_binary_int(true, corbathy->pad[i], &buffer[index]);
			index += 4;
		}

		/* now add the checksum */
		mb_put_binary_int(true, corbathy->checksum, &buffer[index]);
		index += 4;
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
int mbr_wasspenl_wr_rawsonar(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
	char *buffer;
	int index;
	size_t rawdata_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_rawsonar_struct *rawsonar = &(store->rawsonar);
	rawsonar->version = 2;
	rawsonar->checksum = 0x8806CBA5;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       rawsonar->version:                    %u\n", rawsonar->version);
		fprintf(stderr, "dbg5       rawsonar->msec:                       %f\n", rawsonar->msec);
		fprintf(stderr, "dbg5       rawsonar->ping_number:                %u\n", rawsonar->ping_number);
		fprintf(stderr, "dbg5       rawsonar->sample_rate:                %f\n", rawsonar->sample_rate);
		fprintf(stderr, "dbg5       rawsonar->n:                          %u\n", rawsonar->n);
		fprintf(stderr, "dbg5       rawsonar->m:                          %u\n", rawsonar->m);
		fprintf(stderr, "dbg5       rawsonar->tx_power:                   %f\n", rawsonar->tx_power);
		fprintf(stderr, "dbg5       rawsonar->pulse_width:                %f\n", rawsonar->pulse_width);
		fprintf(stderr, "dbg5       rawsonar->sample_type:                %u\n", rawsonar->sample_type);
		for (unsigned int i = 0; i < rawsonar->n; i++) {
			fprintf(stderr, "dbg5       rawsonar->spare[%3d]:                 %u\n", i, rawsonar->spare[i]);
			fprintf(stderr, "dbg5       rawsonar->beam_index[%3d]:            %u\n", i, rawsonar->beam_index[i]);
			fprintf(stderr, "dbg5       rawsonar->detection_point[%3d]:       %u\n", i, rawsonar->detection_point[i]);
			fprintf(stderr, "dbg5       rawsonar->beam_angle[%3d]:            %f\n", i, rawsonar->beam_angle[i]);
		}
		fprintf(stderr, "dbg5       rawsonar->rawdata_alloc:              %zu\n", rawsonar->rawdata_alloc);
		for (unsigned int i = 0; i < rawsonar->m; i++)
			for (unsigned int j = 0; j < rawsonar->n; j++) {
				const int k = i * rawsonar->n + j;
				fprintf(stderr, "dbg5       rawsonar->rawdata[%4d][%4d]:          %u\n", i, j, rawsonar->rawdata[k]);
			}
		fprintf(stderr, "dbg5       rawsonar->checksum:                   %u\n", rawsonar->checksum);
	}

	/* figure out size of output record */
	*size = 60 + 12 * rawsonar->n + 2 * rawsonar->m * rawsonar->n;

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

		/* insert the data */
		index = 0;
		mb_put_binary_int(true, MBSYS_WASSP_SYNC, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, *size, &buffer[index]);
		index += 4;
		strncpy(&buffer[index], "RAWSONAR", 8);
		index += 8;
		mb_put_binary_int(true, rawsonar->version, &buffer[index]);
		index += 4;
		mb_put_binary_double(true, rawsonar->msec, &buffer[index]);
		index += 8;
		mb_put_binary_int(true, rawsonar->ping_number, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, rawsonar->sample_rate, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, rawsonar->n, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, rawsonar->m, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, rawsonar->tx_power, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, rawsonar->pulse_width, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, rawsonar->sample_type, &buffer[index]);
		index += 4;
		for (unsigned int i = 0; i < rawsonar->n; i++) {
			mb_put_binary_int(true, rawsonar->spare[i], &buffer[index]);
			index += 4;
		}
		for (unsigned int i = 0; i < rawsonar->n; i++) {
			mb_put_binary_int(true, rawsonar->beam_index[i], &buffer[index]);
			index += 4;
		}
		for (unsigned int i = 0; i < rawsonar->n; i++) {
			mb_put_binary_int(true, rawsonar->detection_point[i], &buffer[index]);
			index += 4;
		}
		for (unsigned int i = 0; i < rawsonar->n; i++) {
			mb_put_binary_float(true, rawsonar->beam_angle[i], &buffer[index]);
			index += 4;
		}
		rawdata_len = (size_t)(rawsonar->n * rawsonar->m);
		memcpy(&buffer[index], rawsonar->rawdata, rawdata_len);
		index += rawdata_len;

		/* now add the checksum */
		mb_put_binary_int(true, rawsonar->checksum, &buffer[index]);
		index += 4;
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
int mbr_wasspenl_wr_gen_sens(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
	char *buffer;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_gen_sens_struct *gen_sens = &(store->gen_sens);
	gen_sens->version = 2;
	gen_sens->checksum = 0x8806CBA5;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       gen_sens->version:                    %u\n", gen_sens->version);
		fprintf(stderr, "dbg5       gen_sens->msec:                       %f\n", gen_sens->msec);
		fprintf(stderr, "dbg5       gen_sens->port_number:                %u\n", gen_sens->port_number);
		fprintf(stderr, "dbg5       gen_sens->message_length:             %u\n", gen_sens->message_length);
		fprintf(stderr, "dbg5       gen_sens->message:                    %s\n", gen_sens->message);
		fprintf(stderr, "dbg5       gen_sens->checksum:                   %u\n", gen_sens->checksum);
	}

	/* figure out size of output record */
	*size = 33 + gen_sens->message_length;

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

		/* insert the data */
		index = 0;
		mb_put_binary_int(true, MBSYS_WASSP_SYNC, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, *size, &buffer[index]);
		index += 4;
		strncpy(&buffer[index], "GEN_SENS", 8);
		index += 8;
		mb_put_binary_int(true, gen_sens->version, &buffer[index]);
		index += 4;
		mb_put_binary_double(true, gen_sens->msec, &buffer[index]);
		index += 8;
		mb_put_binary_int(true, gen_sens->port_number, &buffer[index]);
		index += 4;
		buffer[index] = gen_sens->message_length;
		index++;
		memcpy(&buffer[index], gen_sens->message, (size_t)gen_sens->message_length);
		index += gen_sens->message_length;

		/* now add the checksum */
		mb_put_binary_int(true, gen_sens->checksum, &buffer[index]);
		index += 4;
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

int mbr_wasspenl_wr_nvupdate(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
	char *buffer;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_nvupdate_struct *nvupdate = &(store->nvupdate);
	nvupdate->version = 4;
	nvupdate->checksum = 0x8806CBA5;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       nvupdate->version:                    %u\n", nvupdate->version);
		fprintf(stderr, "dbg5       nvupdate->latitude:                   %f\n", nvupdate->latitude);
		fprintf(stderr, "dbg5       nvupdate->longitude:                  %f\n", nvupdate->longitude);
		fprintf(stderr, "dbg5       nvupdate->sog:                        %f\n", nvupdate->sog);
		fprintf(stderr, "dbg5       nvupdate->cog:                        %f\n", nvupdate->cog);
		fprintf(stderr, "dbg5       nvupdate->heading:                    %f\n", nvupdate->heading);
		fprintf(stderr, "dbg5       nvupdate->roll:                       %f\n", nvupdate->roll);
		fprintf(stderr, "dbg5       nvupdate->pitch:                      %f\n", nvupdate->pitch);
		fprintf(stderr, "dbg5       nvupdate->heave:                      %f\n", nvupdate->heave);
		fprintf(stderr, "dbg5       nvupdate->nadir_depth:                %f\n", nvupdate->nadir_depth);
		fprintf(stderr, "dbg5       nvupdate->checksum:                   %u\n", nvupdate->checksum);
	}

	/* figure out size of output record */
	*size = 68;

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

		/* insert the data */
		index = 0;
		mb_put_binary_int(true, MBSYS_WASSP_SYNC, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, *size, &buffer[index]);
		index += 4;
		strncpy(&buffer[index], "NVUPDATE", 8);
		index += 8;
		mb_put_binary_int(true, nvupdate->version, &buffer[index]);
		index += 4;
		mb_put_binary_double(true, nvupdate->latitude, &buffer[index]);
		index += 8;
		mb_put_binary_double(true, nvupdate->longitude, &buffer[index]);
		index += 8;
		mb_put_binary_float(true, nvupdate->sog, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, nvupdate->cog, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, nvupdate->heading, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, nvupdate->roll, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, nvupdate->pitch, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, nvupdate->heave, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, nvupdate->nadir_depth, &buffer[index]);
		index += 4;

		/* now add the checksum */
		mb_put_binary_int(true, nvupdate->checksum, &buffer[index]);
		index += 4;
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

int mbr_wasspenl_wr_wcd_navi(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
	char *buffer;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_wcd_navi_struct *wcd_navi = &(store->wcd_navi);
	wcd_navi->version = 4;
	wcd_navi->checksum = 0x8806CBA5;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       wcd_navi->version:                    %u\n", wcd_navi->version);
		fprintf(stderr, "dbg5       wcd_navi->latitude:                   %f\n", wcd_navi->latitude);
		fprintf(stderr, "dbg5       wcd_navi->longitude:                  %f\n", wcd_navi->longitude);
		fprintf(stderr, "dbg5       wcd_navi->num_points:                 %u\n", wcd_navi->num_points);
		fprintf(stderr, "dbg5       wcd_navi->bearing:                    %f\n", wcd_navi->bearing);
		fprintf(stderr, "dbg5       wcd_navi->msec:                       %f\n", wcd_navi->msec);
		fprintf(stderr, "dbg5       wcd_navi->ping_number:                %u\n", wcd_navi->ping_number);
		fprintf(stderr, "dbg5       wcd_navi->sample_type:                %f\n", wcd_navi->sample_rate);
		for (unsigned int i = 0; i < wcd_navi->num_points; i++) {
			fprintf(stderr, "dbg5       wcd_navi->wcdata_x[%3d]:              %f\n", i, wcd_navi->wcdata_x[i]);
			fprintf(stderr, "dbg5       wcd_navi->wcdata_y[%3d]:              %f\n", i, wcd_navi->wcdata_y[i]);
			fprintf(stderr, "dbg5       wcd_navi->wcdata_mag[%3d]:            %f\n", i, wcd_navi->wcdata_mag[i]);
		}
		fprintf(stderr, "dbg5       wcd_navi->checksum:                   %u\n", wcd_navi->checksum);
	}

	/* figure out size of output record */
	*size = 64 + 12 * wcd_navi->num_points;

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

		/* insert the data */
		index = 0;
		mb_put_binary_int(true, MBSYS_WASSP_SYNC, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, *size, &buffer[index]);
		index += 4;
		strncpy(&buffer[index], "WCD_NAVI", 8);
		index += 8;
		mb_put_binary_int(true, wcd_navi->version, &buffer[index]);
		index += 4;
		mb_put_binary_double(true, wcd_navi->latitude, &buffer[index]);
		index += 8;
		mb_put_binary_double(true, wcd_navi->longitude, &buffer[index]);
		index += 8;
		mb_put_binary_int(true, wcd_navi->num_points, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, wcd_navi->bearing, &buffer[index]);
		index += 4;
		mb_put_binary_double(true, wcd_navi->msec, &buffer[index]);
		index += 8;
		mb_put_binary_int(true, wcd_navi->ping_number, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, wcd_navi->sample_rate, &buffer[index]);
		index += 4;
		for (unsigned int i = 0; i < wcd_navi->num_points; i++) {
			mb_put_binary_float(true, wcd_navi->wcdata_x[i], &buffer[index]);
			index += 4;
			mb_put_binary_float(true, wcd_navi->wcdata_y[i], &buffer[index]);
			index += 4;
			mb_put_binary_float(true, wcd_navi->wcdata_mag[i], &buffer[index]);
			index += 4;
		}

		/* now add the checksum */
		mb_put_binary_int(true, wcd_navi->checksum, &buffer[index]);
		index += 4;
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

int mbr_wasspenl_wr_sensprop(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
	char *buffer;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_sensprop_struct *sensprop = &(store->sensprop);
	sensprop->version = 1;
	sensprop->checksum = 0x8806CBA5;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sensprop->version:                    %u\n", sensprop->version);
		fprintf(stderr, "dbg5       sensprop->flags:                      %u\n", sensprop->flags);
		fprintf(stderr, "dbg5       sensprop->sea_level_reference:        %f\n", sensprop->sea_level_reference);
		fprintf(stderr, "dbg5       sensprop->element_spacing:            %f\n", sensprop->element_spacing);
		for (int i = 0; i < 8; i++)
			fprintf(stderr, "dbg5       sensprop->spare[%d]:                   %d\n", i, sensprop->spare[i]);
		fprintf(stderr, "dbg5       sensprop->n:                          %d\n", sensprop->n);
		for (unsigned int i = 0; i < sensprop->n; i++) {
			fprintf(stderr, "dbg5       sensprop->sensors[%d].sensor_type:    %u\n", i, sensprop->sensors[i].sensor_type);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].flags:          %u\n", i, sensprop->sensors[i].flags);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].port_number:    %u\n", i, sensprop->sensors[i].port_number);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].device:         %u\n", i, sensprop->sensors[i].device);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].sentence:       %u\n", i, sensprop->sensors[i].sentence);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].sensor_model:   %u\n", i, sensprop->sensors[i].sensor_model);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].latency:        %f\n", i, sensprop->sensors[i].latency);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].roll_bias:      %f\n", i, sensprop->sensors[i].roll_bias);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].pitch_bias:     %f\n", i, sensprop->sensors[i].pitch_bias);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].yaw_bias:       %f\n", i, sensprop->sensors[i].yaw_bias);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].offset_x:       %f\n", i, sensprop->sensors[i].offset_x);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].offset_y:       %f\n", i, sensprop->sensors[i].offset_y);
			fprintf(stderr, "dbg5       sensprop->sensors[%d].offset_z:       %f\n", i, sensprop->sensors[i].offset_z);
		}
		fprintf(stderr, "dbg5       sensprop->checksum:                   %u\n", sensprop->checksum);
	}

	/* figure out size of output record */
	*size = 72 + 40 * sensprop->n;

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

		/* insert the data */
		index = 0;
		mb_put_binary_int(true, MBSYS_WASSP_SYNC, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, *size, &buffer[index]);
		index += 4;
		strncpy(&buffer[index], "SENSPROP", 8);
		index += 8;
		mb_put_binary_int(true, sensprop->version, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, sensprop->flags, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, sensprop->sea_level_reference, &buffer[index]);
		index += 4;
		mb_put_binary_float(true, sensprop->element_spacing, &buffer[index]);
		index += 4;
		for (int i = 0; i < 8; i++) {
			mb_put_binary_int(true, sensprop->spare[i], &buffer[index]);
			index += 4;
		}
		mb_put_binary_int(true, sensprop->n, &buffer[index]);
		index += 4;
		for (unsigned int i = 0; i < sensprop->n; i++) {
			mb_put_binary_int(true, sensprop->sensors[i].sensor_type, &buffer[index]);
			index += 4;
			mb_put_binary_int(true, sensprop->sensors[i].flags, &buffer[index]);
			index += 4;
			buffer[index] = sensprop->sensors[i].port_number;
			index++;
			buffer[index] = sensprop->sensors[i].device;
			index++;
			buffer[index] = sensprop->sensors[i].sentence;
			index++;
			buffer[index] = sensprop->sensors[i].sensor_model;
			index++;
			mb_put_binary_float(true, sensprop->sensors[i].latency, &buffer[index]);
			index += 4;
			mb_put_binary_float(true, sensprop->sensors[i].roll_bias, &buffer[index]);
			index += 4;
			mb_put_binary_float(true, sensprop->sensors[i].pitch_bias, &buffer[index]);
			index += 4;
			mb_put_binary_float(true, sensprop->sensors[i].yaw_bias, &buffer[index]);
			index += 4;
			mb_put_binary_float(true, sensprop->sensors[i].offset_x, &buffer[index]);
			index += 4;
			mb_put_binary_float(true, sensprop->sensors[i].offset_y, &buffer[index]);
			index += 4;
			mb_put_binary_float(true, sensprop->sensors[i].offset_z, &buffer[index]);
			index += 4;
		}

		/* now add the checksum */
		mb_put_binary_int(true, sensprop->checksum, &buffer[index]);
		index += 4;
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

int mbr_wasspenl_wr_sys_prop(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
	char *buffer;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_sys_prop_struct *sys_prop = &(store->sys_prop);
	sys_prop->version = 1;
	sys_prop->checksum = 0x8806CBA5;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sys_prop->version:                    %u\n", sys_prop->version);
		fprintf(stderr, "dbg5       sys_prop->product_type:               %u\n", sys_prop->product_type);
		fprintf(stderr, "dbg5       sys_prop->protocol_version:           %u\n", sys_prop->protocol_version);
		for (int i = 0; i < 4; i++)
			fprintf(stderr, "dbg5       sys_prop->sw_version[%d]:             %u\n", i, sys_prop->sw_version[i]);
		fprintf(stderr, "dbg5       sys_prop->fw_version:                 %u\n", sys_prop->fw_version);
		fprintf(stderr, "dbg5       sys_prop->hw_version:                 %u\n", sys_prop->hw_version);
		fprintf(stderr, "dbg5       sys_prop->transducer_sn:              %u\n", sys_prop->transducer_sn);
		fprintf(stderr, "dbg5       sys_prop->transceiver_sn:             %u\n", sys_prop->transceiver_sn);
		for (int i = 0; i < 8; i++)
			fprintf(stderr, "dbg5       sys_prop->spare[%d]:                  %u\n", i, sys_prop->spare[i]);
		fprintf(stderr, "dbg5       sys_prop->checksum:                   %u\n", sys_prop->checksum);
	}

	/* figure out size of output record */
	*size = 96;

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

		/* insert the data */
		index = 0;
		mb_put_binary_int(true, MBSYS_WASSP_SYNC, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, *size, &buffer[index]);
		index += 4;
		strncpy(&buffer[index], "SYS_PROP", 8);
		index += 8;
		mb_put_binary_int(true, sys_prop->version, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, sys_prop->product_type, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, sys_prop->protocol_version, &buffer[index]);
		index += 4;
		for (int i = 0; i < 4; i++) {
			mb_put_binary_int(true, sys_prop->sw_version[i], &buffer[index]);
			index += 4;
		}
		mb_put_binary_int(true, sys_prop->fw_version, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, sys_prop->hw_version, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, sys_prop->transducer_sn, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, sys_prop->transceiver_sn, &buffer[index]);
		index += 4;
		for (int i = 0; i < 8; i++) {
			mb_put_binary_int(true, sys_prop->spare[i], &buffer[index]);
			index += 4;
		}

		/* now add the checksum */
		mb_put_binary_int(true, sys_prop->checksum, &buffer[index]);
		index += 4;
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

int mbr_wasspenl_wr_sys_cfg1(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = &(store->sys_cfg1);

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		for (unsigned int i = 0; i < sys_cfg1->sys_cfg1_len; i++) {
			fprintf(stderr, "dbg5       sys_cfg1->sys_cfg1_data[%3d]:           %u\n", i, sys_cfg1->sys_cfg1_data[i]);
		}
	}

	/* figure out size of output record */
	*size = sys_cfg1->sys_cfg1_len;

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

		/* insert the data */
		memcpy(buffer, sys_cfg1->sys_cfg1_data, (size_t)sys_cfg1->sys_cfg1_len);
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

int mbr_wasspenl_wr_mcomment(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
	char *buffer;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
		fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_mcomment_struct *mcomment = &(store->mcomment);
	mcomment->checksum = 0x8806CBA5;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       mcomment->comment_length:             %u\n", mcomment->comment_length);
		fprintf(stderr, "dbg5       mcomment->comment_message:            %s\n", mcomment->comment_message);
		fprintf(stderr, "dbg5       mcomment->checksum:                   %u\n", mcomment->checksum);
	}

	/* figure out size of output record */
	*size = 24 + mcomment->comment_length;

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

		/* insert the data */
		index = 0;
		mb_put_binary_int(true, MBSYS_WASSP_SYNC, &buffer[index]);
		index += 4;
		mb_put_binary_int(true, *size, &buffer[index]);
		index += 4;
		strncpy(&buffer[index], "MCOMMENT", 8);
		index += 8;
		mb_put_binary_int(true, mcomment->comment_length, &buffer[index]);
		index += 4;
		memcpy(&buffer[index], mcomment->comment_message, (size_t)mcomment->comment_length);
		index += mcomment->comment_length;

		/* now add the checksum */
		mb_put_binary_int(true, mcomment->checksum, &buffer[index]);
		index += 4;
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

int mbr_wasspenl_wr_unknown1(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
	int status = MB_SUCCESS;
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_unknown1_struct *unknown1 = &(store->unknown1);

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		for (unsigned int i = 0; i < unknown1->unknown1_len; i++) {
			fprintf(stderr, "dbg5       unknown1->unknown1_data[%3d]:           %u\n", i, unknown1->unknown1_data[i]);
		}
	}

	/* figure out size of output record */
	*size = unknown1->unknown1_len;

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

		/* insert the data */
		memcpy(buffer, unknown1->unknown1_data, (size_t)unknown1->unknown1_len);
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
int mbr_wasspenl_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int status = MB_SUCCESS;
	char **bufferptr;
	char *buffer;
	unsigned int *bufferalloc;
	unsigned int size;
	size_t write_len;

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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;

	/* get saved values */
	bufferptr = (char **)&mb_io_ptr->saveptr1;
	buffer = (char *)*bufferptr;
	bufferalloc = (unsigned int *)&mb_io_ptr->save6;

	/* write the current data record */

	/* write GENBATHY record */
	if (store->kind == MB_DATA_DATA) {
		status = mbr_wasspenl_wr_genbathy(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);

		status = mbr_wasspenl_wr_corbathy(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

	/* write RAWSONAR record */
	else if (store->kind == MB_DATA_WATER_COLUMN) {
		status = mbr_wasspenl_wr_rawsonar(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

	/* write GEN_SENS record */
	else if (store->kind == MB_DATA_GEN_SENS) {
		status = mbr_wasspenl_wr_gen_sens(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

	/* write NVUPDATE record */
	else if (store->kind == MB_DATA_NAV) {
		status = mbr_wasspenl_wr_nvupdate(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

	/* write WCD_NAVI record */
	else if (store->kind == MB_DATA_WC_PICKS) {
		status = mbr_wasspenl_wr_wcd_navi(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

	/* write SENSPROP record */
	else if (store->kind == MB_DATA_SENSOR_PARAMETERS) {
		status = mbr_wasspenl_wr_sensprop(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

	/* write SYS_PROP record */
	else if (store->kind == MB_DATA_INSTALLATION) {
		status = mbr_wasspenl_wr_sys_prop(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

	/* write SYS_CFG1 record */
	else if (store->kind == MB_DATA_PARAMETER) {
		status = mbr_wasspenl_wr_sys_cfg1(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

	/* write MCOMMENT_ record */
	else if (store->kind == MB_DATA_COMMENT) {
		status = mbr_wasspenl_wr_mcomment(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

	/* write unknown1 record */
	else if (store->kind == MB_DATA_RAW_LINE) {
		status = mbr_wasspenl_wr_sys_cfg1(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
		buffer = (char *)*bufferptr;
		write_len = (size_t)size;
		status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
	}

#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr, "WASSPENL DATA WRITTEN: type:%d status:%d error:%d\n\n", store->kind, status, *error);
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
int mbr_wt_wasspenl(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

#ifdef MBR_WASSPENLDEBUG
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	fprintf(stderr, "About to call mbr_wasspenl_wr_data record kind:%d\n", store->kind);
#endif

	/* write next data to file */
	const int status = mbr_wasspenl_wr_data(verbose, mbio_ptr, store_ptr, error);

#ifdef MBR_WASSPENLDEBUG
	fprintf(stderr, "Done with mbr_wasspenl_wr_data: status:%d error:%d\n", status, *error);
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
int mbr_register_wasspenl(int verbose, void *mbio_ptr, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	status = mbr_info_wasspenl(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_wasspenl;
	mb_io_ptr->mb_io_format_free = &mbr_dem_wasspenl;
	mb_io_ptr->mb_io_store_alloc = &mbsys_wassp_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_wassp_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_wasspenl;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_wasspenl;
	mb_io_ptr->mb_io_dimensions = &mbsys_wassp_dimensions;
	mb_io_ptr->mb_io_pingnumber = &mbsys_wassp_pingnumber;
	mb_io_ptr->mb_io_sonartype = &mbsys_wassp_sonartype;
	mb_io_ptr->mb_io_sidescantype = NULL;
	mb_io_ptr->mb_io_extract = &mbsys_wassp_extract;
	mb_io_ptr->mb_io_insert = &mbsys_wassp_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_wassp_extract_nav;
	mb_io_ptr->mb_io_extract_nnav = NULL;
	mb_io_ptr->mb_io_insert_nav = &mbsys_wassp_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_wassp_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_wassp_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_wassp_detects;
	mb_io_ptr->mb_io_gains = &mbsys_wassp_gains;
	mb_io_ptr->mb_io_copyrecord = &mbsys_wassp_copy;
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
