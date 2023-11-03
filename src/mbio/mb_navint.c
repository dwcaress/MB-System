/*--------------------------------------------------------------------
 *    The MB-system:	mb_time.c	10/30/2000
  *
 *    Copyright (c) 2000-2023 by
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
 * mb_navint.c includes the "mb_" functions used to interpolate
 * navigation for data formats using asynchronous nav.
 *
 * Author:	D. W. Caress
 * Date:	October 30, 2000
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"

//    #define MB_NAVINT_DEBUG 1
//    #define MB_ATTINT_DEBUG 1
//    #define MB_HEDINT_DEBUG 1
//    #define MB_DEPINT_DEBUG 1
//    #define MB_ALTINT_DEBUG 1

/*--------------------------------------------------------------------*/
/* 	function mb_navint_add adds a nav fix to the internal
        list used for interpolation/extrapolation. */
int mb_navint_add(int verbose, void *mbio_ptr, double time_d, double lon_easting, double lat_northing, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:       %f\n", time_d);
		fprintf(stderr, "dbg2       lon_easting:  %f\n", lon_easting);
		fprintf(stderr, "dbg2       lat_northing: %f\n", lat_northing);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Current nav fix values:\n");
		for (int i = 0; i < mb_io_ptr->nfix; i++)
			fprintf(stderr, "dbg2       nav fix[%2d]:   %f %f %f\n", i, mb_io_ptr->fix_time_d[i], mb_io_ptr->fix_lon[i],
			        mb_io_ptr->fix_lat[i]);
	}

	/* add another fix only if time stamp has changed */
	if (mb_io_ptr->nfix == 0 || (time_d > mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1])) {
		/* if list if full make room for another nav fix */
		if (mb_io_ptr->nfix >= MB_ASYNCH_SAVE_MAX) {
			int shift = MB_ASYNCH_SAVE_MAX / 2;
			for (int i = 0; i < mb_io_ptr->nfix - shift; i++) {
				mb_io_ptr->fix_time_d[i] = mb_io_ptr->fix_time_d[i + shift];
				mb_io_ptr->fix_lon[i] = mb_io_ptr->fix_lon[i + shift];
				mb_io_ptr->fix_lat[i] = mb_io_ptr->fix_lat[i + shift];
			}
			mb_io_ptr->nfix -= shift;
		}

		/* add new fix to list */
		mb_io_ptr->fix_time_d[mb_io_ptr->nfix] = time_d;
		mb_io_ptr->fix_lon[mb_io_ptr->nfix] = lon_easting;
		mb_io_ptr->fix_lat[mb_io_ptr->nfix] = lat_northing;
		mb_io_ptr->nfix++;
#ifdef MB_NAVINT_DEBUG
		fprintf(stderr, "mb_navint_add:    Nav fix %d %f %f added\n", mb_io_ptr->nfix, lon_easting, lat_northing);
#endif

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Nav fix added to list by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New fix values:\n");
			fprintf(stderr, "dbg4       nfix:       %d\n", mb_io_ptr->nfix);
			fprintf(stderr, "dbg4       time_d:     %f\n", mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1]);
			fprintf(stderr, "dbg4       fix_lon:    %f\n", mb_io_ptr->fix_lon[mb_io_ptr->nfix - 1]);
			fprintf(stderr, "dbg4       fix_lat:    %f\n", mb_io_ptr->fix_lat[mb_io_ptr->nfix - 1]);
		}
	}

	/* assume success */
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		fprintf(stderr, "\ndbg2  Current nav fix values:\n");
		for (int i = 0; i < mb_io_ptr->nfix; i++)
			fprintf(stderr, "dbg2       nav fix[%2d]:   %f %f %f\n", i, mb_io_ptr->fix_time_d[i], mb_io_ptr->fix_lon[i],
			        mb_io_ptr->fix_lat[i]);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_navint_interp interpolates or extrapolates a
        nav fix from the internal list. */
int mb_navint_interp(int verbose, void *mbio_ptr, double time_d, double heading, double rawspeed, double *lon, double *lat,
                     double *speed, int *error) {
	double mtodeglon = 0.0;
	double mtodeglat = 0.0;
	double dx, dy, dt, dd;
	double factor, headingx, headingy;
	double speed_mps;
	int ifix = 0;
	int ifix0, ifix1;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
		fprintf(stderr, "dbg2       rawspeed:   %f\n", rawspeed);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Current nav fix values:\n");
		for (int i = 0; i < mb_io_ptr->nfix; i++)
			fprintf(stderr, "dbg2       nav fix[%2d]:   %f %f %f\n", i, mb_io_ptr->fix_time_d[i], mb_io_ptr->fix_lon[i],
			        mb_io_ptr->fix_lat[i]);
	}

	/* get degrees to meters conversion if fix available */
	if (mb_io_ptr->nfix > 0) {
		mb_coor_scale(verbose, mb_io_ptr->fix_lat[mb_io_ptr->nfix - 1], &mtodeglon, &mtodeglat);
	}

	/* find location of time_d in the list arrays */
	if (mb_io_ptr->nfix > 1) {
		if (time_d <= mb_io_ptr->fix_time_d[0])
			ifix = 0;
		else if (time_d >= mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1])
			ifix = mb_io_ptr->nfix - 1;
		else {
			ifix = (mb_io_ptr->nfix - 1) * (time_d - mb_io_ptr->fix_time_d[0]) /
			       (mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1] - mb_io_ptr->fix_time_d[0]);
			while (time_d > mb_io_ptr->fix_time_d[ifix]) {
				ifix++;
			}
			while (time_d < mb_io_ptr->fix_time_d[ifix - 1]) {
				ifix--;
			}
		}
	}
	else if (mb_io_ptr->nfix == 1) {
		ifix = 0;
	}

	/* use raw speed if available */
	if (rawspeed > 0.0)
		*speed = rawspeed; /* km/hr */

	/* else get speed averaged over as many as 100 fixes */
	else if (mb_io_ptr->nfix > 1) {
		ifix0 = MAX(ifix - 50, 0);
		ifix1 = MIN(ifix + 50, mb_io_ptr->nfix - 1);
		dx = (mb_io_ptr->fix_lon[ifix1] - mb_io_ptr->fix_lon[ifix0]) / mtodeglon;
		dy = (mb_io_ptr->fix_lat[ifix1] - mb_io_ptr->fix_lat[ifix0]) / mtodeglat;
		dt = mb_io_ptr->fix_time_d[ifix1] - mb_io_ptr->fix_time_d[ifix0];
		*speed = 3.6 * sqrt(dx * dx + dy * dy) / dt; /* km/hr */
	}

	/* else speed unknown */
	else
		*speed = 0.0;

	/* get speed in m/s */
	speed_mps = *speed / 3.6;

	int status = MB_SUCCESS;

	/* interpolate if possible */
	if (mb_io_ptr->nfix > 1 && (time_d >= mb_io_ptr->fix_time_d[0]) && (time_d <= mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1])) {
		factor = (time_d - mb_io_ptr->fix_time_d[ifix - 1]) / (mb_io_ptr->fix_time_d[ifix] - mb_io_ptr->fix_time_d[ifix - 1]);
		*lon = mb_io_ptr->fix_lon[ifix - 1] + factor * (mb_io_ptr->fix_lon[ifix] - mb_io_ptr->fix_lon[ifix - 1]);
		*lat = mb_io_ptr->fix_lat[ifix - 1] + factor * (mb_io_ptr->fix_lat[ifix] - mb_io_ptr->fix_lat[ifix - 1]);
		status = MB_SUCCESS;
#ifdef MB_NAVINT_DEBUG
		fprintf(stderr, "mb_navint_interp: Nav  %f %f interpolated at fix %d of %d with factor:%f\n", *lon, *lat, ifix,
		        mb_io_ptr->nfix, factor);
#endif
	}

	/* extrapolate from last fix - note zero speed
	    results in just using the last fix */
	else if (mb_io_ptr->nfix > 1 && (time_d > mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1])) {
		/* extrapolated position using average speed */
		dd = (time_d - mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1]) * speed_mps; /* meters */
		headingx = sin(DTR * heading);
		headingy = cos(DTR * heading);
		*lon = mb_io_ptr->fix_lon[mb_io_ptr->nfix - 1] + headingx * mtodeglon * dd;
		*lat = mb_io_ptr->fix_lat[mb_io_ptr->nfix - 1] + headingy * mtodeglat * dd;
		status = MB_SUCCESS;
#ifdef MB_NAVINT_DEBUG
		fprintf(stderr, "mb_navint_interp: Nav %f %f extrapolated from last fix of %d with distance:%f and speed:%f\n", *lon,
		        *lat, mb_io_ptr->nfix, dd, speed_mps);
