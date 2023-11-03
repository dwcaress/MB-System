/*--------------------------------------------------------------------
 *    The MB-system:	mbr_soiusbln.c	5/20/99
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
 * mbr_soiusbln.c contains the functions for reading and writing
 * multibeam data in the SOIROVNV format.
 * These functions include:
 *   mbr_alm_soiusbln	- allocate read/write memory
 *   mbr_dem_soiusbln	- deallocate read/write memory
 *   mbr_rt_soiusbln	- read and translate data
 *   mbr_wt_soiusbln	- translate and write data
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
int mbr_info_soiusbln(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	        "Format name:          MBF_SOIUSBLN\nInformal Description: SOI USBL navigation format\nAttributes:           SOI "
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
int mbr_alm_soiusbln(int verbose, void *mbio_ptr, int *error) {
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
int mbr_dem_soiusbln(int verbose, void *mbio_ptr, int *error) {
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
int mbr_rt_soiusbln(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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

		double rawlat = 0.0;
		char NorS = 'N';
		double rawlon = 0.0;
		char EorW = 'E';
		double sensor_height = 0.0;
		
		/* Example string:
			2023-03-22T20:52:58.968700Z,$GPGGA,205258.672,2328.43166,N,04459.20602,W,2,00,8.8,-1156.992,M,0.0,M,0.0,0001*42
		*/
		/* read data */
		int nget = sscanf(line, "%d-%d-%dT%d:%d:%d.%dZ,$GPGGA,%lf,%lf,%c,%lf,%c,%d,%d,%lf,%lf,",
							&store->time_i[0], &store->time_i[1], &store->time_i[2],
		             		&store->time_i[3], &store->time_i[4], &store->time_i[5], &store->time_i[6], 
		             		&store->gps_time, &rawlat, &NorS, &rawlon, &EorW, &store->gps_quality, 
		             		&store->gps_nsat, &store->gps_dilution, &sensor_height);
		if ((nget == 16) && store->time_i[0] != 0) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;

			mb_get_time(verbose, store->time_i, &store->time_d);
			double londeg = floor(rawlon / 100.0);
			store->longitude = londeg + (rawlon - 100.0 * londeg) / 60.0;
			if (EorW == 'W')
				store->longitude *= -1.0;
			double latdeg = floor(rawlat / 100.0);
			store->latitude = latdeg + (rawlat - 100.0 * latdeg) / 60.0;
			if (NorS == 'S')
				store->latitude *= -1.0;

			store->heading = 0.0; //no heading in USBL tracking
			store->sonar_depth = -sensor_height;
			
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
			fprintf(stderr, "dbg4       time_i[0]:    %d\n", store->time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:    %d\n", store->time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:    %d\n", store->time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:    %d\n", store->time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:    %d\n", store->time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:    %d\n", store->time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:    %d\n", store->time_i[6]);
			fprintf(stderr, "dbg4       time_d:       %f\n", store->time_d);
			fprintf(stderr, "dbg4       gps_time:     %f\n", store->gps_time);
			fprintf(stderr, "dbg4       latitude:     %f\n", store->latitude);
			fprintf(stderr, "dbg4       longitude:    %f\n", store->longitude);
			fprintf(stderr, "dbg4       sonar_depth:  %f\n", store->sonar_depth);
			fprintf(stderr, "dbg4       gps_quality:  %d\n", store->gps_quality);
			fprintf(stderr, "dbg4       gps_nsat:     %d\n", store->gps_nsat);
			fprintf(stderr, "dbg4       gps_dilution: %f\n", store->gps_dilution);
			fprintf(stderr, "dbg4       error:        %d\n", *error);
			fprintf(stderr, "dbg4       status:       %d\n", status);
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
int mbr_wt_soiusbln(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
			fprintf(stderr, "dbg4       time_i[0]:    %d\n", store->time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:    %d\n", store->time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:    %d\n", store->time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:    %d\n", store->time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:    %d\n", store->time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:    %d\n", store->time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:    %d\n", store->time_i[6]);
			fprintf(stderr, "dbg4       time_d:       %f\n", store->time_d);
			fprintf(stderr, "dbg4       gps_time:     %f\n", store->gps_time);
			fprintf(stderr, "dbg4       latitude:     %f\n", store->latitude);
			fprintf(stderr, "dbg4       longitude:    %f\n", store->longitude);
			fprintf(stderr, "dbg4       gps_quality:  %d\n", store->gps_quality);
			fprintf(stderr, "dbg4       gps_nsat:     %d\n", store->gps_nsat);
			fprintf(stderr, "dbg4       gps_dilution: %f\n", store->gps_dilution);
			fprintf(stderr, "dbg4       gps_height:   %d\n", store->gps_height);
			fprintf(stderr, "dbg4       error:        %d\n", *error);
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
		
		
			/* Example string:
				2023-03-22T20:52:58.968700Z,$GPGGA,205258.672,2328.43166,N,04459.20602,W,2,00,8.8,-1156.992,M,0.0,M,0.0,0001*42
			*/
		
			int londeg = (int)floor(fabs(store->longitude));
			char EorW = 'E';
			if (store->longitude < 0.0)
				EorW = 'W';
			double lonmin = (fabs(store->longitude) - (double)londeg) * 60.0;
			int latdeg = (int)floor(fabs(store->latitude));
			char NorS = 'N';
			if (store->latitude < 0.0)
				NorS = 'E';
			double latmin = (fabs(store->latitude) - (double)latdeg) * 60.0;
			double sensor_height = -store->sonar_depth;
			snprintf(line, MB_COMMENT_MAXLINE, 
							"%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6dZ,$GPGGA,%10.3f,%2.2d%8.5f,%c,%3.3d%8.5f,%c,%d,%2.2d,%.1f,%.3f,M,0.0,M,0.0,0001*FF\n",
							store->time_i[0], store->time_i[1], store->time_i[2],
		             		store->time_i[3], store->time_i[4], store->time_i[5], store->time_i[6], 
		             		store->gps_time, latdeg, latmin, NorS, londeg, lonmin, EorW,
		             		store->gps_quality, store->gps_nsat, store->gps_dilution, sensor_height);
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
int mbr_register_soiusbln(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_soiusbln(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_soiusbln;
	mb_io_ptr->mb_io_format_free = &mbr_dem_soiusbln;
	mb_io_ptr->mb_io_store_alloc = &mbsys_singlebeam_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_singlebeam_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_soiusbln;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_soiusbln;
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
