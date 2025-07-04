/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_elac.c	3.00	8/20/94
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
 * mbsys_elac.c contains the functions for handling the data structure
 * used by MBIO functions to store data
 * from Elac BottomChart Mark II multibeam sonar systems.
 * The data formats which are commonly used to store Elac
 * data in files include
 *      MBF_ELMK2UNB : MBIO ID 92
 *
 * Author:	D. W. Caress
 * Date:	August 20, 1994
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_elacmk2.h"

/*--------------------------------------------------------------------*/
int mbsys_elacmk2_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_elacmk2_struct), store_ptr, error);

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)*store_ptr;

	/* initialize everything */
	store->kind = MB_DATA_NONE;
	store->sonar = MBSYS_ELACMK2_UNKNOWN;

	/* parameter telegram */
	store->par_year = 0;
	store->par_month = 0;
	store->par_day = 0;
	store->par_hour = 0;
	store->par_minute = 0;
	store->par_second = 0;
	store->par_hundredth_sec = 0;
	store->par_thousandth_sec = 0;
	store->roll_offset = 0;    /* roll offset (degrees) */
	store->pitch_offset = 0;   /* pitch offset (degrees) */
	store->heading_offset = 0; /* heading offset (degrees) */
	store->time_delay = 0;     /* positioning system delay (sec) */
	store->transducer_port_height = 0;
	store->transducer_starboard_height = 0;
	store->transducer_port_depth = 0;
	store->transducer_starboard_depth = 0;
	store->transducer_port_x = 0;
	store->transducer_starboard_x = 0;
	store->transducer_port_y = 0;
	store->transducer_starboard_y = 0;
	store->transducer_port_error = 0;
	store->transducer_starboard_error = 0;
	store->antenna_height = 0;
	store->antenna_x = 0;
	store->antenna_y = 0;
	store->vru_height = 0;
	store->vru_x = 0;
	store->vru_y = 0;
	store->line_number = 0;
	store->start_or_stop = 0;
	store->transducer_serial_number = 0;
	for (int i = 0; i < MBSYS_ELACMK2_COMMENT_LENGTH; i++)
		store->comment[i] = '\0';

	/* position (position telegrams) */
	store->pos_year = 0;
	store->pos_month = 0;
	store->pos_day = 0;
	store->pos_hour = 0;
	store->pos_minute = 0;
	store->pos_second = 0;
	store->pos_hundredth_sec = 0;
	store->pos_thousandth_sec = 0;
	store->pos_latitude = 0;
	store->pos_longitude = 0;
	store->utm_northing = 0;
	store->utm_easting = 0;
	store->utm_zone_lon = 0;
	store->utm_zone = 0;
	store->hemisphere = 0;
	store->ellipsoid = 0;
	store->pos_spare = 0;
	store->semi_major_axis = 0;
	store->other_quality = 0;

	/* sound velocity profile */
	store->svp_year = 0;
	store->svp_month = 0;
	store->svp_day = 0;
	store->svp_hour = 0;
	store->svp_minute = 0;
	store->svp_second = 0;
	store->svp_hundredth_sec = 0;
	store->svp_thousandth_sec = 0;
	store->svp_num = 0;
	for (int i = 0; i < 500; i++) {
		store->svp_depth[i] = 0; /* 0.1 meters */
		store->svp_vel[i] = 0;   /* 0.1 meters/sec */
	}

	/* depth telegram */
	store->year = 0;
	store->month = 0;
	store->day = 0;
	store->hour = 0;
	store->minute = 0;
	store->second = 0;
	store->hundredth_sec = 0;
	store->thousandth_sec = 0;
	store->longitude = 0.0;
	store->latitude = 0.0;
	store->speed = 0.0;
	store->ping_num = 0;
	store->sound_vel = 0;
	store->heading = 0;
	store->pulse_length = 0;
	store->mode = 0;
	store->pulse_length = 0;
	store->source_power = 0;
	store->receiver_gain_stbd = 0;
	store->receiver_gain_port = 0;
	store->reserved = 0;
	store->beams_bath = 0;
	for (int i = 0; i < MBSYS_ELACMK2_MAXBEAMS; i++) {
		store->beams[i].bath = 0;
		store->beams[i].bath_acrosstrack = 0;
		store->beams[i].bath_alongtrack = 0;
		store->beams[i].tt = 0;
		store->beams[i].quality = 0;
		store->beams[i].amplitude = 0;
		store->beams[i].time_offset = 0;
		store->beams[i].heave = 0;
		store->beams[i].roll = 0;
		store->beams[i].pitch = 0;
		store->beams[i].angle = 0;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* deallocate memory for data structure */
	const int status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		*nbath = store->beams_bath;
		*namp = store->beams_bath;
		*nss = 0;
	}
	else {
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		fprintf(stderr, "dbg2        namp:      %d\n", *namp);
		fprintf(stderr, "dbg2        nss:       %d\n", *nss);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                          double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                          double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
	(void)ss;  // Unused arg
	(void)ssacrosstrack;  // Unused arg
	(void)ssalongtrack;  // Unused arg
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		mb_fix_y2k(verbose, store->year, &time_i[0]);
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = 10000 * store->hundredth_sec + 100 * store->thousandth_sec;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		*navlon = store->longitude;
		*navlat = store->latitude;

		/* get heading */
		*heading = 0.01 * store->heading;

		/* get speed  */
		*speed = 3.6 * store->speed;

		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 2.8;
		mb_io_ptr->beamwidth_xtrack = 1.5;

		/* read distance and depth values into storage arrays */
		*nbath = store->beams_bath;
		*namp = store->beams_bath;
		*nss = 0;
		double depthscale = 0.01;
		double dacrscale = -0.01;
		double daloscale = 0.01;
		double reflscale = 1.0;
		for (int i = 0; i < store->beams_bath; i++) {
			const int j = store->beams_bath - i - 1;
			if (store->beams[j].quality == 1)
				beamflag[i] = MB_FLAG_NONE;
			else if (store->beams[j].quality < 8)
				beamflag[i] = (char)(MB_FLAG_SONAR | MB_FLAG_FLAG);
			else if (store->beams[j].quality == 8)
				beamflag[i] = MB_FLAG_NULL;
			else if (store->beams[j].quality == 10)
				beamflag[i] = (char)(MB_FLAG_MANUAL | MB_FLAG_FLAG);
			else if (store->beams[j].quality == 20)
				beamflag[i] = (char)(MB_FLAG_FILTER | MB_FLAG_FLAG);
			else
				beamflag[i] = MB_FLAG_NULL;
			bath[i] = depthscale * store->beams[j].bath;
			bathacrosstrack[i] = dacrscale * store->beams[j].bath_acrosstrack;
			bathalongtrack[i] = daloscale * store->beams[j].bath_alongtrack;
			amp[i] = reflscale * store->beams[j].amplitude;
		}

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
			fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
			fprintf(stderr, "dbg4       speed:      %f\n", *speed);
			fprintf(stderr, "dbg4       heading:    %f\n", *heading);
			fprintf(stderr, "dbg4       nbath:      %d\n", *nbath);
			for (int i = 0; i < *nbath; i++)
				fprintf(stderr, "dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
			fprintf(stderr, "dbg4        namp:     %d\n", *namp);
			for (int i = 0; i < *namp; i++)
				fprintf(stderr, "dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		mb_fix_y2k(verbose, store->pos_year, &time_i[0]);
		time_i[1] = store->pos_month;
		time_i[2] = store->pos_day;
		time_i[3] = store->pos_hour;
		time_i[4] = store->pos_minute;
		time_i[5] = store->pos_second;
		time_i[6] = 10000 * store->pos_hundredth_sec + 100 * store->pos_thousandth_sec;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		*navlon = 0.00000009 * store->pos_longitude;
		*navlat = 0.00000009 * store->pos_latitude;

		/* get heading */
		*heading = 0.01 * store->heading;

		/* get speed  */
		*speed = 0.0;

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
			fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
			fprintf(stderr, "dbg4       speed:      %f\n", *speed);
			fprintf(stderr, "dbg4       heading:    %f\n", *heading);
			fprintf(stderr, "dbg4       nbath:      %d\n", *nbath);
			fprintf(stderr, "dbg4        namp:     %d\n", *namp);
		}

		/* done translating values */
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* copy comment */
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
		strncpy(comment, store->comment, MIN(MB_COMMENT_MAXLINE, MBSYS_ELACMK2_COMMENT_LENGTH) - 1);

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  New ping read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New ping values:\n");
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       comment:    %s\n", comment);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
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
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		for (int i = 0; i < *nbath; i++)
			fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
			        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2        namp:     %d\n", *namp);
		for (int i = 0; i < *namp; i++)
			fprintf(stderr, "dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
			        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:      %d\n", *nss);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                         double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                         double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                         double *ssalongtrack, char *comment, int *error) {
	(void)nss;  // Unused arg
	(void)ss;  // Unused arg
	(void)ssacrosstrack;  // Unused arg
	(void)ssalongtrack;  // Unused arg
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
	}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV)) {
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", speed);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
	}
	if (verbose >= 2 && kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
		if (verbose >= 3)
			for (int i = 0; i < nbath; i++)
				fprintf(stderr, "dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2       namp:       %d\n", namp);
		if (verbose >= 3)
			for (int i = 0; i < namp; i++)
				fprintf(stderr, "dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		mb_unfix_y2k(verbose, time_i[0], &store->year);
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];
		store->hundredth_sec = time_i[6] / 10000;
		store->thousandth_sec = (time_i[6] - 10000 * store->hundredth_sec) / 100;

		/*get navigation */
		store->longitude = navlon;
		store->latitude = navlat;

		/* get heading */
		store->heading = (int)(heading * 100);

		/* get speed */
		store->speed = speed / 3.6;

		/* insert distance and depth values into storage arrays */
		if (store->beams_bath == nbath) {
			double depthscale = 0.01;
			double dacrscale = -0.01;
			double daloscale = 0.01;
			double reflscale = 1.0;
			for (int i = 0; i < store->beams_bath; i++) {
				const int j = store->beams_bath - i - 1;
				if (mb_beam_check_flag(beamflag[i])) {
					if (mb_beam_check_flag_null(beamflag[i]))
						store->beams[j].quality = 8;
					else if (mb_beam_check_flag_manual(beamflag[i]))
						store->beams[j].quality = 10;
					else if (mb_beam_check_flag_filter(beamflag[i]))
						store->beams[j].quality = 20;
					else if (store->beams[j].quality == 1)
						store->beams[j].quality = 7;
				}
				else
					store->beams[j].quality = 1;
				store->beams[j].bath = (unsigned int)fabs(bath[i] / depthscale);
				store->beams[j].bath_acrosstrack = (int)(bathacrosstrack[i] / dacrscale);
				store->beams[j].bath_alongtrack = (int)(bathalongtrack[i] / daloscale);
				store->beams[j].amplitude = (int)(amp[i] / reflscale);
			}
		}
	}

	/* insert data in structure */
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		mb_unfix_y2k(verbose, time_i[0], &store->pos_year);
		store->pos_month = time_i[1];
		store->pos_day = time_i[2];
		store->pos_hour = time_i[3];
		store->pos_minute = time_i[4];
		store->pos_second = time_i[5];
		store->pos_hundredth_sec = time_i[6] / 10000;
		store->pos_thousandth_sec = (time_i[6] - 10000 * store->hundredth_sec) / 100;

		/*get navigation */
		store->pos_longitude = navlon / 0.00000009;
		store->pos_latitude = navlat / 0.00000009;

		/* get heading */
		store->heading = (int)(heading * 100);
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
    memset((void *)store->comment, 0, MBSYS_ELACMK2_COMMENT_LENGTH);
    strncpy(store->comment, comment, MIN(MBSYS_ELACMK2_COMMENT_LENGTH, MB_COMMENT_MAXLINE) - 1);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                         double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                         double *ssv, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       ttimes:     %p\n", (void *)ttimes);
		fprintf(stderr, "dbg2       angles_xtrk:%p\n", (void *)angles);
		fprintf(stderr, "dbg2       angles_ltrk:%p\n", (void *)angles_forward);
		fprintf(stderr, "dbg2       angles_null:%p\n", (void *)angles_null);
		fprintf(stderr, "dbg2       heave:      %p\n", (void *)heave);
		fprintf(stderr, "dbg2       ltrk_off:   %p\n", (void *)alongtrack_offset);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = store->beams_bath;

		/* get draft */
		*draft = 0.005 * (store->transducer_starboard_depth + store->transducer_port_depth);

		/* get ssv */
		*ssv = 0.1 * store->sound_vel;

		/* get travel times, angles */
		const double daloscale = 0.01;
		const double ttscale = 0.0001;
		const double angscale = 0.005;
		for (int i = 0; i < *nbeams; i++) {
			const int j = store->beams_bath - i - 1;
			ttimes[i] = ttscale * store->beams[j].tt;
			const double angle = 90.0 + angscale * store->beams[j].angle;
			const double pitch = angscale * store->beams[j].pitch;
			mb_rollpitch_to_takeoff(verbose, pitch, angle, &angles[i], &angles_forward[i], error);
			if (store->beams[j].angle < 0) {
				angles_null[i] = 37.5 + angscale * store->transducer_port_error;
			}
			else {
				angles_null[i] = 37.5 + angscale * store->transducer_starboard_error;
			}
			heave[i] = 0.001 * store->beams[j].heave;
			alongtrack_offset[i] = daloscale * store->beams[j].bath_alongtrack;
			;
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}
	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}
	/* deal with other record type */
	else {
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       draft:      %f\n", *draft);
		fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  heave:%f  ltrk_off:%f\n", i,
			        ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = store->beams_bath;

		/* get detects */
		for (int i = 0; i < *nbeams; i++) {
			detects[i] = MB_DETECT_AMPLITUDE;
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: detect:%d\n", i, detects[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                   double *altitude, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get draft */
		*transducer_depth = 0.005 * (store->transducer_starboard_depth + store->transducer_port_depth);
		const double depthscale = 0.01;
		const double dacrscale = -0.01;
		double bath_best = 0.0;
		if (store->beams[store->beams_bath / 2].quality == 1)
			bath_best = depthscale * store->beams[store->beams_bath / 2].bath;
		else {
			double xtrack_min = 99999999.9;
			for (int i = 0; i < store->beams_bath; i++) {
				if (store->beams[i].quality == 1 && fabs(dacrscale * store->beams[i].bath_acrosstrack) < xtrack_min) {
					xtrack_min = fabs(dacrscale * store->beams[i].bath_acrosstrack);
					bath_best = depthscale * store->beams[i].bath;
				}
			}
		}
		if (bath_best <= 0.0) {
			double xtrack_min = 99999999.9;
			for (int i = 0; i < store->beams_bath; i++) {
				if (store->beams[i].quality < 8 && fabs(dacrscale * store->beams[i].bath_acrosstrack) < xtrack_min) {
					xtrack_min = fabs(dacrscale * store->beams[i].bath_acrosstrack);
					bath_best = depthscale * store->beams[i].bath;
				}
			}
		}
		*altitude = bath_best - *transducer_depth;

		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                              double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                              double *pitch, double *heave, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		mb_fix_y2k(verbose, store->year, &time_i[0]);
		time_i[1] = store->month;
		time_i[2] = store->day;
		time_i[3] = store->hour;
		time_i[4] = store->minute;
		time_i[5] = store->second;
		time_i[6] = 10000 * store->hundredth_sec + 100 * store->thousandth_sec;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		*navlon = store->longitude;
		*navlat = store->latitude;

		/* get heading */
		*heading = 0.01 * store->heading;

		/* get speed  */
		*speed = 3.6 * store->speed;

		/* get draft */
		*draft = 0.005 * (store->transducer_starboard_depth + store->transducer_port_depth);

		/* get roll pitch and heave */
		if (store->beams_bath > 4) {
			*roll = 0.005 * store->beams[4].roll;
			*pitch = 0.005 * store->beams[4].pitch;
			*heave = 0.001 * store->beams[4].heave;
		}
		else {
			*roll = 0.0;
			*pitch = 0.0;
			*heave = 0.0;
		}

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
			fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
			fprintf(stderr, "dbg4       speed:      %f\n", *speed);
			fprintf(stderr, "dbg4       heading:    %f\n", *heading);
			fprintf(stderr, "dbg4       draft:      %f\n", *draft);
			fprintf(stderr, "dbg4       roll:       %f\n", *roll);
			fprintf(stderr, "dbg4       pitch:      %f\n", *pitch);
			fprintf(stderr, "dbg4       heave:      %f\n", *heave);
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		mb_fix_y2k(verbose, store->pos_year, &time_i[0]);
		time_i[1] = store->pos_month;
		time_i[2] = store->pos_day;
		time_i[3] = store->pos_hour;
		time_i[4] = store->pos_minute;
		time_i[5] = store->pos_second;
		time_i[6] = 10000 * store->pos_hundredth_sec + 100 * store->pos_thousandth_sec;
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		*navlon = 0.00000009 * store->pos_longitude;
		*navlat = 0.00000009 * store->pos_latitude;

		/* get heading */
		*heading = 0.01 * store->heading;

		/* get speed  */
		*speed = 0.0;

		/* get draft */
		*draft = 0.005 * (store->transducer_starboard_depth + store->transducer_port_depth);

		/* get roll pitch and heave */
		if (store->beams_bath > 4) {
			*roll = 0.005 * store->beams[4].roll;
			*pitch = 0.005 * store->beams[4].pitch;
			*heave = 0.001 * store->beams[4].heave;
		}
		else {
			*roll = 0.0;
			*pitch = 0.0;
			*heave = 0.0;
		}

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
			fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
			fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
			fprintf(stderr, "dbg4       speed:      %f\n", *speed);
			fprintf(stderr, "dbg4       heading:    %f\n", *heading);
			fprintf(stderr, "dbg4       draft:      %f\n", *draft);
			fprintf(stderr, "dbg4       roll:       %f\n", *roll);
			fprintf(stderr, "dbg4       pitch:      %f\n", *pitch);
			fprintf(stderr, "dbg4       heave:      %f\n", *heave);
		}

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
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
		fprintf(stderr, "dbg2       draft:         %f\n", *draft);
		fprintf(stderr, "dbg2       roll:          %f\n", *roll);
		fprintf(stderr, "dbg2       pitch:         %f\n", *pitch);
		fprintf(stderr, "dbg2       heave:         %f\n", *heave);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                             double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", speed);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
		fprintf(stderr, "dbg2       draft:      %f\n", draft);
		fprintf(stderr, "dbg2       roll:       %f\n", roll);
		fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
		fprintf(stderr, "dbg2       heave:      %f\n", heave);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		mb_unfix_y2k(verbose, time_i[0], &store->year);
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->second = time_i[5];
		store->hundredth_sec = time_i[6] / 10000;
		store->thousandth_sec = (time_i[6] - 10000 * store->hundredth_sec) / 100;

		/*get navigation */
		store->longitude = navlon;
		store->latitude = navlat;

		/* get heading */
		store->heading = (int)(heading * 100);

		/* get speed */
		store->speed = speed / 3.6;

		/* get draft */
		store->transducer_starboard_depth = 100 * draft;
		store->transducer_port_depth = 100 * draft;

		/* get roll pitch and heave */
	}

	/* insert data in structure */
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		mb_unfix_y2k(verbose, time_i[0], &store->pos_year);
		store->pos_month = time_i[1];
		store->pos_day = time_i[2];
		store->pos_hour = time_i[3];
		store->pos_minute = time_i[4];
		store->pos_second = time_i[5];
		store->pos_hundredth_sec = time_i[6] / 10000;
		store->pos_thousandth_sec = (time_i[6] - 10000 * store->hundredth_sec) / 100;

		/*get navigation */
		store->pos_longitude = navlon / 0.00000009;
		store->pos_latitude = navlat / 0.00000009;

		/* get heading */
		store->heading = (int)(heading * 100);

		/* get draft */
		store->transducer_starboard_depth = 100 * draft;
		store->transducer_port_depth = 100 * draft;

		/* get roll pitch and heave */
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                              int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		*nsvp = store->svp_num;

		/* get profile */
		for (int i = 0; i < *nsvp; i++) {
			depth[i] = 0.1 * store->svp_depth[i];
			velocity[i] = 0.1 * store->svp_vel[i];
		}

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
		for (int i = 0; i < *nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
		for (int i = 0; i < nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		store->svp_num = MIN(nsvp, MBSYS_ELACMK2_MAXSVP);

		/* get profile */
		for (int i = 0; i < store->svp_num; i++) {
			store->svp_depth[i] = (int)(10 * depth[i]);
			store->svp_vel[i] = (int)(10 * velocity[i]);
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_elacmk2_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	struct mbsys_elacmk2_struct *store = (struct mbsys_elacmk2_struct *)store_ptr;
	struct mbsys_elacmk2_struct *copy = (struct mbsys_elacmk2_struct *)copy_ptr;

	/* copy the data */
	*copy = *store;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
