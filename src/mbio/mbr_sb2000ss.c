/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sb2000ss.c	10/14/94
 *
 *    Copyright (c) 1994-2025 by
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
 * mbr_sb2000ss.c contains the functions for reading and writing
 * multibeam data in the SB2000SS format.
 * These functions include:
 *   mbr_alm_sb2000ss	- allocate read/write memory
 *   mbr_dem_sb2000ss	- deallocate read/write memory
 *   mbr_rt_sb2000ss	- read and translate data
 *   mbr_wt_sb2000ss	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 14, 1994
 *
 *
 */
/*--------------------------------------------------------------------
 * Notes on the MBF_SB2000SS data format:
 *   1. This data format is used to store sidescan data from
 *      Sea Beam 2000 sonars.
 *      This format was created and used by the Scripps
 *      Institution of Oceanography; most data files in this format
 *      consist of Sea Beam data collected on the R/V George Melville.
 *      This format is one of the "swathbathy" formats created by
 *      Jim Charters of Scripps.
 *   2. The data records consist of three logical records: the header
 *      record, the sensor specific record and the data record.
 *   3. The header record consists of 36 bytes, including the sizes
 *      of the following sensor specific and data records.
 *   4. The sensor specific records are 32 bytes long.
 *   5. The data record lengths are variable.
 *   6. Comments are included in text records, which are of variable
 *      length.
 *   7. Information on this format was obtained from the Geological
 *      Data Center and the Shipboard Computer Group at the Scripps
 *      Institution of Oceanography
 *
 * The kind value in the mbf_sb2000ss_struct indicates whether the
 * mbf_sb2000ss_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_sb2000.h"

/*--------------------------------------------------------------------*/
int mbr_info_sb2000ss(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_SB2000;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 2000;
	strncpy(format_name, "SB2000SS", MB_NAME_LENGTH);
	strncpy(system_name, "SB2000", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_SB2000SS\nInformal Description: SIO Swath-bathy SeaBeam 2000 format\nAttributes:          "
	        " SeaBeam 2000, sidescan,\n                      1000 pixels for 4-bit sidescan,\n                      2000 pixels "
	        "for 12+-bit sidescan,\n                      binary,  SIO.\n",
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
int mbr_alm_sb2000ss(int verbose, void *mbio_ptr, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_sb2000_struct), &mb_io_ptr->store_data, error);

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
int mbr_dem_sb2000ss(int verbose, void *mbio_ptr, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->store_data, error);

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
int mbr_rt_sb2000ss(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int read_status;
	char buffer[2 * MBSYS_SB2000_PIXELS + 4];
	unsigned short *short_ptr;
	short test_sensor_size = 0;
  short test_data_size = 0;

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
	struct mbsys_sb2000_struct *store = (struct mbsys_sb2000_struct *)store_ptr;

	/* read next header record from file */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	int skip = 0;
	bool found = false;
	int status = MB_SUCCESS;
	if ((status = fread(buffer, 1, MBSYS_SB2000_HEADER_SIZE, mb_io_ptr->mbfp)) == MBSYS_SB2000_HEADER_SIZE) {
		mb_io_ptr->file_bytes += status;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;

		/* check if header is ok */
		if (strncmp(&buffer[34], "SR", 2) == 0 || strncmp(&buffer[34], "RS", 2) == 0 || strncmp(&buffer[34], "SP", 2) == 0 ||
		    strncmp(&buffer[34], "TR", 2) == 0 || strncmp(&buffer[34], "IR", 2) == 0 || strncmp(&buffer[34], "AT", 2) == 0 ||
		    strncmp(&buffer[34], "SC", 2) == 0) {
			mb_get_binary_short(false, &buffer[26], &test_sensor_size);
			mb_get_binary_short(false, &buffer[28], &test_data_size);
			if (test_sensor_size <= 32 && test_data_size <= 2 * MBSYS_SB2000_PIXELS + 4)
				found = true;
		}
	}
	else {
		mb_io_ptr->file_bytes += status;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* if not a good header search through file to find one */
	while (status == MB_SUCCESS && !found) {
		/* shift bytes by one */
		for (int i = 0; i < MBSYS_SB2000_HEADER_SIZE - 1; i++)
			buffer[i] = buffer[i + 1];
		mb_io_ptr->file_pos += 1;
		skip++;

		/* read next byte */
		if ((read_status = fread(&buffer[MBSYS_SB2000_HEADER_SIZE - 1], 1, 1, mb_io_ptr->mbfp)) == 1) {
			mb_io_ptr->file_bytes += read_status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;

			if (strncmp(&buffer[34], "SR", 2) == 0 || strncmp(&buffer[34], "RS", 2) == 0 || strncmp(&buffer[34], "SP", 2) == 0 ||
			    strncmp(&buffer[34], "TR", 2) == 0 || strncmp(&buffer[34], "IR", 2) == 0 || strncmp(&buffer[34], "AT", 2) == 0 ||
			    strncmp(&buffer[34], "SC", 2) == 0) {
				mb_get_binary_short(false, &buffer[26], &test_sensor_size);
				mb_get_binary_short(false, &buffer[28], &test_data_size);
				if (test_sensor_size <= 32 && test_data_size <= 2 * MBSYS_SB2000_PIXELS + 4)
					found = true;
			}
		}
		else {
			found = true;
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
	}

	/* report data skips */
	if (skip > 0 && verbose >= 2)
		fprintf(stderr, "\ndgb2           DATA SKIPPED: %d bytes\n", skip);

	/* get header values */
	mb_get_binary_short(false, &buffer[0], &store->year);
	mb_get_binary_short(false, &buffer[2], &store->day);
	mb_get_binary_short(false, &buffer[4], &store->min);
	mb_get_binary_short(false, &buffer[6], &store->sec);
	mb_get_binary_int(false, &buffer[8], &store->lat);
	mb_get_binary_int(false, &buffer[12], &store->lon);
	mb_get_binary_short(false, &buffer[16], &store->heading);
	mb_get_binary_short(false, &buffer[18], &store->course);
	mb_get_binary_short(false, &buffer[20], &store->speed);
	mb_get_binary_short(false, &buffer[22], &store->speed_ps);
	mb_get_binary_short(false, &buffer[24], &store->quality);
	mb_get_binary_short(false, &buffer[26], (short *)&store->sensor_size);
	mb_get_binary_short(false, &buffer[28], (short *)&store->data_size);
	store->speed_ref[0] = buffer[30];
	store->speed_ref[1] = buffer[31];
	store->sensor_type[0] = buffer[32];
	store->sensor_type[1] = buffer[33];
	store->data_type[0] = buffer[34];
	store->data_type[1] = buffer[35];

	/* check for unintelligible records */
	if (status == MB_SUCCESS) {
		if ((strncmp(store->sensor_type, "SS", 2) != 0 || strncmp(store->data_type, "SC", 2) != 0) &&
		    strncmp(store->data_type, "TR", 2) != 0 && strncmp(store->data_type, "SP", 2) != 0) {
			/* read rest of record */
			for (int i = 0; (i < store->sensor_size + store->data_size) && status == MB_SUCCESS; i++) {
				if ((read_status = fread(buffer, 1, 1, mb_io_ptr->mbfp)) != 1) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					store->kind = MB_DATA_NONE;
				}
				else {
					mb_io_ptr->file_bytes += read_status;
				}
			}

			/* if eof not reached set unintelligible error */
			if (status == MB_SUCCESS) {
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				store->kind = MB_DATA_NONE;
			}
		}
		else if (strncmp(store->data_type, "SC", 2) == 0) {
			store->kind = MB_DATA_DATA;
		}
		else if (strncmp(store->data_type, "SP", 2) == 0) {
			store->kind = MB_DATA_VELOCITY_PROFILE;
		}
		else {
			store->kind = MB_DATA_COMMENT;
		}
	}

	/* fix incorrect header records */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && store->data_size == 1000) {
		store->sensor_size = 32;
		store->data_size = 1001;
	}

	if (status == MB_SUCCESS && verbose >= 5) {
		fprintf(stderr, "\ndbg5  New header record in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  New header values:\n");
		fprintf(stderr, "dbg5       year:       %d\n", store->year);
		fprintf(stderr, "dbg5       day:        %d\n", store->day);
		fprintf(stderr, "dbg5       min:        %d\n", store->min);
		fprintf(stderr, "dbg5       sec:        %d\n", store->sec);
		fprintf(stderr, "dbg5       lat:        %d\n", store->lat);
		fprintf(stderr, "dbg5       lon:        %d\n", store->lon);
		fprintf(stderr, "dbg5       heading:    %d\n", store->heading);
		fprintf(stderr, "dbg5       course:     %d\n", store->course);
		fprintf(stderr, "dbg5       speed:      %d\n", store->speed);
		fprintf(stderr, "dbg5       speed_ps:   %d\n", store->speed_ps);
		fprintf(stderr, "dbg5       quality:    %d\n", store->quality);
		fprintf(stderr, "dbg5       sensor size:%d\n", store->sensor_size);
		fprintf(stderr, "dbg5       data size:  %d\n", store->data_size);
		fprintf(stderr, "dbg5       speed_ref:  %c%c\n", store->speed_ref[0], store->speed_ref[1]);
		fprintf(stderr, "dbg5       sensor_type:%c%c\n", store->sensor_type[0], store->sensor_type[1]);
		fprintf(stderr, "dbg5       data_type:  %c%c\n", store->data_type[0], store->data_type[1]);
	}

	/* read sensor record from file */
	if (status == MB_SUCCESS && store->sensor_size > 0) {
		if ((status = fread(buffer, 1, store->sensor_size, mb_io_ptr->mbfp)) == store->sensor_size) {
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
	}

	/* extract sensor data */
	if (status == MB_SUCCESS && store->sensor_size > 0) {
		/* extract the values */
		mb_get_binary_int(false, &buffer[0], &store->ping_number);
		mb_get_binary_short(false, &buffer[4], &store->ping_length);
		mb_get_binary_short(false, &buffer[6], &store->pixel_size);
		mb_get_binary_short(false, &buffer[8], &store->ss_min);
		mb_get_binary_short(false, &buffer[10], &store->ss_max);
		mb_get_binary_short(false, &buffer[12], &store->sample_rate);
		mb_get_binary_short(false, &buffer[14], &store->start_time);
		mb_get_binary_short(false, &buffer[16], &store->tot_slice);
		mb_get_binary_short(false, &buffer[18], &store->pixels_ss);
		for (int i = 0; i < store->sensor_size - 20; i++)
			store->spare_ss[i] = buffer[18 + i];
	}

	/* read data record from file */
	if (status == MB_SUCCESS && store->data_size > 0) {
		if ((status = fread(buffer, 1, store->data_size, mb_io_ptr->mbfp)) == store->data_size) {
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
	}

	/* extract sidescan data */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA) {

		/* fix some files with incorrect sensor records */
		if (buffer[0] == 'G' && (store->data_size == 1001 || store->data_size == 1004) && store->pixels_ss != 1000) {
			store->pixels_ss = 1000;
		}

		/* correct data size if needed */
		if (buffer[0] == 'G' && store->data_size == 1001) {
			store->data_size = 1004;
			store->ss[1001] = 'G';
			store->ss[1002] = 'G';
			store->ss[1003] = 'G';
		}

		/* fix some files with incorrect data size id's */
		if (buffer[0] == 'R' && (2 * store->pixels_ss) > store->data_size && store->pixels_ss <= MBSYS_SB2000_PIXELS) {
			buffer[0] = 'G';
		}

		/* deal with 1-byte data */
		if (buffer[0] == 'G') {
			store->ss_type = 'G';
			for (int i = 0; i < store->pixels_ss; i++) {
				store->ss[i] = buffer[i + 1];
			}
		}

		/* deal with 2-byte data */
		else if (buffer[0] == 'R') {
			store->ss_type = 'R';
			for (int i = 0; i < store->pixels_ss; i++) {
				mb_get_binary_short(false, (short *)&(buffer[4 + 2 * i]), (short *)&(store->ss[2 * i]));
			}
		}

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  New data record read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5  New data values:\n");
			fprintf(stderr, "dbg5       ping_number:     %d\n", store->ping_number);
			fprintf(stderr, "dbg5       ping_length:     %d\n", store->ping_length);
			fprintf(stderr, "dbg5       pixel_size:      %d\n", store->pixel_size);
			fprintf(stderr, "dbg5       ss_min:          %d\n", store->ss_min);
			fprintf(stderr, "dbg5       ss_max:          %d\n", store->ss_max);
			fprintf(stderr, "dbg5       sample_rate:     %d\n", store->sample_rate);
			fprintf(stderr, "dbg5       start_time:      %d\n", store->start_time);
			fprintf(stderr, "dbg5       tot_slice:       %d\n", store->tot_slice);
			fprintf(stderr, "dbg5       pixels_ss:       %d\n", store->pixels_ss);
			fprintf(stderr, "dbg5       spare_ss:        ");
			for (int i = 0; i < 12; i++)
				fprintf(stderr, "%c", store->spare_ss[i]);
			fprintf(stderr, "dbg5       sidescan_type:%c\n", store->ss_type);
			if (store->ss_type == 'G') {
				for (int i = 0; i < store->pixels_ss; i++)
					fprintf(stderr, "dbg5       pixel: %d  ss: %d\n", i, store->ss[i]);
			}
			else if (store->ss_type == 'R') {
				for (int i = 1; i <= store->pixels_ss; i++) {
					short_ptr = (unsigned short *)&store->ss[i * 2];
					fprintf(stderr, "dbg5       pixel: %d  ss: %d\n", i, *short_ptr);
				}
			}
		}
	}

	/* extract velocity profile record */
	if (status == MB_SUCCESS && store->kind == MB_DATA_VELOCITY_PROFILE) {
		/* extract the values */
		mb_get_binary_int(false, &buffer[0], &store->svp_mean);
		mb_get_binary_short(false, &buffer[4], &store->svp_number);
		mb_get_binary_short(false, &buffer[6], &store->svp_spare);
		for (int i = 0; i < MIN(store->svp_number, 30); i++) {
			mb_get_binary_short(false, &buffer[8 + i * 4], &store->svp_depth[i]);
			mb_get_binary_short(false, &buffer[10 + i * 4], &store->svp_vel[i]);
		}
		mb_get_binary_short(false, &buffer[128], &store->vru1);
		mb_get_binary_short(false, &buffer[130], &store->vru1_port);
		mb_get_binary_short(false, &buffer[132], &store->vru1_forward);
		mb_get_binary_short(false, &buffer[134], &store->vru1_vert);
		mb_get_binary_short(false, &buffer[136], &store->vru2);
		mb_get_binary_short(false, &buffer[138], &store->vru2_port);
		mb_get_binary_short(false, &buffer[140], &store->vru2_forward);
		mb_get_binary_short(false, &buffer[142], &store->vru2_vert);

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  New svp record read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5  New svp values:\n");
			fprintf(stderr, "dbg5       svp_mean:     %d\n", store->svp_mean);
			fprintf(stderr, "dbg5       svp_number:   %d\n", store->svp_number);
			fprintf(stderr, "dbg5       svp_spare:    %d\n", store->svp_spare);
			for (int i = 0; i < 30; i++)
				fprintf(stderr, "dbg5       %d  depth: %d  vel: %d\n", i, store->svp_depth[i], store->svp_vel[i]);
			fprintf(stderr, "dbg5       vru1:         %d\n", store->vru1);
			fprintf(stderr, "dbg5       vru1_port:    %d\n", store->vru1_port);
			fprintf(stderr, "dbg5       vru1_forward: %d\n", store->vru1_forward);
			fprintf(stderr, "dbg5       vru1_vert:    %d\n", store->vru1_vert);
			fprintf(stderr, "dbg5       vru2:         %d\n", store->vru2);
			fprintf(stderr, "dbg5       vru2_port:    %d\n", store->vru2_port);
			fprintf(stderr, "dbg5       vru2_forward: %d\n", store->vru2_forward);
			fprintf(stderr, "dbg5       vru2_vert:    %d\n", store->vru2_vert);
			fprintf(stderr, "dbg5       pitch_bias:    %d\n", store->pitch_bias);
			fprintf(stderr, "dbg5       roll_bias:    %d\n", store->roll_bias);
			fprintf(stderr, "dbg5       vru:          %c%c%c%c%c%c%c%c\n", store->vru[0], store->vru[1], store->vru[2],
			        store->vru[3], store->vru[4], store->vru[5], store->vru[6], store->vru[7]);
		}
	}

	/* extract comment record */
	if (status == MB_SUCCESS && store->kind == MB_DATA_COMMENT) {
		strncpy(store->comment, buffer, MIN(store->data_size, MBSYS_SB2000_COMMENT_LENGTH - 1));
		store->comment[MIN(store->data_size, MBSYS_SB2000_COMMENT_LENGTH - 1)] = '\0';

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  New comment record read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5  New comment:\n");
			fprintf(stderr, "dbg5       comment:   %s\n", store->comment);
		}
	}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = store->kind;
	mb_io_ptr->new_error = *error;

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
int mbr_wt_sb2000ss(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char buffer[2 * MBSYS_SB2000_PIXELS + 4];
	unsigned short *short_ptr;

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
	struct mbsys_sb2000_struct *store = (struct mbsys_sb2000_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Ready to write data in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:       %d\n", store->kind);
		fprintf(stderr, "dbg5       error:      %d\n", *error);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Header record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  Header values:\n");
		fprintf(stderr, "dbg5       year:       %d\n", store->year);
		fprintf(stderr, "dbg5       day:        %d\n", store->day);
		fprintf(stderr, "dbg5       min:        %d\n", store->min);
		fprintf(stderr, "dbg5       sec:        %d\n", store->sec);
		fprintf(stderr, "dbg5       lat:        %d\n", store->lat);
		fprintf(stderr, "dbg5       lon:        %d\n", store->lon);
		fprintf(stderr, "dbg5       heading:    %d\n", store->heading);
		fprintf(stderr, "dbg5       course:     %d\n", store->course);
		fprintf(stderr, "dbg5       speed:      %d\n", store->speed);
		fprintf(stderr, "dbg5       speed_ps:   %d\n", store->speed_ps);
		fprintf(stderr, "dbg5       quality:    %d\n", store->quality);
		fprintf(stderr, "dbg5       sensor size:%d\n", store->sensor_size);
		fprintf(stderr, "dbg5       data size:  %d\n", store->data_size);
		fprintf(stderr, "dbg5       speed_ref:  %c%c\n", store->speed_ref[0], store->speed_ref[1]);
		fprintf(stderr, "dbg5       sensor_type:%c%c\n", store->sensor_type[0], store->sensor_type[1]);
		fprintf(stderr, "dbg5       data_type:  %c%c\n", store->data_type[0], store->data_type[1]);
	}

	if (verbose >= 5 && store->kind == MB_DATA_DATA) {
		fprintf(stderr, "\ndbg5  Sensor record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  Sensor values:\n");
		fprintf(stderr, "dbg5       ping_number:     %d\n", store->ping_number);
		fprintf(stderr, "dbg5       ping_length:     %d\n", store->ping_length);
		fprintf(stderr, "dbg5       pixel_size:      %d\n", store->pixel_size);
		fprintf(stderr, "dbg5       ss_min:          %d\n", store->ss_min);
		fprintf(stderr, "dbg5       ss_max:          %d\n", store->ss_max);
		fprintf(stderr, "dbg5       sample_rate:     %d\n", store->sample_rate);
		fprintf(stderr, "dbg5       start_time:      %d\n", store->start_time);
		fprintf(stderr, "dbg5       tot_slice:       %d\n", store->tot_slice);
		fprintf(stderr, "dbg5       pixels_ss:       %d\n", store->pixels_ss);
		fprintf(stderr, "dbg5       spare_ss:        ");
		for (int i = 0; i < 12; i++)
			fprintf(stderr, "%c", store->spare_ss[i]);
		fprintf(stderr, "\n");
	}

	if (verbose >= 5 && store->kind == MB_DATA_VELOCITY_PROFILE) {
		fprintf(stderr, "\ndbg5  SVP record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  SVP values:\n");
		fprintf(stderr, "dbg5       svp_mean:     %d\n", store->svp_mean);
		fprintf(stderr, "dbg5       svp_number:   %d\n", store->svp_number);
		fprintf(stderr, "dbg5       svp_spare:   %d\n", store->svp_spare);
		for (int i = 0; i < 30; i++)
			fprintf(stderr, "dbg5       %d  depth: %d  vel: %d\n", i, store->svp_depth[i], store->svp_vel[i]);
		fprintf(stderr, "dbg5       vru1:         %d\n", store->vru1);
		fprintf(stderr, "dbg5       vru1_port:    %d\n", store->vru1_port);
		fprintf(stderr, "dbg5       vru1_forward: %d\n", store->vru1_forward);
		fprintf(stderr, "dbg5       vru1_vert:    %d\n", store->vru1_vert);
		fprintf(stderr, "dbg5       vru2:         %d\n", store->vru2);
		fprintf(stderr, "dbg5       vru2_port:    %d\n", store->vru2_port);
		fprintf(stderr, "dbg5       vru2_forward: %d\n", store->vru2_forward);
		fprintf(stderr, "dbg5       vru2_vert:    %d\n", store->vru2_vert);
		fprintf(stderr, "dbg5       pitch_bias:    %d\n", store->pitch_bias);
		fprintf(stderr, "dbg5       roll_bias:    %d\n", store->roll_bias);
		fprintf(stderr, "dbg5       vru:          %c%c%c%c%c%c%c%c\n", store->vru[0], store->vru[1], store->vru[2], store->vru[3],
		        store->vru[4], store->vru[5], store->vru[6], store->vru[7]);
	}

	if (verbose >= 5 && store->kind == MB_DATA_DATA) {
		fprintf(stderr, "\ndbg5  Data record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  Data values:\n");
		fprintf(stderr, "dbg5       sidescan_type:%c\n", store->ss_type);
		if (store->ss_type == 'G') {
			for (int i = 0; i < store->pixels_ss; i++)
				fprintf(stderr, "dbg5       pixel: %d  ss: %d\n", i, store->ss[i]);
		}
		else if (store->ss_type == 'R') {
			for (int i = 1; i <= store->pixels_ss; i++) {
				short_ptr = (unsigned short *)&store->ss[i * 2];
				fprintf(stderr, "dbg5       pixel: %d  ss: %d\n", i, *short_ptr);
			}
		}
	}

	if (verbose >= 5 && store->kind == MB_DATA_COMMENT) {
		fprintf(stderr, "\ndbg5  Comment record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  Comment:\n");
		fprintf(stderr, "dbg5       comment:   %s\n", store->comment);
	}

	int status = MB_SUCCESS;

	/* put header values */
	if (status == MB_SUCCESS) {
		/* put header values */
		mb_put_binary_short(false, store->year, &buffer[0]);
		mb_put_binary_short(false, store->day, &buffer[2]);
		mb_put_binary_short(false, store->min, &buffer[4]);
		mb_put_binary_short(false, store->sec, &buffer[6]);
		mb_put_binary_int(false, store->lat, &buffer[8]);
		mb_put_binary_int(false, store->lon, &buffer[12]);
		mb_put_binary_short(false, store->heading, &buffer[16]);
		mb_put_binary_short(false, store->course, &buffer[18]);
		mb_put_binary_short(false, store->speed, &buffer[20]);
		mb_put_binary_short(false, store->speed_ps, &buffer[22]);
		mb_put_binary_short(false, store->quality, &buffer[24]);
		mb_put_binary_short(false, store->sensor_size, &buffer[26]);
		mb_put_binary_short(false, store->data_size, &buffer[28]);
		buffer[30] = store->speed_ref[0];
		buffer[31] = store->speed_ref[1];
		buffer[32] = store->sensor_type[0];
		buffer[33] = store->sensor_type[1];
		buffer[34] = store->data_type[0];
		buffer[35] = store->data_type[1];

		/* write header record to file */
		if ((status = fwrite(buffer, 1, MBSYS_SB2000_HEADER_SIZE, mb_io_ptr->mbfp)) == MBSYS_SB2000_HEADER_SIZE) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	/* put sensor data */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && store->sensor_size > 0) {
		/* put sensor values */
		mb_put_binary_int(false, store->ping_number, &buffer[0]);
		mb_put_binary_short(false, store->ping_length, &buffer[4]);
		mb_put_binary_short(false, store->pixel_size, &buffer[6]);
		mb_put_binary_short(false, store->ss_min, &buffer[8]);
		mb_put_binary_short(false, store->ss_max, &buffer[10]);
		mb_put_binary_short(false, store->sample_rate, &buffer[12]);
		mb_put_binary_short(false, store->start_time, &buffer[14]);
		mb_put_binary_short(false, store->tot_slice, &buffer[16]);
		mb_put_binary_short(false, store->pixels_ss, &buffer[18]);
		for (int i = 0; i < store->sensor_size - 20; i++)
			buffer[18 + i] = store->spare_ss[i];

		/* write sensor record to file */
		if ((status = fwrite(buffer, 1, store->sensor_size, mb_io_ptr->mbfp)) == store->sensor_size) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	if (status == MB_SUCCESS && store->kind == MB_DATA_VELOCITY_PROFILE && store->data_size > 0) {
		/* extract the values */
		mb_put_binary_int(false, store->svp_mean, &buffer[0]);
		mb_put_binary_short(false, store->svp_number, &buffer[4]);
		mb_put_binary_short(false, store->svp_spare, &buffer[6]);
		for (int i = 0; i < MIN(store->svp_number, 30); i++) {
			mb_put_binary_short(false, store->svp_depth[i], &buffer[8 + i * 4]);
			mb_put_binary_short(false, store->svp_vel[i], &buffer[10 + i * 4]);
		}
		mb_put_binary_short(false, store->vru1, &buffer[128]);
		mb_put_binary_short(false, store->vru1_port, &buffer[130]);
		mb_put_binary_short(false, store->vru1_forward, &buffer[132]);
		mb_put_binary_short(false, store->vru1_vert, &buffer[134]);
		mb_put_binary_short(false, store->vru2, &buffer[136]);
		mb_put_binary_short(false, store->vru2_port, &buffer[138]);
		mb_put_binary_short(false, store->vru2_forward, &buffer[140]);
		mb_put_binary_short(false, store->vru2_vert, &buffer[142]);

		/* write svp profile */
		if ((status = fwrite(buffer, 1, store->data_size, mb_io_ptr->mbfp)) == store->data_size) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && store->data_size > 0) {
		/* put the values */

		/* deal with 1-byte data */
		if (store->ss_type == 'G') {
			buffer[0] = 'G';
			for (int i = 0; i < store->pixels_ss; i++) {
				buffer[i + 1] = store->ss[i];
			}
			buffer[store->pixels_ss + 1] = 'G';
			buffer[store->pixels_ss + 2] = 'G';
			buffer[store->pixels_ss + 3] = 'G';
		}

		/* deal with 2-byte data */
		else if (store->ss_type == 'R') {
			buffer[0] = 'R';
			buffer[1] = 'R';
			buffer[2] = 'R';
			buffer[3] = 'R';
			for (int i = 0; i < store->pixels_ss; i++) {
				mb_get_binary_short(false, (short *)&(store->ss[2 * i]), (short *)&(buffer[4 + 2 * i]));
			}
		}

		/* write survey data */
		if ((status = fwrite(buffer, 1, store->data_size, mb_io_ptr->mbfp)) == store->data_size) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	if (status == MB_SUCCESS && store->kind == MB_DATA_COMMENT && store->data_size > 0) {
		/* put the comment */
		strncpy(buffer, store->comment, MIN(store->data_size, MBSYS_SB2000_COMMENT_LENGTH - 1));
		buffer[MIN(store->data_size, MBSYS_SB2000_COMMENT_LENGTH - 1)] = '\0';

		/* write comment */
		if ((status = fwrite(buffer, 1, store->data_size, mb_io_ptr->mbfp)) == store->data_size) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
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
int mbr_register_sb2000ss(int verbose, void *mbio_ptr, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	status = mbr_info_sb2000ss(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sb2000ss;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sb2000ss;
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb2000_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_sb2000_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sb2000ss;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sb2000ss;
	mb_io_ptr->mb_io_dimensions = &mbsys_sb2000_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_sb2000_extract;
	mb_io_ptr->mb_io_insert = &mbsys_sb2000_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb2000_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb2000_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb2000_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_sb2000_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_sb2000_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb2000_copy;
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