#endif
	}

	/* extrapolate from first fix - note zero speed
	    results in just using the first fix */
	else if (mb_io_ptr->nfix >= 1) {
		/* extrapolated position using average speed */
		dd = (time_d - mb_io_ptr->fix_time_d[0]) * speed_mps; /* meters */
		headingx = sin(DTR * heading);
		headingy = cos(DTR * heading);
		*lon = mb_io_ptr->fix_lon[0] + headingx * mtodeglon * dd;
		*lat = mb_io_ptr->fix_lat[0] + headingy * mtodeglat * dd;
		status = MB_SUCCESS;
#ifdef MB_NAVINT_DEBUG
		fprintf(stderr, "mb_navint_interp: Nav %f %f extrapolated from first fix of %d with distance %f and speed:%f\n", *lon,
		        *lat, mb_io_ptr->nfix, dd, speed_mps);
#endif
	}

	/* else no fix */
	else {
		*lon = 0.0;
		*lat = 0.0;
		*speed = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_NAVINT_DEBUG
		fprintf(stderr, "mb_navint_interp: Nav zeroed\n");
#endif
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       lon:        %f\n", *lon);
		fprintf(stderr, "dbg2       lat:        %f\n", *lat);
		fprintf(stderr, "dbg2       speed:      %f\n", *speed);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_navint_prjinterp interpolates or extrapolates a
        nav fix from the internal list treating the position
        list as being in a projected coordinate system
        rather than in geographic lon lat. */
int mb_navint_prjinterp(int verbose, void *mbio_ptr, double time_d, double heading, double rawspeed, double *easting,
                        double *northing, double *speed, int *error) {
	double dx, dy, dt, dd;
	double factor, headingx, headingy;
	double speed_mps;
	int ifix = 0;
	int ifix0, ifix1;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
		fprintf(stderr, "dbg2       rawspeed:   %f\n", rawspeed);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Current nav fix values:\n");
		for (int i = 0; i < mb_io_ptr->nfix; i++)
			fprintf(stderr, "dbg2       nav fix[%2d]:   %f %f %f\n", i, mb_io_ptr->fix_time_d[i], mb_io_ptr->fix_lon[i],
			        mb_io_ptr->fix_lat[i]);
	}

	/* find location of time_d in the list arrays */
	if (mb_io_ptr->nfix > 1) {
		if (time_d <= mb_io_ptr->fix_time_d[0])
			ifix = 0;
		else if (time_d >= mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1])
			ifix = mb_io_ptr->nfix - 1;
		else {
			ifix = (mb_io_ptr->nfix - 1) * (time_d - mb_io_ptr->fix_time_d[0]) /
			       (mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1] - mb_io_ptr->fix_time_d[0]);
			while (time_d > mb_io_ptr->fix_time_d[ifix]) {
				ifix++;
			}
			while (time_d < mb_io_ptr->fix_time_d[ifix - 1]) {
				ifix--;
			}
		}
	}
	else if (mb_io_ptr->nfix == 1) {
		ifix = 0;
	}

	/* use raw speed if available */
	if (rawspeed > 0.0)
		*speed = rawspeed; /* km/hr */

	/* else get speed averaged over as many as 100 fixes */
	else if (mb_io_ptr->nfix > 1) {
		ifix0 = MAX(ifix - 50, 0);
		ifix1 = MIN(ifix + 50, mb_io_ptr->nfix - 1);
		dx = (mb_io_ptr->fix_lon[ifix1] - mb_io_ptr->fix_lon[ifix0]);
		dy = (mb_io_ptr->fix_lat[ifix1] - mb_io_ptr->fix_lat[ifix0]);
		dt = mb_io_ptr->fix_time_d[ifix1] - mb_io_ptr->fix_time_d[ifix0];
		*speed = 3.6 * sqrt(dx * dx + dy * dy) / dt; /* km/hr */
	}

	/* else speed unknown */
	else
		*speed = 0.0;

	/* get speed in m/s */
	speed_mps = *speed / 3.6;

	int status = MB_SUCCESS;

	/* interpolate if possible */
	if (mb_io_ptr->nfix > 1 && (time_d >= mb_io_ptr->fix_time_d[0]) && (time_d <= mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1])) {
		factor = (time_d - mb_io_ptr->fix_time_d[ifix - 1]) / (mb_io_ptr->fix_time_d[ifix] - mb_io_ptr->fix_time_d[ifix - 1]);
		*easting = mb_io_ptr->fix_lon[ifix - 1] + factor * (mb_io_ptr->fix_lon[ifix] - mb_io_ptr->fix_lon[ifix - 1]);
		*northing = mb_io_ptr->fix_lat[ifix - 1] + factor * (mb_io_ptr->fix_lat[ifix] - mb_io_ptr->fix_lat[ifix - 1]);
		status = MB_SUCCESS;
#ifdef MB_NAVINT_DEBUG
		fprintf(stderr, "mb_navint_interp: Nav  %f %f interpolated at fix %d of %d with factor:%f\n", *easting, *northing, ifix,
		        mb_io_ptr->nfix, factor);
#endif
	}

	/* extrapolate from last fix - note zero speed
	    results in just using the last fix */
	else if (mb_io_ptr->nfix > 1 && (time_d > mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1])) {
		/* extrapolated position using average speed */
		dd = (time_d - mb_io_ptr->fix_time_d[mb_io_ptr->nfix - 1]) * speed_mps; /* meters */
		headingx = sin(DTR * heading);
		headingy = cos(DTR * heading);
		*easting = mb_io_ptr->fix_lon[mb_io_ptr->nfix - 1] + headingx * dd;
		*northing = mb_io_ptr->fix_lat[mb_io_ptr->nfix - 1] + headingy * dd;
		status = MB_SUCCESS;
#ifdef MB_NAVINT_DEBUG
		fprintf(stderr, "mb_navint_interp: Nav %f %f extrapolated from last fix of %d with distance:%f and speed:%f\n", *easting,
		        *northing, mb_io_ptr->nfix, dd, speed_mps);
#endif
	}

	/* extrapolate from first fix - note zero speed
	    results in just using the first fix */
	else if (mb_io_ptr->nfix >= 1) {
		/* extrapolated position using average speed */
		dd = (time_d - mb_io_ptr->fix_time_d[0]) * speed_mps; /* meters */
		headingx = sin(DTR * heading);
		headingy = cos(DTR * heading);
		*easting = mb_io_ptr->fix_lon[0] + headingx * dd;
		*northing = mb_io_ptr->fix_lat[0] + headingy * dd;
		status = MB_SUCCESS;
#ifdef MB_NAVINT_DEBUG
		fprintf(stderr, "mb_navint_interp: Nav %f %f extrapolated from first fix of %d with distance %f and speed:%f\n", *easting,
		        *northing, mb_io_ptr->nfix, dd, speed_mps);
#endif
	}

	/* else no fix */
	else {
		*easting = 0.0;
		*northing = 0.0;
		*speed = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_NAVINT_DEBUG
		fprintf(stderr, "mb_navint_interp: Nav zeroed\n");
#endif
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       easting:    %f\n", *easting);
		fprintf(stderr, "dbg2       northing:   %f\n", *northing);
		fprintf(stderr, "dbg2       speed:      %f\n", *speed);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_attint_add adds a attitude fix to the internal
        list used for interpolation/extrapolation. */
int mb_attint_add(int verbose, void *mbio_ptr, double time_d, double heave, double roll, double pitch, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       heave:      %f\n", heave);
		fprintf(stderr, "dbg2       roll:       %f\n", roll);
		fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* add another attitude fix only if time stamp has changed */
	if (mb_io_ptr->nattitude == 0 || (time_d > mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude - 1])) {
		/* if list if full make room for another attitude fix */
		if (mb_io_ptr->nattitude >= MB_ASYNCH_SAVE_MAX) {
			int shift = MB_ASYNCH_SAVE_MAX / 2;
			for (int i = 0; i < mb_io_ptr->nattitude - shift; i++) {
				mb_io_ptr->attitude_time_d[i] = mb_io_ptr->attitude_time_d[i  + shift];
				mb_io_ptr->attitude_heave[i] = mb_io_ptr->attitude_heave[i  + shift];
				mb_io_ptr->attitude_roll[i] = mb_io_ptr->attitude_roll[i  + shift];
				mb_io_ptr->attitude_pitch[i] = mb_io_ptr->attitude_pitch[i  + shift];
			}
			mb_io_ptr->nattitude -= shift;
		}

		/* add new fix to list */
		mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude] = time_d;
		mb_io_ptr->attitude_heave[mb_io_ptr->nattitude] = heave;
		mb_io_ptr->attitude_roll[mb_io_ptr->nattitude] = roll;
		mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude] = pitch;
		mb_io_ptr->nattitude++;
#ifdef MB_ATTINT_DEBUG
		fprintf(stderr, "mb_attint_add:    Attitude fix %d time_d:%f roll:%f pitch:%f heave:%f added\n", mb_io_ptr->nattitude,
		        time_d, roll, pitch, heave);
#endif

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Attitude fix added to list by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New fix values:\n");
			fprintf(stderr, "dbg4       nattitude:       %d\n", mb_io_ptr->nattitude);
			fprintf(stderr, "dbg4       time_d:     %f\n", mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude - 1]);
			fprintf(stderr, "dbg4       attitude_heave:    %f\n", mb_io_ptr->attitude_heave[mb_io_ptr->nattitude - 1]);
			fprintf(stderr, "dbg4       attitude_roll:     %f\n", mb_io_ptr->attitude_roll[mb_io_ptr->nattitude - 1]);
			fprintf(stderr, "dbg4       attitude_pitch:    %f\n", mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude - 1]);
		}
	}

	/* assume success */
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_attint_nadd adds multiple attitude fixes to the internal
        list used for interpolation/extrapolation. */
int mb_attint_nadd(int verbose, void *mbio_ptr, int nsamples, double *time_d, double *heave, double *roll, double *pitch,
                   int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       nsamples:   %d\n", nsamples);
		for (int i = 0; i < nsamples; i++) {
			fprintf(stderr, "dbg2       %d time_d:%f heave:%f roll:%f pitch:%f\n", i, time_d[i], heave[i], roll[i], pitch[i]);
		}
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* if necessary make room for attitude fixes */
	if (mb_io_ptr->nattitude + nsamples >= MB_ASYNCH_SAVE_MAX) {
		int shift = MB_ASYNCH_SAVE_MAX / 2;
		if (mb_io_ptr->nattitude - shift + nsamples >= MB_ASYNCH_SAVE_MAX)
			shift = mb_io_ptr->nattitude + nsamples - MB_ASYNCH_SAVE_MAX;
		for (int i = 0; i < mb_io_ptr->nattitude - shift; i++) {
			mb_io_ptr->attitude_time_d[i] = mb_io_ptr->attitude_time_d[i + shift];
			mb_io_ptr->attitude_heave[i] = mb_io_ptr->attitude_heave[i + shift];
			mb_io_ptr->attitude_roll[i] = mb_io_ptr->attitude_roll[i + shift];
			mb_io_ptr->attitude_pitch[i] = mb_io_ptr->attitude_pitch[i + shift];
		}
		mb_io_ptr->nattitude = mb_io_ptr->nattitude - shift;
	}

	/* add fixes */
	for (int i = 0; i < nsamples; i++) {
		/* add new fix to list */
		mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude] = time_d[i];
		mb_io_ptr->attitude_heave[mb_io_ptr->nattitude] = heave[i];
		mb_io_ptr->attitude_roll[mb_io_ptr->nattitude] = roll[i];
		mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude] = pitch[i];
#ifdef MB_ATTINT_DEBUG
		fprintf(stderr, "mb_attint_add:    Attitude fix %d of %d: time:%f roll:%f pitch:%f heave:%f added\n", i,
		        mb_io_ptr->nattitude, time_d[i], roll[i], pitch[i], heave[i]);
#endif
		mb_io_ptr->nattitude++;

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Attitude fixes added to list by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New fix values:\n");
			fprintf(stderr, "dbg4       nattitude:       %d\n", mb_io_ptr->nattitude);
			fprintf(stderr, "dbg4       time_d:     %f\n", mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude - 1]);
			fprintf(stderr, "dbg4       attitude_heave:    %f\n", mb_io_ptr->attitude_heave[mb_io_ptr->nattitude - 1]);
			fprintf(stderr, "dbg4       attitude_roll:     %f\n", mb_io_ptr->attitude_roll[mb_io_ptr->nattitude - 1]);
			fprintf(stderr, "dbg4       attitude_pitch:    %f\n", mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude - 1]);
		}
	}

	/* assume success */
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_attint_interp interpolates or extrapolates a
        attitude fix from the internal list. */
int mb_attint_interp(int verbose, void *mbio_ptr, double time_d, double *heave, double *roll, double *pitch, int *error) {
	double factor;
	int ifix;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	/* interpolate if possible */
	if (mb_io_ptr->nattitude > 1 && (mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude - 1] >= time_d) &&
	    (mb_io_ptr->attitude_time_d[0] <= time_d)) {
		/* get interpolated position */
		ifix = (mb_io_ptr->nattitude - 1) * (time_d - mb_io_ptr->attitude_time_d[0]) /
		       (mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude - 1] - mb_io_ptr->attitude_time_d[0]);
		while (time_d > mb_io_ptr->attitude_time_d[ifix])
			ifix++;
		while (time_d < mb_io_ptr->attitude_time_d[ifix - 1])
			ifix--;

		factor = (time_d - mb_io_ptr->attitude_time_d[ifix - 1]) /
		         (mb_io_ptr->attitude_time_d[ifix] - mb_io_ptr->attitude_time_d[ifix - 1]);
		*heave = mb_io_ptr->attitude_heave[ifix - 1] +
		         factor * (mb_io_ptr->attitude_heave[ifix] - mb_io_ptr->attitude_heave[ifix - 1]);
		*roll =
		    mb_io_ptr->attitude_roll[ifix - 1] + factor * (mb_io_ptr->attitude_roll[ifix] - mb_io_ptr->attitude_roll[ifix - 1]);
		*pitch = mb_io_ptr->attitude_pitch[ifix - 1] +
		         factor * (mb_io_ptr->attitude_pitch[ifix] - mb_io_ptr->attitude_pitch[ifix - 1]);
		status = MB_SUCCESS;
#ifdef MB_ATTINT_DEBUG
		fprintf(stderr,
		        "mb_attint_interp: Attitude time_d:%f roll:%f pitch:%f heave:%f interpolated at fix %d of %d with factor:%f\n",
		        time_d, *roll, *pitch, *heave, ifix, mb_io_ptr->nattitude, factor);
