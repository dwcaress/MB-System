/*--------------------------------------------------------------------
 *    The MB-system:	mbr_elmk2unb.c	6/6/97
 *
 *    Copyright (c) 1997-2023 by
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
 * mbr_elmk2unb.c contains the functions for reading and writing
 * multibeam data in the ELMK2UNB format.
 * These functions include:
 *   mbr_alm_elmk2unb	- allocate read/write memory
 *   mbr_dem_elmk2unb	- deallocate read/write memory
 *   mbr_rt_elmk2unb	- read and translate data
 *   mbr_wt_elmk2unb	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	June 6, 1997
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
#include "mb_swap.h"
#include "mbf_elmk2unb.h"
#include "mbsys_elacmk2.h"

/*--------------------------------------------------------------------*/
int mbr_info_elmk2unb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_ELACMK2;
	*beams_bath_max = 126;
	*beams_amp_max = 126;
	*pixels_ss_max = 0;
	strncpy(format_name, "ELMK2UNB", MB_NAME_LENGTH);
	strncpy(system_name, "ELACMK2", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_ELMK2UNB\nInformal Description: Elac BottomChart MkII shallow water "
	        "multibeam\nAttributes:           126 beam bathymetry and amplitude,\n                      binary, University of "
	        "New Brunswick.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = true;
	*traveltime = true;
	*beam_flagging = true;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_NAV;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 3.0;
	*beamwidth_ltrack = 3.0;

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
int mbr_zero_elmk2unb(int verbose, void *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to data descriptor */
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)data_ptr;

	/* initialize everything to zeros */
	if (data != NULL) {
		data->kind = MB_DATA_NONE;
		data->sonar = MBSYS_ELACMK2_BOTTOMCHART_MARKII;
		data->par_year = 0;
		data->par_month = 0;
		data->par_day = 0;
		data->par_hour = 0;
		data->par_minute = 0;
		data->par_second = 0;
		data->par_hundredth_sec = 0;
		data->par_thousandth_sec = 0;
		data->roll_offset = 0;    /* roll offset (degrees) */
		data->pitch_offset = 0;   /* pitch offset (degrees) */
		data->heading_offset = 0; /* heading offset (degrees) */
		data->time_delay = 0;     /* positioning system
		                  delay (sec) */
		data->transducer_port_height = 0;
		data->transducer_starboard_height = 0;
		data->transducer_port_depth = 192;
		data->transducer_starboard_depth = 192;
		data->transducer_port_x = 0;
		data->transducer_starboard_x = 0;
		data->transducer_port_y = 0;
		data->transducer_starboard_y = 0;
		data->transducer_port_error = 0;
		data->transducer_starboard_error = 0;
		data->antenna_height = 0;
		data->antenna_x = 0;
		data->antenna_y = 0;
		data->vru_height = 0;
		data->vru_x = 0;
		data->vru_y = 0;
		data->line_number = 0;
		data->start_or_stop = 0;
		data->transducer_serial_number = 0;
		for (int i = 0; i < MBF_ELMK2UNB_COMMENT_LENGTH; i++)
			data->comment[i] = '\0';

		/* position (position telegrams) */
		data->pos_year = 0;
		data->pos_month = 0;
		data->pos_day = 0;
		data->pos_hour = 0;
		data->pos_minute = 0;
		data->pos_second = 0;
		data->par_hundredth_sec = 0;
		data->pos_thousandth_sec = 0;
		data->pos_latitude = 0;
		data->pos_longitude = 0;
		data->utm_northing = 0;
		data->utm_easting = 0;
		data->utm_zone_lon = 0;
		data->utm_zone = 0;
		data->hemisphere = 0;
		data->ellipsoid = 0;
		data->pos_spare = 0;
		data->semi_major_axis = 0;
		data->other_quality = 0;

		/* sound velocity profile */
		data->svp_year = 0;
		data->svp_month = 0;
		data->svp_day = 0;
		data->svp_hour = 0;
		data->svp_minute = 0;
		data->svp_second = 0;
		data->svp_hundredth_sec = 0;
		data->svp_thousandth_sec = 0;
		data->svp_num = 0;
		for (int i = 0; i < 100; i++) {
			data->svp_depth[i] = 0; /* 0.1 meters */
			data->svp_vel[i] = 0;   /* 0.1 meters/sec */
		}

		/* depth telegram */
		data->year = 0;
		data->month = 0;
		data->day = 0;
		data->hour = 0;
		data->minute = 0;
		data->second = 0;
		data->hundredth_sec = 0;
		data->thousandth_sec = 0;
		data->latitude = 0;
		data->longitude = 0;
		data->ping_num = 0;
		data->sound_vel = 0;
		data->heading = 0;
		data->pulse_length = 0;
		data->mode = 0;
		data->pulse_length = 0;
		data->source_power = 0;
		data->receiver_gain_stbd = 0;
		data->receiver_gain_port = 0;
		data->reserved = 0;
		data->beams_bath = 0;
		for (int i = 0; i < MBF_ELMK2UNB_MAXBEAMS; i++) {
			data->beams[i].bath = 0;
			data->beams[i].bath_acrosstrack = 0;
			data->beams[i].bath_alongtrack = 0;
			data->beams[i].tt = 0;
			data->beams[i].quality = 0;
			data->beams[i].amplitude = 0;
			data->beams[i].time_offset = 0;
			data->beams[i].heave = 0;
			data->beams[i].roll = 0;
			data->beams[i].pitch = 0;
			data->beams[i].angle = 0;
		}
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
int mbr_alm_elmk2unb(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_elmk2unb_struct);
	mb_io_ptr->data_structure_size = 0;
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	mbsys_elacmk2_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* initialize everything to zeros */
	mbr_zero_elmk2unb(verbose, mb_io_ptr->raw_data, error);

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
int mbr_dem_elmk2unb(int verbose, void *mbio_ptr, int *error) {
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
int mbr_elmk2unb_rd_comment(int verbose, FILE *mbfp, struct mbf_elmk2unb_struct *data, int *error) {
	char line[ELACMK2_COMMENT_SIZE + 3];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
	}

	/* read record into char array */
	int status = fread(line, 1, ELACMK2_COMMENT_SIZE + 3, mbfp);
	if (status == ELACMK2_COMMENT_SIZE + 3)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get data */
	if (status == MB_SUCCESS) {
		data->kind = MB_DATA_COMMENT;
		strncpy(data->comment, line, MBF_ELMK2UNB_COMMENT_LENGTH - 1);
	}

	if (verbose >= 5) {
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
int mbr_elmk2unb_rd_parameter(int verbose, FILE *mbfp, struct mbf_elmk2unb_struct *data, int *error) {
	char line[ELACMK2_PARAMETER_SIZE + 3];
	short int *short_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
	}

	/* read record into char array */
	int status = fread(line, 1, ELACMK2_PARAMETER_SIZE + 3, mbfp);
	if (status == ELACMK2_PARAMETER_SIZE + 3)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get data */
	if (status == MB_SUCCESS) {
		data->kind = MB_DATA_PARAMETER;
		data->par_day = (int)line[0];
		data->par_month = (int)line[1];
		data->par_year = (int)line[2];
		data->par_hour = (int)line[3];
		data->par_minute = (int)line[4];
		data->par_second = (int)line[5];
		data->par_hundredth_sec = (int)line[6];
		data->par_thousandth_sec = (int)line[7];
#ifndef BYTESWAPPED
		short_ptr = (short int *)&line[8];
		data->roll_offset = *short_ptr;
		short_ptr = (short int *)&line[10];
		data->pitch_offset = *short_ptr;
		short_ptr = (short int *)&line[12];
		data->heading_offset = *short_ptr;
		short_ptr = (short int *)&line[14];
		data->time_delay = *short_ptr;
		short_ptr = (short int *)&line[16];
		data->transducer_port_height = *short_ptr;
		short_ptr = (short int *)&line[18];
		data->transducer_starboard_height = *short_ptr;
		short_ptr = (short int *)&line[20];
		data->transducer_port_depth = *short_ptr;
		short_ptr = (short int *)&line[22];
		data->transducer_starboard_depth = *short_ptr;
		short_ptr = (short int *)&line[24];
		data->transducer_port_x = *short_ptr;
		short_ptr = (short int *)&line[26];
		data->transducer_starboard_x = *short_ptr;
		short_ptr = (short int *)&line[28];
		data->transducer_port_y = *short_ptr;
		short_ptr = (short int *)&line[30];
		data->transducer_starboard_y = *short_ptr;
		short_ptr = (short int *)&line[32];
		data->transducer_port_error = *short_ptr;
		short_ptr = (short int *)&line[34];
		data->transducer_starboard_error = *short_ptr;
		short_ptr = (short int *)&line[36];
		data->antenna_height = *short_ptr;
		short_ptr = (short int *)&line[38];
		data->antenna_x = *short_ptr;
		short_ptr = (short int *)&line[40];
		data->antenna_y = *short_ptr;
		short_ptr = (short int *)&line[42];
		data->vru_height = *short_ptr;
		short_ptr = (short int *)&line[44];
		data->vru_x = *short_ptr;
		short_ptr = (short int *)&line[46];
		data->vru_y = *short_ptr;
		short_ptr = (short int *)&line[48];
		data->line_number = *short_ptr;
		short_ptr = (short int *)&line[50];
		data->start_or_stop = *short_ptr;
		short_ptr = (short int *)&line[52];
		data->transducer_serial_number = *short_ptr;
#else
		short_ptr = (short int *)&line[8];
		data->roll_offset = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[10];
		data->pitch_offset = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[12];
		data->heading_offset = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[14];
		data->time_delay = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[16];
		data->transducer_port_height = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[18];
		data->transducer_starboard_height = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[20];
		data->transducer_port_depth = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[22];
		data->transducer_starboard_depth = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[24];
		data->transducer_port_x = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[26];
		data->transducer_starboard_x = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[28];
		data->transducer_port_y = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[30];
		data->transducer_starboard_y = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[32];
		data->transducer_port_error = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[34];
		data->transducer_starboard_error = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[36];
		data->antenna_height = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[38];
		data->antenna_x = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[40];
		data->antenna_y = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[42];
		data->vru_height = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[44];
		data->vru_x = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[46];
		data->vru_y = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[48];
		data->line_number = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[50];
		data->start_or_stop = (short int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[52];
		data->transducer_serial_number = (short int)mb_swap_short(*short_ptr);
#endif
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", data->par_year);
		fprintf(stderr, "dbg5       month:            %d\n", data->par_month);
		fprintf(stderr, "dbg5       day:              %d\n", data->par_day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->par_hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->par_minute);
		fprintf(stderr, "dbg5       sec:              %d\n", data->par_second);
		fprintf(stderr, "dbg5       hundredth_sec:    %d\n", data->par_hundredth_sec);
		fprintf(stderr, "dbg5       thousandth_sec:   %d\n", data->par_thousandth_sec);
		fprintf(stderr, "dbg5       roll_offset:      %d\n", data->roll_offset);
		fprintf(stderr, "dbg5       pitch_offset:     %d\n", data->pitch_offset);
		fprintf(stderr, "dbg5       heading_offset:   %d\n", data->heading_offset);
		fprintf(stderr, "dbg5       time_delay:       %d\n", data->time_delay);
		fprintf(stderr, "dbg5       transducer_port_height: %d\n", data->transducer_port_height);
		fprintf(stderr, "dbg5       transducer_starboard_height:%d\n", data->transducer_starboard_height);
		fprintf(stderr, "dbg5       transducer_port_depth:     %d\n", data->transducer_port_depth);
		fprintf(stderr, "dbg5       transducer_starboard_depth:     %d\n", data->transducer_starboard_depth);
		fprintf(stderr, "dbg5       transducer_port_x:        %d\n", data->transducer_port_x);
		fprintf(stderr, "dbg5       transducer_starboard_x:        %d\n", data->transducer_starboard_x);
		fprintf(stderr, "dbg5       transducer_port_y:        %d\n", data->transducer_port_y);
		fprintf(stderr, "dbg5       transducer_starboard_y:  %d\n", data->transducer_starboard_y);
		fprintf(stderr, "dbg5       transducer_port_error:  %d\n", data->transducer_port_error);
		fprintf(stderr, "dbg5       transducer_starboard_error:  %d\n", data->transducer_starboard_error);
		fprintf(stderr, "dbg5       antenna_height:            %d\n", data->antenna_height);
		fprintf(stderr, "dbg5       antenna_x:      %d\n", data->antenna_x);
		fprintf(stderr, "dbg5       antenna_y:    %d\n", data->antenna_y);
		fprintf(stderr, "dbg5       vru_height:%d\n", data->vru_height);
		fprintf(stderr, "dbg5       vru_x:%d\n", data->vru_x);
		fprintf(stderr, "dbg5       vru_y:%d\n", data->vru_y);
		fprintf(stderr, "dbg5       line_number:%d\n", data->line_number);
		fprintf(stderr, "dbg5       start_or_stop:%d\n", data->start_or_stop);
		fprintf(stderr, "dbg5       transducer_serial_number:%d\n", data->transducer_serial_number);
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
int mbr_elmk2unb_rd_pos(int verbose, FILE *mbfp, struct mbf_elmk2unb_struct *data, int *error) {
	char line[ELACMK2_POS_SIZE + 3];
	short int *short_ptr;
	int *int_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
	}

	/* read record into char array */
	int status = fread(line, 1, ELACMK2_POS_SIZE + 3, mbfp);
	if (status == ELACMK2_POS_SIZE + 3)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get data */
	if (status == MB_SUCCESS) {
		data->kind = MB_DATA_NAV;
		data->pos_day = (int)line[0];
		data->pos_month = (int)line[1];
		data->pos_year = (int)line[2];
		data->pos_hour = (int)line[3];
		data->pos_minute = (int)line[4];
		data->pos_second = (int)line[5];
		data->pos_hundredth_sec = (int)line[6];
		data->pos_thousandth_sec = (int)line[7];
#ifndef BYTESWAPPED
		int_ptr = (int *)&line[8];
		data->pos_latitude = *int_ptr;
		int_ptr = (int *)&line[12];
		data->pos_longitude = *int_ptr;
		int_ptr = (int *)&line[16];
		data->utm_northing = *int_ptr;
		int_ptr = (int *)&line[20];
		data->utm_easting = *int_ptr;
		int_ptr = (int *)&line[24];
		data->utm_zone_lon = *int_ptr;
		data->utm_zone = line[28];
		data->hemisphere = line[29];
		data->ellipsoid = line[30];
		data->pos_spare = line[31];
		short_ptr = (short int *)&line[32];
		data->semi_major_axis = (int)*short_ptr;
		short_ptr = (short int *)&line[34];
		data->other_quality = (int)*short_ptr;
#else
		int_ptr = (int *)&line[8];
		data->pos_latitude = (int)mb_swap_int(*int_ptr);
		int_ptr = (int *)&line[12];
		data->pos_longitude = (int)mb_swap_int(*int_ptr);
		int_ptr = (int *)&line[16];
		data->utm_northing = (int)mb_swap_int(*int_ptr);
		int_ptr = (int *)&line[20];
		data->utm_easting = (int)mb_swap_int(*int_ptr);
		int_ptr = (int *)&line[24];
		data->utm_zone_lon = (int)mb_swap_int(*int_ptr);
		data->utm_zone = line[28];
		data->hemisphere = line[29];
		data->ellipsoid = line[30];
		data->pos_spare = line[31];
		short_ptr = (short int *)&line[32];
		data->semi_major_axis = (int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[34];
		data->other_quality = (int)mb_swap_short(*short_ptr);
#endif
	}

	/* KLUGE for 1996 UNB TRAINING COURSE - FLIP LONGITUDE */
	if (data->pos_year == 96 && data->pos_month >= 6 && data->pos_month <= 8)
		data->pos_longitude = -data->pos_longitude;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", data->pos_year);
		fprintf(stderr, "dbg5       month:            %d\n", data->pos_month);
		fprintf(stderr, "dbg5       day:              %d\n", data->pos_day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->pos_hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->pos_minute);
		fprintf(stderr, "dbg5       sec:              %d\n", data->pos_second);
		fprintf(stderr, "dbg5       hundredth_sec:    %d\n", data->pos_hundredth_sec);
		fprintf(stderr, "dbg5       thousandth_sec:   %d\n", data->pos_thousandth_sec);
		fprintf(stderr, "dbg5       pos_latitude:     %d\n", data->pos_latitude);
		fprintf(stderr, "dbg5       pos_longitude:    %d\n", data->pos_longitude);
		fprintf(stderr, "dbg5       utm_northing:     %d\n", data->utm_northing);
		fprintf(stderr, "dbg5       utm_easting:      %d\n", data->utm_easting);
		fprintf(stderr, "dbg5       utm_zone_lon:     %d\n", data->utm_zone_lon);
		fprintf(stderr, "dbg5       utm_zone:         %c\n", data->utm_zone);
		fprintf(stderr, "dbg5       hemisphere:       %c\n", data->hemisphere);
		fprintf(stderr, "dbg5       ellipsoid:        %c\n", data->ellipsoid);
		fprintf(stderr, "dbg5       pos_spare:        %c\n", data->pos_spare);
		fprintf(stderr, "dbg5       semi_major_axis:  %d\n", data->semi_major_axis);
		fprintf(stderr, "dbg5       other_quality:    %d\n", data->other_quality);
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
int mbr_elmk2unb_rd_svp(int verbose, FILE *mbfp, struct mbf_elmk2unb_struct *data, int *error) {
	char line[ELACMK2_SVP_SIZE + 3];
	short int *short_ptr;
	short int *short_ptr2;
	int *int_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
	}

	/* read record into char array */
	int status = fread(line, 1, ELACMK2_SVP_SIZE + 3, mbfp);
	if (status == ELACMK2_SVP_SIZE + 3)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get data */
	if (status == MB_SUCCESS) {
		data->kind = MB_DATA_VELOCITY_PROFILE;
		data->svp_day = (int)line[0];
		data->svp_month = (int)line[1];
		data->svp_year = (int)line[2];
		data->svp_hour = (int)line[3];
		data->svp_minute = (int)line[4];
		data->svp_second = (int)line[5];
		data->svp_hundredth_sec = (int)line[6];
		data->svp_thousandth_sec = (int)line[7];
#ifndef BYTESWAPPED
		int_ptr = (int *)&line[8];
		data->svp_latitude = *int_ptr;
		int_ptr = (int *)&line[12];
		data->svp_longitude = *int_ptr;
#else
		int_ptr = (int *)&line[8];
		data->svp_latitude = (int)mb_swap_int(*int_ptr);
		int_ptr = (int *)&line[12];
		data->svp_latitude = (int)mb_swap_int(*int_ptr);
#endif
		data->svp_num = 0;
		for (int i = 0; i < 500; i++) {
			short_ptr = (short int *)&line[16 + 4 * i];
			short_ptr2 = (short int *)&line[18 + 4 * i];
#ifndef BYTESWAPPED
			data->svp_depth[i] = *short_ptr;
			data->svp_vel[i] = *short_ptr2;
#else
			data->svp_depth[i] = (short int)mb_swap_short(*short_ptr);
			data->svp_vel[i] = (short int)mb_swap_short(*short_ptr2);
#endif
			if (data->svp_vel[i] > 0)
				data->svp_num = i + 1;
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", data->svp_year);
		fprintf(stderr, "dbg5       month:            %d\n", data->svp_month);
		fprintf(stderr, "dbg5       day:              %d\n", data->svp_day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->svp_hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->svp_minute);
		fprintf(stderr, "dbg5       sec:              %d\n", data->svp_second);
		fprintf(stderr, "dbg5       hundredth_sec:    %d\n", data->svp_hundredth_sec);
		fprintf(stderr, "dbg5       thousandth_sec:   %d\n", data->svp_thousandth_sec);
		fprintf(stderr, "dbg5       svp_latitude:     %d\n", data->svp_latitude);
		fprintf(stderr, "dbg5       svp_longitude:    %d\n", data->svp_longitude);
		fprintf(stderr, "dbg5       svp_num:          %d\n", data->svp_num);
		for (int i = 0; i < data->svp_num; i++)
			fprintf(stderr, "dbg5       depth: %d     vel: %d\n", data->svp_depth[i], data->svp_vel[i]);
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
int mbr_elmk2unb_rd_bathgen(int verbose, FILE *mbfp, struct mbf_elmk2unb_struct *data, int *error) {
	char line[ELACMK2_COMMENT_SIZE];
	short int *short_ptr;
	int *int_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data:       %p\n", (void *)data);
	}

	/* read record into char array */
	int status = fread(line, 1, ELACMK2_BATHGEN_HDR_SIZE, mbfp);
	if (status == ELACMK2_BATHGEN_HDR_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get data */
	if (status == MB_SUCCESS) {
		data->kind = MB_DATA_DATA;

		data->day = (int)line[0];
		data->month = (int)line[1];
		data->year = (int)line[2];
		data->hour = (int)line[3];
		data->minute = (int)line[4];
		data->second = (int)line[5];
		data->hundredth_sec = (int)line[6];
		data->thousandth_sec = (int)line[7];
#ifndef BYTESWAPPED
		short_ptr = (short int *)&line[8];
		data->ping_num = (int)(unsigned short)*short_ptr;
		short_ptr = (short int *)&line[10];
		data->sound_vel = (int)(unsigned short)*short_ptr;
		short_ptr = (short int *)&line[12];
		data->heading = (int)(unsigned short)*short_ptr;
		short_ptr = (short int *)&line[14];
		data->pulse_length = (int)(unsigned short)*short_ptr;
#else
		short_ptr = (short int *)&line[8];
		data->ping_num = (int)(unsigned short)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[10];
		data->sound_vel = (int)(unsigned short)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[12];
		data->heading = (int)(unsigned short)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[14];
		data->pulse_length = (int)(unsigned short)mb_swap_short(*short_ptr);
#endif
		data->mode = (int)line[16];
		data->source_power = (int)line[17];
		data->receiver_gain_stbd = (int)line[18];
		data->receiver_gain_port = (int)line[19];
#ifndef BYTESWAPPED
		short_ptr = (short int *)&line[20];
		data->reserved = (int)*short_ptr;
		short_ptr = (short int *)&line[22];
		data->beams_bath = (int)*short_ptr;
#else
		short_ptr = (short int *)&line[20];
		data->reserved = (int)mb_swap_short(*short_ptr);
		short_ptr = (short int *)&line[22];
		data->beams_bath = (int)mb_swap_short(*short_ptr);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       sec:              %d\n", data->second);
		fprintf(stderr, "dbg5       hundredth_sec:    %d\n", data->hundredth_sec);
		fprintf(stderr, "dbg5       thousandth_sec:   %d\n", data->thousandth_sec);
		fprintf(stderr, "dbg5       ping_num:         %d\n", data->ping_num);
		fprintf(stderr, "dbg5       sound_vel:        %d\n", data->sound_vel);
		fprintf(stderr, "dbg5       heading:          %d\n", data->heading);
		fprintf(stderr, "dbg5       pulse_length:     %d\n", data->pulse_length);
		fprintf(stderr, "dbg5       mode:             %d\n", data->mode);
		fprintf(stderr, "dbg5       source_power:     %d\n", data->source_power);
		fprintf(stderr, "dbg5       receiver_gain_stbd:%d\n", data->receiver_gain_stbd);
		fprintf(stderr, "dbg5       receiver_gain_port:%d\n", data->receiver_gain_port);
		fprintf(stderr, "dbg5       reserved:         %d\n", data->reserved);
		fprintf(stderr, "dbg5       beams_bath:       %d\n", data->beams_bath);
	}

	/* now read each of the beams */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < data->beams_bath; i++) {
			/* read record into char array */
			status = fread(line, 1, ELACMK2_BATHGEN_BEAM_SIZE, mbfp);
			if (status == ELACMK2_BATHGEN_BEAM_SIZE)
				status = MB_SUCCESS;
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
			/* get data */
			if (status == MB_SUCCESS) {
#ifndef BYTESWAPPED
				int_ptr = (int *)&line[0];
				data->beams[i].bath = (unsigned int)*int_ptr;
				int_ptr = (int *)&line[4];
				data->beams[i].bath_acrosstrack = (int)*int_ptr;
				int_ptr = (int *)&line[8];
				data->beams[i].bath_alongtrack = (int)*int_ptr;
				int_ptr = (int *)&line[12];
				data->beams[i].tt = (unsigned int)*int_ptr;
#else
				int_ptr = (int *)&line[0];
				data->beams[i].bath = (unsigned int)mb_swap_int(*int_ptr);
				int_ptr = (int *)&line[4];
				data->beams[i].bath_acrosstrack = (int)mb_swap_int(*int_ptr);
				int_ptr = (int *)&line[8];
				data->beams[i].bath_alongtrack = (int)mb_swap_int(*int_ptr);
				int_ptr = (int *)&line[12];
				data->beams[i].tt = (unsigned int)mb_swap_int(*int_ptr);
#endif
				data->beams[i].quality = line[16];
				if (data->beams[i].quality <= 0)
					data->beams[i].quality = 8;
				data->beams[i].amplitude = (int)((mb_s_char)line[17] + 128);
#ifndef BYTESWAPPED
				short_ptr = (short *)&line[18];
				data->beams[i].time_offset = (unsigned short)*short_ptr;
				short_ptr = (short *)&line[20];
				data->beams[i].heave = (short)*short_ptr;
				short_ptr = (short *)&line[22];
				data->beams[i].roll = (short)*short_ptr;
				short_ptr = (short *)&line[24];
				data->beams[i].pitch = (short)*short_ptr;
				short_ptr = (short *)&line[26];
				data->beams[i].angle = (short)*short_ptr;
#else
				short_ptr = (short *)&line[18];
				data->beams[i].time_offset = (unsigned short)mb_swap_short(*short_ptr);
				short_ptr = (short *)&line[20];
				data->beams[i].heave = (short)mb_swap_short(*short_ptr);
				short_ptr = (short *)&line[22];
				data->beams[i].roll = (short)mb_swap_short(*short_ptr);
				short_ptr = (short *)&line[24];
				data->beams[i].pitch = (short)mb_swap_short(*short_ptr);
				short_ptr = (short *)&line[26];
				data->beams[i].angle = (short)mb_swap_short(*short_ptr);
#endif
			}

			if (status == MB_SUCCESS && verbose >= 5) {
				fprintf(stderr, "\ndbg5       beam:             %d\n", i);
				fprintf(stderr, "dbg5       bath:             %d\n", data->beams[i].bath);
				fprintf(stderr, "dbg5       bath_acrosstrack: %d\n", data->beams[i].bath_acrosstrack);
				fprintf(stderr, "dbg5       bath_alongtrack: %d\n", data->beams[i].bath_alongtrack);
				fprintf(stderr, "dbg5       tt:              %d\n", data->beams[i].tt);
				fprintf(stderr, "dbg5       quality:         %d\n", data->beams[i].quality);
				fprintf(stderr, "dbg5       amplitude:       %d\n", data->beams[i].amplitude);
				fprintf(stderr, "dbg5       time_offset:     %d\n", data->beams[i].time_offset);
				fprintf(stderr, "dbg5       heave:           %d\n", data->beams[i].heave);
				fprintf(stderr, "dbg5       roll:            %d\n", data->beams[i].roll);
				fprintf(stderr, "dbg5       pitch:           %d\n", data->beams[i].pitch);
				fprintf(stderr, "dbg5       angle:           %d\n", data->beams[i].angle);
			}
		}
	}

	/* read end of record into char array */
	status = fread(line, 1, 3, mbfp);
	if (status == 3)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
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
int mbr_elmk2unb_rd_data(int verbose, void *mbio_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)mb_io_ptr->raw_data;
	FILE *mbfp = mb_io_ptr->mbfp;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	char label[2];
	short int *type = (short int *)label;
	bool done = false;
	while (!done) {
		/* get next record label */
		if ((status = fread(&label[0], 1, 1, mb_io_ptr->mbfp)) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		if (label[0] == 0x02)
			if ((status = fread(&label[1], 1, 1, mb_io_ptr->mbfp)) != 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}

/* swap bytes if necessary */
#ifdef BYTESWAPPED
		*type = (short int)mb_swap_short(*type);
#endif

		/* read the appropriate data records */
		if (status == MB_FAILURE) {
			done = true;
		}
		else if (*type == ELACMK2_COMMENT) {
			status = mbr_elmk2unb_rd_comment(verbose, mbfp, data, error);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_COMMENT;
			}
		}
		else if (*type == ELACMK2_PARAMETER) {
			status = mbr_elmk2unb_rd_parameter(verbose, mbfp, data, error);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_PARAMETER;
			}
		}
		else if (*type == ELACMK2_POS) {
			status = mbr_elmk2unb_rd_pos(verbose, mbfp, data, error);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_NAV;
			}
		}
		else if (*type == ELACMK2_SVP) {
			status = mbr_elmk2unb_rd_svp(verbose, mbfp, data, error);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_VELOCITY_PROFILE;
			}
		}
		else if (*type == ELACMK2_BATHGEN) {
			status = mbr_elmk2unb_rd_bathgen(verbose, mbfp, data, error);
			if (status == MB_SUCCESS) {
				done = true;
				data->kind = MB_DATA_DATA;
			}
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done = true;
	}

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mbfp);

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
int mbr_rt_elmk2unb(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int time_i[7];
	double time_d;
	double lon, lat, heading, speed;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)mb_io_ptr->raw_data;
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* read next data from file */
	const int status = mbr_elmk2unb_rd_data(verbose, mbio_ptr, error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* add nav records to list for interpolation */
	if (status == MB_SUCCESS && data->kind == MB_DATA_NAV) {
		mb_fix_y2k(verbose, data->pos_year, &time_i[0]);
		time_i[1] = data->pos_month;
		time_i[2] = data->pos_day;
		time_i[3] = data->pos_hour;
		time_i[4] = data->pos_minute;
		time_i[5] = data->pos_second;
		time_i[6] = 10000 * data->pos_hundredth_sec + 100 * data->pos_thousandth_sec;
		mb_get_time(verbose, time_i, &time_d);
		lon = data->pos_longitude * 0.00000009;
		lat = data->pos_latitude * 0.00000009;
		mb_navint_add(verbose, mbio_ptr, time_d, lon, lat, error);
	}

	/* interpolate navigation for survey pings if needed */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA && mb_io_ptr->nfix >= 1) {
		mb_fix_y2k(verbose, data->year, &time_i[0]);
		time_i[1] = data->month;
		time_i[2] = data->day;
		time_i[3] = data->hour;
		time_i[4] = data->minute;
		time_i[5] = data->second;
		time_i[6] = 10000 * data->hundredth_sec + 100 * data->thousandth_sec;
		mb_get_time(verbose, time_i, &time_d);
		heading = 0.01 * data->heading;
		mb_navint_interp(verbose, mbio_ptr, time_d, heading, 0.0, &lon, &lat, &speed, error);
		data->longitude = (int)(lon / 0.00000009);
		data->latitude = (int)(lat / 0.00000009);
	}

	/* translate values to elac data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		store->kind = data->kind;
		store->sonar = data->sonar;

		/* parameter telegram */
		if (store->kind == MB_DATA_PARAMETER) {
			store->par_year = data->par_year;
			store->par_month = data->par_month;
			store->par_day = data->par_day;
			store->par_hour = data->par_hour;
			store->par_minute = data->par_minute;
			store->par_second = data->par_second;
			store->par_hundredth_sec = data->par_hundredth_sec;
			store->par_thousandth_sec = data->par_thousandth_sec;
			store->roll_offset = data->roll_offset;
			store->pitch_offset = data->pitch_offset;
			store->heading_offset = data->heading_offset;
			store->time_delay = data->time_delay;
			store->transducer_port_height = data->transducer_port_height;
			store->transducer_starboard_height = data->transducer_starboard_height;
			store->transducer_port_depth = data->transducer_port_depth;
			store->transducer_starboard_depth = data->transducer_starboard_depth;
			store->transducer_port_x = data->transducer_port_x;
			store->transducer_starboard_x = data->transducer_starboard_x;
			store->transducer_port_y = data->transducer_port_y;
			store->transducer_starboard_y = data->transducer_starboard_y;
			store->transducer_port_error = data->transducer_port_error;
			store->transducer_starboard_error = data->transducer_starboard_error;
			store->antenna_height = data->antenna_height;
			store->antenna_x = data->antenna_x;
			store->antenna_y = data->antenna_y;
			store->vru_height = data->vru_height;
			store->vru_x = data->vru_x;
			store->vru_y = data->vru_y;
			store->line_number = data->line_number;
			store->start_or_stop = data->start_or_stop;
			store->transducer_serial_number = data->transducer_serial_number;
		}

		/* comment */
		if (store->kind == MB_DATA_COMMENT) {
			for (int i = 0; i < MBF_ELMK2UNB_COMMENT_LENGTH; i++)
				store->comment[i] = data->comment[i];
		}

		/* position (position telegrams) */
		if (store->kind == MB_DATA_NAV) {
			store->pos_year = data->pos_year;
			store->pos_month = data->pos_month;
			store->pos_day = data->pos_day;
			store->pos_hour = data->pos_hour;
			store->pos_minute = data->pos_minute;
			store->pos_second = data->pos_second;
			store->pos_hundredth_sec = data->pos_hundredth_sec;
			store->pos_thousandth_sec = data->pos_thousandth_sec;
			store->pos_latitude = data->pos_latitude;
			store->pos_longitude = data->pos_longitude;
			store->utm_northing = data->utm_northing;
			store->utm_easting = data->utm_easting;
			store->utm_zone_lon = data->utm_zone_lon;
			store->utm_zone = data->utm_zone;
			store->hemisphere = data->hemisphere;
			store->ellipsoid = data->ellipsoid;
			store->pos_spare = data->pos_spare;
			store->semi_major_axis = data->semi_major_axis;
			store->other_quality = data->other_quality;
		}

		/* sound velocity profile */
		if (store->kind == MB_DATA_VELOCITY_PROFILE) {
			store->svp_year = data->svp_year;
			store->svp_month = data->svp_month;
			store->svp_day = data->svp_day;
			store->svp_hour = data->svp_hour;
			store->svp_minute = data->svp_minute;
			store->svp_second = data->svp_second;
			store->svp_hundredth_sec = data->svp_hundredth_sec;
			store->svp_thousandth_sec = data->svp_thousandth_sec;
			store->svp_num = data->svp_num;
			for (int i = 0; i < 500; i++) {
				store->svp_depth[i] = data->svp_depth[i];
				store->svp_vel[i] = data->svp_vel[i];
			}
		}

		/* depth telegram */
		if (store->kind == MB_DATA_DATA) {
			store->year = data->year;
			store->month = data->month;
			store->day = data->day;
			store->hour = data->hour;
			store->minute = data->minute;
			store->second = data->second;
			store->hundredth_sec = data->hundredth_sec;
			store->thousandth_sec = data->thousandth_sec;
			store->longitude = lon;
			store->latitude = lat;
			store->speed = speed / 3.6;
			store->ping_num = data->ping_num;
			store->sound_vel = data->sound_vel;
			store->heading = data->heading;
			store->pulse_length = data->pulse_length;
			store->mode = data->mode;
			store->source_power = data->source_power;
			store->receiver_gain_stbd = data->receiver_gain_stbd;
			store->receiver_gain_port = data->receiver_gain_port;
			store->reserved = data->reserved;
			store->beams_bath = data->beams_bath;
			for (int i = 0; i < data->beams_bath; i++) {
				store->beams[i].bath = data->beams[i].bath;
				store->beams[i].bath_acrosstrack = data->beams[i].bath_acrosstrack;
				store->beams[i].bath_alongtrack = data->beams[i].bath_alongtrack;
				store->beams[i].tt = data->beams[i].tt;
				store->beams[i].quality = data->beams[i].quality;
				store->beams[i].amplitude = data->beams[i].amplitude;
				store->beams[i].time_offset = data->beams[i].time_offset;
				store->beams[i].heave = data->beams[i].heave;
				store->beams[i].roll = data->beams[i].roll;
				store->beams[i].pitch = data->beams[i].pitch;
				store->beams[i].angle = data->beams[i].angle;
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
int mbr_elmk2unb_wr_comment(int verbose, FILE *mbfp, void *data_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       comment:          %s\n", data->comment);
	}

	/* write the record label */
	char line[ELACMK2_COMMENT_SIZE + 3];
	short int label = ELACMK2_COMMENT;
#ifdef BYTESWAPPED
	label = (short)mb_swap_short(label);
#endif
	int status = fwrite(&label, 1, 2, mbfp);
	if (status != 2) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* construct record */
		int len = strlen(data->comment);
		if (len > MBSYS_ELACMK2_COMMENT_LENGTH)
			len = MBSYS_ELACMK2_COMMENT_LENGTH;
		for (int i = 0; i < len; i++)
			line[i] = data->comment[i];
		for (int i = len; i < MBSYS_ELACMK2_COMMENT_LENGTH; i++)
			line[i] = '\0';
		line[ELACMK2_COMMENT_SIZE] = 0x03;
		line[ELACMK2_COMMENT_SIZE + 1] = '\0';
		line[ELACMK2_COMMENT_SIZE + 2] = '\0';

		/* write out data */
		status = fwrite(line, 1, ELACMK2_COMMENT_SIZE + 3, mbfp);
		if (status != ELACMK2_COMMENT_SIZE + 3) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
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
int mbr_elmk2unb_wr_parameter(int verbose, FILE *mbfp, void *data_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", data->par_year);
		fprintf(stderr, "dbg5       month:            %d\n", data->par_month);
		fprintf(stderr, "dbg5       day:              %d\n", data->par_day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->par_hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->par_minute);
		fprintf(stderr, "dbg5       sec:              %d\n", data->par_second);
		fprintf(stderr, "dbg5       hundredth_sec:    %d\n", data->par_hundredth_sec);
		fprintf(stderr, "dbg5       thousandth_sec:   %d\n", data->par_thousandth_sec);
		fprintf(stderr, "dbg5       roll_offset:      %d\n", data->roll_offset);
		fprintf(stderr, "dbg5       pitch_offset:     %d\n", data->pitch_offset);
		fprintf(stderr, "dbg5       heading_offset:   %d\n", data->heading_offset);
		fprintf(stderr, "dbg5       time_delay:       %d\n", data->time_delay);
		fprintf(stderr, "dbg5       transducer_port_height: %d\n", data->transducer_port_height);
		fprintf(stderr, "dbg5       transducer_starboard_height:%d\n", data->transducer_starboard_height);
		fprintf(stderr, "dbg5       transducer_port_depth:     %d\n", data->transducer_port_depth);
		fprintf(stderr, "dbg5       transducer_starboard_depth:     %d\n", data->transducer_starboard_depth);
		fprintf(stderr, "dbg5       transducer_port_x:        %d\n", data->transducer_port_x);
		fprintf(stderr, "dbg5       transducer_starboard_x:        %d\n", data->transducer_starboard_x);
		fprintf(stderr, "dbg5       transducer_port_y:        %d\n", data->transducer_port_y);
		fprintf(stderr, "dbg5       transducer_starboard_y:  %d\n", data->transducer_starboard_y);
		fprintf(stderr, "dbg5       transducer_port_error:  %d\n", data->transducer_port_error);
		fprintf(stderr, "dbg5       transducer_starboard_error:  %d\n", data->transducer_starboard_error);
		fprintf(stderr, "dbg5       antenna_height:            %d\n", data->antenna_height);
		fprintf(stderr, "dbg5       antenna_x:      %d\n", data->antenna_x);
		fprintf(stderr, "dbg5       antenna_y:    %d\n", data->antenna_y);
		fprintf(stderr, "dbg5       vru_height:%d\n", data->vru_height);
		fprintf(stderr, "dbg5       vru_x:%d\n", data->vru_x);
		fprintf(stderr, "dbg5       vru_y:%d\n", data->vru_y);
		fprintf(stderr, "dbg5       line_number:%d\n", data->line_number);
		fprintf(stderr, "dbg5       start_or_stop:%d\n", data->start_or_stop);
		fprintf(stderr, "dbg5       transducer_serial_number:%d\n", data->transducer_serial_number);
	}

	/* write the record label */
	short int *short_ptr;
	char line[ELACMK2_PARAMETER_SIZE + 3];
	short int label = ELACMK2_PARAMETER;
#ifdef BYTESWAPPED
	label = (short)mb_swap_short(label);
#endif
	int status = fwrite(&label, 1, 2, mbfp);
	if (status != 2) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* construct record */
		line[0] = (char)data->par_day;
		line[1] = (char)data->par_month;
		line[2] = (char)data->par_year;
		line[3] = (char)data->par_hour;
		line[4] = (char)data->par_minute;
		line[5] = (char)data->par_second;
		line[6] = (char)data->par_hundredth_sec;
		line[7] = (char)data->par_thousandth_sec;
#ifndef BYTESWAPPED
		short_ptr = (short int *)&line[8];
		*short_ptr = data->roll_offset;
		short_ptr = (short int *)&line[10];
		*short_ptr = data->pitch_offset;
		short_ptr = (short int *)&line[12];
		*short_ptr = data->heading_offset;
		short_ptr = (short int *)&line[14];
		*short_ptr = data->time_delay;
		short_ptr = (short int *)&line[16];
		*short_ptr = data->transducer_port_height;
		short_ptr = (short int *)&line[18];
		*short_ptr = data->transducer_starboard_height;
		short_ptr = (short int *)&line[20];
		*short_ptr = data->transducer_port_depth;
		short_ptr = (short int *)&line[22];
		*short_ptr = data->transducer_starboard_depth;
		short_ptr = (short int *)&line[24];
		*short_ptr = data->transducer_port_x;
		short_ptr = (short int *)&line[26];
		*short_ptr = data->transducer_starboard_x;
		short_ptr = (short int *)&line[28];
		*short_ptr = data->transducer_port_y;
		short_ptr = (short int *)&line[30];
		*short_ptr = data->transducer_starboard_y;
		short_ptr = (short int *)&line[32];
		*short_ptr = data->transducer_port_error;
		short_ptr = (short int *)&line[34];
		*short_ptr = data->transducer_starboard_error;
		short_ptr = (short int *)&line[36];
		*short_ptr = data->antenna_height;
		short_ptr = (short int *)&line[38];
		*short_ptr = data->antenna_x;
		short_ptr = (short int *)&line[40];
		*short_ptr = data->antenna_y;
		short_ptr = (short int *)&line[42];
		*short_ptr = data->vru_height;
		short_ptr = (short int *)&line[44];
		*short_ptr = data->vru_x;
		short_ptr = (short int *)&line[46];
		*short_ptr = data->vru_y;
		short_ptr = (short int *)&line[48];
		*short_ptr = data->line_number;
		short_ptr = (short int *)&line[50];
		*short_ptr = data->start_or_stop;
		short_ptr = (short int *)&line[52];
		*short_ptr = data->transducer_serial_number;
#else
		short_ptr = (short int *)&line[8];
		*short_ptr = (short int)mb_swap_short(data->roll_offset);
		short_ptr = (short int *)&line[10];
		*short_ptr = (short int)mb_swap_short(data->pitch_offset);
		short_ptr = (short int *)&line[12];
		*short_ptr = (short int)mb_swap_short(data->heading_offset);
		short_ptr = (short int *)&line[14];
		*short_ptr = (short int)mb_swap_short(data->time_delay);
		short_ptr = (short int *)&line[16];
		*short_ptr = (short int)mb_swap_short(data->transducer_port_height);
		short_ptr = (short int *)&line[18];
		*short_ptr = (short int)mb_swap_short(data->transducer_starboard_height);
		short_ptr = (short int *)&line[20];
		*short_ptr = (short int)mb_swap_short(data->transducer_port_depth);
		short_ptr = (short int *)&line[22];
		*short_ptr = (short int)mb_swap_short(data->transducer_starboard_depth);
		short_ptr = (short int *)&line[24];
		*short_ptr = (short int)mb_swap_short(data->transducer_port_x);
		short_ptr = (short int *)&line[26];
		*short_ptr = (short int)mb_swap_short(data->transducer_starboard_x);
		short_ptr = (short int *)&line[28];
		*short_ptr = (short int)mb_swap_short(data->transducer_port_y);
		short_ptr = (short int *)&line[30];
		*short_ptr = (short int)mb_swap_short(data->transducer_starboard_y);
		short_ptr = (short int *)&line[32];
		*short_ptr = (short int)mb_swap_short(data->transducer_port_error);
		short_ptr = (short int *)&line[34];
		*short_ptr = (short int)mb_swap_short(data->transducer_starboard_error);
		short_ptr = (short int *)&line[36];
		*short_ptr = (short int)mb_swap_short(data->antenna_height);
		short_ptr = (short int *)&line[38];
		*short_ptr = (short int)mb_swap_short(data->antenna_x);
		short_ptr = (short int *)&line[40];
		*short_ptr = (short int)mb_swap_short(data->antenna_y);
		short_ptr = (short int *)&line[42];
		*short_ptr = (short int)mb_swap_short(data->vru_height);
		short_ptr = (short int *)&line[44];
		*short_ptr = (short int)mb_swap_short(data->vru_x);
		short_ptr = (short int *)&line[46];
		*short_ptr = (short int)mb_swap_short(data->vru_y);
		short_ptr = (short int *)&line[48];
		*short_ptr = (short int)mb_swap_short(data->line_number);
		short_ptr = (short int *)&line[50];
		*short_ptr = (short int)mb_swap_short(data->start_or_stop);
		short_ptr = (short int *)&line[52];
		*short_ptr = (short int)mb_swap_short(data->transducer_serial_number);
#endif
		line[ELACMK2_PARAMETER_SIZE] = 0x03;
		line[ELACMK2_PARAMETER_SIZE + 1] = '\0';
		line[ELACMK2_PARAMETER_SIZE + 2] = '\0';

		/* write out data */
		status = fwrite(line, 1, ELACMK2_PARAMETER_SIZE + 3, mbfp);
		if (status != ELACMK2_PARAMETER_SIZE + 3) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
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
int mbr_elmk2unb_wr_pos(int verbose, FILE *mbfp, void *data_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", data->pos_year);
		fprintf(stderr, "dbg5       month:            %d\n", data->pos_month);
		fprintf(stderr, "dbg5       day:              %d\n", data->pos_day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->pos_hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->pos_minute);
		fprintf(stderr, "dbg5       sec:              %d\n", data->pos_second);
		fprintf(stderr, "dbg5       hundredth_sec:    %d\n", data->pos_hundredth_sec);
		fprintf(stderr, "dbg5       thousandth_sec:   %d\n", data->pos_thousandth_sec);
		fprintf(stderr, "dbg5       pos_latitude:     %d\n", data->pos_latitude);
		fprintf(stderr, "dbg5       pos_longitude:    %d\n", data->pos_longitude);
		fprintf(stderr, "dbg5       utm_northing:     %d\n", data->utm_northing);
		fprintf(stderr, "dbg5       utm_easting:      %d\n", data->utm_easting);
		fprintf(stderr, "dbg5       utm_zone_lon:     %d\n", data->utm_zone_lon);
		fprintf(stderr, "dbg5       utm_zone:         %c\n", data->utm_zone);
		fprintf(stderr, "dbg5       hemisphere:       %c\n", data->hemisphere);
		fprintf(stderr, "dbg5       ellipsoid:        %c\n", data->ellipsoid);
		fprintf(stderr, "dbg5       pos_spare:        %c\n", data->pos_spare);
		fprintf(stderr, "dbg5       semi_major_axis:  %d\n", data->semi_major_axis);
		fprintf(stderr, "dbg5       other_quality:    %d\n", data->other_quality);
	}

	/* write the record label */
	char line[ELACMK2_POS_SIZE + 3];
	short int *short_ptr;
	int *int_ptr;
	short int label = ELACMK2_POS;
#ifdef BYTESWAPPED
	label = (short)mb_swap_short(label);
#endif
	int status = fwrite(&label, 1, 2, mbfp);
	if (status != 2) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* construct record */
		line[0] = (char)data->pos_day;
		line[1] = (char)data->pos_month;
		line[2] = (char)data->pos_year;
		line[3] = (char)data->pos_hour;
		line[4] = (char)data->pos_minute;
		line[5] = (char)data->pos_second;
		line[6] = (char)data->pos_hundredth_sec;
		line[7] = (char)data->pos_thousandth_sec;
#ifndef BYTESWAPPED
		int_ptr = (int *)&line[8];
		*int_ptr = data->pos_latitude;
		int_ptr = (int *)&line[12];
		*int_ptr = data->pos_longitude;
		int_ptr = (int *)&line[16];
		*int_ptr = data->utm_northing;
		int_ptr = (int *)&line[20];
		*int_ptr = data->utm_easting;
		int_ptr = (int *)&line[24];
		*int_ptr = data->utm_zone_lon;
		line[28] = data->utm_zone;
		line[29] = data->hemisphere;
		line[30] = data->ellipsoid;
		line[31] = data->pos_spare;
		short_ptr = (short int *)&line[32];
		*short_ptr = (short int)data->semi_major_axis;
		short_ptr = (short int *)&line[34];
		*short_ptr = (short int)data->other_quality;
#else
		int_ptr = (int *)&line[8];
		*int_ptr = (int)mb_swap_int(data->pos_latitude);
		int_ptr = (int *)&line[12];
		*int_ptr = (int)mb_swap_int(data->pos_longitude);
		int_ptr = (int *)&line[16];
		*int_ptr = (int)mb_swap_int(data->utm_northing);
		int_ptr = (int *)&line[20];
		*int_ptr = (int)mb_swap_int(data->utm_easting);
		int_ptr = (int *)&line[24];
		*int_ptr = (int)mb_swap_int(data->utm_zone_lon);
		line[28] = data->utm_zone;
		line[29] = data->hemisphere;
		line[30] = data->ellipsoid;
		line[31] = data->pos_spare;
		short_ptr = (short int *)&line[32];
		*short_ptr = (int)mb_swap_short(data->semi_major_axis);
		short_ptr = (short int *)&line[34];
		*short_ptr = (int)mb_swap_short(data->other_quality);
#endif
		line[ELACMK2_POS_SIZE] = 0x03;
		line[ELACMK2_POS_SIZE + 1] = '\0';
		line[ELACMK2_POS_SIZE + 2] = '\0';

		/* write out data */
		status = fwrite(line, 1, ELACMK2_POS_SIZE + 3, mbfp);
		if (status != ELACMK2_POS_SIZE + 3) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
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
int mbr_elmk2unb_wr_svp(int verbose, FILE *mbfp, void *data_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", data->svp_year);
		fprintf(stderr, "dbg5       month:            %d\n", data->svp_month);
		fprintf(stderr, "dbg5       day:              %d\n", data->svp_day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->svp_hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->svp_minute);
		fprintf(stderr, "dbg5       sec:              %d\n", data->svp_second);
		fprintf(stderr, "dbg5       hundredth_sec:    %d\n", data->svp_hundredth_sec);
		fprintf(stderr, "dbg5       thousandth_sec:   %d\n", data->svp_thousandth_sec);
		fprintf(stderr, "dbg5       svp_latitude:     %d\n", data->svp_latitude);
		fprintf(stderr, "dbg5       svp_longitude:    %d\n", data->svp_longitude);
		fprintf(stderr, "dbg5       svp_num:          %d\n", data->svp_num);
		for (int i = 0; i < data->svp_num; i++)
			fprintf(stderr, "dbg5       depth: %d     vel: %d\n", data->svp_depth[i], data->svp_vel[i]);
	}

	/* write the record label */
	char line[ELACMK2_SVP_SIZE + 3];
	short int *short_ptr;
	short int *short_ptr2;
	int *int_ptr;
	short int label = ELACMK2_SVP;
#ifdef BYTESWAPPED
	label = (short)mb_swap_short(label);
#endif
	int status = fwrite(&label, 1, 2, mbfp);
	if (status != 2) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS) {
		/* construct record */
		line[0] = (char)data->svp_day;
		line[1] = (char)data->svp_month;
		line[2] = (char)data->svp_year;
		line[3] = (char)data->svp_hour;
		line[4] = (char)data->svp_minute;
		line[5] = (char)data->svp_second;
		line[6] = (char)data->svp_hundredth_sec;
		line[7] = (char)data->svp_thousandth_sec;
#ifndef BYTESWAPPED
		int_ptr = (int *)&line[8];
		*int_ptr = data->svp_latitude;
		int_ptr = (int *)&line[12];
		*int_ptr = data->svp_longitude;
#else
		int_ptr = (int *)&line[8];
		*int_ptr = (int)mb_swap_int(data->svp_latitude);
		int_ptr = (int *)&line[12];
		*int_ptr = (int)mb_swap_int(data->svp_longitude);
#endif
		for (int i = 0; i < data->svp_num; i++) {
			short_ptr = (short int *)&line[16 + 4 * i];
			short_ptr2 = (short int *)&line[18 + 4 * i];
#ifndef BYTESWAPPED
			*short_ptr = (short int)data->svp_depth[i];
			*short_ptr2 = (short int)data->svp_vel[i];
#else
			*short_ptr = (short int)mb_swap_short((short int)data->svp_depth[i]);
			*short_ptr2 = (short int)mb_swap_short((short int)data->svp_vel[i]);
#endif
		}
		for (int i = data->svp_num; i < 500; i++) {
			short_ptr = (short int *)&line[16 + 4 * i];
			short_ptr2 = (short int *)&line[18 + 4 * i];
			*short_ptr = 0;
			*short_ptr2 = 0;
		}
		line[ELACMK2_SVP_SIZE] = 0x03;
		line[ELACMK2_SVP_SIZE + 1] = '\0';
		line[ELACMK2_SVP_SIZE + 2] = '\0';

		/* write out data */
		status = fwrite(line, 1, ELACMK2_SVP_SIZE + 3, mbfp);
		if (status != ELACMK2_SVP_SIZE + 3) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
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
int mbr_elmk2unb_wr_bathgen(int verbose, FILE *mbfp, void *data_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to raw data structure */
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)data_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       year:             %d\n", data->year);
		fprintf(stderr, "dbg5       month:            %d\n", data->month);
		fprintf(stderr, "dbg5       day:              %d\n", data->day);
		fprintf(stderr, "dbg5       hour:             %d\n", data->hour);
		fprintf(stderr, "dbg5       minute:           %d\n", data->minute);
		fprintf(stderr, "dbg5       sec:              %d\n", data->second);
		fprintf(stderr, "dbg5       hundredth_sec:    %d\n", data->hundredth_sec);
		fprintf(stderr, "dbg5       thousandth_sec:   %d\n", data->thousandth_sec);
		fprintf(stderr, "dbg5       ping_num:         %d\n", data->ping_num);
		fprintf(stderr, "dbg5       sound_vel:        %d\n", data->sound_vel);
		fprintf(stderr, "dbg5       heading:          %d\n", data->heading);
		fprintf(stderr, "dbg5       pulse_length:     %d\n", data->pulse_length);
		fprintf(stderr, "dbg5       mode:             %d\n", data->mode);
		fprintf(stderr, "dbg5       source_power:     %d\n", data->source_power);
		fprintf(stderr, "dbg5       receiver_gain_stbd:%d\n", data->receiver_gain_stbd);
		fprintf(stderr, "dbg5       receiver_gain_port:%d\n", data->receiver_gain_port);
		fprintf(stderr, "dbg5       reserved:         %d\n", data->reserved);
		fprintf(stderr, "dbg5       beams_bath:       %d\n", data->beams_bath);
		for (int i = 0; i < data->beams_bath; i++) {
			fprintf(stderr, "\ndbg5       beam:             %d\n", i);
			fprintf(stderr, "dbg5       bath:             %d\n", data->beams[i].bath);
			fprintf(stderr, "dbg5       bath_acrosstrack: %d\n", data->beams[i].bath_acrosstrack);
			fprintf(stderr, "dbg5       bath_alongtrack: %d\n", data->beams[i].bath_alongtrack);
			fprintf(stderr, "dbg5       tt:              %d\n", data->beams[i].tt);
			fprintf(stderr, "dbg5       quality:         %d\n", data->beams[i].quality);
			fprintf(stderr, "dbg5       amplitude:       %d\n", data->beams[i].amplitude);
			fprintf(stderr, "dbg5       time_offset:     %d\n", data->beams[i].time_offset);
			fprintf(stderr, "dbg5       heave:           %d\n", data->beams[i].heave);
			fprintf(stderr, "dbg5       roll:            %d\n", data->beams[i].roll);
			fprintf(stderr, "dbg5       pitch:           %d\n", data->beams[i].pitch);
			fprintf(stderr, "dbg5       angle:           %d\n", data->beams[i].angle);
		}
	}

	/* write the record label */
	char line[ELACMK2_COMMENT_SIZE];
	short int *short_ptr;
	int *int_ptr;
	short int label = ELACMK2_BATHGEN;
#ifdef BYTESWAPPED
	label = (short)mb_swap_short(label);
#endif
	int status = fwrite(&label, 1, 2, mbfp);
	if (status != 2) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write out the header data */
	if (status == MB_SUCCESS) {
		/* construct record */
		line[0] = (char)data->day;
		line[1] = (char)data->month;
		line[2] = (char)data->year;
		line[3] = (char)data->hour;
		line[4] = (char)data->minute;
		line[5] = (char)data->second;
		line[6] = (char)data->hundredth_sec;
		line[7] = (char)data->thousandth_sec;
#ifndef BYTESWAPPED
		short_ptr = (short int *)&line[8];
		*short_ptr = (unsigned short)data->ping_num;
		short_ptr = (short int *)&line[10];
		*short_ptr = (unsigned short)data->sound_vel;
		short_ptr = (short int *)&line[12];
		*short_ptr = (unsigned short)data->heading;
		short_ptr = (short int *)&line[14];
		*short_ptr = (unsigned short)data->pulse_length;
#else
		short_ptr = (short int *)&line[8];
		*short_ptr = (unsigned short)mb_swap_short((unsigned short)data->ping_num);
		short_ptr = (short int *)&line[10];
		*short_ptr = (unsigned short)mb_swap_short((unsigned short)data->sound_vel);
		short_ptr = (short int *)&line[12];
		*short_ptr = (unsigned short)mb_swap_short((unsigned short)data->heading);
		short_ptr = (short int *)&line[14];
		*short_ptr = (unsigned short)mb_swap_short((unsigned short)data->pulse_length);
#endif
		line[16] = (char)data->mode;
		line[17] = (char)data->source_power;
		line[18] = (char)data->receiver_gain_stbd;
		line[19] = (char)data->receiver_gain_port;
#ifndef BYTESWAPPED
		short_ptr = (short int *)&line[20];
		*short_ptr = (short int)data->reserved;
		short_ptr = (short int *)&line[22];
		*short_ptr = (short int)data->beams_bath;
#else
		short_ptr = (short int *)&line[20];
		*short_ptr = (short int)mb_swap_short((short int)data->reserved);
		short_ptr = (short int *)&line[22];
		*short_ptr = (short int)mb_swap_short((short int)data->beams_bath);
#endif

		/* write out data */
		status = fwrite(line, 1, ELACMK2_BATHGEN_HDR_SIZE, mbfp);
		if (status != ELACMK2_BATHGEN_HDR_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* write out the beam data */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < data->beams_bath; i++) {
#ifndef BYTESWAPPED
			int_ptr = (int *)&line[0];
			*int_ptr = (unsigned int)data->beams[i].bath;
			int_ptr = (int *)&line[4];
			*int_ptr = (int)data->beams[i].bath_acrosstrack;
			int_ptr = (int *)&line[8];
			*int_ptr = (int)data->beams[i].bath_alongtrack;
			int_ptr = (int *)&line[12];
			*int_ptr = (unsigned int)data->beams[i].tt;
#else
			int_ptr = (int *)&line[0];
			*int_ptr = (unsigned int)mb_swap_int(data->beams[i].bath);
			int_ptr = (int *)&line[4];
			*int_ptr = (int)mb_swap_int(data->beams[i].bath_acrosstrack);
			int_ptr = (int *)&line[8];
			*int_ptr = (int)mb_swap_int(data->beams[i].bath_alongtrack);
			int_ptr = (int *)&line[12];
			*int_ptr = (unsigned int)mb_swap_int(data->beams[i].tt);
#endif
			line[16] = data->beams[i].quality;
			line[17] = (char)((mb_s_char)(data->beams[i].amplitude - 128));
#ifndef BYTESWAPPED
			short_ptr = (short *)&line[18];
			*short_ptr = (unsigned short)data->beams[i].time_offset;
			short_ptr = (short *)&line[20];
			*short_ptr = (short)data->beams[i].heave;
			short_ptr = (short *)&line[22];
			*short_ptr = (short)data->beams[i].roll;
			short_ptr = (short *)&line[24];
			*short_ptr = (short)data->beams[i].pitch;
			short_ptr = (short *)&line[26];
			*short_ptr = (short)data->beams[i].angle;
#else
			short_ptr = (short *)&line[18];
			*short_ptr = (unsigned short)mb_swap_short(data->beams[i].time_offset);
			short_ptr = (short *)&line[20];
			*short_ptr = (short)mb_swap_short(data->beams[i].heave);
			short_ptr = (short *)&line[22];
			*short_ptr = (short)mb_swap_short(data->beams[i].roll);
			short_ptr = (short *)&line[24];
			*short_ptr = (short)mb_swap_short(data->beams[i].pitch);
			short_ptr = (short *)&line[26];
			*short_ptr = (short)mb_swap_short(data->beams[i].angle);
#endif

			/* write out data */
			status = fwrite(line, 1, ELACMK2_BATHGEN_BEAM_SIZE, mbfp);
			if (status != ELACMK2_BATHGEN_BEAM_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
		}
	}

	/* write out the eor data */
	if (status == MB_SUCCESS) {
		line[0] = 0x03;
		line[1] = '\0';
		line[2] = '\0';

		/* write out data */
		status = fwrite(line, 1, 3, mbfp);
		if (status != 3) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
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
int mbr_elmk2unb_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error) {
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
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)data_ptr;
	FILE *mbfp = mb_io_ptr->mbfp;

	int status = MB_SUCCESS;

	if (data->kind == MB_DATA_COMMENT) {
		status = mbr_elmk2unb_wr_comment(verbose, mbfp, data, error);
	}
	else if (data->kind == MB_DATA_PARAMETER) {
		status = mbr_elmk2unb_wr_parameter(verbose, mbfp, data, error);
	}
	else if (data->kind == MB_DATA_NAV) {
		status = mbr_elmk2unb_wr_pos(verbose, mbfp, data, error);
	}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE) {
		status = mbr_elmk2unb_wr_svp(verbose, mbfp, data, error);
	}
	else if (data->kind == MB_DATA_DATA) {
		status = mbr_elmk2unb_wr_bathgen(verbose, mbfp, data, error);
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
int mbr_wt_elmk2unb(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_elmk2unb_struct *data = (struct mbf_elmk2unb_struct *)mb_io_ptr->raw_data;
	char *data_ptr = (char *)data;
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL) {
		data->kind = store->kind;
		data->sonar = store->sonar;

		/* parameter telegram */
		data->par_year = store->par_year;
		data->par_month = store->par_month;
		data->par_day = store->par_day;
		data->par_hour = store->par_hour;
		data->par_minute = store->par_minute;
		data->par_second = store->par_second;
		data->par_hundredth_sec = store->par_hundredth_sec;
		data->par_thousandth_sec = store->par_thousandth_sec;
		data->roll_offset = store->roll_offset;
		data->pitch_offset = store->pitch_offset;
		data->heading_offset = store->heading_offset;
		data->time_delay = store->time_delay;
		data->transducer_port_height = store->transducer_port_height;
		data->transducer_starboard_height = store->transducer_starboard_height;
		data->transducer_port_depth = store->transducer_port_depth;
		data->transducer_starboard_depth = store->transducer_starboard_depth;
		data->transducer_port_x = store->transducer_port_x;
		data->transducer_starboard_x = store->transducer_starboard_x;
		data->transducer_port_y = store->transducer_port_y;
		data->transducer_starboard_y = store->transducer_starboard_y;
		data->transducer_port_error = store->transducer_port_error;
		data->transducer_starboard_error = store->transducer_starboard_error;
		data->antenna_height = store->antenna_height;
		data->antenna_x = store->antenna_x;
		data->antenna_y = store->antenna_y;
		data->vru_height = store->vru_height;
		data->vru_x = store->vru_x;
		data->vru_y = store->vru_y;
		data->line_number = store->line_number;
		data->start_or_stop = store->start_or_stop;
		data->transducer_serial_number = store->transducer_serial_number;
		for (int i = 0; i < MBF_ELMK2UNB_COMMENT_LENGTH; i++)
			data->comment[i] = store->comment[i];

		/* position (position telegrams) */
		data->pos_year = store->pos_year;
		data->pos_month = store->pos_month;
		data->pos_day = store->pos_day;
		data->pos_hour = store->pos_hour;
		data->pos_minute = store->pos_minute;
		data->pos_second = store->pos_second;
		data->pos_hundredth_sec = store->pos_hundredth_sec;
		data->pos_thousandth_sec = store->pos_thousandth_sec;
		data->pos_latitude = store->pos_latitude;
		data->pos_longitude = store->pos_longitude;
		data->utm_northing = store->utm_northing;
		data->utm_easting = store->utm_easting;
		data->utm_zone_lon = store->utm_zone_lon;
		data->utm_zone = store->utm_zone;
		data->hemisphere = store->hemisphere;
		data->ellipsoid = store->ellipsoid;
		data->pos_spare = store->pos_spare;
		data->semi_major_axis = store->semi_major_axis;
		data->other_quality = store->other_quality;

		/* sound velocity profile */
		data->svp_year = store->svp_year;
		data->svp_month = store->svp_month;
		data->svp_day = store->svp_day;
		data->svp_hour = store->svp_hour;
		data->svp_minute = store->svp_minute;
		data->svp_second = store->svp_second;
		data->svp_hundredth_sec = store->svp_hundredth_sec;
		data->svp_thousandth_sec = store->svp_thousandth_sec;
		data->svp_num = store->svp_num;
		for (int i = 0; i < 500; i++) {
			data->svp_depth[i] = store->svp_depth[i];
			data->svp_vel[i] = store->svp_vel[i];
		}

		/* depth telegram */
		data->year = store->year;
		data->month = store->month;
		data->day = store->day;
		data->hour = store->hour;
		data->minute = store->minute;
		data->second = store->second;
		data->hundredth_sec = store->hundredth_sec;
		data->thousandth_sec = store->thousandth_sec;
		data->ping_num = store->ping_num;
		data->sound_vel = store->sound_vel;
		data->heading = store->heading;
		data->pulse_length = store->pulse_length;
		data->mode = store->mode;
		data->source_power = store->source_power;
		data->receiver_gain_stbd = store->receiver_gain_stbd;
		data->receiver_gain_port = store->receiver_gain_port;
		data->reserved = store->reserved;
		data->beams_bath = store->beams_bath;
		for (int i = 0; i < store->beams_bath; i++) {
			data->beams[i].bath = store->beams[i].bath;
			data->beams[i].bath_acrosstrack = store->beams[i].bath_acrosstrack;
			data->beams[i].bath_alongtrack = store->beams[i].bath_alongtrack;
			data->beams[i].tt = store->beams[i].tt;
			data->beams[i].quality = store->beams[i].quality;
			data->beams[i].amplitude = store->beams[i].amplitude;
			data->beams[i].time_offset = store->beams[i].time_offset;
			data->beams[i].heave = store->beams[i].heave;
			data->beams[i].roll = store->beams[i].roll;
			data->beams[i].pitch = store->beams[i].pitch;
			data->beams[i].angle = store->beams[i].angle;
		}
	}

	/* write next data to file */
	const int status = mbr_elmk2unb_wr_data(verbose, mbio_ptr, data_ptr, error);

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
int mbr_register_elmk2unb(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_elmk2unb(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_elmk2unb;
	mb_io_ptr->mb_io_format_free = &mbr_dem_elmk2unb;
	mb_io_ptr->mb_io_store_alloc = &mbsys_elacmk2_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_elacmk2_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_elmk2unb;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_elmk2unb;
	mb_io_ptr->mb_io_dimensions = &mbsys_elacmk2_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_elacmk2_extract;
	mb_io_ptr->mb_io_insert = &mbsys_elacmk2_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_elacmk2_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_elacmk2_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_elacmk2_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_elacmk2_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_elacmk2_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_elacmk2_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_elacmk2_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_elacmk2_copy;
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
