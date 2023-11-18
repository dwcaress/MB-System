/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em12darw.c	2/2/93
 *
 *    Copyright (c) 1994-2023 by
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
 * mbr_em12darw.c contains the functions for reading and writing
 * multibeam data in the EM12DARW format.
 * These functions include:
 *   mbr_alm_em12darw	- allocate read/write memory
 *   mbr_dem_em12darw	- deallocate read/write memory
 *   mbr_rt_em12darw	- read and translate data
 *   mbr_wt_em12darw	- translate and write data
 *
 * Author:	R. B. Owens
 * Date:	January 24, 1994
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbf_em12darw.h"
#include "mbsys_simrad.h"

/*--------------------------------------------------------------------*/
int mbr_info_em12darw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_SIMRAD;
	*beams_bath_max = MBF_EM12DARW_BEAMS;
	*beams_amp_max = MBF_EM12DARW_BEAMS;
	*pixels_ss_max = 0;
	strncpy(format_name, "EM12DARW", MB_NAME_LENGTH);
	strncpy(system_name, "SIMRAD", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_EM12DARW\nInformal Description: Simrad EM12S RRS Darwin processed format\nAttributes:     "
	        "      Simrad EM12S, bathymetry and amplitude,\n                      81 beams, binary, Oxford University.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = false;
	*traveltime = true;
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
int mbr_zero_em12darw(int verbose, char *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to data descriptor */
	struct mbf_em12darw_struct *data = (struct mbf_em12darw_struct *)data_ptr;

	/* initialize everything to zeros */
	if (data != NULL) {
		/* record type */
		data->func = 150;

		/* time */
		data->year = 0;
		data->jday = 0;
		data->minute = 0;
		data->secs = 0;

		/* navigation */
		data->latitude = 0.0;
		data->longitude = 0.0;
		data->speed = 0.0;
		data->gyro = 0.0;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;

		/* other parameters */
		data->corflag = 0;
		data->utm_merd = 0.0;
		data->utm_zone = 0;
		data->posq = 0;
		data->pingno = 0;
		data->mode = 0;
		data->depthl = 0.0;
		data->sndval = 0.0;

		/* beam values */
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			data->depth[i] = 0;
			data->distacr[i] = 0;
			data->distalo[i] = 0;
			data->range[i] = 0;
			data->refl[i] = 0;
			data->beamq[i] = 0;
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
int mbr_alm_em12darw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_em12darw_struct);
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status &= mbsys_simrad_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbf_em12darw_struct *data = (struct mbf_em12darw_struct *)mb_io_ptr->raw_data;
	char *data_ptr = (char *)data;

	/* initialize everything to zeros */
	mbr_zero_em12darw(verbose, data_ptr, error);

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
int mbr_dem_em12darw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->raw_data, error);
	status &= mbsys_simrad_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_rt_em12darw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	struct mbsys_simrad_survey_struct *ping;
	char line[MBF_EM12DARW_RECORD_LENGTH];
	int index;
	int time_j[5];
	int time_i[7];
	int kind;

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
	struct mbf_em12darw_struct *data = (struct mbf_em12darw_struct *)mb_io_ptr->raw_data;
	char *datacomment = (char *)&line[80];
	struct mbsys_simrad_struct *store = (struct mbsys_simrad_struct *)store_ptr;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	int status = MB_SUCCESS;

	/* read next record from file */
	if ((status = fread(line, 1, MBF_EM12DARW_RECORD_LENGTH, mb_io_ptr->mbfp)) == MBF_EM12DARW_RECORD_LENGTH) {
		mb_io_ptr->file_bytes += status;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	else {
		mb_io_ptr->file_bytes += status;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get data type */
	kind = MB_DATA_NONE;
	if (status == MB_SUCCESS) {
		mb_get_binary_short(false, &line[0], &(data->func));
	}

	/* deal with comment */
	if (status == MB_SUCCESS && data->func == 100) {
		kind = MB_DATA_COMMENT;

		strncpy(store->comment, datacomment, MBSYS_SIMRAD_COMMENT_LENGTH);
	}

	/* deal with data */
	else if (status == MB_SUCCESS && data->func == 150) {
		kind = MB_DATA_DATA;

		index = 2;
		mb_get_binary_short(false, &line[index], &(data->year));
		index += 2;
		mb_get_binary_short(false, &line[index], &(data->jday));
		index += 2;
		mb_get_binary_short(false, &line[index], &(data->minute));
		index += 2;
		mb_get_binary_short(false, &line[index], &(data->secs));
		index += 8;
		mb_get_binary_double(false, &line[index], &(data->latitude));
		index += 8;
		mb_get_binary_double(false, &line[index], &(data->longitude));
		index += 8;
		mb_get_binary_short(false, &line[index], &(data->corflag));
		index += 4;
		mb_get_binary_float(false, &line[index], &(data->utm_merd));
		index += 4;
		mb_get_binary_short(false, &line[index], &(data->utm_zone));
		index += 2;
		mb_get_binary_short(false, &line[index], &(data->posq));
		index += 2;
		mb_get_binary_int(false, &line[index], &(data->pingno));
		index += 4;
		mb_get_binary_short(false, &line[index], &(data->mode));
		index += 4;
		mb_get_binary_float(false, &line[index], &(data->depthl));
		index += 4;
		mb_get_binary_float(false, &line[index], &(data->speed));
		index += 4;
		mb_get_binary_float(false, &line[index], &(data->gyro));
		index += 4;
		mb_get_binary_float(false, &line[index], &(data->roll));
		index += 4;
		mb_get_binary_float(false, &line[index], &(data->pitch));
		index += 4;
		mb_get_binary_float(false, &line[index], &(data->heave));
		index += 4;
		mb_get_binary_float(false, &line[index], &(data->sndval));
		index += 4;
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_get_binary_short(false, &line[index], &(data->depth[i]));
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_get_binary_short(false, &line[index], &(data->distacr[i]));
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_get_binary_short(false, &line[index], &(data->distalo[i]));
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_get_binary_short(false, &line[index], &(data->range[i]));
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_get_binary_short(false, &line[index], &(data->refl[i]));
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_get_binary_short(false, &line[index], &(data->beamq[i]));
			index += 2;
		}

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Data read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Read values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       year:       %d\n", data->year);
			fprintf(stderr, "dbg4       jday:       %d\n", data->jday);
			fprintf(stderr, "dbg4       minute:     %d\n", data->minute);
			fprintf(stderr, "dbg4       secs:       %d\n", data->secs);
			fprintf(stderr, "dbg4       latitude:   %f\n", data->latitude);
			fprintf(stderr, "dbg4       longitude:  %f\n", data->longitude);
			fprintf(stderr, "dbg4       corflag:    %d\n", data->corflag);
			fprintf(stderr, "dbg4       utm_merd:   %f\n", data->utm_merd);
			fprintf(stderr, "dbg4       utm_zone:   %d\n", data->utm_zone);
			fprintf(stderr, "dbg4       posq:       %d\n", data->posq);
			fprintf(stderr, "dbg4       pingno:     %d\n", data->pingno);
			fprintf(stderr, "dbg4       mode:       %d\n", data->mode);
			fprintf(stderr, "dbg4       depthl:     %f\n", data->depthl);
			fprintf(stderr, "dbg4       speed:      %f\n", data->speed);
			fprintf(stderr, "dbg4       gyro:       %f\n", data->gyro);
			fprintf(stderr, "dbg4       roll:       %f\n", data->roll);
			fprintf(stderr, "dbg4       pitch:      %f\n", data->pitch);
			fprintf(stderr, "dbg4       heave:      %f\n", data->heave);
			fprintf(stderr, "dbg4       sndval:     %f\n", data->sndval);
			for (int i = 0; i < MBF_EM12DARW_BEAMS; i++)
				fprintf(stderr, "dbg4       beam:%d  depth:%d  distacr:%d  distalo:%d  range:%d refl:%d beamq:%d\n", i,
				        data->depth[i], data->distacr[i], data->distalo[i], data->range[i], data->refl[i], data->beamq[i]);
		}
	}

	/* else unintelligible */
	else if (status == MB_SUCCESS) {
		kind = MB_DATA_NONE;
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = kind;
	mb_io_ptr->new_error = *error;

	/* translate values to em12 data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		/* type of data record */
		store->kind = kind;
		store->sonar = MBSYS_SIMRAD_EM12S;

		/* time */
		mb_fix_y2k(verbose, (int)data->year, &time_j[0]);
		time_j[1] = data->jday;
		time_j[2] = data->minute;
		time_j[3] = data->secs / 100;
		time_j[4] = 0.0001 * (100 * time_j[3] - data->secs);
		mb_get_itime(verbose, time_j, time_i);
		store->year = data->year;
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];
		store->centisecond = 0.0001 * time_i[6];
		store->pos_year = store->year;
		store->pos_month = store->month;
		store->pos_day = store->day;
		store->pos_hour = store->hour;
		store->pos_minute = store->minute;
		store->pos_second = store->second;
		store->pos_centisecond = store->centisecond;

		/* navigation */
		if (data->corflag == 0) {
			store->pos_latitude = data->latitude;
			store->pos_longitude = data->longitude;
			store->utm_northing = 0.0;
			store->utm_easting = 0.0;
		}
		else {
			store->pos_latitude = 0.0;
			store->pos_longitude = 0.0;
			store->utm_northing = data->latitude;
			store->utm_easting = data->longitude;
		}
		store->utm_zone = data->utm_zone;
		store->utm_zone_lon = data->utm_merd;
		store->utm_system = data->corflag;
		store->pos_quality = data->posq;
		store->speed = data->speed;
		store->line_heading = 10 * data->gyro;

		/* allocate secondary data structure for
		    survey data if needed */
		if (kind == MB_DATA_DATA && store->ping == NULL) {
			status = mbsys_simrad_survey_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* deal with putting survey data into
		secondary data structure */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
			/* get data structure pointer */
			ping = (struct mbsys_simrad_survey_struct *)store->ping;

			/* copy data */
			ping->longitude = data->longitude;
			ping->latitude = data->latitude;
			ping->swath_id = EM_SWATH_CENTER;
			ping->ping_number = data->pingno;
			ping->beams_bath = MBF_EM12DARW_BEAMS;
			ping->bath_mode = 0;
			ping->bath_res = data->mode;
			ping->bath_quality = 0;
			ping->keel_depth = data->depthl;
			ping->heading = (int)10 * data->gyro;
			ping->roll = (int)100 * data->roll;
			ping->pitch = (int)100 * data->pitch;
			ping->xducer_pitch = (int)100 * data->pitch;
			ping->ping_heave = (int)100 * data->heave;
			ping->sound_vel = (int)10 * data->sndval;
			ping->pixels_ss = 0;
			ping->ss_mode = 0;
			for (int i = 0; i < ping->beams_bath; i++) {
				if (data->depth[i] > 0) {
					ping->bath[i] = data->depth[i];
					ping->beamflag[i] = MB_FLAG_NONE;
				}
				else if (data->depth[i] < 0) {
					ping->bath[i] = -data->depth[i];
					ping->beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
				}
				else {
					ping->bath[i] = data->depth[i];
					ping->beamflag[i] = MB_FLAG_NULL;
				}
				ping->bath_acrosstrack[i] = data->distacr[i];
				ping->bath_alongtrack[i] = data->distalo[i];
				ping->tt[i] = data->range[i];
				ping->amp[i] = (mb_s_char)data->refl[i];
				ping->quality[i] = (mb_u_char)data->beamq[i];
				ping->heave[i] = (mb_s_char)0;
				ping->beam_frequency[i] = 0;
				ping->beam_samples[i] = 0;
				ping->beam_center_sample[i] = 0;
			}
		}

		else if (status == MB_SUCCESS && kind == MB_DATA_COMMENT) {
			/* comment */
			strncpy(store->comment, datacomment, MBSYS_SIMRAD_COMMENT_LENGTH);
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
int mbr_wt_em12darw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	struct mbsys_simrad_survey_struct *ping;
	char line[MBF_EM12DARW_RECORD_LENGTH];
	int index;
	int time_i[7];
	int time_j[5];
	int year;

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
	struct mbf_em12darw_struct *data = (struct mbf_em12darw_struct *)mb_io_ptr->raw_data;
	char *datacomment = (char *)&line[80];
	struct mbsys_simrad_struct *store = (struct mbsys_simrad_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Status at beginning of MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       store->kind:    %d\n", store->kind);
		fprintf(stderr, "dbg5       error:          %d\n", *error);
	}

	/*  translate values from em12 data storage structure */
	if (store->kind == MB_DATA_DATA) {
		/* record type */
		data->func = 150;

		/* time */
		time_i[0] = store->year;
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = store->centisecond;
		mb_get_jtime(verbose, time_i, time_j);
		mb_unfix_y2k(verbose, time_j[0], &year);
		data->year = (short)year;
		data->jday = time_j[1];
		data->minute = time_j[2];
		data->secs = 100 * time_j[3] + 0.0001 * time_j[4];

		/* navigation */
		data->utm_zone = store->utm_zone;
		data->utm_merd = store->utm_zone_lon;
		data->corflag = store->utm_system;
		data->posq = store->pos_quality;
		data->speed = store->speed;
		if (data->corflag == 0) {
			data->latitude = store->pos_latitude;
			data->longitude = store->pos_longitude;
		}
		else {
			data->latitude = store->utm_northing;
			data->longitude = store->utm_easting;
		}

		/* deal with survey data
		    in secondary data structure */
		if (store->ping != NULL) {
			/* get data structure pointer */
			ping = (struct mbsys_simrad_survey_struct *)store->ping;

			/* copy survey data */
			data->latitude = ping->latitude;
			data->longitude = ping->longitude;
			data->pingno = ping->ping_number;
			data->mode = ping->bath_res;
			data->depthl = ping->keel_depth;
			data->gyro = 0.1 * ping->heading;
			data->roll = 0.01 * ping->roll;
			data->pitch = 0.01 * ping->pitch;
			data->heave = 0.01 * ping->ping_heave;
			data->sndval = 0.1 * ping->sound_vel;
			for (int i = 0; i < ping->beams_bath; i++) {
				if (ping->beamflag[i] == MB_FLAG_NULL)
					data->depth[i] = 0;
				else if (!mb_beam_ok(ping->beamflag[i]))
					data->depth[i] = -ping->bath[i];
				else
					data->depth[i] = ping->bath[i];
				data->distacr[i] = ping->bath_acrosstrack[i];
				data->distalo[i] = ping->bath_alongtrack[i];
				data->range[i] = ping->tt[i];
				data->refl[i] = (short int)ping->amp[i];
				data->beamq[i] = (short int)ping->quality[i];
			}
		}
	}

	/* comment */
	else if (store->kind == MB_DATA_COMMENT) {
		data->func = 100;
		strncpy(datacomment, store->comment, MBSYS_SIMRAD_COMMENT_LENGTH);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Ready to write data in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       store->kind:       %d\n", store->kind);
		fprintf(stderr, "dbg5       error:             %d\n", *error);
	}

	if (verbose >= 4) {
		fprintf(stderr, "\ndbg4  Data to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg4  Status values:\n");
		fprintf(stderr, "dbg4       store->kind:%d\n", store->kind);
		fprintf(stderr, "dbg4       error:      %d\n", *error);
		if (store->kind == MB_DATA_DATA) {
			fprintf(stderr, "dbg4  Survey values:\n");
			fprintf(stderr, "dbg4       year:       %d\n", data->year);
			fprintf(stderr, "dbg4       jday:       %d\n", data->jday);
			fprintf(stderr, "dbg4       minute:     %d\n", data->minute);
			fprintf(stderr, "dbg4       secs:       %d\n", data->secs);
			fprintf(stderr, "dbg4       latitude:   %f\n", data->latitude);
			fprintf(stderr, "dbg4       longitude:  %f\n", data->longitude);
			fprintf(stderr, "dbg4       corflag:    %d\n", data->corflag);
			fprintf(stderr, "dbg4       utm_merd:   %f\n", data->utm_merd);
			fprintf(stderr, "dbg4       utm_zone:   %d\n", data->utm_zone);
			fprintf(stderr, "dbg4       posq:       %d\n", data->posq);
			fprintf(stderr, "dbg4       pingno:     %d\n", data->pingno);
			fprintf(stderr, "dbg4       mode:       %d\n", data->mode);
			fprintf(stderr, "dbg4       depthl:     %f\n", data->depthl);
			fprintf(stderr, "dbg4       speed:      %f\n", data->speed);
			fprintf(stderr, "dbg4       gyro:       %f\n", data->gyro);
			fprintf(stderr, "dbg4       roll:       %f\n", data->roll);
			fprintf(stderr, "dbg4       pitch:      %f\n", data->pitch);
			fprintf(stderr, "dbg4       heave:      %f\n", data->heave);
			fprintf(stderr, "dbg4       sndval:     %f\n", data->sndval);
			for (int i = 0; i < MBF_EM12DARW_BEAMS; i++)
				fprintf(stderr, "dbg4       beam:%d  depth:%d  distacr:%d  distalo:%d  range:%d refl:%d beamq:%d\n", i,
				        data->depth[i], data->distacr[i], data->distalo[i], data->range[i], data->refl[i], data->beamq[i]);
		}
		else if (store->kind == MB_DATA_COMMENT) {
			fprintf(stderr, "dbg4  Comment:\n");
			fprintf(stderr, "dbg4       comment:    %s\n", datacomment);
		}
	}

	/* deal with comment */
	if (store->kind == MB_DATA_COMMENT) {
		index = 0;
		for (int i = 0; i < MBF_EM12DARW_RECORD_LENGTH; i++)
			line[i] = 0;
		mb_put_binary_short(false, data->func, &line[0]);
		index += 2;
		strncpy(datacomment, store->comment, MBSYS_SIMRAD_COMMENT_LENGTH);
	}

	/* deal with data */
	else if (store->kind == MB_DATA_DATA) {
		index = 0;
		mb_put_binary_short(false, data->func, &line[0]);
		index += 2;
		mb_put_binary_short(false, data->year, &line[index]);
		index += 2;
		mb_put_binary_short(false, data->jday, &line[index]);
		index += 2;
		mb_put_binary_short(false, data->minute, &line[index]);
		index += 2;
		mb_put_binary_short(false, data->secs, &line[index]);
		index += 8;
		mb_put_binary_double(false, data->latitude, &line[index]);
		index += 8;
		mb_put_binary_double(false, data->longitude, &line[index]);
		index += 8;
		mb_put_binary_short(false, data->corflag, &line[index]);
		index += 4;
		mb_put_binary_float(false, data->utm_merd, &line[index]);
		index += 4;
		mb_put_binary_short(false, data->utm_zone, &line[index]);
		index += 2;
		mb_put_binary_short(false, data->posq, &line[index]);
		index += 2;
		mb_put_binary_int(false, data->pingno, &line[index]);
		index += 4;
		mb_put_binary_short(false, data->mode, &line[index]);
		index += 4;
		mb_put_binary_float(false, data->depthl, &line[index]);
		index += 4;
		mb_put_binary_float(false, data->speed, &line[index]);
		index += 4;
		mb_put_binary_float(false, data->gyro, &line[index]);
		index += 4;
		mb_put_binary_float(false, data->roll, &line[index]);
		index += 4;
		mb_put_binary_float(false, data->pitch, &line[index]);
		index += 4;
		mb_put_binary_float(false, data->heave, &line[index]);
		index += 4;
		mb_put_binary_float(false, data->sndval, &line[index]);
		index += 4;
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_put_binary_short(false, data->depth[i], &line[index]);
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_put_binary_short(false, data->distacr[i], &line[index]);
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_put_binary_short(false, data->distalo[i], &line[index]);
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_put_binary_short(false, data->range[i], &line[index]);
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_put_binary_short(false, data->refl[i], &line[index]);
			index += 2;
		}
		for (int i = 0; i < MBF_EM12DARW_BEAMS; i++) {
			mb_put_binary_short(false, data->beamq[i], &line[index]);
			index += 2;
		}
	}

	int status = MB_SUCCESS;

	/* write next record to file */
	if (store->kind == MB_DATA_DATA || store->kind == MB_DATA_COMMENT) {
		if ((status = fwrite(line, 1, MBF_EM12DARW_RECORD_LENGTH, mb_io_ptr->mbfp)) == MBF_EM12DARW_RECORD_LENGTH) {
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
int mbr_register_em12darw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_em12darw(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_em12darw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_em12darw;
	mb_io_ptr->mb_io_store_alloc = &mbsys_simrad_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_simrad_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_em12darw;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_em12darw;
	mb_io_ptr->mb_io_dimensions = &mbsys_simrad_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_simrad_extract;
	mb_io_ptr->mb_io_insert = &mbsys_simrad_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_simrad_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_simrad_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_simrad_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_simrad_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_simrad_detects;
	mb_io_ptr->mb_io_gains = &mbsys_simrad_gains;
	mb_io_ptr->mb_io_copyrecord = &mbsys_simrad_copy;
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