#endif
	}

	/* extrapolate from last fix */
	else if (mb_io_ptr->nattitude > 1 && (mb_io_ptr->attitude_time_d[mb_io_ptr->nattitude - 1] < time_d)) {
		/* extrapolated position using average speed */
		*heave = mb_io_ptr->attitude_heave[mb_io_ptr->nattitude - 1];
		*roll = mb_io_ptr->attitude_roll[mb_io_ptr->nattitude - 1];
		*pitch = mb_io_ptr->attitude_pitch[mb_io_ptr->nattitude - 1];
		status = MB_SUCCESS;
#ifdef MB_ATTINT_DEBUG
		fprintf(stderr, "mb_attint_interp: Attitude time_d:%f roll:%f pitch:%f heave:%f extrapolated from last fix of %d\n",
		        time_d, *roll, *pitch, *heave, mb_io_ptr->nattitude);
#endif
	}

	/* extrapolate from first fix */
	else if (mb_io_ptr->nattitude >= 1) {
		*heave = mb_io_ptr->attitude_heave[0];
		*roll = mb_io_ptr->attitude_roll[0];
		*pitch = mb_io_ptr->attitude_pitch[0];
		status = MB_SUCCESS;
#ifdef MB_ATTINT_DEBUG
		fprintf(stderr, "mb_attint_interp: Attitude time_d:%f roll:%f pitch:%f heave:%f extrapolated from first fix of %d\n",
		        time_d, *roll, *pitch, *heave, mb_io_ptr->nattitude);
#endif
	}

	/* else no fix */
	else {
		*heave = 0.0;
		*roll = 0.0;
		*pitch = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_ATTINT_DEBUG
		fprintf(stderr, "mb_attint_interp: Attitude zeroed\n");
#endif
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       heave:        %f\n", *heave);
		fprintf(stderr, "dbg2       roll:         %f\n", *roll);
		fprintf(stderr, "dbg2       pitch:        %f\n", *pitch);
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_hedint_add adds a heading fix to the internal
        list used for interpolation/extrapolation. */
int mb_hedint_add(int verbose, void *mbio_ptr, double time_d, double heading, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* add another fix only if time stamp has changed */
	if (mb_io_ptr->nheading == 0 || (time_d > mb_io_ptr->heading_time_d[mb_io_ptr->nheading - 1])) {
		/* if list if full make room for another heading fix */
		if (mb_io_ptr->nheading >= MB_ASYNCH_SAVE_MAX) {
			int shift = MB_ASYNCH_SAVE_MAX / 2;
			for (int i = 0; i < mb_io_ptr->nheading - shift; i++) {
				mb_io_ptr->heading_time_d[i] = mb_io_ptr->heading_time_d[i + shift];
				mb_io_ptr->heading_heading[i] = mb_io_ptr->heading_heading[i + shift];
			}
			mb_io_ptr->nheading -= shift;
		}

		/* add new fix to list */
		mb_io_ptr->heading_time_d[mb_io_ptr->nheading] = time_d;
		mb_io_ptr->heading_heading[mb_io_ptr->nheading] = heading;
		mb_io_ptr->nheading++;
#ifdef MB_HEDINT_DEBUG
		fprintf(stderr, "mb_hedint_add:    Heading fix %d %f added\n", mb_io_ptr->nheading, heading);
#endif

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Heading fix added to list by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New fix values:\n");
			fprintf(stderr, "dbg4       nheading:       %d\n", mb_io_ptr->nheading);
			fprintf(stderr, "dbg4       time_d:     %f\n", mb_io_ptr->heading_time_d[mb_io_ptr->nheading - 1]);
			fprintf(stderr, "dbg4       heading_heading:  %f\n", mb_io_ptr->heading_heading[mb_io_ptr->nheading - 1]);
		}
	}

	/* assume success */
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_hedint_nadd adds multiple heading fixes to the internal
        list used for interpolation/extrapolation. */
int mb_hedint_nadd(int verbose, void *mbio_ptr, int nsamples, double *time_d, double *heading, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       nsamples:   %d\n", nsamples);
		for (int i = 0; i < nsamples; i++) {
			fprintf(stderr, "dbg2       %d time_d:%f heading:%f\n", i, time_d[i], heading[i]);
		}
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* if necessary make room for heading fixes */
	if (mb_io_ptr->nheading + nsamples >= MB_ASYNCH_SAVE_MAX) {
		int shift = MB_ASYNCH_SAVE_MAX / 2;
		if (mb_io_ptr->nheading - shift + nsamples >= MB_ASYNCH_SAVE_MAX)
			shift = mb_io_ptr->nheading + nsamples - MB_ASYNCH_SAVE_MAX;
		for (int i = 0; i < mb_io_ptr->nheading - shift; i++) {
			mb_io_ptr->heading_time_d[i] = mb_io_ptr->heading_time_d[i + shift];
			mb_io_ptr->heading_heading[i] = mb_io_ptr->heading_heading[i + shift];
		}
		mb_io_ptr->nheading = mb_io_ptr->nheading - shift;
	}

	/* add fixes */
	for (int i = 0; i < nsamples; i++) {
		/* add new fix to list */
		mb_io_ptr->heading_time_d[mb_io_ptr->nheading] = time_d[i];
		mb_io_ptr->heading_heading[mb_io_ptr->nheading] = heading[i];
#ifdef MB_HEDINT_DEBUG
		fprintf(stderr, "mb_hedint_nadd:    Heading fix %d of %d: %f added\n", i, mb_io_ptr->nheading, heading[i]);
#endif
		mb_io_ptr->nheading++;

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Heading fixes added to list by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New fix values:\n");
			fprintf(stderr, "dbg4       nheading:       %d\n", mb_io_ptr->nheading);
			fprintf(stderr, "dbg4       time_d:          %f\n", mb_io_ptr->heading_time_d[mb_io_ptr->nheading - 1]);
			fprintf(stderr, "dbg4       heading_heading: %f\n", mb_io_ptr->heading_heading[mb_io_ptr->nheading - 1]);
		}
	}

	/* assume success */
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_hedint_interp interpolates or extrapolates a
        heading fix from the internal list. */
int mb_hedint_interp(int verbose, void *mbio_ptr, double time_d, double *heading, int *error) {
	double factor;
	int ifix;
	double heading1, heading2;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	/* interpolate if possible */
	if (mb_io_ptr->nheading > 1 && (mb_io_ptr->heading_time_d[mb_io_ptr->nheading - 1] >= time_d) &&
	    (mb_io_ptr->heading_time_d[0] <= time_d)) {
		/* get interpolated heading */
		ifix = (mb_io_ptr->nheading - 1) * (time_d - mb_io_ptr->heading_time_d[0]) /
		       (mb_io_ptr->heading_time_d[mb_io_ptr->nheading - 1] - mb_io_ptr->heading_time_d[0]);
		while (time_d > mb_io_ptr->heading_time_d[ifix])
			ifix++;
		while (time_d < mb_io_ptr->heading_time_d[ifix - 1])
			ifix--;

		factor = (time_d - mb_io_ptr->heading_time_d[ifix - 1]) /
		         (mb_io_ptr->heading_time_d[ifix] - mb_io_ptr->heading_time_d[ifix - 1]);
		heading1 = mb_io_ptr->heading_heading[ifix - 1];
		heading2 = mb_io_ptr->heading_heading[ifix];
		if (heading2 - heading1 > 180.0)
			heading2 -= 360.0;
		else if (heading2 - heading1 < -180.0)
			heading2 += 360.0;
		*heading = heading1 + factor * (heading2 - heading1);
		if (*heading < 0.0)
			*heading += 360.0;
		else if (*heading > 360.0)
			*heading -= 360.0;
		status = MB_SUCCESS;
#ifdef MB_HEDINT_DEBUG
		fprintf(stderr, "mb_hedint_interp: Heading %f interpolated at value %d of %d with factor:%f\n", *heading, ifix,
		        mb_io_ptr->nheading, factor);
#endif
	}

	/* extrapolate from last fix */
	else if (mb_io_ptr->nheading > 1 && (mb_io_ptr->heading_time_d[mb_io_ptr->nheading - 1] < time_d)) {
		/* extrapolated heading using average speed */
		*heading = mb_io_ptr->heading_heading[mb_io_ptr->nheading - 1];
		status = MB_SUCCESS;
#ifdef MB_HEDINT_DEBUG
		fprintf(stderr, "mb_hedint_interp: Heading %f taken from last value of %d\n", *heading, mb_io_ptr->nheading);
#endif
	}

	/* extrapolate from first fix */
	else if (mb_io_ptr->nheading >= 1) {
		*heading = mb_io_ptr->heading_heading[0];
		status = MB_SUCCESS;
#ifdef MB_HEDINT_DEBUG
		fprintf(stderr, "mb_hedint_interp: Heading %f taken from first value of %d\n", *heading, mb_io_ptr->nheading);
#endif
	}

	/* else no fix */
	else {
		*heading = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_HEDINT_DEBUG
		fprintf(stderr, "mb_hedint_interp: Heading zeroed\n");
#endif
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       heading:      %f\n", *heading);
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_depint_add adds a sonar depth fix to the internal
        list used for interpolation/extrapolation. */
int mb_depint_add(int verbose, void *mbio_ptr, double time_d, double sensordepth, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       sensordepth: %f\n", sensordepth);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* add another fix only if time stamp has changed */
	if (mb_io_ptr->nsensordepth == 0 || (time_d > mb_io_ptr->sensordepth_time_d[mb_io_ptr->nsensordepth - 1])) {
		/* if list if full make room for another sensordepth fix */
		if (mb_io_ptr->nsensordepth >= MB_ASYNCH_SAVE_MAX) {
			int shift = MB_ASYNCH_SAVE_MAX / 2;
			for (int i = 0; i < mb_io_ptr->nsensordepth - shift; i++) {
				mb_io_ptr->sensordepth_time_d[i] = mb_io_ptr->sensordepth_time_d[i + shift];
				mb_io_ptr->sensordepth_sensordepth[i] = mb_io_ptr->sensordepth_sensordepth[i + shift];
			}
			mb_io_ptr->nsensordepth -= shift;
		}

		/* add new fix to list */
		mb_io_ptr->sensordepth_time_d[mb_io_ptr->nsensordepth] = time_d;
		mb_io_ptr->sensordepth_sensordepth[mb_io_ptr->nsensordepth] = sensordepth;
		mb_io_ptr->nsensordepth++;
#ifdef MB_DEPINT_DEBUG
		fprintf(stderr, "mb_depint_add:    sensordepth fix %d %f added\n", mb_io_ptr->nsensordepth, sensordepth);
#endif

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Sonar depth fix added to list by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New fix values:\n");
			fprintf(stderr, "dbg4       nsensordepth:       %d\n", mb_io_ptr->nsensordepth);
			fprintf(stderr, "dbg4       time_d:     %f\n", mb_io_ptr->sensordepth_time_d[mb_io_ptr->nsensordepth - 1]);
			fprintf(stderr, "dbg4       sensordepth_sensordepth:  %f\n",
			        mb_io_ptr->sensordepth_sensordepth[mb_io_ptr->nsensordepth - 1]);
		}
	}

	/* assume success */
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_depint_interp interpolates or extrapolates a
        sonar depth fix from the internal list. */
int mb_depint_interp(int verbose, void *mbio_ptr, double time_d, double *sensordepth, int *error) {
	double factor;
	int ifix;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	/* interpolate if possible */
	if (mb_io_ptr->nsensordepth > 1 && (mb_io_ptr->sensordepth_time_d[mb_io_ptr->nsensordepth - 1] >= time_d) &&
	    (mb_io_ptr->sensordepth_time_d[0] <= time_d)) {
		/* get interpolated position */
		ifix = (mb_io_ptr->nsensordepth - 1) * (time_d - mb_io_ptr->sensordepth_time_d[0]) /
		       (mb_io_ptr->sensordepth_time_d[mb_io_ptr->nsensordepth - 1] - mb_io_ptr->sensordepth_time_d[0]);
		while (time_d > mb_io_ptr->sensordepth_time_d[ifix])
			ifix++;
		while (time_d < mb_io_ptr->sensordepth_time_d[ifix - 1])
			ifix--;

		factor = (time_d - mb_io_ptr->sensordepth_time_d[ifix - 1]) /
		         (mb_io_ptr->sensordepth_time_d[ifix] - mb_io_ptr->sensordepth_time_d[ifix - 1]);
		*sensordepth = mb_io_ptr->sensordepth_sensordepth[ifix - 1] +
		              factor * (mb_io_ptr->sensordepth_sensordepth[ifix] - mb_io_ptr->sensordepth_sensordepth[ifix - 1]);
		status = MB_SUCCESS;
#ifdef MB_DEPINT_DEBUG
		fprintf(stderr, "mb_depint_interp: sensordepth %f interpolated at fix %d of %d with factor:%f\n", *sensordepth, ifix,
		        mb_io_ptr->nsensordepth, factor);
#endif
	}

	/* extrapolate from last value */
	else if (mb_io_ptr->nsensordepth > 1 && (mb_io_ptr->sensordepth_time_d[mb_io_ptr->nsensordepth - 1] < time_d)) {
		/* extrapolated depth using last value */
		*sensordepth = mb_io_ptr->sensordepth_sensordepth[mb_io_ptr->nsensordepth - 1];
		status = MB_SUCCESS;
#ifdef MB_DEPINT_DEBUG
		fprintf(stderr, "mb_depint_interp: sensordepth %f extrapolated from last fix of %d\n", *sensordepth,
		        mb_io_ptr->nsensordepth);
#endif
	}

	/* extrapolate from first fix */
	else if (mb_io_ptr->nsensordepth >= 1) {
		*sensordepth = mb_io_ptr->sensordepth_sensordepth[0];
		status = MB_SUCCESS;
#ifdef MB_DEPINT_DEBUG
		fprintf(stderr, "mb_depint_interp: sensordepth %f extrapolated from first fix of %d\n", *sensordepth,
		        mb_io_ptr->nsensordepth);
#endif
	}

	/* else no fix */
	else {
		*sensordepth = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_DEPINT_DEBUG
		fprintf(stderr, "mb_depint_interp: sensordepth zeroed\n");
#endif
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       sensordepth:   %f\n", *sensordepth);
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_altint_add adds a heading fix to the internal
        list used for interpolation/extrapolation. */
int mb_altint_add(int verbose, void *mbio_ptr, double time_d, double altitude, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       altitude:   %f\n", altitude);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* add another fix only if time stamp has changed */
	if (mb_io_ptr->naltitude == 0 || (time_d > mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude - 1])) {
		/* if list if full make room for another altitude fix */
		if (mb_io_ptr->naltitude >= MB_ASYNCH_SAVE_MAX) {
			int shift = MB_ASYNCH_SAVE_MAX / 2;
			for (int i = 0; i < mb_io_ptr->naltitude - shift; i++) {
				mb_io_ptr->altitude_time_d[i] = mb_io_ptr->altitude_time_d[i + shift];
				mb_io_ptr->altitude_altitude[i] = mb_io_ptr->altitude_altitude[i + shift];
			}
			mb_io_ptr->naltitude -= shift;
		}

		/* add new fix to list */
		mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude] = time_d;
		mb_io_ptr->altitude_altitude[mb_io_ptr->naltitude] = altitude;
		mb_io_ptr->naltitude++;
#ifdef MB_ALTINT_DEBUG
		fprintf(stderr, "mb_altint_add:    altitude fix %d %f added\n", mb_io_ptr->naltitude, altitude);
#endif

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Altitude fix added to list by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New fix values:\n");
			fprintf(stderr, "dbg4       naltitude:       %d\n", mb_io_ptr->naltitude);
			fprintf(stderr, "dbg4       time_d:     %f\n", mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude - 1]);
			fprintf(stderr, "dbg4       altitude_altitude:  %f\n", mb_io_ptr->altitude_altitude[mb_io_ptr->naltitude - 1]);
		}
	}

	/* assume success */
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_altint_interp interpolates or extrapolates a
        altitude fix from the internal list. */
int mb_altint_interp(int verbose, void *mbio_ptr, double time_d, double *altitude, int *error) {
	double factor;
	int ifix;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	/* interpolate if possible */
	if (mb_io_ptr->naltitude > 1 && (mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude - 1] >= time_d) &&
	    (mb_io_ptr->altitude_time_d[0] <= time_d)) {
		/* get interpolated position */
		ifix = (mb_io_ptr->naltitude - 1) * (time_d - mb_io_ptr->altitude_time_d[0]) /
		       (mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude - 1] - mb_io_ptr->altitude_time_d[0]);
		while (time_d > mb_io_ptr->altitude_time_d[ifix])
			ifix++;
		while (time_d < mb_io_ptr->fix_time_d[ifix - 1])
			ifix--;

		factor = (time_d - mb_io_ptr->altitude_time_d[ifix - 1]) /
		         (mb_io_ptr->altitude_time_d[ifix] - mb_io_ptr->altitude_time_d[ifix - 1]);
		*altitude = mb_io_ptr->altitude_altitude[ifix - 1] +
		            factor * (mb_io_ptr->altitude_altitude[ifix] - mb_io_ptr->altitude_altitude[ifix - 1]);
		status = MB_SUCCESS;
#ifdef MB_ALTINT_DEBUG
		fprintf(stderr, "mb_altint_interp: altitude %f interpolated at fix %d of %d with factor:%f\n", *altitude, ifix,
		        mb_io_ptr->naltitude, factor);
#endif
	}

	/* extrapolate from last fix */
	else if (mb_io_ptr->naltitude > 1 && (mb_io_ptr->altitude_time_d[mb_io_ptr->naltitude - 1] < time_d)) {
		/* extrapolated position using average speed */
		*altitude = mb_io_ptr->altitude_altitude[mb_io_ptr->naltitude - 1];
		status = MB_SUCCESS;
#ifdef MB_ALTINT_DEBUG
		fprintf(stderr, "mb_altint_interp: altitude %f extrapolated from last fix of %d\n", *altitude, mb_io_ptr->naltitude);
#endif
	}

	/* extrapolate from first fix */
	else if (mb_io_ptr->naltitude >= 1) {
		*altitude = mb_io_ptr->altitude_altitude[0];
		status = MB_SUCCESS;
#ifdef MB_ALTINT_DEBUG
		fprintf(stderr, "mb_altint_interp: altitude %f extrapolated from first fix of %d\n", *altitude, mb_io_ptr->naltitude);
#endif
	}

	/* else no fix */
	else {
		*altitude = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
#ifdef MB_ALTINT_DEBUG
		fprintf(stderr, "mb_altint_interp: altitude zeroed\n");
#endif
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       altitude:     %f\n", *altitude);
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/

int mb_loadnavdata(int verbose, char *merge_nav_file, int merge_nav_format, int merge_nav_lonflip, int *merge_nav_num,
                   int *merge_nav_alloc, double **merge_nav_time_d, double **merge_nav_lon, double **merge_nav_lat,
                   double **merge_nav_speed, int *error) {
	char buffer[MBP_FILENAMESIZE], dummy[MBP_FILENAMESIZE], *result, *bufftmp;
	int nrecord;
	int nchar, nget;
	size_t size;
	FILE *tfp;
	int time_i[7], time_j[6], ihr, ioff;
	char NorS[2], EorW[2];
	double mlon, llon, mlat, llat;
	int degree;
	double sec, hr, dminute;
	double time_d, heading, sensordepth, roll, pitch, heave;
	int len;
	int quality, nsatellite, dilution, gpsheight;
	double *n_time_d;
	double *n_lon;
	double *n_lat;
	double *n_speed;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                %d\n", verbose);
		fprintf(stderr, "dbg2       merge_nav_file:         %s\n", merge_nav_file);
		fprintf(stderr, "dbg2       merge_nav_format:       %d\n", merge_nav_format);
		fprintf(stderr, "dbg2       merge_nav_lonflip:      %d\n", merge_nav_lonflip);
		fprintf(stderr, "dbg2       merge_nav_num *:        %p\n", merge_nav_num);
		fprintf(stderr, "dbg2       merge_nav_num:          %d\n", *merge_nav_num);
		fprintf(stderr, "dbg2       merge_nav_alloc *:      %p\n", merge_nav_alloc);
		fprintf(stderr, "dbg2       merge_nav_alloc:        %d\n", *merge_nav_alloc);
		fprintf(stderr, "dbg2       merge_nav_time_d **:    %p\n", merge_nav_time_d);
		fprintf(stderr, "dbg2       merge_nav_time_d *:     %p\n", *merge_nav_time_d);
		fprintf(stderr, "dbg2       merge_nav_lon **:       %p\n", merge_nav_lon);
		fprintf(stderr, "dbg2       merge_nav_lon *:        %p\n", *merge_nav_lon);
		fprintf(stderr, "dbg2       merge_nav_lat **:       %p\n", merge_nav_lat);
		fprintf(stderr, "dbg2       merge_nav_lat *:        %p\n", *merge_nav_lat);
		fprintf(stderr, "dbg2       merge_nav_speed **:     %p\n", merge_nav_speed);
		fprintf(stderr, "dbg2       merge_nav_speed *:      %p\n", *merge_nav_speed);
	}

	/* set max number of characters to be read at a time */
	if (merge_nav_format == 8)
		nchar = 96;
	else
		nchar = MBP_FILENAMESIZE - 1;

	int status = MB_SUCCESS;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_nav_file, "r")) != NULL) {
		/* loop over reading the records */
		while ((result = fgets(buffer, nchar, tfp)) == buffer)
			nrecord++;

		/* close the file */
		fclose(tfp);
		tfp = NULL;
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	}

	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_nav_alloc < nrecord) {
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_nav_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_nav_lon, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_nav_lat, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_nav_speed, error);
		if (status == MB_SUCCESS)
			*merge_nav_alloc = nrecord;
		n_time_d = *merge_nav_time_d;
		n_lon = *merge_nav_lon;
		n_lat = *merge_nav_lat;
		n_speed = *merge_nav_speed;
	}

	/* read the records */
	if (status == MB_SUCCESS) {
		bool time_set;
		bool nav_ok;
		nrecord = 0;
		if ((tfp = fopen(merge_nav_file, "r")) == NULL) {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
		else {
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer, nchar, tfp)) == buffer) {
				nav_ok = false;

				/* deal with nav in form: time_d lon lat speed */
				if (merge_nav_format == 1) {
					nget = sscanf(buffer, "%lf %lf %lf %lf", &n_time_d[nrecord], &n_lon[nrecord], &n_lat[nrecord],
					              &n_speed[nrecord]);
					if (nget == 3)
						n_speed[nrecord] = 0.0;
					if (nget >= 3)
						nav_ok = true;
				}

				/* deal with nav in form: yr mon day hour min sec lon lat */
				else if (merge_nav_format == 2) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3],
					              &time_i[4], &sec, &n_lon[nrecord], &n_lat[nrecord]);
					time_i[5] = (int)sec;
					time_i[6] = 1000000 * (sec - time_i[5]);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					n_speed[nrecord] = 0.0;
					if (nget == 8)
						nav_ok = true;
				}

				/* deal with nav in form: yr jday hour min sec lon lat */
				else if (merge_nav_format == 3) {
					nget = sscanf(buffer, "%d %d %d %d %lf %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
					              &n_lon[nrecord], &n_lat[nrecord]);
					time_j[2] = time_j[2] + 60 * ihr;
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					n_speed[nrecord] = 0.0;
					if (nget == 7)
						nav_ok = true;
				}

				/* deal with nav in form: yr jday daymin sec lon lat */
				else if (merge_nav_format == 4) {
					nget = sscanf(buffer, "%d %d %d %lf %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec, &n_lon[nrecord],
					              &n_lat[nrecord]);
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					n_speed[nrecord] = 0.0;
					if (nget == 6)
						nav_ok = true;
				}

				/* deal with nav in L-DEO processed nav format */
				else if (merge_nav_format == 5) {
					strncpy(dummy, "", 128);
					if (buffer[2] == '+') {
						time_j[0] = strtol(strncpy(dummy, buffer, 2), NULL, 10);
						mb_fix_y2k(verbose, time_j[0], &time_j[0]);
						ioff = 3;
					}
					else {
						time_j[0] = strtol(strncpy(dummy, buffer, 4), NULL, 10);
						ioff = 5;
					}
					strncpy(dummy, "", 128);
					time_j[1] = strtol(strncpy(dummy, buffer + ioff, 3), NULL, 10);
					strncpy(dummy, "", 128);
					ioff += 4;
					hr = strtol(strncpy(dummy, buffer + ioff, 2), NULL, 10);
					strncpy(dummy, "", 128);
					ioff += 3;
					time_j[2] = strtol(strncpy(dummy, buffer + ioff, 2), NULL, 10) + 60 * hr;
					strncpy(dummy, "", 128);
					ioff += 3;
					time_j[3] = strtol(strncpy(dummy, buffer + ioff, 2), NULL, 10);
					time_j[4] = 0;
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;

					strncpy(NorS, "", sizeof(NorS));
					ioff += 7;
					NorS[0] = buffer[ioff];
					ioff += 1;
					strncpy(dummy, "", 128);
					mlat = strtod(strncpy(dummy, buffer + ioff, 3), NULL);
					strncpy(dummy, "", 128);
					ioff += 3;
					llat = strtod(strncpy(dummy, buffer + ioff, 8), NULL);
					strncpy(EorW, "", sizeof(EorW));
					ioff += 9;
					EorW[0] = buffer[ioff];
					strncpy(dummy, "", 128);
					ioff += 1;
					mlon = strtod(strncpy(dummy, buffer + ioff, 4), NULL);
					strncpy(dummy, "", 128);
					ioff += 4;
					llon = strtod(strncpy(dummy, buffer + ioff, 8), NULL);
					n_lon[nrecord] = mlon + llon / 60.;
					if (strncmp(EorW, "W", 1) == 0)
						n_lon[nrecord] = -n_lon[nrecord];
					n_lat[nrecord] = mlat + llat / 60.;
					if (strncmp(NorS, "S", 1) == 0)
						n_lat[nrecord] = -n_lat[nrecord];
					n_speed[nrecord] = 0.0;
					nav_ok = true;
				}

				/* deal with nav in real and pseudo NMEA 0183 format */
				else if (merge_nav_format == 6 || merge_nav_format == 7) {
					/* check if real sentence */
					len = strlen(buffer);
					if (strncmp(buffer, "$", 1) == 0) {
						if (strncmp(&buffer[3], "DAT", 3) == 0 && len > 15) {
							time_set = false;
							strncpy(dummy, "", 128);
							time_i[0] = strtol(strncpy(dummy, buffer + 7, 4), NULL, 10);
							time_i[1] = strtol(strncpy(dummy, buffer + 11, 2), NULL, 10);
							time_i[2] = strtol(strncpy(dummy, buffer + 13, 2), NULL, 10);
						}
						else if ((strncmp(&buffer[3], "ZDA", 3) == 0 || strncmp(&buffer[3], "UNX", 3) == 0) && len > 14) {
							time_set = false;
							/* find start of ",hhmmss.ss" */
							if ((bufftmp = strchr(buffer, ',')) != NULL) {
								strncpy(dummy, "", 128);
								time_i[3] = strtol(strncpy(dummy, bufftmp + 1, 2), NULL, 10);
								strncpy(dummy, "", 128);
								time_i[4] = strtol(strncpy(dummy, bufftmp + 3, 2), NULL, 10);
								strncpy(dummy, "", 128);
								time_i[5] = strtol(strncpy(dummy, bufftmp + 5, 2), NULL, 10);
								if (bufftmp[7] == '.') {
									strncpy(dummy, "", 128);
									time_i[6] = 10000 * strtol(strncpy(dummy, bufftmp + 8, 2), NULL, 10);
								}
								else
									time_i[6] = 0;
								/* find start of ",dd,mm,yyyy" */
								if ((bufftmp = strchr(&bufftmp[1], ',')) != NULL) {
									strncpy(dummy, "", 128);
									time_i[2] = strtol(strncpy(dummy, bufftmp + 1, 2), NULL, 10);
									strncpy(dummy, "", 128);
									time_i[1] = strtol(strncpy(dummy, bufftmp + 4, 2), NULL, 10);
									strncpy(dummy, "", 128);
									time_i[0] = strtol(strncpy(dummy, bufftmp + 7, 4), NULL, 10);
									time_set = true;
								}
							}
						}
						else if (((merge_nav_format == 6 && strncmp(&buffer[3], "GLL", 3) == 0) ||
						          (merge_nav_format == 7 && strncmp(&buffer[3], "GGA", 3) == 0)) &&
						         time_set && len > 26) {
							time_set = false;
							/* find start of ",ddmm.mm,N,ddmm.mm,E" */
							if ((bufftmp = strchr(buffer, ',')) != NULL) {
								if (merge_nav_format == 7)
									bufftmp = strchr(&bufftmp[1], ',');
								strncpy(dummy, "", 128);
								degree = strtol(strncpy(dummy, bufftmp + 1, 2), NULL, 10);
								strncpy(dummy, "", 128);
								dminute = strtod(strncpy(dummy, bufftmp + 3, 5), NULL);
								strncpy(NorS, "", sizeof(NorS));
								bufftmp = strchr(&bufftmp[1], ',');
								strncpy(NorS, bufftmp + 1, 1);
								n_lat[nrecord] = degree + dminute / 60.;
								if (strncmp(NorS, "S", 1) == 0)
									n_lat[nrecord] = -n_lat[nrecord];
								bufftmp = strchr(&bufftmp[1], ',');
								strncpy(dummy, "", 128);
								degree = strtol(strncpy(dummy, bufftmp + 1, 3), NULL, 10);
								strncpy(dummy, "", 128);
								dminute = strtod(strncpy(dummy, bufftmp + 4, 5), NULL);
								bufftmp = strchr(&bufftmp[1], ',');
								strncpy(EorW, "", sizeof(EorW));
								strncpy(EorW, bufftmp + 1, 1);
								n_lon[nrecord] = degree + dminute / 60.;
								if (strncmp(EorW, "W", 1) == 0)
									n_lon[nrecord] = -n_lon[nrecord];
								mb_get_time(verbose, time_i, &time_d);
								n_time_d[nrecord] = time_d;
								nav_ok = true;
							}
						}
					}
					n_speed[nrecord] = 0.0;
				}

				/* deal with nav in Simrad 90 format */
				else if (merge_nav_format == 8) {
					mb_get_int(&(time_i[2]), buffer + 2, 2);
					mb_get_int(&(time_i[1]), buffer + 4, 2);
					mb_get_int(&(time_i[0]), buffer + 6, 2);
					mb_fix_y2k(verbose, time_i[0], &time_i[0]);
					mb_get_int(&(time_i[3]), buffer + 9, 2);
					mb_get_int(&(time_i[4]), buffer + 11, 2);
					mb_get_int(&(time_i[5]), buffer + 13, 2);
					mb_get_int(&(time_i[6]), buffer + 15, 2);
					time_i[6] = 10000 * time_i[6];
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;

					mb_get_double(&mlat, buffer + 18, 2);
					mb_get_double(&llat, buffer + 20, 7);
					NorS[0] = buffer[27];
					n_lat[nrecord] = mlat + llat / 60.0;
					if (NorS[0] == 'S' || NorS[0] == 's')
						n_lat[nrecord] = -n_lat[nrecord];
					mb_get_double(&mlon, buffer + 29, 3);
					mb_get_double(&llon, buffer + 32, 7);
					EorW[0] = buffer[39];
					n_lon[nrecord] = mlon + llon / 60.0;
					if (EorW[0] == 'W' || EorW[0] == 'w')
						n_lon[nrecord] = -n_lon[nrecord];
					n_speed[nrecord] = 0.0;
					nav_ok = true;
				}

				/* deal with nav in form: yr mon day hour min sec time_d lon lat heading speed sensordepth */
				else if (merge_nav_format == 9) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0], &time_i[1],
					              &time_i[2], &time_i[3], &time_i[4], &sec, &n_time_d[nrecord], &n_lon[nrecord], &n_lat[nrecord],
					              &heading, &n_speed[nrecord], &sensordepth, &roll, &pitch, &heave);
					if (nget >= 9)
						nav_ok = true;
					if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1])
						nav_ok = false;
				}

				/* deal with nav in r2rnav form:
				    yyyy-mm-ddThh:mm:ss.sssZ decimalLongitude decimalLatitude quality nsat dilution height */
				else if (merge_nav_format == 10) {
					nget =
					    sscanf(buffer, "%d-%d-%dT%d:%d:%lfZ %lf %lf %d %d %d %d", &time_i[0], &time_i[1], &time_i[2], &time_i[3],
					           &time_i[4], &sec, &n_lon[nrecord], &n_lat[nrecord], &quality, &nsatellite, &dilution, &gpsheight);
					if (nget != 12) {
						quality = 0;
						nsatellite = 0;
						dilution = 0;
						gpsheight = 0;
					}
					time_i[5] = (int)floor(sec);
					time_i[6] = (int)((sec - time_i[5]) * 1000000);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					n_speed[nrecord] = 0.0;
					if (nget >= 8)
						nav_ok = true;
				}

				/* make sure longitude is defined according to lonflip */
				if (nav_ok) {
					if (merge_nav_lonflip == -1 && n_lon[nrecord] > 0.0)
						n_lon[nrecord] = n_lon[nrecord] - 360.0;
					else if (merge_nav_lonflip == 0 && n_lon[nrecord] < -180.0)
						n_lon[nrecord] = n_lon[nrecord] + 360.0;
					else if (merge_nav_lonflip == 0 && n_lon[nrecord] > 180.0)
						n_lon[nrecord] = n_lon[nrecord] - 360.0;
					else if (merge_nav_lonflip == 1 && n_lon[nrecord] < 0.0)
						n_lon[nrecord] = n_lon[nrecord] + 360.0;
				}

				/* output some debug values */
				if (verbose >= 5 && nav_ok) {
					fprintf(stderr, "\ndbg5  New navigation point read in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nrecord, n_time_d[nrecord], n_lon[nrecord], n_lat[nrecord]);
				}
				else if (verbose >= 5) {
					fprintf(stderr, "\ndbg5  Error parsing line in navigation file in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       line: %s\n", buffer);
				}

				/* check for reverses or repeats in time */
				if (nav_ok) {
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord - 1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1] && verbose >= 5) {
						fprintf(stderr, "\ndbg5  Navigation time error in function <%s>\n", __func__);
						fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nrecord - 1, n_time_d[nrecord - 1], n_lon[nrecord - 1],
						        n_lat[nrecord - 1]);
						fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nrecord, n_time_d[nrecord], n_lon[nrecord],
						        n_lat[nrecord]);
					}
				}
				strncpy(buffer, "", sizeof(buffer));
			}

			/* get the good record count */
			*merge_nav_num = nrecord;

			/* close the file */
			fclose(tfp);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       merge_nav_num:          %d\n", *merge_nav_num);
		fprintf(stderr, "dbg2       merge_nav_alloc:        %d\n", *merge_nav_alloc);
		fprintf(stderr, "dbg2       merge_nav_time_d *:     %p\n", *merge_nav_time_d);
		fprintf(stderr, "dbg2       merge_nav_lon *:        %p\n", *merge_nav_lon);
		fprintf(stderr, "dbg2       merge_nav_lat *:        %p\n", *merge_nav_lat);
		fprintf(stderr, "dbg2       merge_nav_speed *:      %p\n", *merge_nav_speed);
		fprintf(stderr, "dbg2       error:                  %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                 %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/

int mb_loadsensordepthdata(int verbose, char *merge_sensordepth_file, int merge_sensordepth_format, int *merge_sensordepth_num,
                           int *merge_sensordepth_alloc, double **merge_sensordepth_time_d,
                           double **merge_sensordepth_sensordepth, int *error) {
	char buffer[MBP_FILENAMESIZE], *result;
	int nrecord;
	int nchar, nget;
	size_t size;
	FILE *tfp;
	int time_i[7], time_j[6], ihr;
	double sec;
	double time_d, lon, lat, heading, speed, roll, pitch, heave;
	double *n_time_d;
	double *n_sensordepth;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                          %d\n", verbose);
		fprintf(stderr, "dbg2       merge_sensordepth_file:           %s\n", merge_sensordepth_file);
		fprintf(stderr, "dbg2       merge_sensordepth_format:         %d\n", merge_sensordepth_format);
		fprintf(stderr, "dbg2       merge_sensordepth_num *:          %p\n", merge_sensordepth_num);
		fprintf(stderr, "dbg2       merge_sensordepth_num:            %d\n", *merge_sensordepth_num);
		fprintf(stderr, "dbg2       merge_sensordepth_alloc *:        %p\n", merge_sensordepth_alloc);
		fprintf(stderr, "dbg2       merge_sensordepth_alloc:          %d\n", *merge_sensordepth_alloc);
		fprintf(stderr, "dbg2       merge_sensordepth_time_d **:      %p\n", merge_sensordepth_time_d);
		fprintf(stderr, "dbg2       merge_sensordepth_time_d *:       %p\n", *merge_sensordepth_time_d);
		fprintf(stderr, "dbg2       merge_sensordepth_sensordepth **: %p\n", merge_sensordepth_sensordepth);
		fprintf(stderr, "dbg2       merge_sensordepth_sensordepth *:  %p\n", *merge_sensordepth_sensordepth);
	}

	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE - 1;

	int status = MB_SUCCESS;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_sensordepth_file, "r")) != NULL) {
		/* loop over reading the records */
		while ((result = fgets(buffer, nchar, tfp)) == buffer)
			nrecord++;

		/* close the file */
		fclose(tfp);
		tfp = NULL;
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	}

	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_sensordepth_alloc < nrecord) {
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_sensordepth_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_sensordepth_sensordepth, error);
		if (status == MB_SUCCESS)
			*merge_sensordepth_alloc = nrecord;
		n_time_d = *merge_sensordepth_time_d;
		n_sensordepth = *merge_sensordepth_sensordepth;
	}

	/* read the records */
	if (status == MB_SUCCESS) {
		bool sensordepth_ok;
		nrecord = 0;
		if ((tfp = fopen(merge_sensordepth_file, "r")) == NULL) {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
		else {
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer, nchar, tfp)) == buffer) {
				sensordepth_ok = false;

				/* deal with sensordepth in form: time_d sensordepth */
				if (merge_sensordepth_format == 1) {
					nget = sscanf(buffer, "%lf %lf", &n_time_d[nrecord], &n_sensordepth[nrecord]);
					if (nget == 2)
						sensordepth_ok = true;
				}

				/* deal with sensordepth in form: yr mon day hour min sec sensordepth */
				else if (merge_sensordepth_format == 2) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3], &time_i[4],
					              &sec, &n_sensordepth[nrecord]);
					time_i[5] = (int)sec;
					time_i[6] = 1000000 * (sec - time_i[5]);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						sensordepth_ok = true;
				}

				/* deal with sensordepth in form: yr jday hour min sec sensordepth */
				else if (merge_sensordepth_format == 3) {
					nget = sscanf(buffer, "%d %d %d %d %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
					              &n_sensordepth[nrecord]);
					time_j[2] = time_j[2] + 60 * ihr;
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 6)
						sensordepth_ok = true;
				}

				/* deal with sensordepth in form: yr jday daymin sec sensordepth */
				else if (merge_sensordepth_format == 4) {
					nget = sscanf(buffer, "%d %d %d %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec, &n_sensordepth[nrecord]);
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 5)
						sensordepth_ok = true;
				}

				/* deal with sensordepth in form: yr mon day hour min sec time_d lon lat heading speed draft*/
				else if (merge_sensordepth_format == 9) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0], &time_i[1],
					              &time_i[2], &time_i[3], &time_i[4], &sec, &n_time_d[nrecord], &lon, &lat, &heading, &speed,
					              &n_sensordepth[nrecord], &roll, &pitch, &heave);
					if (nget >= 9)
						sensordepth_ok = true;
					if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1])
						sensordepth_ok = false;
				}

				/* output some debug values */
				if (verbose >= 5 && sensordepth_ok) {
					fprintf(stderr, "\ndbg5  New sensordepth point read in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       sensordepth[%d]: %f %f\n", nrecord, n_time_d[nrecord], n_sensordepth[nrecord]);
				}
				else if (verbose >= 5) {
					fprintf(stderr, "\ndbg5  Error parsing line in sensordepth file in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       line: %s\n", buffer);
				}

				/* check for reverses or repeats in time */
				if (sensordepth_ok) {
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord - 1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1] && verbose >= 5) {
						fprintf(stderr, "\ndbg5  sensordepth time error in function <%s>\n", __func__);
						fprintf(stderr, "dbg5       sensordepth[%d]: %f %f\n", nrecord - 1, n_time_d[nrecord - 1],
						        n_sensordepth[nrecord - 1]);
						fprintf(stderr, "dbg5       sensordepth[%d]: %f %f\n", nrecord, n_time_d[nrecord],
						        n_sensordepth[nrecord]);
					}
				}
				strncpy(buffer, "", sizeof(buffer));
			}

			/* get the good record count */
			*merge_sensordepth_num = nrecord;

			/* close the file */
			fclose(tfp);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       merge_sensordepth_num:            %d\n", *merge_sensordepth_num);
		fprintf(stderr, "dbg2       merge_sensordepth_alloc:          %d\n", *merge_sensordepth_alloc);
		fprintf(stderr, "dbg2       merge_sensordepth_time_d *:       %p\n", *merge_sensordepth_time_d);
		fprintf(stderr, "dbg2       merge_sensordepth_sensordepth *:  %p\n", *merge_sensordepth_sensordepth);
		fprintf(stderr, "dbg2       error:                            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/

int mb_loadaltitudedata(int verbose, char *merge_altitude_file, int merge_altitude_format, int *merge_altitude_num,
                        int *merge_altitude_alloc, double **merge_altitude_time_d, double **merge_altitude_altitude, int *error) {
	char buffer[MBP_FILENAMESIZE], *result;
	int nrecord;
	int nchar, nget;
	size_t size;
	FILE *tfp;
	int time_i[7], time_j[6], ihr;
	double sec;
	double time_d;
	double *n_time_d;
	double *n_altitude;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                          %d\n", verbose);
		fprintf(stderr, "dbg2       merge_altitude_file:              %s\n", merge_altitude_file);
		fprintf(stderr, "dbg2       merge_altitude_format:            %d\n", merge_altitude_format);
		fprintf(stderr, "dbg2       merge_altitude_num *:             %p\n", merge_altitude_num);
		fprintf(stderr, "dbg2       merge_altitude_num:               %d\n", *merge_altitude_num);
		fprintf(stderr, "dbg2       merge_altitude_alloc *:           %p\n", merge_altitude_alloc);
		fprintf(stderr, "dbg2       merge_altitude_alloc:             %d\n", *merge_altitude_alloc);
		fprintf(stderr, "dbg2       merge_altitude_time_d **:         %p\n", merge_altitude_time_d);
		fprintf(stderr, "dbg2       merge_altitude_time_d *:          %p\n", *merge_altitude_time_d);
		fprintf(stderr, "dbg2       merge_altitude_altitude **:       %p\n", merge_altitude_altitude);
		fprintf(stderr, "dbg2       merge_altitude_altitude *:        %p\n", *merge_altitude_altitude);
	}

	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE - 1;

	int status = MB_SUCCESS;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_altitude_file, "r")) != NULL) {
		/* loop over reading the records */
		while ((result = fgets(buffer, nchar, tfp)) == buffer)
			nrecord++;

		/* close the file */
		fclose(tfp);
		tfp = NULL;
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	}

	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_altitude_alloc < nrecord) {
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_altitude_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_altitude_altitude, error);
		if (status == MB_SUCCESS)
			*merge_altitude_alloc = nrecord;
		n_time_d = *merge_altitude_time_d;
		n_altitude = *merge_altitude_altitude;
	}

	/* read the records */
	if (status == MB_SUCCESS) {
		bool altitude_ok;
		nrecord = 0;
		if ((tfp = fopen(merge_altitude_file, "r")) == NULL) {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
		else {
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer, nchar, tfp)) == buffer) {
				altitude_ok = false;

				/* deal with altitude in form: time_d altitude */
				if (merge_altitude_format == 1) {
					nget = sscanf(buffer, "%lf %lf", &n_time_d[nrecord], &n_altitude[nrecord]);
					if (nget == 2)
						altitude_ok = true;
				}

				/* deal with altitude in form: yr mon day hour min sec altitude */
				else if (merge_altitude_format == 2) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3], &time_i[4],
					              &sec, &n_altitude[nrecord]);
					time_i[5] = (int)sec;
					time_i[6] = 1000000 * (sec - time_i[5]);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						altitude_ok = true;
				}

				/* deal with altitude in form: yr jday hour min sec altitude */
				else if (merge_altitude_format == 3) {
					nget = sscanf(buffer, "%d %d %d %d %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
					              &n_altitude[nrecord]);
					time_j[2] = time_j[2] + 60 * ihr;
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 6)
						altitude_ok = true;
				}

				/* deal with altitude in form: yr jday daymin sec altitude */
				else if (merge_altitude_format == 4) {
					nget = sscanf(buffer, "%d %d %d %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec, &n_altitude[nrecord]);
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 5)
						altitude_ok = true;
				}

				/* output some debug values */
				if (verbose >= 5 && altitude_ok) {
					fprintf(stderr, "\ndbg5  New altitude point read in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       altitude[%d]: %f %f\n", nrecord, n_time_d[nrecord], n_altitude[nrecord]);
				}
				else if (verbose >= 5) {
					fprintf(stderr, "\ndbg5  Error parsing line in altitude file in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       line: %s\n", buffer);
				}

				/* check for reverses or repeats in time */
				if (altitude_ok) {
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord - 1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1] && verbose >= 5) {
						fprintf(stderr, "\ndbg5  altitude time error in function <%s>\n", __func__);
						fprintf(stderr, "dbg5       altitude[%d]: %f %f\n", nrecord - 1, n_time_d[nrecord - 1],
						        n_altitude[nrecord - 1]);
						fprintf(stderr, "dbg5       altitude[%d]: %f %f\n", nrecord, n_time_d[nrecord], n_altitude[nrecord]);
					}
				}
				strncpy(buffer, "", sizeof(buffer));
			}

			/* get the good record count */
			*merge_altitude_num = nrecord;

			/* close the file */
			fclose(tfp);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       merge_altitude_num:               %d\n", *merge_altitude_num);
		fprintf(stderr, "dbg2       merge_altitude_alloc:             %d\n", *merge_altitude_alloc);
		fprintf(stderr, "dbg2       merge_altitude_time_d *:          %p\n", *merge_altitude_time_d);
		fprintf(stderr, "dbg2       merge_altitude_altitude *:        %p\n", *merge_altitude_altitude);
		fprintf(stderr, "dbg2       error:                            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/

int mb_loadheadingdata(int verbose, char *merge_heading_file, int merge_heading_format, int *merge_heading_num,
                       int *merge_heading_alloc, double **merge_heading_time_d, double **merge_heading_heading, int *error) {
	char buffer[MBP_FILENAMESIZE], *result;
	int nrecord;
	int nchar, nget;
	size_t size;
	FILE *tfp;
	int time_i[7], time_j[6], ihr;
	double sec;
	double time_d, lon, lat, sensordepth, speed, roll, pitch, heave;
	double *n_time_d;
	double *n_heading;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                          %d\n", verbose);
		fprintf(stderr, "dbg2       merge_heading_file:               %s\n", merge_heading_file);
		fprintf(stderr, "dbg2       merge_heading_format:             %d\n", merge_heading_format);
		fprintf(stderr, "dbg2       merge_heading_num *:              %p\n", merge_heading_num);
		fprintf(stderr, "dbg2       merge_heading_num:                %d\n", *merge_heading_num);
		fprintf(stderr, "dbg2       merge_heading_alloc *:            %p\n", merge_heading_alloc);
		fprintf(stderr, "dbg2       merge_heading_alloc:              %d\n", *merge_heading_alloc);
		fprintf(stderr, "dbg2       merge_heading_time_d **:          %p\n", merge_heading_time_d);
		fprintf(stderr, "dbg2       merge_heading_time_d *:           %p\n", *merge_heading_time_d);
		fprintf(stderr, "dbg2       merge_heading_heading **:         %p\n", merge_heading_heading);
		fprintf(stderr, "dbg2       merge_heading_heading *:          %p\n", *merge_heading_heading);
	}

	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE - 1;

	int status = MB_SUCCESS;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_heading_file, "r")) != NULL) {
		/* loop over reading the records */
		while ((result = fgets(buffer, nchar, tfp)) == buffer)
			nrecord++;

		/* close the file */
		fclose(tfp);
		tfp = NULL;
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	}

	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_heading_alloc < nrecord) {
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_heading_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_heading_heading, error);
		if (status == MB_SUCCESS)
			*merge_heading_alloc = nrecord;
		n_time_d = *merge_heading_time_d;
		n_heading = *merge_heading_heading;
	}

	/* read the records */
	if (status == MB_SUCCESS) {
		bool heading_ok;
		nrecord = 0;
		if ((tfp = fopen(merge_heading_file, "r")) == NULL) {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
		else {
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer, nchar, tfp)) == buffer) {
				heading_ok = false;

				/* deal with heading in form: time_d heading */
				if (merge_heading_format == 1) {
					nget = sscanf(buffer, "%lf %lf", &n_time_d[nrecord], &n_heading[nrecord]);
					if (nget == 2)
						heading_ok = true;
				}

				/* deal with heading in form: yr mon day hour min sec heading */
				else if (merge_heading_format == 2) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3], &time_i[4],
					              &sec, &n_heading[nrecord]);
					time_i[5] = (int)sec;
					time_i[6] = 1000000 * (sec - time_i[5]);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						heading_ok = true;
				}

				/* deal with heading in form: yr jday hour min sec heading */
				else if (merge_heading_format == 3) {
					nget = sscanf(buffer, "%d %d %d %d %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
					              &n_heading[nrecord]);
					time_j[2] = time_j[2] + 60 * ihr;
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 6)
						heading_ok = true;
				}

				/* deal with heading in form: yr jday daymin sec heading */
				else if (merge_heading_format == 4) {
					nget = sscanf(buffer, "%d %d %d %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec, &n_heading[nrecord]);
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 5)
						heading_ok = true;
				}

				/* deal with heading in form: yr mon day hour min sec time_d lon lat heading speed draft*/
				else if (merge_heading_format == 9) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0], &time_i[1],
					              &time_i[2], &time_i[3], &time_i[4], &sec, &n_time_d[nrecord], &lon, &lat, &n_heading[nrecord],
					              &speed, &sensordepth, &roll, &pitch, &heave);
					if (nget >= 9)
						heading_ok = true;
					if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1])
						heading_ok = false;
				}

				/* output some debug values */
				if (verbose >= 5 && heading_ok) {
					fprintf(stderr, "\ndbg5  New heading point read in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       heading[%d]: %f %f\n", nrecord, n_time_d[nrecord], n_heading[nrecord]);
				}
				else if (verbose >= 5) {
					fprintf(stderr, "\ndbg5  Error parsing line in heading file in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       line: %s\n", buffer);
				}

				/* check for reverses or repeats in time */
				if (heading_ok) {
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord - 1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1] && verbose >= 5) {
						fprintf(stderr, "\ndbg5  heading time error in function <%s>\n", __func__);
						fprintf(stderr, "dbg5       heading[%d]: %f %f\n", nrecord - 1, n_time_d[nrecord - 1],
						        n_heading[nrecord - 1]);
						fprintf(stderr, "dbg5       heading[%d]: %f %f\n", nrecord, n_time_d[nrecord], n_heading[nrecord]);
					}
				}
				strncpy(buffer, "", sizeof(buffer));
			}

			/* get the good record count */
			*merge_heading_num = nrecord;

			/* close the file */
			fclose(tfp);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       merge_heading_num:                %d\n", *merge_heading_num);
		fprintf(stderr, "dbg2       merge_heading_alloc:              %d\n", *merge_heading_alloc);
		fprintf(stderr, "dbg2       merge_heading_time_d *:           %p\n", *merge_heading_time_d);
		fprintf(stderr, "dbg2       merge_heading_heading *:          %p\n", *merge_heading_heading);
		fprintf(stderr, "dbg2       error:                            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/

int mb_loadattitudedata(int verbose, char *merge_attitude_file, int merge_attitude_format, int *merge_attitude_num,
                        int *merge_attitude_alloc, double **merge_attitude_time_d, double **merge_attitude_roll,
                        double **merge_attitude_pitch, double **merge_attitude_heave, int *error) {
	char buffer[MBP_FILENAMESIZE], *result;
	int nrecord;
	int nchar, nget;
	size_t size;
	FILE *tfp;
	bool attitude_ok;
	int time_i[7], time_j[6], ihr;
	double sec;
	double time_d, lon, lat, sensordepth, heading, speed;
	double *n_time_d;
	double *n_roll;
	double *n_pitch;
	double *n_heave;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                          %d\n", verbose);
		fprintf(stderr, "dbg2       merge_attitude_file:              %s\n", merge_attitude_file);
		fprintf(stderr, "dbg2       merge_attitude_format:            %d\n", merge_attitude_format);
		fprintf(stderr, "dbg2       merge_attitude_num *:             %p\n", merge_attitude_num);
		fprintf(stderr, "dbg2       merge_attitude_num:               %d\n", *merge_attitude_num);
		fprintf(stderr, "dbg2       merge_attitude_alloc *:           %p\n", merge_attitude_alloc);
		fprintf(stderr, "dbg2       merge_attitude_alloc:             %d\n", *merge_attitude_alloc);
		fprintf(stderr, "dbg2       merge_attitude_time_d **:         %p\n", merge_attitude_time_d);
		fprintf(stderr, "dbg2       merge_attitude_time_d *:          %p\n", *merge_attitude_time_d);
		fprintf(stderr, "dbg2       merge_attitude_roll **:           %p\n", merge_attitude_roll);
		fprintf(stderr, "dbg2       merge_attitude_roll *:            %p\n", *merge_attitude_roll);
		fprintf(stderr, "dbg2       merge_attitude_pitch **:          %p\n", merge_attitude_pitch);
		fprintf(stderr, "dbg2       merge_attitude_pitch *:           %p\n", *merge_attitude_pitch);
		fprintf(stderr, "dbg2       merge_attitude_heave **:          %p\n", merge_attitude_heave);
		fprintf(stderr, "dbg2       merge_attitude_heave *:           %p\n", *merge_attitude_heave);
	}

	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE - 1;

	int status = MB_SUCCESS;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_attitude_file, "r")) != NULL) {
		/* loop over reading the records */
		while ((result = fgets(buffer, nchar, tfp)) == buffer)
			nrecord++;

		/* close the file */
		fclose(tfp);
		tfp = NULL;
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	}

	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_attitude_alloc < nrecord) {
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_attitude_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_attitude_roll, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_attitude_pitch, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_attitude_heave, error);
		if (status == MB_SUCCESS)
			*merge_attitude_alloc = nrecord;
		n_time_d = *merge_attitude_time_d;
		n_roll = *merge_attitude_roll;
		n_pitch = *merge_attitude_pitch;
		n_heave = *merge_attitude_heave;
	}

	/* read the records */
	if (status == MB_SUCCESS) {
		nrecord = 0;
		if ((tfp = fopen(merge_attitude_file, "r")) == NULL) {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
		else {
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer, nchar, tfp)) == buffer) {
				attitude_ok = false;

				/* deal with attitude in form: time_d roll pitch heave */
				if (merge_attitude_format == 1) {
					nget = sscanf(buffer, "%lf %lf %lf %lf", &n_time_d[nrecord], &n_roll[nrecord], &n_pitch[nrecord],
					              &n_heave[nrecord]);
					if (nget == 4)
						attitude_ok = true;
				}

				/* deal with attitude in form: yr mon day hour min sec roll pitch heave */
				else if (merge_attitude_format == 2) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3],
					              &time_i[4], &sec, &n_roll[nrecord], &n_pitch[nrecord], &n_heave[nrecord]);
					time_i[5] = (int)sec;
					time_i[6] = 1000000 * (sec - time_i[5]);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 9)
						attitude_ok = true;
				}

				/* deal with attitude in form: yr jday hour min sec roll pitch heave */
				else if (merge_attitude_format == 3) {
					nget = sscanf(buffer, "%d %d %d %d %lf %lf %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
					              &n_roll[nrecord], &n_pitch[nrecord], &n_heave[nrecord]);
					time_j[2] = time_j[2] + 60 * ihr;
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 8)
						attitude_ok = true;
				}

				/* deal with attitude in form: yr jday daymin sec roll pitch heave */
				else if (merge_attitude_format == 4) {
					nget = sscanf(buffer, "%d %d %d %lf %lf %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec, &n_roll[nrecord],
					              &n_pitch[nrecord], &n_heave[nrecord]);
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						attitude_ok = true;
				}

				/* deal with attitude in form: yr mon day hour min sec time_d lon lat heading speed sensordepth roll pitch heave
				 */
				else if (merge_attitude_format == 9) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0], &time_i[1],
					              &time_i[2], &time_i[3], &time_i[4], &sec, &n_time_d[nrecord], &lon, &lat, &heading, &speed,
					              &sensordepth, &n_roll[nrecord], &n_pitch[nrecord], &n_heave[nrecord]);
					if (nget >= 9)
						attitude_ok = true;
					if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1])
						attitude_ok = false;
				}

				/* output some debug values */
				if (verbose >= 5 && attitude_ok) {
					fprintf(stderr, "\ndbg5  New attitude point read in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       attitude[%d]: %f %f %f %f\n", nrecord, n_time_d[nrecord], n_roll[nrecord],
					        n_pitch[nrecord], n_heave[nrecord]);
				}
				else if (verbose >= 5) {
					fprintf(stderr, "\ndbg5  Error parsing line in attitude file in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       line: %s\n", buffer);
				}

				/* check for reverses or repeats in time */
				if (attitude_ok) {
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord - 1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1] && verbose >= 5) {
						fprintf(stderr, "\ndbg5  attitude time error in function <%s>\n", __func__);
						fprintf(stderr, "dbg5       attitude[%d]: %f %f %f %f\n", nrecord - 1, n_time_d[nrecord - 1],
						        n_roll[nrecord - 1], n_pitch[nrecord - 1], n_heave[nrecord - 1]);
						fprintf(stderr, "dbg5       attitude[%d]: %f %f %f %f\n", nrecord, n_time_d[nrecord], n_roll[nrecord],
						        n_pitch[nrecord], n_heave[nrecord]);
					}
				}
				strncpy(buffer, "", sizeof(buffer));
			}

			/* get the good record count */
			*merge_attitude_num = nrecord;

			/* close the file */
			fclose(tfp);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       merge_attitude_num:               %d\n", *merge_attitude_num);
		fprintf(stderr, "dbg2       merge_attitude_alloc:             %d\n", *merge_attitude_alloc);
		fprintf(stderr, "dbg2       merge_attitude_time_d *:          %p\n", *merge_attitude_time_d);
		fprintf(stderr, "dbg2       merge_attitude_roll *:            %p\n", *merge_attitude_roll);
		fprintf(stderr, "dbg2       merge_attitude_pitch *:           %p\n", *merge_attitude_pitch);
		fprintf(stderr, "dbg2       merge_attitude_heave *:           %p\n", *merge_attitude_heave);
		fprintf(stderr, "dbg2       error:                            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                           %d\n", status);
	}

	/* return success */
	return (status);
}
/*--------------------------------------------------------------------*/

