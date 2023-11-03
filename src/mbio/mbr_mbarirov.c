/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mbarirov.c	5/20/99
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
 * mbr_mbarirov.c contains the functions for reading and writing
 * multibeam data in the MBARIROV format.
 * These functions include:
 *   mbr_alm_mbarirov	- allocate read/write memory
 *   mbr_dem_mbarirov	- deallocate read/write memory
 *   mbr_rt_mbarirov	- read and translate data
 *   mbr_wt_mbarirov	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	May 20, 1999
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
#include "mbf_mbarirov.h"
#include "mbsys_singlebeam.h"

static const char header[] =
    "Year,Day,Time,Usec,Lat,Lon,East,North,Pres,Head,Alti,Pitch,Roll,PosFlag,PresFlag,HeadFlag,AltiFlag,AttitFlag\n";

/*--------------------------------------------------------------------*/
int mbr_info_mbarirov(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*beams_bath_max = 1;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "MBARIROV", MB_NAME_LENGTH);
	strncpy(system_name, "SINGLEBEAM", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_MBARIROV\nInformal Description: MBARI ROV navigation format\nAttributes:           ROV "
	        "navigation, MBARI\n",
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
int mbr_zero_mbarirov(int verbose, char *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to data descriptor */
	struct mbf_mbarirov_struct *data = (struct mbf_mbarirov_struct *)data_ptr;

	/* initialize everything to zeros */
	if (data != NULL) {
		data->kind = MB_DATA_NONE;
		data->time_d = 0.0;
		for (int i = 0; i < 7; i++)
			data->time_i[i] = 0;
		data->longitude = 0.0;
		data->latitude = 0.0;
		data->easting = 0.0;
		data->northing = 0.0;
		data->rov_depth = 0.0;
		data->rov_pressure = 0.0;
		data->rov_heading = 0.0;
		data->rov_altitude = 0.0;
		data->rov_pitch = 0.0;
		data->rov_roll = 0.0;
		data->position_flag = 0;
		data->pressure_flag = 0;
		data->heading_flag = 0;
		data->altitude_flag = 0;
		data->attitude_flag = 0;
		for (int i = 0; i < MBF_MBARIROV_MAXLINE; i++)
			data->comment[i] = 0;
	}

	/* assume success */
	int status = MB_SUCCESS;
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
int mbr_alm_mbarirov(int verbose, void *mbio_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_mbarirov_struct);
	mb_io_ptr->data_structure_size = 0;
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status &= mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_singlebeam_struct), &mb_io_ptr->store_data, error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbf_mbarirov_struct *data = (struct mbf_mbarirov_struct *)mb_io_ptr->raw_data;
	char *data_ptr = (char *)data;

	/* set number of records read or written to zero */
	mb_io_ptr->save1 = 0;

	/* initialize everything to zeros */
	mbr_zero_mbarirov(verbose, data_ptr, error);

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
int mbr_dem_mbarirov(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	int status = mb_freed(verbose, __FILE__, __LINE__, &mb_io_ptr->raw_data, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, &mb_io_ptr->store_data, error);

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
int mbr_mbarirov_rd_data(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_mbarirov_struct *data = (struct mbf_mbarirov_struct *)mb_io_ptr->raw_data;

	/* initialize everything to zeros */
	mbr_zero_mbarirov(verbose, mb_io_ptr->raw_data, error);

	/* set file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	int status = MB_SUCCESS;

	char line[MBF_MBARIROV_MAXLINE + 1] = "";
	char *line_ptr;
	/* read next record */
	if ((line_ptr = fgets(line, MBF_MBARIROV_MAXLINE, mb_io_ptr->mbfp)) != NULL) {
		/* set status */
		mb_io_ptr->file_bytes += strlen(line);
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;

		/* if header found, read another line */
		if (strncmp(line, header, 25) == 0) {
			if ((line_ptr = fgets(line, MBF_MBARIROV_MAXLINE, mb_io_ptr->mbfp)) != NULL) {
				/* set status */
				mb_io_ptr->file_bytes += strlen(line);
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

	/* handle the data */
	if (status == MB_SUCCESS && line[0] == '#') {
		data->kind = MB_DATA_COMMENT;
		strncpy(data->comment, &line[1], MBF_MBARIROV_MAXLINE);
		if (data->comment[strlen(data->comment) - 1] == '\n')
			data->comment[strlen(data->comment) - 1] = '\0';
	}
	else if (status == MB_SUCCESS) {
		data->kind = MB_DATA_DATA;

		int nread;
		double timetag;
	 	int year;
	 	int jday;

		/* read data */
		if (strchr(line, ',') != NULL) {
			nread = sscanf(line, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%d,%d,%d", &year, &jday, &timetag,
			               &data->time_d, &data->latitude, &data->longitude, &data->easting, &data->northing, &data->rov_pressure,
			               &data->rov_heading, &data->rov_altitude, &data->rov_pitch, &data->rov_roll, &data->position_flag,
			               &data->pressure_flag, &data->heading_flag, &data->altitude_flag, &data->attitude_flag);
		}
		else {
			nread = sscanf(line, "%d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf,%d,%d,%d,%d,%d", &year, &jday, &timetag,
			               &data->time_d, &data->latitude, &data->longitude, &data->easting, &data->northing, &data->rov_pressure,
			               &data->rov_heading, &data->rov_altitude, &data->rov_pitch, &data->rov_roll, &data->position_flag,
			               &data->pressure_flag, &data->heading_flag, &data->altitude_flag, &data->attitude_flag);
		}

		/* catch erroneous records with wrong number of columns */
		if (nread == 8) {
			data->easting = 0.0;
			data->northing = 0.0;
		}
		else if (nread != 13 && nread != 18) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}

		/* catch erroneous records with zero time */
		else if (year == 0) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}

		else if (nread == 13 || nread == 18) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;

			/* get time */
			mb_get_date(verbose, data->time_d, data->time_i);

			/* get depth */
			data->rov_depth =
			    data->rov_pressure / (1.0052405 * (1 + 5.28E-3 * sin(DTR * data->latitude) * sin(DTR * data->latitude)));

			if (verbose >= 4) {
				fprintf(stderr, "\ndbg4  Data read in MBIO function <%s>\n", __func__);
				fprintf(stderr, "dbg4  Values,read:\n");
				fprintf(stderr, "dbg4       time_d:       %f\n", data->time_d);
				fprintf(stderr, "dbg4       latitude:     %f\n", data->latitude);
				fprintf(stderr, "dbg4       longitude:    %f\n", data->longitude);
				fprintf(stderr, "dbg4       easting:      %f\n", data->easting);
				fprintf(stderr, "dbg4       northing:     %f\n", data->northing);
				fprintf(stderr, "dbg4       rov_pressure: %f\n", data->rov_pressure);
				fprintf(stderr, "dbg4       rov_heading:  %f\n", data->rov_heading);
				fprintf(stderr, "dbg4       rov_altitude: %f\n", data->rov_altitude);
				fprintf(stderr, "dbg4       rov_pitch:    %f\n", data->rov_pitch);
				fprintf(stderr, "dbg4       rov_roll:     %f\n", data->rov_roll);
				fprintf(stderr, "dbg4       position_flag:%d\n", data->position_flag);
				fprintf(stderr, "dbg4       pressure_flag:%d\n", data->pressure_flag);
				fprintf(stderr, "dbg4       heading_flag: %d\n", data->heading_flag);
				fprintf(stderr, "dbg4       altitude_flag:%d\n", data->altitude_flag);
				fprintf(stderr, "dbg4       attitude_flag:%d\n", data->attitude_flag);
				fprintf(stderr, "dbg4       error:        %d\n", *error);
				fprintf(stderr, "dbg4       status:       %d\n", status);
			}
		}

		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
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
int mbr_rt_mbarirov(int verbose, void *mbio_ptr, void *store_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbf_mbarirov_struct *data = (struct mbf_mbarirov_struct *)mb_io_ptr->raw_data;
	struct mbsys_singlebeam_struct *store = (struct mbsys_singlebeam_struct *)store_ptr;

	/* read next data from file */
	const int status = mbr_mbarirov_rd_data(verbose, mbio_ptr, error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate values to data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		store->kind = data->kind;
		store->time_d = data->time_d;
		for (int i = 0; i < 7; i++)
			store->time_i[i] = data->time_i[i];
		store->longitude = data->longitude;
		store->latitude = data->latitude;
		store->easting = data->easting;
		store->northing = data->northing;
		store->sonar_depth = data->rov_depth;
		store->rov_pressure = data->rov_pressure;
		store->heading = data->rov_heading;
		store->rov_altitude = data->rov_altitude;
		store->roll = data->rov_roll;
		store->pitch = data->rov_pitch;
		store->position_flag = data->position_flag;
		store->pressure_flag = data->pressure_flag;
		store->heading_flag = data->heading_flag;
		store->altitude_flag = data->altitude_flag;
		store->attitude_flag = data->attitude_flag;
		for (int i = 0; i < MBF_MBARIROV_MAXLINE; i++)
			store->comment[i] = data->comment[i];
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
int mbr_mbarirov_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error) {
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
	struct mbf_mbarirov_struct *data = (struct mbf_mbarirov_struct *)data_ptr;

	/* get pointer to write counter */
	int *write_count = (int *)&mb_io_ptr->save1;

	char line[MBF_MBARIROV_MAXLINE + 1] = "";

	/* handle the data */
	if (data->kind == MB_DATA_COMMENT) {
		line[0] = '#';
		strncpy(&line[1], data->comment, MBF_MBARIROV_MAXLINE - 2);
		const int len = strlen(line);
		if (line[len - 1] != '\n') {
			line[len] = '\n';
			line[len + 1] = '\0';
		}
	}
	else if (data->kind == MB_DATA_DATA) {
		/* get pressure */
		data->rov_pressure =
		    data->rov_depth * (1.0052405 * (1 + 5.28E-3 * sin(DTR * data->latitude) * sin(DTR * data->latitude)));

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Data to be written in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Values,read:\n");
			fprintf(stderr, "dbg4       time_d:       %f\n", data->time_d);
			fprintf(stderr, "dbg4       latitude:     %f\n", data->latitude);
			fprintf(stderr, "dbg4       longitude:    %f\n", data->longitude);
			fprintf(stderr, "dbg4       easting:      %f\n", data->easting);
			fprintf(stderr, "dbg4       northing:     %f\n", data->northing);
			fprintf(stderr, "dbg4       rov_pressure: %f\n", data->rov_pressure);
			fprintf(stderr, "dbg4       rov_heading:  %f\n", data->rov_heading);
			fprintf(stderr, "dbg4       rov_altitude: %f\n", data->rov_altitude);
			fprintf(stderr, "dbg4       rov_pitch:    %f\n", data->rov_pitch);
			fprintf(stderr, "dbg4       rov_roll:     %f\n", data->rov_roll);
			fprintf(stderr, "dbg4       position_flag:%d\n", data->position_flag);
			fprintf(stderr, "dbg4       pressure_flag:%d\n", data->pressure_flag);
			fprintf(stderr, "dbg4       heading_flag: %d\n", data->heading_flag);
			fprintf(stderr, "dbg4       altitude_flag:%d\n", data->altitude_flag);
			fprintf(stderr, "dbg4       attitude_flag:%d\n", data->attitude_flag);
			fprintf(stderr, "dbg4       error:        %d\n", *error);
		}

		int time_j[6];
		mb_get_jtime(verbose, data->time_i, time_j);
		const int year = data->time_i[0];
		const int jday = time_j[1];
		const int timetag = 10000 * data->time_i[3] + 100 * data->time_i[4] + data->time_i[5];
		sprintf(line, "%4.4d,%3.3d,%6.6d,%9.0f,%10.6f,%11.6f,%7.0f,%7.0f,%7.2f,%5.1f,%6.2f,%4.1f,%4.1f,%d,%d,%d,%d,%d\n", year,
		        jday, timetag, data->time_d, data->latitude, data->longitude, data->easting, data->northing, data->rov_pressure,
		        data->rov_heading, data->rov_altitude, data->rov_pitch, data->rov_roll, data->position_flag, data->pressure_flag,
		        data->heading_flag, data->altitude_flag, data->attitude_flag);
	}

	int status = MB_SUCCESS;

	/* write file header if needed */
	if (*write_count == 0) {
		if (fputs(header, mb_io_ptr->mbfp) == EOF) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
int mbr_wt_mbarirov(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_mbarirov_struct *data = (struct mbf_mbarirov_struct *)mb_io_ptr->raw_data;
	struct mbsys_singlebeam_struct *store = (struct mbsys_singlebeam_struct *)store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL) {
		data->kind = store->kind;
		data->time_d = store->time_d;
		for (int i = 0; i < 7; i++)
			data->time_i[i] = store->time_i[i];
		data->longitude = store->longitude;
		data->latitude = store->latitude;
		data->easting = store->easting;
		data->northing = store->northing;
		data->rov_depth = store->sonar_depth;
		data->rov_pressure = store->rov_pressure;
		data->rov_heading = store->heading;
		data->rov_altitude = store->rov_altitude;
		data->rov_roll = store->roll;
		data->rov_pitch = store->pitch;
		data->position_flag = store->position_flag;
		data->pressure_flag = store->pressure_flag;
		data->heading_flag = store->heading_flag;
		data->altitude_flag = store->altitude_flag;
		data->attitude_flag = store->attitude_flag;
		for (int i = 0; i < MBF_MBARIROV_MAXLINE; i++)
			data->comment[i] = store->comment[i];
		data->comment[MBF_MBARIROV_MAXLINE - 1] = '\0';
	}

	/* write next data to file */
	const int status = mbr_mbarirov_wr_data(verbose, mbio_ptr, (void *)data, error);

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
int mbr_register_mbarirov(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_mbarirov(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_mbarirov;
	mb_io_ptr->mb_io_format_free = &mbr_dem_mbarirov;
	mb_io_ptr->mb_io_store_alloc = &mbsys_singlebeam_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_singlebeam_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_mbarirov;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_mbarirov;
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
