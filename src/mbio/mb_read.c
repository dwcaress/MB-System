/*--------------------------------------------------------------------
 *    The MB-system:	mb_read.c	2/20/93
 *
 *    Copyright (c) 1993-2025 by
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
 * mb_read.c reads and averages multibeam data from a file
 * which has been initialized by mb_read_init(). Crosstrack distances
 * are mapped into lon and lat.
 *
 * Author:	D. W. Caress
 * Date:	February 20, 1993
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_read(int verbose, void *mbio_ptr, int *kind, int *pings, int time_i[7], double *time_d, double *navlon, double *navlat,
            double *speed, double *heading, double *distance, double *altitude, double *sensordepth, int *nbath, int *namp,
            int *nss, char *beamflag, double *bath, double *amp, double *bathlon, double *bathlat, double *ss, double *sslon,
            double *sslat, char *comment, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	char *store_ptr = (char *)mb_io_ptr->store_data;

	/* initialize binning values */
	mb_io_ptr->pings_read = 0;
	mb_io_ptr->pings_binned = 0;
	mb_io_ptr->time_d = 0.0;
	mb_io_ptr->lon = 0.0;
	mb_io_ptr->lat = 0.0;
	mb_io_ptr->speed = 0.0;
	mb_io_ptr->heading = 0.0;
	for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
		mb_io_ptr->beamflag[i] = MB_FLAG_NULL;
		mb_io_ptr->bath[i] = 0.0;
		mb_io_ptr->bath_acrosstrack[i] = 0.0;
		mb_io_ptr->bath_alongtrack[i] = 0.0;
		mb_io_ptr->bath_num[i] = 0;
	}
	for (int i = 0; i < mb_io_ptr->beams_amp_max; i++) {
		mb_io_ptr->amp[i] = 0.0;
		mb_io_ptr->amp_num[i] = 0;
	}
	for (int i = 0; i < mb_io_ptr->pixels_ss_max; i++) {
		mb_io_ptr->ss[i] = 0.0;
		mb_io_ptr->ss_acrosstrack[i] = 0.0;
		mb_io_ptr->ss_alongtrack[i] = 0.0;
		mb_io_ptr->ss_num[i] = 0;
	}

	int status = MB_SUCCESS;
	bool reset_last;
	double mtodeglon;
	double mtodeglat;
	double headingx = 0.0;
	double headingy = 0.0;

	/* read the data */
	bool done = false;
	while (!done) {

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  About to read ping in function <%s>\n", __func__);
			fprintf(stderr, "dbg2       need_new_ping: %d\n", mb_io_ptr->need_new_ping);
			fprintf(stderr, "dbg2       ping_count:    %d\n", mb_io_ptr->ping_count);
			fprintf(stderr, "dbg2       pings_read:    %d\n", mb_io_ptr->pings_read);
			fprintf(stderr, "dbg2       status:        %d\n", status);
			fprintf(stderr, "dbg2       error:         %d\n", *error);
		}

		/* get next ping */
		if (mb_io_ptr->need_new_ping) {
			status = mb_read_ping(verbose, mbio_ptr, store_ptr, &mb_io_ptr->new_kind, error);

			/* log errors */
			if (*error < MB_ERROR_NO_ERROR)
				mb_notice_log_error(verbose, mbio_ptr, *error);

			/* if io arrays have been reallocated, update the
			    pointers of arrays passed into this function,
			    as these pointers may have changed */
			if (status == MB_SUCCESS && mb_io_ptr->new_kind == MB_DATA_DATA) {
				if (mb_io_ptr->bath_arrays_reallocated) {
					status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&beamflag, error);
					status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&bath, error);
					status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&bathlon, error);
					status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&bathlat, error);
					mb_io_ptr->bath_arrays_reallocated = false;
				}
				if (mb_io_ptr->amp_arrays_reallocated) {
					status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&amp, error);
					mb_io_ptr->amp_arrays_reallocated = false;
				}
				if (mb_io_ptr->ss_arrays_reallocated) {
					status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&ss, error);
					status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&sslon, error);
					status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&sslat, error);
					mb_io_ptr->ss_arrays_reallocated = false;
				}
			}

			/* if survey data read into storage array */
			if (status == MB_SUCCESS && (mb_io_ptr->new_kind == MB_DATA_DATA || mb_io_ptr->new_kind == MB_DATA_COMMENT)) {
				status = mb_extract(verbose, mbio_ptr, store_ptr, &mb_io_ptr->new_kind, mb_io_ptr->new_time_i,
				                    &mb_io_ptr->new_time_d, &mb_io_ptr->new_lon, &mb_io_ptr->new_lat, &mb_io_ptr->new_speed,
				                    &mb_io_ptr->new_heading, &mb_io_ptr->new_beams_bath, &mb_io_ptr->new_beams_amp,
				                    &mb_io_ptr->new_pixels_ss, mb_io_ptr->new_beamflag, mb_io_ptr->new_bath, mb_io_ptr->new_amp,
				                    mb_io_ptr->new_bath_acrosstrack, mb_io_ptr->new_bath_alongtrack, mb_io_ptr->new_ss,
				                    mb_io_ptr->new_ss_acrosstrack, mb_io_ptr->new_ss_alongtrack, mb_io_ptr->new_comment, error);
			}
			if (status == MB_SUCCESS && mb_io_ptr->new_kind == MB_DATA_DATA) {
				status = mb_extract_altitude(verbose, mbio_ptr, store_ptr, &mb_io_ptr->new_kind, sensordepth, altitude, error);
			}

			/* if alternative nav is available use it for survey records */
			if (status == MB_SUCCESS && mb_io_ptr->new_kind == MB_DATA_DATA && mb_io_ptr->alternative_navigation) {
				double zoffset = 0.0;
				double tsensordepth = 0.0;
				int inavadjtime = 0;
				mb_linear_interp_longitude(	verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_navlon - 1,      mb_io_ptr->nav_alt_num, mb_io_ptr->new_time_d, &mb_io_ptr->new_lon,     &inavadjtime, error);
		        mb_linear_interp_latitude(	verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_navlat - 1,      mb_io_ptr->nav_alt_num, mb_io_ptr->new_time_d, &mb_io_ptr->new_lat,     &inavadjtime, error);
		        mb_linear_interp(			verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_speed - 1,       mb_io_ptr->nav_alt_num, mb_io_ptr->new_time_d, &mb_io_ptr->new_speed,   &inavadjtime, error);
		        mb_linear_interp_heading(	verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_heading - 1,     mb_io_ptr->nav_alt_num, mb_io_ptr->new_time_d, &mb_io_ptr->new_heading, &inavadjtime, error);
		      	mb_linear_interp(           verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_sensordepth - 1, mb_io_ptr->nav_alt_num, mb_io_ptr->new_time_d, &tsensordepth, 			&inavadjtime, error);
		        mb_linear_interp(           verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_zoffset - 1,     mb_io_ptr->nav_alt_num, mb_io_ptr->new_time_d, &zoffset,     			&inavadjtime, error);
		        if (mb_io_ptr->new_heading < 0.0)
		          mb_io_ptr->new_heading += 360.0;
		        else if (mb_io_ptr->new_heading > 360.0)
		          mb_io_ptr->new_heading -= 360.0;
    			double bath_correction = tsensordepth - *sensordepth + zoffset;
    			*sensordepth = tsensordepth + zoffset;
		      	for (int ibeam=0; ibeam<mb_io_ptr->new_beams_bath; ibeam++) {
		      		mb_io_ptr->new_bath[ibeam] += bath_correction;
		      	}
			}

			/* set errors if not survey data */
			if (status == MB_SUCCESS) {
				mb_io_ptr->need_new_ping = false;
				if (mb_io_ptr->new_kind == MB_DATA_DATA)
					mb_io_ptr->ping_count++;
				else if (mb_io_ptr->new_kind == MB_DATA_COMMENT) {
					mb_io_ptr->comment_count++;
					status = MB_FAILURE;
					*error = MB_ERROR_COMMENT;
					mb_io_ptr->new_error = *error;
					mb_notice_log_error(verbose, mbio_ptr, *error);
				}
				else if (mb_io_ptr->new_kind == MB_DATA_SUBBOTTOM_MCS || mb_io_ptr->new_kind == MB_DATA_SUBBOTTOM_CNTRBEAM ||
				         mb_io_ptr->new_kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
					status = MB_FAILURE;
					*error = MB_ERROR_SUBBOTTOM;
					mb_io_ptr->new_error = *error;
					mb_notice_log_error(verbose, mbio_ptr, *error);
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_OTHER;
					mb_io_ptr->new_error = *error;
					mb_notice_log_error(verbose, mbio_ptr, *error);
				}
			}
		}
		else {
			*error = mb_io_ptr->new_error;
			if (*error == MB_ERROR_NO_ERROR)
				status = MB_SUCCESS;
			else
				status = MB_FAILURE;
		}

		/* if not a fatal error, increment ping counter */
		if (status == MB_SUCCESS && mb_io_ptr->new_kind == MB_DATA_DATA)
			mb_io_ptr->pings_read++;

		/* if first ping read set "old" navigation values */
		if (status == MB_SUCCESS && mb_io_ptr->new_kind == MB_DATA_DATA && mb_io_ptr->ping_count == 1) {
			mb_io_ptr->old_time_d = mb_io_ptr->new_time_d;
			mb_io_ptr->old_lon = mb_io_ptr->new_lon;
			mb_io_ptr->old_lat = mb_io_ptr->new_lat;
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  New ping read in function <%s>\n", __func__);
			fprintf(stderr, "dbg2       need_new_ping: %d\n", mb_io_ptr->need_new_ping);
			fprintf(stderr, "dbg2       ping_count:    %d\n", mb_io_ptr->ping_count);
			fprintf(stderr, "dbg2       comment_count: %d\n", mb_io_ptr->comment_count);
			fprintf(stderr, "dbg2       pings_read:    %d\n", mb_io_ptr->pings_read);
			fprintf(stderr, "dbg2       status:        %d\n", status);
			fprintf(stderr, "dbg2       error:         %d\n", *error);
			fprintf(stderr, "dbg2       new_error:     %d\n", mb_io_ptr->new_error);
		}

		/* check for out of location or time bounds */
		if (status == MB_SUCCESS && mb_io_ptr->new_kind == MB_DATA_DATA) {
			if (mb_io_ptr->new_lon < mb_io_ptr->bounds[0] || mb_io_ptr->new_lon > mb_io_ptr->bounds[1] ||
			    mb_io_ptr->new_lat < mb_io_ptr->bounds[2] || mb_io_ptr->new_lat > mb_io_ptr->bounds[3]) {
				status = MB_FAILURE;
				*error = MB_ERROR_OUT_BOUNDS;
				mb_notice_log_error(verbose, mbio_ptr, *error);
			}
			else if (mb_io_ptr->etime_d > mb_io_ptr->btime_d && mb_io_ptr->new_time_d > MB_TIME_D_UNKNOWN &&
			         (mb_io_ptr->new_time_d > mb_io_ptr->etime_d || mb_io_ptr->new_time_d < mb_io_ptr->btime_d)) {
				status = MB_FAILURE;
				*error = MB_ERROR_OUT_TIME;
				mb_notice_log_error(verbose, mbio_ptr, *error);
			}
			else if (mb_io_ptr->etime_d < mb_io_ptr->btime_d && mb_io_ptr->new_time_d > MB_TIME_D_UNKNOWN &&
			         (mb_io_ptr->new_time_d > mb_io_ptr->etime_d && mb_io_ptr->new_time_d < mb_io_ptr->btime_d)) {
				status = MB_FAILURE;
				*error = MB_ERROR_OUT_TIME;
				mb_notice_log_error(verbose, mbio_ptr, *error);
			}
		}

		/* check for time gap */
		if (status == MB_SUCCESS && mb_io_ptr->new_time_d > MB_TIME_D_UNKNOWN && mb_io_ptr->new_kind == MB_DATA_DATA &&
		    mb_io_ptr->ping_count > 1) {
			if ((mb_io_ptr->new_time_d - mb_io_ptr->last_time_d) > 60 * mb_io_ptr->timegap) {
				status = MB_FAILURE;
				*error = MB_ERROR_TIME_GAP;
				mb_notice_log_error(verbose, mbio_ptr, *error);
			}
		}

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  New ping checked by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New ping values:\n");
			fprintf(stderr, "dbg4       ping_count:    %d\n", mb_io_ptr->ping_count);
			fprintf(stderr, "dbg4       comment_count: %d\n", mb_io_ptr->comment_count);
			fprintf(stderr, "dbg4       pings_avg:     %d\n", mb_io_ptr->pings_avg);
			fprintf(stderr, "dbg4       pings_read:    %d\n", mb_io_ptr->pings_read);
			fprintf(stderr, "dbg4       error:         %d\n", mb_io_ptr->new_error);
			fprintf(stderr, "dbg4       status:        %d\n", status);
		}
		if (verbose >= 4 && mb_io_ptr->new_kind == MB_DATA_COMMENT) {
			fprintf(stderr, "dbg4       comment:     \ndbg4       %s\n", mb_io_ptr->new_comment);
		}
		else if (verbose >= 4 && mb_io_ptr->new_kind == MB_DATA_DATA && *error <= MB_ERROR_NO_ERROR &&
		         *error > MB_ERROR_COMMENT) {
			fprintf(stderr, "dbg4       time_i[0]:     %d\n", mb_io_ptr->new_time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:     %d\n", mb_io_ptr->new_time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:     %d\n", mb_io_ptr->new_time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:     %d\n", mb_io_ptr->new_time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:     %d\n", mb_io_ptr->new_time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:     %d\n", mb_io_ptr->new_time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:     %d\n", mb_io_ptr->new_time_i[6]);
			fprintf(stderr, "dbg4       time_d:        %f\n", mb_io_ptr->new_time_d);
			fprintf(stderr, "dbg4       longitude:     %f\n", mb_io_ptr->new_lon);
			fprintf(stderr, "dbg4       latitude:      %f\n", mb_io_ptr->new_lat);
			fprintf(stderr, "dbg4       speed:         %f\n", mb_io_ptr->new_speed);
			fprintf(stderr, "dbg4       heading:       %f\n", mb_io_ptr->new_heading);
			fprintf(stderr, "dbg4       beams_bath:    %d\n", mb_io_ptr->new_beams_bath);
			if (mb_io_ptr->new_beams_bath > 0) {
				fprintf(stderr, "dbg4       beam   bath  crosstrack alongtrack\n");
				for (int i = 0; i < mb_io_ptr->new_beams_bath; i++)
					fprintf(stderr, "dbg4       %4d   %3d    %f    %f     %f\n", i, mb_io_ptr->new_beamflag[i],
					        mb_io_ptr->new_bath[i], mb_io_ptr->new_bath_acrosstrack[i], mb_io_ptr->new_bath_alongtrack[i]);
			}
			fprintf(stderr, "dbg4       beams_amp:     %d\n", mb_io_ptr->new_beams_amp);
			if (mb_io_ptr->new_beams_amp > 0) {
				fprintf(stderr, "dbg4       beam    amp  crosstrack alongtrack\n");
				for (int i = 0; i < mb_io_ptr->new_beams_amp; i++)
					fprintf(stderr, "dbg4       %4d   %f    %f     %f\n", i, mb_io_ptr->new_amp[i],
					        mb_io_ptr->new_bath_acrosstrack[i], mb_io_ptr->new_bath_alongtrack[i]);
			}
			fprintf(stderr, "dbg4       pixels_ss:     %d\n", mb_io_ptr->new_pixels_ss);
			if (mb_io_ptr->new_pixels_ss > 0) {
				fprintf(stderr, "dbg4       pixel sidescan crosstrack alongtrack\n");
				for (int i = 0; i < mb_io_ptr->new_pixels_ss; i++)
					fprintf(stderr, "dbg4       %4d   %f    %f     %f\n", i, mb_io_ptr->new_ss[i],
					        mb_io_ptr->new_ss_acrosstrack[i], mb_io_ptr->new_ss_alongtrack[i]);
			}
		}

		/* now bin the data if appropriate */
		if (mb_io_ptr->new_kind == MB_DATA_DATA &&

		    /* if data is ok */
		    (status == MB_SUCCESS

		     /* or if nonfatal error and only one ping read,
		         bin the ping */
		     || (*error<MB_ERROR_NO_ERROR && * error> MB_ERROR_COMMENT && mb_io_ptr->pings_read == 1)))

		{
			/* bin the values */
			mb_io_ptr->pings_binned++;
			mb_io_ptr->time_d = mb_io_ptr->time_d + mb_io_ptr->new_time_d;
			mb_io_ptr->lon = mb_io_ptr->lon + mb_io_ptr->new_lon;
			mb_io_ptr->lat = mb_io_ptr->lat + mb_io_ptr->new_lat;
			mb_io_ptr->speed = mb_io_ptr->speed + mb_io_ptr->new_speed;
			mb_io_ptr->heading = mb_io_ptr->heading + mb_io_ptr->new_heading;
			headingx = headingx + sin(DTR * mb_io_ptr->new_heading);
			headingy = headingy + cos(DTR * mb_io_ptr->new_heading);
			if (mb_io_ptr->pings == 1) {
				for (int i = 0; i < mb_io_ptr->new_beams_bath; i++) {
					mb_io_ptr->beamflag[i] = mb_io_ptr->new_beamflag[i];
					mb_io_ptr->bath[i] = mb_io_ptr->new_bath[i];
					mb_io_ptr->bath_acrosstrack[i] = mb_io_ptr->new_bath_acrosstrack[i];
					mb_io_ptr->bath_alongtrack[i] = mb_io_ptr->new_bath_alongtrack[i];
					mb_io_ptr->bath_num[i] = 1;
				}
				for (int i = 0; i < mb_io_ptr->new_beams_amp; i++) {
					mb_io_ptr->amp[i] = mb_io_ptr->new_amp[i];
					mb_io_ptr->amp_num[i] = 1;
				}
				for (int i = 0; i < mb_io_ptr->new_pixels_ss; i++) {
					mb_io_ptr->ss[i] = mb_io_ptr->new_ss[i];
					mb_io_ptr->ss_acrosstrack[i] = mb_io_ptr->new_ss_acrosstrack[i];
					mb_io_ptr->ss_alongtrack[i] = mb_io_ptr->new_ss_alongtrack[i];
					mb_io_ptr->ss_num[i] = 1;
				}
			}
			else {
				for (int i = 0; i < mb_io_ptr->new_beams_bath; i++) {
					if (!mb_beam_check_flag(mb_io_ptr->new_beamflag[i])) {
						mb_io_ptr->beamflag[i] = MB_FLAG_NONE;
						mb_io_ptr->bath[i] = mb_io_ptr->bath[i] + mb_io_ptr->new_bath[i];
						mb_io_ptr->bath_acrosstrack[i] = mb_io_ptr->bath_acrosstrack[i] + mb_io_ptr->new_bath_acrosstrack[i];
						mb_io_ptr->bath_alongtrack[i] = mb_io_ptr->bath_alongtrack[i] + mb_io_ptr->new_bath_alongtrack[i];
						mb_io_ptr->bath_num[i]++;
					}
				}
				for (int i = 0; i < mb_io_ptr->new_beams_amp; i++) {
					if (!mb_beam_check_flag(mb_io_ptr->new_beamflag[i])) {
						mb_io_ptr->amp[i] = mb_io_ptr->amp[i] + mb_io_ptr->new_amp[i];
						mb_io_ptr->amp_num[i]++;
					}
				}
				for (int i = 0; i < mb_io_ptr->new_pixels_ss; i++) {
					if (mb_io_ptr->new_ss[i] != MB_SIDESCAN_NULL) {
						mb_io_ptr->ss[i] = mb_io_ptr->ss[i] + mb_io_ptr->new_ss[i];
						mb_io_ptr->ss_acrosstrack[i] = mb_io_ptr->ss_acrosstrack[i] + mb_io_ptr->new_ss_acrosstrack[i];
						mb_io_ptr->ss_alongtrack[i] = mb_io_ptr->ss_alongtrack[i] + mb_io_ptr->new_ss_alongtrack[i];
						mb_io_ptr->ss_num[i]++;
					}
				}
			}
		}

		if (verbose >= 4 && mb_io_ptr->new_kind == MB_DATA_DATA &&
		    (status == MB_SUCCESS || (*error<MB_ERROR_NO_ERROR && * error> MB_ERROR_COMMENT && mb_io_ptr->pings_read == 1))) {
			fprintf(stderr, "\ndbg4  New ping binned by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Current binned ping values:\n");
			fprintf(stderr, "dbg4       pings_binned: %d\n", mb_io_ptr->pings_binned);
			fprintf(stderr, "dbg4       time_d:       %f\n", mb_io_ptr->time_d);
			fprintf(stderr, "dbg4       longitude:    %f\n", mb_io_ptr->lon);
			fprintf(stderr, "dbg4       latitude:     %f\n", mb_io_ptr->lat);
			fprintf(stderr, "dbg4       speed:        %f\n", mb_io_ptr->speed);
			fprintf(stderr, "dbg4       heading:      %f\n", mb_io_ptr->heading);
			fprintf(stderr, "dbg4       beams_bath:    %d\n", mb_io_ptr->beams_bath_max);
			if (mb_io_ptr->beams_bath_max > 0) {
				fprintf(stderr, "dbg4       beam   nbath bath  crosstrack alongtrack\n");
				for (int i = 0; i < mb_io_ptr->beams_bath_max; i++)
					fprintf(stderr, "dbg4       %4d   %4d  %f    %f     %f\n", i, mb_io_ptr->bath_num[i], mb_io_ptr->bath[i],
					        mb_io_ptr->bath_acrosstrack[i], mb_io_ptr->bath_alongtrack[i]);
			}
			fprintf(stderr, "dbg4       beams_amp:    %d\n", mb_io_ptr->beams_amp_max);
			if (mb_io_ptr->beams_amp_max > 0) {
				fprintf(stderr, "dbg4       beam    namp  amp  crosstrack alongtrack\n");
				for (int i = 0; i < mb_io_ptr->beams_amp_max; i++)
					fprintf(stderr, "dbg4       %4d   %4d  %f    %f     %f\n", i, mb_io_ptr->amp_num[i], mb_io_ptr->amp[i],
					        mb_io_ptr->bath_acrosstrack[i], mb_io_ptr->bath_alongtrack[i]);
			}
			fprintf(stderr, "dbg4       pixels_ss:     %d\n", mb_io_ptr->pixels_ss_max);
			if (mb_io_ptr->pixels_ss_max > 0) {
				fprintf(stderr, "dbg4       pixel nss  sidescan crosstrack alongtrack\n");
				for (int i = 0; i < mb_io_ptr->pixels_ss_max; i++)
					fprintf(stderr, "dbg4       %4d   %4d   %f    %f     %f\n", i, mb_io_ptr->ss_num[i], mb_io_ptr->ss[i],
					        mb_io_ptr->ss_acrosstrack[i], mb_io_ptr->ss_alongtrack[i]);
			}
		}

		/* if data is ok but more pings needed keep reading */
		if (status == MB_SUCCESS && mb_io_ptr->new_kind == MB_DATA_DATA && mb_io_ptr->pings_binned < mb_io_ptr->pings_avg) {
			done = false;
			mb_io_ptr->need_new_ping = true;
			reset_last = true;
		}

		/* if data is ok and enough pings binned then done */
		else if (status == MB_SUCCESS && mb_io_ptr->new_kind == MB_DATA_DATA && mb_io_ptr->pings_binned >= mb_io_ptr->pings_avg) {
			done = true;
			mb_io_ptr->need_new_ping = true;
			reset_last = true;
		}

		/* if data gap and only one ping read and more
		    pings needed set error save flag and keep reading */
		else if (*error == MB_ERROR_TIME_GAP && mb_io_ptr->new_kind == MB_DATA_DATA && mb_io_ptr->pings_read == 1 &&
		         mb_io_ptr->pings_avg > 1) {
			done = false;
			mb_io_ptr->need_new_ping = true;
			mb_io_ptr->error_save = *error;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			reset_last = true;
		}

		/* if other kind of data and need more pings
		    then keep reading */
		else if ((*error == MB_ERROR_OTHER || *error == MB_ERROR_UNINTELLIGIBLE) &&
		         mb_io_ptr->pings_binned < mb_io_ptr->pings_avg) {
			done = false;
			mb_io_ptr->need_new_ping = true;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			reset_last = false;
		}

		/* if error and only one ping read then done */
		else if (*error != MB_ERROR_NO_ERROR && mb_io_ptr->pings_read <= 1) {
			done = true;
			mb_io_ptr->need_new_ping = true;
			if (*error == MB_ERROR_TIME_GAP || *error == MB_ERROR_OUT_BOUNDS)
				reset_last = true;
			else
				reset_last = false;
		}

		/* if error and more than one ping read,
		    then done but save the ping */
		else if (*error != MB_ERROR_NO_ERROR) {
			done = true;
			mb_io_ptr->need_new_ping = false;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			reset_last = false;
		}

		/* if needed reset "last" pings */
		if (reset_last) {
			mb_io_ptr->last_time_d = mb_io_ptr->new_time_d;
			mb_io_ptr->last_lon = mb_io_ptr->new_lon;
			mb_io_ptr->last_lat = mb_io_ptr->new_lat;
		}

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  End of reading loop in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Current status values:\n");
			fprintf(stderr, "dbg4       done:          %d\n", done);
			fprintf(stderr, "dbg4       need_new_ping: %d\n", mb_io_ptr->need_new_ping);
			fprintf(stderr, "dbg4       pings_binned:  %d\n", mb_io_ptr->pings_binned);
			fprintf(stderr, "dbg4       error:         %d\n", *error);
			fprintf(stderr, "dbg4       status:        %d\n", status);
		}
	}

	/* set output number of pings */
	*pings = mb_io_ptr->pings_binned;

	/* set data kind */
	if (mb_io_ptr->pings_binned > 0)
		*kind = MB_DATA_DATA;
	else if (*error == MB_ERROR_COMMENT)
		*kind = MB_DATA_COMMENT;
	else
		*kind = mb_io_ptr->new_kind;

	/* get output time */
	if (*error <= MB_ERROR_NO_ERROR && *error > MB_ERROR_COMMENT) {
		if (mb_io_ptr->pings_binned == 1) {
			for (int i = 0; i < 7; i++)
				time_i[i] = mb_io_ptr->new_time_i[i];
			*time_d = mb_io_ptr->new_time_d;
		}
		else if (mb_io_ptr->pings_binned > 1) {
			*time_d = mb_io_ptr->time_d / mb_io_ptr->pings_binned;
			mb_get_date(verbose, *time_d, time_i);
		}
		else {
			*error = MB_ERROR_NO_PINGS_BINNED;
			mb_notice_log_error(verbose, mbio_ptr, *error);
		}
	}

	/* get other output values */
	if (*error <= MB_ERROR_NO_ERROR && *error > MB_ERROR_COMMENT) {
		/* get navigation values */
		*navlon = mb_io_ptr->lon / mb_io_ptr->pings_binned;
		*navlat = mb_io_ptr->lat / mb_io_ptr->pings_binned;
		headingx = headingx / mb_io_ptr->pings_binned;
		headingy = headingy / mb_io_ptr->pings_binned;
		const double denom = sqrt(headingx * headingx + headingy * headingy);
		if (denom > 0.0) {
			headingx = headingx / denom;
			headingy = headingy / denom;
			*heading = RTD * atan2(headingx, headingy);
		}
		else {
			*heading = mb_io_ptr->heading / mb_io_ptr->pings_binned;
			headingx = sin(*heading * DTR);
			headingy = cos(*heading * DTR);
		}
		if (*heading < 0.0)
			*heading = *heading + 360.0;

		/* get coordinate scaling */
		mb_coor_scale(verbose, *navlat, &mtodeglon, &mtodeglat);

		/* get distance value */
		if (mb_io_ptr->old_time_d > 0.0) {
			const double dx = (*navlon - mb_io_ptr->old_lon) / mtodeglon;
			const double dy = (*navlat - mb_io_ptr->old_lat) / mtodeglat;
			*distance = 0.001 * sqrt(dx * dx + dy * dy); /* km */
		}
		else
			*distance = 0.0;

		double delta_time = 0.0;
		/* get speed value */
		if (mb_io_ptr->speed > 0.0)
			*speed = mb_io_ptr->speed / mb_io_ptr->pings_binned;
		else if (mb_io_ptr->old_time_d > 0.0) {
			delta_time = 0.000277778 * (*time_d - mb_io_ptr->old_time_d); /* hours */
			if (delta_time > 0.0)
				*speed = *distance / delta_time; /* km/hr */
			else
				*speed = 0.0;
		}
		else
			*speed = 0.0;

		/* check for less than minimum speed */
		if (*error == MB_ERROR_NO_ERROR || *error == MB_ERROR_TIME_GAP) {
			if (mb_io_ptr->ping_count > 1 && *time_d > MB_TIME_D_UNKNOWN && *speed < mb_io_ptr->speedmin) {
				status = MB_FAILURE;
				*error = MB_ERROR_SPEED_TOO_SMALL;
				mb_notice_log_error(verbose, mbio_ptr, *error);
			}
		}

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Distance and Speed Calculated in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Speed and Distance Related Values:\n");
			fprintf(stderr, "dbg4       binned speed: %f\n", mb_io_ptr->speed);
			fprintf(stderr, "dbg4       pings_binned: %d\n", mb_io_ptr->pings_binned);
			fprintf(stderr, "dbg4       ping_count:   %d\n", mb_io_ptr->ping_count);
			fprintf(stderr, "dbg4       time:         %f\n", *time_d);
			fprintf(stderr, "dbg4       lon:          %f\n", *navlon);
			fprintf(stderr, "dbg4       lat:          %f\n", *navlat);
			fprintf(stderr, "dbg4       old time:     %f\n", mb_io_ptr->old_time_d);
			fprintf(stderr, "dbg4       old lon:      %f\n", mb_io_ptr->old_lon);
			fprintf(stderr, "dbg4       old lat:      %f\n", mb_io_ptr->old_lat);
			fprintf(stderr, "dbg4       distance:     %f\n", *distance);
			fprintf(stderr, "dbg4       altitude:     %f\n", *altitude);
			fprintf(stderr, "dbg4       sensordepth:  %f\n", *sensordepth);
			fprintf(stderr, "dbg4       delta_time:   %f\n", delta_time);
			fprintf(stderr, "dbg4       speed:        %f\n", *speed);
			fprintf(stderr, "dbg4       error:        %d\n", *error);
			fprintf(stderr, "dbg4       status:       %d\n", status);
		}

		/* get swath data */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			beamflag[i] = mb_io_ptr->beamflag[i];
			if (mb_io_ptr->bath_num[i] > 0) {
				bath[i] = (mb_io_ptr->bath[i]) / (mb_io_ptr->bath_num[i]);
				mb_io_ptr->bath_acrosstrack[i] = (mb_io_ptr->bath_acrosstrack[i]) / (mb_io_ptr->bath_num[i]);
				mb_io_ptr->bath_alongtrack[i] = (mb_io_ptr->bath_alongtrack[i]) / (mb_io_ptr->bath_num[i]);
				bathlon[i] = *navlon + headingy * mtodeglon * mb_io_ptr->bath_acrosstrack[i] +
				             headingx * mtodeglon * mb_io_ptr->bath_alongtrack[i];
				bathlat[i] = *navlat - headingx * mtodeglat * mb_io_ptr->bath_acrosstrack[i] +
				             headingy * mtodeglat * mb_io_ptr->bath_alongtrack[i];
				*nbath = i + 1;
			}
			else {
				beamflag[i] = MB_FLAG_NULL;
				bath[i] = 0.0;
				bathlon[i] = 0.0;
				bathlat[i] = 0.0;
			}
		}
		for (int i = 0; i < mb_io_ptr->beams_amp_max; i++) {
			if (mb_io_ptr->amp_num[i] > 0) {
				amp[i] = (mb_io_ptr->amp[i]) / (mb_io_ptr->amp_num[i]);
				*namp = i + 1;
			}
			else {
				amp[i] = 0.0;
			}
		}
		for (int i = 0; i < mb_io_ptr->pixels_ss_max; i++) {
			if (mb_io_ptr->ss_num[i] > 0) {
				ss[i] = (mb_io_ptr->ss[i]) / (mb_io_ptr->ss_num[i]);
				mb_io_ptr->ss_acrosstrack[i] = (mb_io_ptr->ss_acrosstrack[i]) / (mb_io_ptr->ss_num[i]);
				mb_io_ptr->ss_alongtrack[i] = (mb_io_ptr->ss_alongtrack[i]) / (mb_io_ptr->ss_num[i]);
				sslon[i] = *navlon + headingy * mtodeglon * mb_io_ptr->ss_acrosstrack[i] +
				           headingx * mtodeglon * mb_io_ptr->ss_alongtrack[i];
				sslat[i] = *navlat - headingx * mtodeglat * mb_io_ptr->ss_acrosstrack[i] +
				           headingy * mtodeglat * mb_io_ptr->ss_alongtrack[i];
				*nss = i + 1;
			}
			else {
				ss[i] = MB_SIDESCAN_NULL;
				sslon[i] = 0.0;
				sslat[i] = 0.0;
			}
		}
		if (!mb_io_ptr->variable_beams) {
			*nbath = mb_io_ptr->beams_bath_max;
			*namp = mb_io_ptr->beams_amp_max;
			*nss = mb_io_ptr->pixels_ss_max;
		}
	}

	/* get output comment */
	if (*error == MB_ERROR_COMMENT) {
		strcpy(comment, mb_io_ptr->new_comment);
	}

	/* reset "old" navigation values */
	if (*error <= MB_ERROR_NO_ERROR && *error > MB_ERROR_COMMENT) {
		mb_io_ptr->old_time_d = *time_d;
		mb_io_ptr->old_lon = *navlon;
		mb_io_ptr->old_lat = *navlat;
	}

	/* get saved error flag if needed */
	if (*error == MB_ERROR_NO_ERROR && mb_io_ptr->error_save != MB_ERROR_NO_ERROR) {
		*error = mb_io_ptr->error_save;
		status = MB_FAILURE;
		mb_io_ptr->error_save = MB_ERROR_NO_ERROR;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       pings:      %d\n", *pings);
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       navlon:     %f\n", *navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", *navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", *speed);
		fprintf(stderr, "dbg2       heading:    %f\n", *heading);
		fprintf(stderr, "dbg2       distance:   %f\n", *distance);
		fprintf(stderr, "dbg2       altitude:   %f\n", *altitude);
		fprintf(stderr, "dbg2       sensordepth:%f\n", *sensordepth);
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		if (verbose >= 3 && *nbath > 0) {
			fprintf(stderr, "dbg3       beam   nbath flag bath  crosstrack alongtrack\n");
			for (int i = 0; i < *nbath; i++)
				fprintf(stderr, "dbg3       %4d   %4d  %3d  %f    %f     %f\n", i, mb_io_ptr->bath_num[i], beamflag[i], bath[i],
				        bathlon[i], bathlat[i]);
		}
		fprintf(stderr, "dbg2       namp:       %d\n", *namp);
		if (verbose >= 3 && *namp > 0) {
			fprintf(stderr, "dbg3       beam    namp  amp  lon lat\n");
			for (int i = 0; i < *namp; i++)
				fprintf(stderr, "dbg3       %4d   %4d  %f    %f     %f\n", i, mb_io_ptr->amp_num[i], amp[i], bathlon[i],
				        bathlat[i]);
		}
		fprintf(stderr, "dbg2       nss:      %d\n", *nss);
		if (verbose >= 3 && *nss > 0) {
			fprintf(stderr, "dbg3       pixel nss  sidescan crosstrack alongtrack\n");
			for (int i = 0; i < *nss; i++)
				fprintf(stderr, "dbg3       %4d   %4d   %f    %f     %f\n", i, mb_io_ptr->ss_num[i], ss[i], sslon[i], sslat[i]);
		}
	}
	else if (verbose >= 2 && *kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:    %s\n", comment);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