int mb_loadsoundspeeddata(int verbose, char *merge_soundspeed_file, int merge_soundspeed_format, int *merge_soundspeed_num,
                          int *merge_soundspeed_alloc, double **merge_soundspeed_time_d, double **merge_soundspeed_soundspeed,
                          int *error) {
	char buffer[MBP_FILENAMESIZE], *result;
	int nrecord;
	int nchar, nget;
	size_t size;
	FILE *tfp;
	bool soundspeed_ok;
	int time_i[7], time_j[6], ihr;
	double sec;
	double time_d;
	double *n_time_d;
	double *n_soundspeed;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                          %d\n", verbose);
		fprintf(stderr, "dbg2       merge_soundspeed_file:            %s\n", merge_soundspeed_file);
		fprintf(stderr, "dbg2       merge_soundspeed_format:          %d\n", merge_soundspeed_format);
		fprintf(stderr, "dbg2       merge_soundspeed_num *:           %p\n", merge_soundspeed_num);
		fprintf(stderr, "dbg2       merge_soundspeed_num:             %d\n", *merge_soundspeed_num);
		fprintf(stderr, "dbg2       merge_soundspeed_alloc *:         %p\n", merge_soundspeed_alloc);
		fprintf(stderr, "dbg2       merge_soundspeed_alloc:           %d\n", *merge_soundspeed_alloc);
		fprintf(stderr, "dbg2       merge_soundspeed_time_d **:       %p\n", merge_soundspeed_time_d);
		fprintf(stderr, "dbg2       merge_soundspeed_time_d *:        %p\n", *merge_soundspeed_time_d);
		fprintf(stderr, "dbg2       merge_soundspeed_soundspeed **:   %p\n", merge_soundspeed_soundspeed);
		fprintf(stderr, "dbg2       merge_soundspeed_soundspeed *:    %p\n", *merge_soundspeed_soundspeed);
	}

	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE - 1;

	int status = MB_SUCCESS;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_soundspeed_file, "r")) != NULL) {
		/* loop over reading the records */
		while ((result = fgets(buffer, nchar, tfp)) == buffer)
			nrecord++;

		/* close the file */
		fclose(tfp);
		tfp = NULL;
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	}

	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_soundspeed_alloc < nrecord) {
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_soundspeed_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_soundspeed_soundspeed, error);
		if (status == MB_SUCCESS)
			*merge_soundspeed_alloc = nrecord;
		n_time_d = *merge_soundspeed_time_d;
		n_soundspeed = *merge_soundspeed_soundspeed;
	}

	/* read the records */
	if (status == MB_SUCCESS) {
		nrecord = 0;
		if ((tfp = fopen(merge_soundspeed_file, "r")) == NULL) {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
		else {
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer, nchar, tfp)) == buffer) {
				soundspeed_ok = false;

				/* deal with soundspeed in form: time_d soundspeed */
				if (merge_soundspeed_format == 1) {
					nget = sscanf(buffer, "%lf %lf", &n_time_d[nrecord], &n_soundspeed[nrecord]);
					if (nget == 2)
						soundspeed_ok = true;
				}

				/* deal with soundspeed in form: yr mon day hour min sec soundspeed */
				else if (merge_soundspeed_format == 2) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3], &time_i[4],
					              &sec, &n_soundspeed[nrecord]);
					time_i[5] = (int)sec;
					time_i[6] = 1000000 * (sec - time_i[5]);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						soundspeed_ok = true;
				}

				/* deal with soundspeed in form: yr jday hour min sec soundspeed */
				else if (merge_soundspeed_format == 3) {
					nget = sscanf(buffer, "%d %d %d %d %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
					              &n_soundspeed[nrecord]);
					time_j[2] = time_j[2] + 60 * ihr;
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 6)
						soundspeed_ok = true;
				}

				/* deal with soundspeed in form: yr jday daymin sec soundspeed */
				else if (merge_soundspeed_format == 4) {
					nget = sscanf(buffer, "%d %d %d %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec, &n_soundspeed[nrecord]);
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 5)
						soundspeed_ok = true;
				}

				/* output some debug values */
				if (verbose >= 5 && soundspeed_ok) {
					fprintf(stderr, "\ndbg5  New soundspeed point read in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       soundspeed[%d]: %f %f\n", nrecord, n_time_d[nrecord], n_soundspeed[nrecord]);
				}
				else if (verbose >= 5) {
					fprintf(stderr, "\ndbg5  Error parsing line in soundspeed file in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       line: %s\n", buffer);
				}

				/* check for reverses or repeats in time */
				if (soundspeed_ok) {
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord - 1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1] && verbose >= 5) {
						fprintf(stderr, "\ndbg5  soundspeed time error in function <%s>\n", __func__);
						fprintf(stderr, "dbg5       soundspeed[%d]: %f %f\n", nrecord - 1, n_time_d[nrecord - 1],
						        n_soundspeed[nrecord - 1]);
						fprintf(stderr, "dbg5       soundspeed[%d]: %f %f\n", nrecord, n_time_d[nrecord], n_soundspeed[nrecord]);
					}
				}
				strncpy(buffer, "", sizeof(buffer));
			}

			/* get the good record count */
			*merge_soundspeed_num = nrecord;

			/* close the file */
			fclose(tfp);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       merge_soundspeed_num:             %d\n", *merge_soundspeed_num);
		fprintf(stderr, "dbg2       merge_soundspeed_alloc:           %d\n", *merge_soundspeed_alloc);
		fprintf(stderr, "dbg2       merge_soundspeed_time_d *:        %p\n", *merge_soundspeed_time_d);
		fprintf(stderr, "dbg2       merge_soundspeed_soundspeed *:    %p\n", *merge_soundspeed_soundspeed);
		fprintf(stderr, "dbg2       error:                            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/

int mb_loadtimeshiftdata(int verbose, char *merge_timeshift_file, int merge_timeshift_format, int *merge_timeshift_num,
                         int *merge_timeshift_alloc, double **merge_timeshift_time_d, double **merge_timeshift_timeshift,
                         int *error) {
	char buffer[MBP_FILENAMESIZE], *result;
	int nrecord;
	int nchar, nget;
	size_t size;
	FILE *tfp;
	bool timeshift_ok;
	int time_i[7], time_j[6], ihr;
	double sec;
	double time_d;
	double *n_time_d;
	double *n_timeshift;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                          %d\n", verbose);
		fprintf(stderr, "dbg2       merge_timeshift_file:             %s\n", merge_timeshift_file);
		fprintf(stderr, "dbg2       merge_timeshift_format:           %d\n", merge_timeshift_format);
		fprintf(stderr, "dbg2       merge_timeshift_num *:            %p\n", merge_timeshift_num);
		fprintf(stderr, "dbg2       merge_timeshift_num:              %d\n", *merge_timeshift_num);
		fprintf(stderr, "dbg2       merge_timeshift_alloc *:          %p\n", merge_timeshift_alloc);
		fprintf(stderr, "dbg2       merge_timeshift_alloc:            %d\n", *merge_timeshift_alloc);
		fprintf(stderr, "dbg2       merge_timeshift_time_d **:        %p\n", merge_timeshift_time_d);
		fprintf(stderr, "dbg2       merge_timeshift_time_d *:         %p\n", *merge_timeshift_time_d);
		fprintf(stderr, "dbg2       merge_timeshift_timeshift **:     %p\n", merge_timeshift_timeshift);
		fprintf(stderr, "dbg2       merge_timeshift_timeshift *:      %p\n", *merge_timeshift_timeshift);
	}

	/* set max number of characters to be read at a time */
	nchar = MBP_FILENAMESIZE - 1;

	int status = MB_SUCCESS;

	/* count the records */
	*error = MB_ERROR_NO_ERROR;
	nrecord = 0;
	if ((tfp = fopen(merge_timeshift_file, "r")) != NULL) {
		/* loop over reading the records */
		while ((result = fgets(buffer, nchar, tfp)) == buffer)
			nrecord++;

		/* close the file */
		fclose(tfp);
		tfp = NULL;
	}
	else {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	}

	/* allocate memory if necessary */
	if (status == MB_SUCCESS && *merge_timeshift_alloc < nrecord) {
		size = nrecord * sizeof(double);
		status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_timeshift_time_d, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)merge_timeshift_timeshift, error);
		if (status == MB_SUCCESS)
			*merge_timeshift_alloc = nrecord;
		n_time_d = *merge_timeshift_time_d;
		n_timeshift = *merge_timeshift_timeshift;
	}

	/* read the records */
	if (status == MB_SUCCESS) {
		nrecord = 0;
		if ((tfp = fopen(merge_timeshift_file, "r")) == NULL) {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
		else {
			/* loop over reading the records - handle the different formats */
			while ((result = fgets(buffer, nchar, tfp)) == buffer) {
				timeshift_ok = false;

				/* deal with timeshift in form: time_d timeshift */
				if (merge_timeshift_format == 1) {
					nget = sscanf(buffer, "%lf %lf", &n_time_d[nrecord], &n_timeshift[nrecord]);
					if (nget == 2)
						timeshift_ok = true;
				}

				/* deal with timeshift in form: yr mon day hour min sec timeshift */
				else if (merge_timeshift_format == 2) {
					nget = sscanf(buffer, "%d %d %d %d %d %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3], &time_i[4],
					              &sec, &n_timeshift[nrecord]);
					time_i[5] = (int)sec;
					time_i[6] = 1000000 * (sec - time_i[5]);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 7)
						timeshift_ok = true;
				}

				/* deal with timeshift in form: yr jday hour min sec timeshift */
				else if (merge_timeshift_format == 3) {
					nget = sscanf(buffer, "%d %d %d %d %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
					              &n_timeshift[nrecord]);
					time_j[2] = time_j[2] + 60 * ihr;
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 6)
						timeshift_ok = true;
				}

				/* deal with timeshift in form: yr jday daymin sec timeshift */
				else if (merge_timeshift_format == 4) {
					nget = sscanf(buffer, "%d %d %d %lf %lf", &time_j[0], &time_j[1], &time_j[2], &sec, &n_timeshift[nrecord]);
					time_j[3] = (int)sec;
					time_j[4] = 1000000 * (sec - time_j[3]);
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					n_time_d[nrecord] = time_d;
					if (nget == 5)
						timeshift_ok = true;
				}

				/* output some debug values */
				if (verbose >= 5 && timeshift_ok) {
					fprintf(stderr, "\ndbg5  New timeshift point read in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       timeshift[%d]: %f %f\n", nrecord, n_time_d[nrecord], n_timeshift[nrecord]);
				}
				else if (verbose >= 5) {
					fprintf(stderr, "\ndbg5  Error parsing line in timeshift file in function <%s>\n", __func__);
					fprintf(stderr, "dbg5       line: %s\n", buffer);
				}

				/* check for reverses or repeats in time */
				if (timeshift_ok) {
					if (nrecord == 0)
						nrecord++;
					else if (n_time_d[nrecord] > n_time_d[nrecord - 1])
						nrecord++;
					else if (nrecord > 0 && n_time_d[nrecord] <= n_time_d[nrecord - 1] && verbose >= 5) {
						fprintf(stderr, "\ndbg5  timeshift time error in function <%s>\n", __func__);
						fprintf(stderr, "dbg5       timeshift[%d]: %f %f\n", nrecord - 1, n_time_d[nrecord - 1],
						        n_timeshift[nrecord - 1]);
						fprintf(stderr, "dbg5       timeshift[%d]: %f %f\n", nrecord, n_time_d[nrecord], n_timeshift[nrecord]);
					}
				}
				strncpy(buffer, "", sizeof(buffer));
			}

			/* get the good record count */
			*merge_timeshift_num = nrecord;

			/* close the file */
			fclose(tfp);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       merge_timeshift_num:              %d\n", *merge_timeshift_num);
		fprintf(stderr, "dbg2       merge_timeshift_alloc:            %d\n", *merge_timeshift_alloc);
		fprintf(stderr, "dbg2       merge_timeshift_time_d *:         %p\n", *merge_timeshift_time_d);
		fprintf(stderr, "dbg2       merge_timeshift_timeshift *:      %p\n", *merge_timeshift_timeshift);
		fprintf(stderr, "dbg2       error:                            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/

int mb_apply_time_latency(int verbose, int data_num, double *data_time_d, int time_latency_mode, double time_latency_static,
                          int time_latency_num, double *time_latency_time_d, double *time_latency_value, int *error) {
	double time_latency;
	int interp_error = MB_ERROR_NO_ERROR;
	int j;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                          %d\n", verbose);
		fprintf(stderr, "dbg2       data_num:                         %d\n", data_num);
		fprintf(stderr, "dbg2       data_time_d:                      %p\n", data_time_d);
		fprintf(stderr, "dbg2       time_latency_mode:                %d\n", time_latency_mode);
		fprintf(stderr, "dbg2       time_latency_static:              %f\n", time_latency_static);
		fprintf(stderr, "dbg2       time_latency_num:                 %d\n", time_latency_num);
		fprintf(stderr, "dbg2       time_latency_time_d:              %p\n", time_latency_time_d);
		fprintf(stderr, "dbg2       time_latency_value:               %p\n", time_latency_value);
		for (int i = 0; i < time_latency_num; i++)
			fprintf(stderr, "dbg2          time_latency[%d]:              %f %f\n", i, time_latency_time_d[i],
			        time_latency_value[i]);
	}

	/* apply time_latency model to time data */
	if (time_latency_mode == MB_SENSOR_TIME_LATENCY_MODEL) {
		j = 0;
		for (int i = 0; i < data_num; i++) {
			/* int interp_status = */ mb_linear_interp(verbose, time_latency_time_d - 1, time_latency_value - 1, time_latency_num,
			                                 data_time_d[i], &time_latency, &j, &interp_error);
			data_time_d[i] -= time_latency;
		}
	}
	else if (time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC) {
		for (int i = 0; i < data_num; i++) {
			data_time_d[i] -= time_latency_static;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:                            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/

int mb_apply_time_filter(int verbose, int data_num, double *data_time_d, double *data_value, double filter_length, int *error) {
	double *data_value_filtered = NULL;
	double dtime, dtol, filterweight, weight;
	int nhalffilter;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                          %d\n", verbose);
		fprintf(stderr, "dbg2       data_num:                         %d\n", data_num);
		fprintf(stderr, "dbg2       data_time_d:                      %p\n", data_time_d);
		fprintf(stderr, "dbg2       data_value:                       %p\n", data_value);
		fprintf(stderr, "dbg2       filter_length:                    %f\n", filter_length);
	}

	/* apply a Gaussian time domain filter to the time series provided */
	const size_t size = data_num * sizeof(double);
	int status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&data_value_filtered, error);
	if (status == MB_SUCCESS) {
		dtime = (data_time_d[data_num - 1] - data_time_d[0]) / data_num;
		nhalffilter = (int)(4.0 * filter_length / dtime);
		for (int i = 0; i < data_num; i++) {
			data_value_filtered[i] = 0.0;
			filterweight = 0.0;
			const int j1 = MAX(i - nhalffilter, 0);
			const int j2 = MIN(i + nhalffilter, data_num - 1);
			for (int j = j1; j <= j2; j++) {
				dtol = (data_time_d[j] - data_time_d[i]) / filter_length;
				weight = exp(-dtol * dtol);
				data_value_filtered[i] += weight * data_value[j];
				filterweight += weight;
			}
			if (filterweight > 0.0)
				data_value_filtered[i] /= filterweight;
		}
		for (int i = 0; i < data_num; i++) {
			data_value[i] = data_value_filtered[i];
		}
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&data_value_filtered, error);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:                            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                           %d\n", status);
	}

	/* return success */
	return (status);
}

/*--------------------------------------------------------------------*/
