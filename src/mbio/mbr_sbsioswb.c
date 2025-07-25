/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sbsioswb.c	9/18/93
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
 * mbr_sbsioswb.c contains the functions for reading and writing
 * multibeam data in the SBSIOSWB format.
 * These functions include:
 *   mbr_alm_sbsioswb	- allocate read/write memory
 *   mbr_dem_sbsioswb	- deallocate read/write memory
 *   mbr_rt_sbsioswb	- read and translate data
 *   mbr_wt_sbsioswb	- translate and write data
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
#include "mbsys_sb.h"
#include "mbf_sbsioswb.h"

/*--------------------------------------------------------------------*/
int mbr_info_sbsioswb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_SB;
	*beams_bath_max = 19;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "SBSIOSWB", MB_NAME_LENGTH);
	strncpy(system_name, "SB", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_SBSIOSWB\nInformal Description: SIO Swath-bathy SeaBeam\nAttributes:           Sea Beam, "
	        "bathymetry, 19 beams, binary, centered,\n                      SIO.\n",
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
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 2.67;
	*beamwidth_ltrack = 2.67;

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
int mbr_alm_sbsioswb(int verbose, void *mbio_ptr, int *error) {
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
	mb_io_ptr->structure_size = sizeof(struct mbf_sbsioswb_struct);
	status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_sb_struct), &mb_io_ptr->store_data, error);

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
int mbr_dem_sbsioswb(int verbose, void *mbio_ptr, int *error) {
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
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->raw_data, error);
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
int mbr_rt_sbsioswb(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char dummy[2];
	double lon, lat;
	int id;

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
	struct mbf_sbsioswb_struct *data = (struct mbf_sbsioswb_struct *)mb_io_ptr->raw_data;
	struct mbsys_sb_struct *store = (struct mbsys_sb_struct *)store_ptr;

	/* get pointers to records */
	char *headerptr = (char *)&data->year;
	char *sensorptr = (char *)&data->eclipse_time;
	char *datarecptr = (char *)&data->beams_bath;
	char *commentptr = (char *)&data->comment[0];
	int skip = 0;

	int status = MB_SUCCESS;

	/* read next header record from file */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
  size_t read_len = 0;
	if ((read_len = fread(headerptr, 1, MB_SBSIOSWB_HEADER_SIZE, mb_io_ptr->mbfp)) == (size_t) MB_SBSIOSWB_HEADER_SIZE) {
		mb_io_ptr->file_bytes += status;
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
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
	}
#endif

	if (status == MB_SUCCESS && verbose >= 5) {
		fprintf(stderr, "\ndbg5  New header record read by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  New header values:\n");
		fprintf(stderr, "dbg5       year:       %d\n", data->year);
		fprintf(stderr, "dbg5       day:        %d\n", data->day);
		fprintf(stderr, "dbg5       min:        %d\n", data->min);
		fprintf(stderr, "dbg5       sec:        %d\n", data->sec);
		fprintf(stderr, "dbg5       lat:        %d\n", data->lat);
		fprintf(stderr, "dbg5       lon:        %d\n", data->lon);
		fprintf(stderr, "dbg5       heading:    %d\n", data->heading);
		fprintf(stderr, "dbg5       course:     %d\n", data->course);
		fprintf(stderr, "dbg5       speed:      %d\n", data->speed);
		fprintf(stderr, "dbg5       speed_ps:   %d\n", data->speed_ps);
		fprintf(stderr, "dbg5       quality:    %d\n", data->quality);
		fprintf(stderr, "dbg5       sensor size:%d\n", data->sensor_size);
		fprintf(stderr, "dbg5       data size:  %d\n", data->data_size);
		fprintf(stderr, "dbg5       speed_ref:  %c%c\n", data->speed_ref[0], data->speed_ref[1]);
		fprintf(stderr, "dbg5       sensor_type:%c%c\n", data->sensor_type[0], data->sensor_type[1]);
		fprintf(stderr, "dbg5       data_type:  %c%c\n", data->data_type[0], data->data_type[1]);
	}

	/* if not a good header search through file to find one */
	while (status == MB_SUCCESS && (strncmp(data->data_type, "SR", 2) != 0 && strncmp(data->data_type, "RS", 2) != 0 &&
	                                strncmp(data->data_type, "SP", 2) != 0 && strncmp(data->data_type, "TR", 2) != 0 &&
	                                strncmp(data->data_type, "IR", 2) != 0 && strncmp(data->data_type, "AT", 2) != 0 &&
	                                strncmp(data->data_type, "SC", 2) != 0)) {
/* unswap data if necessary */
#ifdef BYTESWAPPED
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
#endif

		/* shift bytes by one */
		for (int i = 0; i < MB_SBSIOSWB_HEADER_SIZE - 1; i++)
			headerptr[i] = headerptr[i + 1];
		mb_io_ptr->file_pos += 1;

		/* read next byte */
    size_t read_len = 0;
		if ((read_len = fread(&headerptr[MB_SBSIOSWB_HEADER_SIZE - 1], 1, 1, mb_io_ptr->mbfp)) == (size_t) 1) {
			mb_io_ptr->file_bytes += read_len;
			skip++;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			mb_io_ptr->file_bytes += read_len;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}

/* swap data if necessary */
#ifdef BYTESWAPPED
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_int(data->lat);
		data->lon = mb_swap_int(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
#endif

		if (status == MB_SUCCESS && verbose >= 5) {
			fprintf(stderr, "\ndbg5  Header record after byte shift in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5  New header values:\n");
			fprintf(stderr, "dbg5       skip:       %d\n", skip);
			fprintf(stderr, "dbg5       year:       %d\n", data->year);
			fprintf(stderr, "dbg5       day:        %d\n", data->day);
			fprintf(stderr, "dbg5       min:        %d\n", data->min);
			fprintf(stderr, "dbg5       sec:        %d\n", data->sec);
			fprintf(stderr, "dbg5       lat:        %d\n", data->lat);
			fprintf(stderr, "dbg5       lon:        %d\n", data->lon);
			fprintf(stderr, "dbg5       heading:    %d\n", data->heading);
			fprintf(stderr, "dbg5       course:     %d\n", data->course);
			fprintf(stderr, "dbg5       speed:      %d\n", data->speed);
			fprintf(stderr, "dbg5       speed_ps:   %d\n", data->speed_ps);
			fprintf(stderr, "dbg5       quality:    %d\n", data->quality);
			fprintf(stderr, "dbg5       sensor size:%d\n", data->sensor_size);
			fprintf(stderr, "dbg5       data size:  %d\n", data->data_size);
			fprintf(stderr, "dbg5       speed_ref:  %c%c\n", data->speed_ref[0], data->speed_ref[1]);
			fprintf(stderr, "dbg5       sensor_type:%c%c\n", data->sensor_type[0], data->sensor_type[1]);
			fprintf(stderr, "dbg5       data_type:  %c%c\n", data->data_type[0], data->data_type[1]);
		}
	}

	/* check for unintelligible records */
	if (status == MB_SUCCESS) {
		if ((strncmp(data->sensor_type, "SB", 2) != 0 || strncmp(data->data_type, "SR", 2) != 0) &&
		    strncmp(data->data_type, "TR", 2) != 0) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			data->kind = MB_DATA_NONE;
		}
		else if (strncmp(data->data_type, "SR", 2) == 0 && data->year == 0) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			data->kind = MB_DATA_NONE;
		}
		else if (strncmp(data->data_type, "SR", 2) == 0) {
			data->kind = MB_DATA_DATA;
		}
		else {
			data->kind = MB_DATA_COMMENT;
		}
	}

	/* deal with unintelligible record */
	if (status == MB_FAILURE && *error == MB_ERROR_UNINTELLIGIBLE) {
		/* read rest of record into dummy */
		for (int i = 0; i < data->sensor_size; i++) {
      	  	size_t read_len = 0;
			if ((read_len = fread(dummy, 1, 1, mb_io_ptr->mbfp)) != (size_t) 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
			mb_io_ptr->file_bytes += read_len;
		}
		for (int i = 0; i < data->data_size; i++) {
			if ((read_len = fread(dummy, 1, 1, mb_io_ptr->mbfp)) != (size_t) 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
			mb_io_ptr->file_bytes += read_len;
		}
	}

	/* read sensor record from file */
	if (status == MB_SUCCESS && data->sensor_size > 0) {
    	size_t read_len = 0;
		if ((read_len = fread(sensorptr, 1, data->sensor_size, mb_io_ptr->mbfp)) == (size_t) data->sensor_size) {
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
			data->eclipse_time = mb_swap_short(data->eclipse_time);
			data->eclipse_heading = mb_swap_short(data->eclipse_heading);
		}
#endif

		if (status == MB_SUCCESS && verbose >= 5) {
			fprintf(stderr, "\ndbg5  New sensor record read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5  New sensor values:\n");
			fprintf(stderr, "dbg5       eclipse_time:    %d\n", data->eclipse_time);
			fprintf(stderr, "dbg5       eclipse_heading: %d\n", data->eclipse_heading);
		}
	}

	/* read data record from file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA && data->data_size > 0) {
    size_t read_len = 0;
		if ((read_len = fread(datarecptr, 1, data->data_size, mb_io_ptr->mbfp)) == (size_t) data->data_size) {
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
		if (status == MB_SUCCESS && data->kind == MB_DATA_DATA) {
			data->beams_bath = mb_swap_short(data->beams_bath);
			data->scale_factor = mb_swap_short(data->scale_factor);
		}
#endif

		/* check for unintelligible records */
		if (status == MB_SUCCESS) {
			if (data->beams_bath < 0 || data->beams_bath > MB_BEAMS_SBSIOSWB) {
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				data->kind = MB_DATA_NONE;
			}
		}

/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		if (status == MB_SUCCESS && data->kind == MB_DATA_DATA) {
			for (int i = 0; i < data->beams_bath; i++) {
				data->bath_struct[i].bath = mb_swap_short(data->bath_struct[i].bath);
				data->bath_struct[i].bath_acrosstrack = mb_swap_short(data->bath_struct[i].bath_acrosstrack);
			}
		}
#endif

		/* check for fewer than expected beams */
		if (status == MB_SUCCESS && (data->data_size / 4) - 1 < data->beams_bath) {
			const int k = (data->data_size / 4) - 2;
			for (int i = k; i < data->beams_bath; i++) {
				data->bath_struct[i].bath = 0;
				data->bath_struct[i].bath_acrosstrack = 0;
			}
		}

		/* zero ridiculous soundings */
		if (status == MB_SUCCESS && data->kind == MB_DATA_DATA) {
			for (int i = 0; i < data->beams_bath; i++) {
				if (data->bath_struct[i].bath > 11000 || data->bath_struct[i].bath_acrosstrack > 11000 ||
				    data->bath_struct[i].bath_acrosstrack < -11000) {
					data->bath_struct[i].bath = 0;
					data->bath_struct[i].bath_acrosstrack = 0;
				}
			}
		}

		if (status == MB_SUCCESS && verbose >= 5 && data->kind == MB_DATA_DATA) {
			fprintf(stderr, "\ndbg5  New data record read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5  New data values:\n");
			fprintf(stderr, "dbg5       beams_bath:   %d\n", data->beams_bath);
			fprintf(stderr, "dbg5       scale_factor: %d\n", data->scale_factor);
			for (int i = 0; i < data->beams_bath; i++)
				fprintf(stderr, "dbg5       beam: %d  bath: %d  across_track: %d\n", i, data->bath_struct[i].bath,
				        data->bath_struct[i].bath_acrosstrack);
		}
	}

	/* read comment record from file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_COMMENT) {
    size_t read_len = 0;
		if ((read_len = fread(commentptr, 1, data->data_size, mb_io_ptr->mbfp)) == (size_t) data->data_size) {
			mb_io_ptr->file_bytes += read_len;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			for (int i = data->data_size; i < MBSYS_SB_MAXLINE; i++)
				commentptr[i] = '\0';
		}
		else {
			mb_io_ptr->file_bytes += read_len;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}

		if (status == MB_SUCCESS && verbose >= 5) {
			fprintf(stderr, "\ndbg5  New comment record read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5  New comment:\n");
			fprintf(stderr, "dbg5       comment:   %s\n", data->comment);
		}
	}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = data->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to seabeam data storage structure */
	if (status == MB_SUCCESS) {
		/* type of data record */
		store->kind = data->kind;

		if (store->kind == MB_DATA_DATA) {
			/* position */
			lon = 0.0000001 * data->lon;
			if (lon < 0.0)
				lon = lon + 360.0;
			store->lon2u = (unsigned short)60.0 * lon;
			store->lon2b = (unsigned short)round(600000.0 * (lon - store->lon2u / 60.0));
			lat = 0.0000001 * data->lat + 90.0;
			store->lat2u = (unsigned short)60.0 * lat;
			store->lat2b = (unsigned short)round(600000.0 * (lat - store->lat2u / 60.0));

			/* time stamp */
			store->year = data->year;
			store->day = data->day;
			store->min = data->min;
			store->sec = 0.01 * data->sec;

			/* heading */
			store->sbhdg = (data->heading < (short)0) ? (unsigned short)round(((int)data->heading + 3600) * 18.204444444)
			                                          : (unsigned short)round(data->heading * 18.204444444);

			/* depths and distances */
			id = data->beams_bath - 1;
			for (int i = 0; i < data->beams_bath; i++) {
				store->deph[id - i] = data->bath_struct[i].bath;
				store->dist[id - i] = data->bath_struct[i].bath_acrosstrack;
			}

			/* additional values */
			store->sbtim = data->eclipse_time;
			store->axis = 0;
			store->major = 0;
			store->minor = 0;
		}

		else if (store->kind == MB_DATA_COMMENT) {
			/* comment */
			strncpy(store->comment, data->comment, MBSYS_SB_MAXLINE);
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
int mbr_wt_sbsioswb(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	double lon, lat;
	int id;
	int sensor_size;
	int data_size;

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
	struct mbf_sbsioswb_struct *data = (struct mbf_sbsioswb_struct *)mb_io_ptr->raw_data;
	struct mbsys_sb_struct *store = (struct mbsys_sb_struct *)store_ptr;

	/* get pointers to records */
	char *headerptr = (char *)&data->year;
	char *sensorptr = (char *)&data->eclipse_time;
	char *datarecptr = (char *)&data->beams_bath;
	char *commentptr = (char *)&data->comment[0];

	if (verbose >= 2 && (store->kind == MB_DATA_DATA || store->kind == MB_DATA_NAV)) {
		fprintf(stderr, "dbg2   Data to be extracted from storage structure: %p %p\n", (void *)store_ptr, (void *)store);
		fprintf(stderr, "dbg2       kind:       %d\n", store->kind);
		fprintf(stderr, "dbg2       lon2u:      %d\n", store->lon2u);
		fprintf(stderr, "dbg2       lon2b:      %d\n", store->lon2b);
		fprintf(stderr, "dbg2       lat2u:      %d\n", store->lat2u);
		fprintf(stderr, "dbg2       lat2b:      %d\n", store->lat2b);
		fprintf(stderr, "dbg2       year:       %d\n", store->year);
		fprintf(stderr, "dbg2       day:        %d\n", store->day);
		fprintf(stderr, "dbg2       min:        %d\n", store->min);
		fprintf(stderr, "dbg2       sec:        %d\n", store->sec);
	}
	if (verbose >= 2 && store->kind == MB_DATA_DATA) {
		for (int i = 0; i < MBSYS_SB_BEAMS; i++)
			fprintf(stderr, "dbg3       dist[%d]: %d  deph[%d]: %d\n", i, store->dist[i], i, store->deph[i]);
		fprintf(stderr, "dbg2       sbtim:        %d\n", store->sbtim);
		fprintf(stderr, "dbg2       sbhdg:        %d\n", store->sbhdg);
		fprintf(stderr, "dbg2       axis:         %d\n", store->axis);
		fprintf(stderr, "dbg2       major:        %d\n", store->major);
		fprintf(stderr, "dbg2       minor:        %d\n", store->minor);
	}
	if (verbose >= 2 && store->kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2   Data inserted into storage structure:\n");
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", store->comment);
	}

	/* first set some plausible amounts for some of the
	    variables in the SBSIOSWB record */
	data->year = 0;
	data->day = 0;
	data->min = 0;
	data->sec = 0;
	data->lat = 0;
	data->lon = 0;
	data->heading = 0;
	data->course = 0;
	data->speed = 0;
	data->speed_ps = 0;
	data->quality = 0;
	data->sensor_size = 0;
	data->data_size = 0;
	data->speed_ref[0] = 0;
	data->speed_ref[1] = 0;
	if (store->kind == MB_DATA_DATA) {
		data->sensor_type[0] = 'S';
		data->sensor_type[1] = 'B';
		data->data_type[0] = 'S';
		data->data_type[1] = 'R';
	}
	else {
		data->sensor_type[0] = 0;
		data->sensor_type[1] = 0;
		data->data_type[0] = 'T';
		data->data_type[1] = 'R';
	}
	data->eclipse_time = 0;
	data->eclipse_heading = 0;
	data->beams_bath = MB_BEAMS_SBSIOSWB;
	data->sensor_size = 4;
	data->data_size = 4 + 4 * data->beams_bath;
	data->scale_factor = 100;
	for (int i = 0; i < MB_BEAMS_SBSIOSWB; i++) {
		data->bath_struct[i].bath = 0;
		data->bath_struct[i].bath_acrosstrack = 0;
	}

	/* translate values from seabeam data storage structure */
	data->kind = store->kind;
	if (store->kind == MB_DATA_DATA) {
		data->sensor_type[0] = 'S';
		data->sensor_type[1] = 'B';
		data->data_type[0] = 'S';
		data->data_type[1] = 'R';

		/* position */
		lon = 10000000 * (store->lon2u / 60. + store->lon2b / 600000.);
		if (lon > 1800000000.)
			lon = lon - 3600000000.;
		lat = 10000000 * (store->lat2u / 60. + store->lat2b / 600000. - 90.);
		data->lon = lon;
		data->lat = lat;

		/* time stamp */
		data->year = store->year;
		data->day = store->day;
		data->min = store->min;
		data->sec = 100 * store->sec;

		/* heading */
		data->heading = (short)round(((int)store->sbhdg) * 0.054931641625);

		/* additional values */
		data->eclipse_time = store->sbtim;
		data->eclipse_heading = store->sbhdg;

		/* put distance and depth values
		    into sbsioswb data structure */
		id = data->beams_bath - 1;
		for (int i = 0; i < MB_BEAMS_SBSIOSWB; i++) {
			data->bath_struct[id - i].bath = store->deph[i];
			;
			data->bath_struct[id - i].bath_acrosstrack = store->dist[i];
			;
		}
	}

	/* comment */
	else if (store->kind == MB_DATA_COMMENT) {
		data->sensor_type[0] = 0;
		data->sensor_type[1] = 0;
		data->data_type[0] = 'T';
		data->data_type[1] = 'R';

		data->data_size = strlen(store->comment);
		if (data->data_size > MBSYS_SB_MAXLINE - 1)
			data->data_size = MBSYS_SB_MAXLINE - 1;
		strncpy(commentptr, store->comment, data->data_size);
		commentptr[data->data_size] = 0;
		data->sensor_size = 0;
	}

	/* save sensor_size and data_size before possible byte swapping */
	sensor_size = data->sensor_size;
	data_size = data->data_size;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Ready to write data in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:       %d\n", data->kind);
		fprintf(stderr, "dbg5       error:      %d\n", *error);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Header record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  Header values:\n");
		fprintf(stderr, "dbg5       year:       %d\n", data->year);
		fprintf(stderr, "dbg5       day:        %d\n", data->day);
		fprintf(stderr, "dbg5       min:        %d\n", data->min);
		fprintf(stderr, "dbg5       sec:        %d\n", data->sec);
		fprintf(stderr, "dbg5       lat:        %d\n", data->lat);
		fprintf(stderr, "dbg5       lon:        %d\n", data->lon);
		fprintf(stderr, "dbg5       heading:    %d\n", data->heading);
		fprintf(stderr, "dbg5       course:     %d\n", data->course);
		fprintf(stderr, "dbg5       speed:      %d\n", data->speed);
		fprintf(stderr, "dbg5       speed_ps:   %d\n", data->speed_ps);
		fprintf(stderr, "dbg5       quality:    %d\n", data->quality);
		fprintf(stderr, "dbg5       sensor size:%d\n", data->sensor_size);
		fprintf(stderr, "dbg5       data size:  %d\n", data->data_size);
		fprintf(stderr, "dbg5       speed_ref:  %c%c\n", data->speed_ref[0], data->speed_ref[1]);
		fprintf(stderr, "dbg5       sensor_type:%c%c\n", data->sensor_type[0], data->sensor_type[1]);
		fprintf(stderr, "dbg5       data_type:  %c%c\n", data->data_type[0], data->data_type[1]);
	}

	if (verbose >= 5 && data->kind == MB_DATA_DATA) {
		fprintf(stderr, "\ndbg5  Sensor record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  Sensor values:\n");
		fprintf(stderr, "dbg5       eclipse_time:    %d\n", data->eclipse_time);
		fprintf(stderr, "dbg5       eclipse_heading: %d\n", data->eclipse_heading);
	}

	if (verbose >= 5 && data->kind == MB_DATA_DATA) {
		fprintf(stderr, "\ndbg5  Data record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  Data values:\n");
		fprintf(stderr, "dbg5       beams_bath:   %d\n", data->beams_bath);
		fprintf(stderr, "dbg5       scale_factor: %d\n", data->scale_factor);
		for (int i = 0; i < data->beams_bath; i++)
			fprintf(stderr, "dbg5       beam: %d  bath: %d  across_track: %d\n", i, data->bath_struct[i].bath,
			        data->bath_struct[i].bath_acrosstrack);
	}

	if (verbose >= 5 && data->kind == MB_DATA_COMMENT) {
		fprintf(stderr, "\ndbg5  Comment record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  Comment:\n");
		fprintf(stderr, "dbg5       comment:   %s\n", data->comment);
	}

/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	data->year = mb_swap_short(data->year);
	data->day = mb_swap_short(data->day);
	data->min = mb_swap_short(data->min);
	data->sec = mb_swap_short(data->sec);
	data->lat = mb_swap_int(data->lat);
	data->lon = mb_swap_int(data->lon);
	data->heading = mb_swap_short(data->heading);
	data->course = mb_swap_short(data->course);
	data->speed = mb_swap_short(data->speed);
	data->speed_ps = mb_swap_short(data->speed_ps);
	data->quality = mb_swap_short(data->quality);
	data->sensor_size = mb_swap_short(data->sensor_size);
	data->data_size = mb_swap_short(data->data_size);
	data->eclipse_time = mb_swap_short(data->eclipse_time);
	data->eclipse_heading = mb_swap_short(data->eclipse_heading);
	data->beams_bath = mb_swap_short(data->beams_bath);
	data->scale_factor = mb_swap_short(data->scale_factor);
	if (store->kind == MB_DATA_DATA) {
		for (int i = 0; i < MB_BEAMS_SBSIOSWB; i++) {
			data->bath_struct[i].bath = mb_swap_short(data->bath_struct[i].bath);
			data->bath_struct[i].bath_acrosstrack = mb_swap_short(data->bath_struct[i].bath_acrosstrack);
		}
	}
#endif

	int status = MB_SUCCESS;
  size_t write_len = 0;

	/* write header record to file */
	if (status == MB_SUCCESS) {
		if ((write_len = fwrite(headerptr, 1, MB_SBSIOSWB_HEADER_SIZE, mb_io_ptr->mbfp)) == (size_t) MB_SBSIOSWB_HEADER_SIZE) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	/* write sensor record to file */
	if (status == MB_SUCCESS) {
		if ((write_len = fwrite(sensorptr, 1, sensor_size, mb_io_ptr->mbfp)) == (size_t) sensor_size) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	/* write data record to file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA) {
		if ((write_len = fwrite(datarecptr, 1, data_size, mb_io_ptr->mbfp)) == (size_t) data_size) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	/* write comment record to file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_COMMENT) {
    size_t write_len = 0;
		if ((write_len = fwrite(commentptr, 1, strlen(data->comment), mb_io_ptr->mbfp)) == (size_t) strlen(data->comment)) {
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
int mbr_register_sbsioswb(int verbose, void *mbio_ptr, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	status = mbr_info_sbsioswb(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sbsioswb;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sbsioswb;
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_sb_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sbsioswb;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sbsioswb;
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
