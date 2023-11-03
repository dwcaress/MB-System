/*--------------------------------------------------------------------
 *    The MB-system:	mbr_soirovnv.c	5/20/99
 *
 *    Copyright (c) 1999-2023 by
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
 * mbr_soirovnv.c contains the functions for reading and writing
 * multibeam data in the SOIROVNV format.
 * These functions include:
 *   mbr_alm_soirovnv	- allocate read/write memory
 *   mbr_dem_soirovnv	- deallocate read/write memory
 *   mbr_rt_soirovnv	- read and translate data
 *   mbr_wt_soirovnv	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 23, 2023
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
#include "mbsys_singlebeam.h"

/*--------------------------------------------------------------------*/
int mbr_info_soirovnv(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_SINGLEBEAM;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "SOIROVNV", MB_NAME_LENGTH);
	strncpy(system_name, "SINGLEBEAM", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_SOIROVNV\nInformal Description: SOI ROV navigation format(s)\nAttributes:           SOI "
	        "navigation, ascii, Schmidt Ocean Institute\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = false;
	*traveltime = false;
	*beam_flagging = false;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_NONE;
	*attitude_source = MB_DATA_NONE;
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
int mbr_alm_soirovnv(int verbose, void *mbio_ptr, int *error) {
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
	const int status = mbsys_singlebeam_alloc(verbose, mbio_ptr, &(mb_io_ptr->store_data), error);

	/* set number of records read or written to zero */
	mb_io_ptr->save1 = 0;

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
int mbr_dem_soirovnv(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	const int status = mbsys_singlebeam_deall(verbose, mbio_ptr, &(mb_io_ptr->store_data), error);

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
int mbr_rt_soirovnv(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structure */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_singlebeam_struct *store = (struct mbsys_singlebeam_struct *)store_ptr;

	/* get pointer to read counter */
	int *read_count = (int *)&mb_io_ptr->save1;

	/* set file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	int status = MB_SUCCESS;

	/* read next record */
	char line[MB_COMMENT_MAXLINE] = "";
	char *line_ptr = fgets(line, MB_PATH_MAXLINE, mb_io_ptr->mbfp);
	if (line_ptr != NULL) {
		/* set status */
		mb_io_ptr->file_bytes += strlen(line);
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

	/* handle the data */
	if (status == MB_SUCCESS && line[0] == '#') {
		store->kind = MB_DATA_COMMENT;
		strncpy(store->comment, &line[1], MB_COMMENT_MAXLINE);
		if (store->comment[strlen(store->comment) - 1] == '\n')
			store->comment[strlen(store->comment) - 1] = '\0';
		(*read_count)++;
	}
	else if (status == MB_SUCCESS) {
		store->kind = MB_DATA_DATA;
		strncpy(store->comment, &line[1], MB_COMMENT_MAXLINE);
		
		/* Included fields:
			Timestamp,Header,Roll_deg,Pitch_deg,HeadingTrue_deg,OrientStatus,
			Latitude_ddeg,Longitude_ddeg,PositionStatus,
			VelocityFwd_m/s,VelocityStbd_m/s,VelocityDown_m/s,
			Altitude_m,Altitude_Status,Depth_m,Depth_Used
		  Sample string:
				"2023-03-23T02:26:28.576022Z,$SPRINT,-1.4502,-9.43726,132.863,1,"
				"23.47362268,-44.98669012,1,-0.068,0.095,0.062,16.31,1,3989.77,1,"
			*/

		/* read data */
		int nget = sscanf(line, "%d-%d-%dT%d:%d:%d.%dZ,$SPRINT,%lf,%lf,%lf,%d,%lf,%lf,%d,%lf,%lf,%lf,%lf,%d,%lf,%d,",
							&store->time_i[0], &store->time_i[1], &store->time_i[2],
		             		&store->time_i[3], &store->time_i[4], &store->time_i[5], &store->time_i[6], 
		             		&store->roll, &store->pitch, &store->heading, &store->orientation_status, 
		             		&store->latitude, &store->longitude, &store->position_status, 
		             		&store->velocity_fwd, &store->velocity_stbd, &store->velocity_down, 
		             		&store->rov_altitude, &store->altitude_status, &store->sonar_depth, &store->depth_used);
		if ((nget == 21) && store->time_i[0] != 0) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;

			mb_get_time(verbose, store->time_i, &store->time_d);				
			store->heading = 0.0; //no heading in USBL tracking
			if (store->sonar_depth != 0.0 && store->altitude_status)
				store->bath = store->sonar_depth + store->rov_altitude;
			
			(*read_count)++;
		}

		/* catch erroneous records */
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	if (status == MB_SUCCESS && verbose >= 4) {
		if (store->kind == MB_DATA_DATA) {
			fprintf(stderr, "\ndbg4  Data read in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Values read:\n");
			fprintf(stderr, "dbg4       time_i[0]:           %d\n", store->time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:           %d\n", store->time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:           %d\n", store->time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:           %d\n", store->time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:           %d\n", store->time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:           %d\n", store->time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:           %d\n", store->time_i[6]);
			fprintf(stderr, "dbg4       time_d:              %f\n", store->time_d);
			fprintf(stderr, "dbg4       roll:                %f\n", store->roll);
			fprintf(stderr, "dbg4       pitch:               %f\n", store->pitch);
			fprintf(stderr, "dbg4       heading:             %f\n", store->heading);
			fprintf(stderr, "dbg4       orientation_status:  %d\n", store->orientation_status);
			fprintf(stderr, "dbg4       latitude:            %f\n", store->latitude);
			fprintf(stderr, "dbg4       longitude:           %f\n", store->longitude);
			fprintf(stderr, "dbg4       position_status:     %d\n", store->position_status);
			fprintf(stderr, "dbg4       velocity_fwd:        %f\n", store->velocity_fwd);
			fprintf(stderr, "dbg4       velocity_stbd:       %f\n", store->velocity_stbd);
			fprintf(stderr, "dbg4       velocity_down:       %f\n", store->velocity_down);
			fprintf(stderr, "dbg4       rov_altitude:        %f\n", store->rov_altitude);
			fprintf(stderr, "dbg4       altitude_status:     %d\n", store->altitude_status);
			fprintf(stderr, "dbg4       sonar_depth:         %f\n", store->sonar_depth);
			fprintf(stderr, "dbg4       depth_used:          %d\n", store->depth_used);
			fprintf(stderr, "dbg4       error:               %d\n", *error);
			fprintf(stderr, "dbg4       status:              %d\n", status);
		}
		else if (store->kind == MB_DATA_COMMENT) {
			fprintf(stderr, "\ndbg4  Data read in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Values read:\n");
			fprintf(stderr, "dbg4       comment:      %s\n", store->comment);
		}
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
}
/*--------------------------------------------------------------------*/
int mbr_wt_soirovnv(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor and data structure*/
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_singlebeam_struct *store = (struct mbsys_singlebeam_struct *)store_ptr;

	/* get pointer to write counter */
	int *write_count = (int *)&mb_io_ptr->save1;

	if (store != NULL && verbose >= 4) {
		if (store->kind == MB_DATA_DATA) {
			fprintf(stderr, "\ndbg4  Data to be written in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Values to be written:\n");
			fprintf(stderr, "dbg4       time_i[0]:           %d\n", store->time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:           %d\n", store->time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:           %d\n", store->time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:           %d\n", store->time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:           %d\n", store->time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:           %d\n", store->time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:           %d\n", store->time_i[6]);
			fprintf(stderr, "dbg4       time_d:              %f\n", store->time_d);
			fprintf(stderr, "dbg4       roll:                %f\n", store->roll);
			fprintf(stderr, "dbg4       pitch:               %f\n", store->pitch);
			fprintf(stderr, "dbg4       heading:             %f\n", store->heading);
			fprintf(stderr, "dbg4       orientation_status:  %d\n", store->orientation_status);
			fprintf(stderr, "dbg4       latitude:            %f\n", store->latitude);
			fprintf(stderr, "dbg4       longitude:           %f\n", store->longitude);
			fprintf(stderr, "dbg4       position_status:     %d\n", store->position_status);
			fprintf(stderr, "dbg4       velocity_fwd:        %f\n", store->velocity_fwd);
			fprintf(stderr, "dbg4       velocity_stbd:       %f\n", store->velocity_stbd);
			fprintf(stderr, "dbg4       velocity_down:       %f\n", store->velocity_down);
			fprintf(stderr, "dbg4       rov_altitude:        %f\n", store->rov_altitude);
			fprintf(stderr, "dbg4       altitude_status:     %d\n", store->altitude_status);
			fprintf(stderr, "dbg4       sonar_depth:         %f\n", store->sonar_depth);
			fprintf(stderr, "dbg4       depth_used:          %d\n", store->depth_used);
			fprintf(stderr, "dbg4       error:               %d\n", *error);
		}
		else if (store->kind == MB_DATA_COMMENT) {
			fprintf(stderr, "\ndbg4  Data to be written in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Values to be written:\n");
			fprintf(stderr, "dbg4       comment:      %s\n", store->comment);
		}
	}

	int status = MB_SUCCESS;

	/* write the record */
	if (store != NULL) {
		char line[MB_COMMENT_MAXLINE] = "";
		/* deal with comment */
		if (store->kind == MB_DATA_COMMENT) {
			line[0] = '#';
			strncpy(&line[1], store->comment, MB_COMMENT_MAXLINE - 2);
			const int len = strlen(line);
			if (line[len - 1] != '\n') {
				line[len] = '\n';
				line[len + 1] = '\0';
			}
		}
		else if (store->kind == MB_DATA_DATA) {
			snprintf(line, MB_COMMENT_MAXLINE, 
							"%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6dZ,$SPRINT,%.4f,%.5f,%.3f,%d,%.8f,%.8f,%d,%.3f,%.3f,%.3f,%.2f,%d,%.2f,%d,\n",
							store->time_i[0], store->time_i[1], store->time_i[2],
		             		store->time_i[3], store->time_i[4], store->time_i[5], store->time_i[6], 
		             		store->roll, store->pitch, store->heading, store->orientation_status, 
		             		store->latitude, store->longitude, store->position_status, 
		             		store->velocity_fwd, store->velocity_stbd, store->velocity_down, 
		             		store->rov_altitude, store->altitude_status,store->sonar_depth,store->depth_used);
		}

		/* write data */
		if (fputs(line, mb_io_ptr->mbfp) == EOF) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			(*write_count)++;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_register_soirovnv(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_soirovnv(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_soirovnv;
	mb_io_ptr->mb_io_format_free = &mbr_dem_soirovnv;
	mb_io_ptr->mb_io_store_alloc = &mbsys_singlebeam_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_singlebeam_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_soirovnv;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_soirovnv;
	mb_io_ptr->mb_io_dimensions = &mbsys_singlebeam_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_singlebeam_extract;
	mb_io_ptr->mb_io_insert = &mbsys_singlebeam_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_singlebeam_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_singlebeam_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_singlebeam_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_singlebeam_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_singlebeam_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_singlebeam_copy;
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
