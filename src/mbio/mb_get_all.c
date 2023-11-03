/*--------------------------------------------------------------------
 *    The MB-system:	mb_get_all.c	1/26/93
 *
 *    Copyright (c) 1993-2023 by
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
 * mb_get_all.c reads swath data from a file
 * which has been initialized by mb_read_init(). Crosstrack distances
 * are not mapped into lon and lat.  The data is not averaged, and
 * values are also read into a storage data structure including
 * all possible values output by the particular multibeam system
 * associated with the specified format.
 *
 * Author:	D. W. Caress
 * Date:	January 26, 1993
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
int mb_get_all(int verbose, void *mbio_ptr, void **store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
               double *navlat, double *speed, double *heading, double *distance, double *altitude, double *sensordepth, int *nbath,
               int *namp, int *nss, char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
               double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
	}

	/* get mbio and data structure descriptors */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	*store_ptr = mb_io_ptr->store_data;

	/* reset status */
	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 4) {
		fprintf(stderr, "\ndbg2  About to read ping in function <%s>\n", __func__);
		fprintf(stderr, "dbg2       ping_count:    %d\n", mb_io_ptr->ping_count);
		fprintf(stderr, "dbg2       error:         %d\n", *error);
	}

	int status = mb_read_ping(verbose, mbio_ptr, *store_ptr, kind, error);

	/* if io arrays have been reallocated, update the
	    pointers of arrays passed into this function,
	    as these pointers may have changed */
	if (status == MB_SUCCESS && mb_io_ptr->new_kind == MB_DATA_DATA) {
		if (mb_io_ptr->bath_arrays_reallocated) {
			status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&beamflag, error);
			status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&bath, error);
			status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&bathacrosstrack, error);
			status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&bathalongtrack, error);
			mb_io_ptr->bath_arrays_reallocated = false;
		}
		if (mb_io_ptr->amp_arrays_reallocated) {
			status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&amp, error);
			mb_io_ptr->amp_arrays_reallocated = false;
		}
		if (mb_io_ptr->ss_arrays_reallocated) {
			status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&ss, error);
			status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&ssacrosstrack, error);
			status &= mb_update_arrayptr(verbose, mbio_ptr, (void **)&ssalongtrack, error);
			mb_io_ptr->ss_arrays_reallocated = false;
		}
	}

	/* if survey data read into storage array */
	if (status == MB_SUCCESS &&
	    (*kind == MB_DATA_DATA || *kind == MB_DATA_SUBBOTTOM_MCS || *kind == MB_DATA_SUBBOTTOM_CNTRBEAM ||
	     *kind == MB_DATA_SUBBOTTOM_SUBBOTTOM || *kind == MB_DATA_SIDESCAN2 || *kind == MB_DATA_SIDESCAN3 ||
	     *kind == MB_DATA_WATER_COLUMN || *kind == MB_DATA_NAV || *kind == MB_DATA_NAV1 || *kind == MB_DATA_NAV2 ||
	     *kind == MB_DATA_NAV3 || *kind == MB_DATA_COMMENT)) {
		/* initialize return values */
		*kind = MB_DATA_NONE;
		for (int i = 0; i < 7; i++)
			time_i[i] = 0;
		*time_d = 0.0;
		*navlon = 0.0;
		*navlat = 0.0;
		*speed = 0.0;
		*heading = 0.0;
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			beamflag[i] = mb_beam_set_flag_null(beamflag[i]);
			bath[i] = 0.0;
			bathacrosstrack[i] = 0.0;
			bathalongtrack[i] = 0.0;
		}
		for (int i = 0; i < mb_io_ptr->beams_amp_max; i++) {
			amp[i] = 0.0;
		}
		for (int i = 0; i < mb_io_ptr->pixels_ss_max; i++) {
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
		}
		strcpy(comment, "");

		/* get the data */
		status =
		    mb_extract(verbose, mbio_ptr, *store_ptr, kind, time_i, time_d, navlon, navlat, speed, heading, nbath, namp, nss,
		               beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, error);
		if (status == MB_SUCCESS && (*kind == MB_DATA_DATA || *kind == MB_DATA_CALIBRATE || *kind == MB_DATA_SUBBOTTOM_MCS ||
		                             *kind == MB_DATA_SUBBOTTOM_CNTRBEAM || *kind == MB_DATA_SUBBOTTOM_SUBBOTTOM ||
		                             *kind == MB_DATA_SIDESCAN2 || *kind == MB_DATA_SIDESCAN3 || *kind == MB_DATA_WATER_COLUMN)) {
			status = mb_extract_altitude(verbose, mbio_ptr, *store_ptr, kind, sensordepth, altitude, error);
		}
		if (status == MB_SUCCESS &&
		    (*kind == MB_DATA_NAV || *kind == MB_DATA_NAV1 || *kind == MB_DATA_NAV2 || *kind == MB_DATA_NAV3)) {
			double roll;
			double pitch;
			double heave;
			status = mb_extract_nav(verbose, mbio_ptr, *store_ptr, kind, time_i, time_d, navlon, navlat, speed, heading,
			                        sensordepth, &roll, &pitch, &heave, error);
		}
	}

	/* if alternative nav is available use it for survey records */
	if (status == MB_SUCCESS && *kind == MB_DATA_DATA && mb_io_ptr->alternative_navigation) {
		double zoffset = 0.0;
		double tsensordepth = 0.0;
		int inavadjtime = 0;
		mb_linear_interp_longitude(	verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_navlon - 1,      mb_io_ptr->nav_alt_num, *time_d, navlon,      &inavadjtime, error);
    mb_linear_interp_latitude(	verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_navlat - 1,      mb_io_ptr->nav_alt_num, *time_d, navlat,      &inavadjtime, error);
    mb_linear_interp(						verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_speed - 1,       mb_io_ptr->nav_alt_num, *time_d, speed,       &inavadjtime, error);
    mb_linear_interp_heading(		verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_heading - 1,     mb_io_ptr->nav_alt_num, *time_d, heading,     &inavadjtime, error);
  	mb_linear_interp(           verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_sensordepth - 1, mb_io_ptr->nav_alt_num, *time_d, &tsensordepth, &inavadjtime, error);
    mb_linear_interp(           verbose, mb_io_ptr->nav_alt_time_d - 1, mb_io_ptr->nav_alt_zoffset - 1,     mb_io_ptr->nav_alt_num, *time_d, &zoffset,     &inavadjtime, error);
    if (*heading < 0.0)
      *heading += 360.0;
    else if (*heading > 360.0)
      *heading -= 360.0;
    double bath_correction = tsensordepth - *sensordepth + zoffset;
    *sensordepth = tsensordepth + zoffset;
  	for (int ibeam=0; ibeam<*nbath; ibeam++) {
  		bath[ibeam] += bath_correction;
  	}
	}

	if (verbose >= 4) {
		fprintf(stderr, "\ndbg2  New ping read in function <%s>\n", __func__);
		fprintf(stderr, "dbg2       status:        %d\n", status);
		fprintf(stderr, "dbg2       error:         %d\n", *error);
		fprintf(stderr, "dbg2       kind:          %d\n", mb_io_ptr->new_kind);
	}

	/* increment counters */
	if (status == MB_SUCCESS) {
		if (*kind == MB_DATA_DATA)
			mb_io_ptr->ping_count++;
		else if (*kind == MB_DATA_NAV)
			mb_io_ptr->nav_count++;
		else if (*kind == MB_DATA_COMMENT)
			mb_io_ptr->comment_count++;
	}

	/* if first ping read set "old" navigation values */
	if (status == MB_SUCCESS && (*kind == MB_DATA_DATA || *kind == MB_DATA_NAV || *kind == MB_DATA_CALIBRATE) &&
	    mb_io_ptr->ping_count == 1) {
		mb_io_ptr->old_time_d = *time_d;
		mb_io_ptr->old_lon = *navlon;
		mb_io_ptr->old_lat = *navlat;
	}

	/* if first nav read set "old" navigation values */
	if (status == MB_SUCCESS && (*kind == MB_DATA_NAV) && mb_io_ptr->nav_count == 1) {
		mb_io_ptr->old_ntime_d = *time_d;
		mb_io_ptr->old_nlon = *navlon;
		mb_io_ptr->old_nlat = *navlat;
	}

	/* calculate speed and distance for ping data */
	if (status == MB_SUCCESS && (*kind == MB_DATA_DATA || *kind == MB_DATA_CALIBRATE || *kind == MB_DATA_SUBBOTTOM_MCS ||
	                             *kind == MB_DATA_SUBBOTTOM_CNTRBEAM || *kind == MB_DATA_SUBBOTTOM_SUBBOTTOM ||
	                             *kind == MB_DATA_SIDESCAN2 || *kind == MB_DATA_SIDESCAN3 || *kind == MB_DATA_WATER_COLUMN)) {
		/* get coordinate scaling */
		double mtodeglon;
		double mtodeglat;
		mb_coor_scale(verbose, *navlat, &mtodeglon, &mtodeglat);

		/* get distance value */
		if (mb_io_ptr->old_time_d > 0.0) {
			const double dx = (*navlon - mb_io_ptr->old_lon) / mtodeglon;
			const double dy = (*navlat - mb_io_ptr->old_lat) / mtodeglat;
			*distance = 0.001 * sqrt(dx * dx + dy * dy); /* km */
		}
		else
			*distance = 0.0;

		/* get speed value */
		double delta_time;
		if (*speed <= 0.0 && mb_io_ptr->old_time_d > 0.0) {
			delta_time = 0.000277778 * (*time_d - mb_io_ptr->old_time_d); /* hours */
			if (delta_time > 0.0)
				*speed = *distance / delta_time; /* km/hr */
			else
				*speed = 0.0;
		}
		else if (*speed < 0.0)
			*speed = 0.0;

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Distance and Speed Calculated in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Speed and Distance Related Values:\n");
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
			fprintf(stderr, "dbg4       raw speed:    %f\n", mb_io_ptr->new_speed);
			fprintf(stderr, "dbg4       speed:        %f\n", *speed);
			fprintf(stderr, "dbg4       error:        %d\n", *error);
			fprintf(stderr, "dbg4       status:       %d\n", status);
		}
	}

	/* calculate speed and distance for nav data */
	else if (status == MB_SUCCESS && (*kind == MB_DATA_NAV || *kind == MB_DATA_NAV || *kind == MB_DATA_NAV1 ||
	                                  *kind == MB_DATA_NAV2 || *kind == MB_DATA_NAV3)) {
		/* get coordinate scaling */
		double mtodeglon;
		double mtodeglat;
		mb_coor_scale(verbose, *navlat, &mtodeglon, &mtodeglat);

		/* get distance value */
		if (mb_io_ptr->old_ntime_d > 0.0) {
			const double dx = (*navlon - mb_io_ptr->old_nlon) / mtodeglon;
			const double dy = (*navlat - mb_io_ptr->old_nlat) / mtodeglat;
			*distance = 0.001 * sqrt(dx * dx + dy * dy); /* km */
		}
		else
			*distance = 0.0;

		/* get speed value */
		double delta_time;
		if (*speed <= 0.0 && mb_io_ptr->old_ntime_d > 0.0) {
			delta_time = 0.000277778 * (*time_d - mb_io_ptr->old_ntime_d); /* hours */
			if (delta_time > 0.0)
				*speed = *distance / delta_time; /* km/hr */
			else
				*speed = 0.0;
		}
		else if (*speed < 0.0)
			*speed = 0.0;

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Distance and Speed Calculated in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Speed and Distance Related Values:\n");
			fprintf(stderr, "dbg4       ping_count:   %d\n", mb_io_ptr->ping_count);
			fprintf(stderr, "dbg4       time:         %f\n", *time_d);
			fprintf(stderr, "dbg4       lon:          %f\n", *navlon);
			fprintf(stderr, "dbg4       lat:          %f\n", *navlat);
			fprintf(stderr, "dbg4       old time:     %f\n", mb_io_ptr->old_ntime_d);
			fprintf(stderr, "dbg4       old lon:      %f\n", mb_io_ptr->old_nlon);
			fprintf(stderr, "dbg4       old lat:      %f\n", mb_io_ptr->old_lat);
			fprintf(stderr, "dbg4       distance:     %f\n", *distance);
			fprintf(stderr, "dbg4       altitude:     %f\n", *altitude);
			fprintf(stderr, "dbg4       sensordepth:  %f\n", *sensordepth);
			fprintf(stderr, "dbg4       delta_time:   %f\n", delta_time);
			fprintf(stderr, "dbg4       raw speed:    %f\n", mb_io_ptr->new_speed);
			fprintf(stderr, "dbg4       speed:        %f\n", *speed);
			fprintf(stderr, "dbg4       error:        %d\n", *error);
			fprintf(stderr, "dbg4       status:       %d\n", status);
		}
	}

	/* else set nav values to zero */
	else {
		*navlon = 0.0;
		*navlat = 0.0;
		*distance = 0.0;
		*altitude = 0.0;
		*sensordepth = 0.0;
		*speed = 0.0;
	}

	/* check for out of location or time bounds */
	if (status == MB_SUCCESS && (*kind == MB_DATA_DATA || *kind == MB_DATA_NAV || *kind == MB_DATA_CALIBRATE)) {
		if (*navlon < mb_io_ptr->bounds[0] || *navlon > mb_io_ptr->bounds[1] || *navlat < mb_io_ptr->bounds[2] ||
		    *navlat > mb_io_ptr->bounds[3]) {
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_BOUNDS;
		}
		else if (mb_io_ptr->etime_d > mb_io_ptr->btime_d && *time_d > MB_TIME_D_UNKNOWN &&
		         (*time_d > mb_io_ptr->etime_d || *time_d < mb_io_ptr->btime_d)) {
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_TIME;
		}
		else if (mb_io_ptr->etime_d<mb_io_ptr->btime_d && * time_d> MB_TIME_D_UNKNOWN &&
		         (*time_d > mb_io_ptr->etime_d && *time_d < mb_io_ptr->btime_d)) {
			status = MB_FAILURE;
			*error = MB_ERROR_OUT_TIME;
		}
	}

	/* check for time gap */
	if (status == MB_SUCCESS && mb_io_ptr->new_time_d > MB_TIME_D_UNKNOWN &&
	    (*kind == MB_DATA_DATA || *kind == MB_DATA_NAV || *kind == MB_DATA_CALIBRATE) && mb_io_ptr->ping_count > 1) {
		if ((*time_d - mb_io_ptr->old_time_d) > 60 * mb_io_ptr->timegap) {
			status = MB_FAILURE;
			*error = MB_ERROR_TIME_GAP;
		}
	}

	/* check for less than minimum speed */
	if ((*error == MB_ERROR_NO_ERROR || *error == MB_ERROR_TIME_GAP) &&
	    (((*kind == MB_DATA_DATA || *kind == MB_DATA_CALIBRATE) && mb_io_ptr->ping_count > 1) ||
	     (*kind == MB_DATA_NAV && mb_io_ptr->nav_count > 1))) {
		if (*time_d > MB_TIME_D_UNKNOWN && *speed < mb_io_ptr->speedmin) {
			status = MB_FAILURE;
			*error = MB_ERROR_SPEED_TOO_SMALL;
		}
	}

	/* log errors */
	if (*error < MB_ERROR_NO_ERROR)
		mb_notice_log_error(verbose, mbio_ptr, *error);

	if (verbose >= 4) {
		fprintf(stderr, "\ndbg4  New ping checked by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg4  New ping values:\n");
		fprintf(stderr, "dbg4       kind:          %d\n", *kind);
		fprintf(stderr, "dbg4       ping_count:    %d\n", mb_io_ptr->ping_count);
		fprintf(stderr, "dbg4       nav_count:     %d\n", mb_io_ptr->nav_count);
		fprintf(stderr, "dbg4       comment_count: %d\n", mb_io_ptr->comment_count);
		fprintf(stderr, "dbg4       error:         %d\n", mb_io_ptr->new_error);
		fprintf(stderr, "dbg4       status:        %d\n", status);
	}

	/* reset "old" navigation values */
	if (*error <= MB_ERROR_NO_ERROR && *error > MB_ERROR_COMMENT && (*kind == MB_DATA_DATA || *kind == MB_DATA_CALIBRATE)) {
		mb_io_ptr->old_time_d = *time_d;
		mb_io_ptr->old_lon = *navlon;
		mb_io_ptr->old_lat = *navlat;
	}

	/* reset "old" navigation values */
	if (*error <= MB_ERROR_NO_ERROR && *error > MB_ERROR_COMMENT && *kind == MB_DATA_NAV) {
		mb_io_ptr->old_ntime_d = *time_d;
		mb_io_ptr->old_nlon = *navlon;
		mb_io_ptr->old_nlat = *navlat;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind != MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
		fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
		fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
		fprintf(stderr, "dbg2       speed:         %f\n", *speed);
		fprintf(stderr, "dbg2       heading:       %f\n", *heading);
		fprintf(stderr, "dbg2       distance:      %f\n", *distance);
		fprintf(stderr, "dbg2       altitude:      %f\n", *altitude);
		fprintf(stderr, "dbg2       sensordepth:   %f\n", *sensordepth);
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		if (verbose >= 3 && *nbath > 0) {
			fprintf(stderr, "dbg3       beam   flag  bath  crosstrack alongtrack\n");
			for (int i = 0; i < *nbath; i++)
				fprintf(stderr, "dbg3       %4d   %3d   %f    %f     %f\n", i, beamflag[i], bath[i], bathacrosstrack[i],
				        bathalongtrack[i]);
		}
		fprintf(stderr, "dbg2       namp:      %d\n", *namp);
		if (verbose >= 3 && *namp > 0) {
			fprintf(stderr, "dbg3       beam   amp  crosstrack alongtrack\n");
			for (int i = 0; i < *namp; i++)
				fprintf(stderr, "dbg3       %4d   %f    %f     %f\n", i, amp[i], bathacrosstrack[i], bathalongtrack[i]);
		}
		fprintf(stderr, "dbg2       nss:      %d\n", *nss);
		if (verbose >= 3 && *nss > 0) {
			fprintf(stderr, "dbg3       pixel sidescan crosstrack alongtrack\n");
			for (int i = 0; i < *nss; i++)
				fprintf(stderr, "dbg3       %4d   %f    %f     %f\n", i, ss[i], ssacrosstrack[i], ssalongtrack[i]);
		}
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
