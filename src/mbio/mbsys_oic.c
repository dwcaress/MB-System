/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_oic.c	3/1/99
 *
 *    Copyright (c) 1999-2025 by
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
 * mbsys_oic.c contains the functions for handling the data structure
 * used by MBIO functions to store swath sonar data derived
 * from OIC systems:
 *      MBF_OICGEODA : MBIO ID 141
 *      MBF_OICMBARI : MBIO ID 142
 *
 * Author:	D. W. Caress
 * Date:	March 1, 1999
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_oic.h"

/*--------------------------------------------------------------------*/
int mbsys_oic_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_oic_struct), (void **)store_ptr, error);

	/* get pointer to data structure */
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)*store_ptr;

	/* initialize values in structure */
	store->kind = MB_DATA_NONE;
	store->type = 0;
	store->proc_status = 0;
	store->data_size = 0;
	store->client_size = 0;
	store->fish_status = 0;
	store->nav_used = 0;
	store->nav_type = 0;
	store->utm_zone = 0;
	store->ship_x = 0.0;
	store->ship_y = 0.0;
	store->ship_course = 0.0;
	store->ship_speed = 0.0;
	store->sec = 0;
	store->usec = 0;
	store->spare_gain = 0.0;
	store->fish_heading = 0.0;
	store->fish_depth = 0.0;
	store->fish_range = 0.0;
	store->fish_pulse_width = 0.0;
	store->gain_c0 = 0.0;
	store->gain_c1 = 0.0;
	store->gain_c2 = 0.0;
	store->fish_pitch = 0.0;
	store->fish_roll = 0.0;
	store->fish_yaw = 0.0;
	store->fish_x = 0.0;
	store->fish_y = 0.0;
	store->fish_layback = 0.0;
	store->fish_altitude = 0.0;
	store->fish_altitude_samples = 0;
	store->fish_ping_period = 0.0;
	store->sound_velocity = 0.0;
	store->num_chan = 0;
	store->beams_bath = 0;
	store->beams_amp = 0;
	store->bath_chan_port = 0;
	store->bath_chan_stbd = 0;
	store->pixels_ss = 0;
	store->ss_chan_port = 0;
	store->ss_chan_stbd = 0;
	store->beamflag = NULL;
	store->bath = NULL;
	store->bathacrosstrack = NULL;
	store->bathalongtrack = NULL;
	store->tt = NULL;
	store->angle = NULL;
	store->amp = NULL;
	store->ss = NULL;
	store->ssacrosstrack = NULL;
	store->ssalongtrack = NULL;
	for (int i = 0; i < MBSYS_OIC_MAX_CHANNELS; i++) {
		store->channel[i].offset = 0;
		store->channel[i].type = 0;
		store->channel[i].side = 0;
		store->channel[i].size = 0;
		store->channel[i].empty = 0;
		store->channel[i].frequency = 0;
		store->channel[i].num_samples = 0;
	}
	memset(store->client, 0, MBSYS_OIC_MAX_CLIENT);
	for (int i = 0; i < MBSYS_OIC_MAX_CHANNELS; i++) {
		store->rawsize[i] = 0;
		store->raw[i] = NULL;
	}
	store->beams_bath_alloc = 0;
	store->beams_amp_alloc = 0;
	store->pixels_ss_alloc = 0;
	store->beamflag = NULL;
	store->bath = NULL;
	store->amp = NULL;
	store->bathacrosstrack = NULL;
	store->bathalongtrack = NULL;
	store->tt = NULL;
	store->angle = NULL;
	store->ss = NULL;
	store->ssacrosstrack = NULL;
	store->ssalongtrack = NULL;

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
int mbsys_oic_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* get pointer to data structure */
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)*store_ptr;

	int status = MB_SUCCESS;

	/* deallocate memory for data structures */
	for (int i = 0; i < store->num_chan; i++)
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->raw[i]), error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->beamflag, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bathacrosstrack, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bathalongtrack, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->tt, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->angle, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->amp, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->ss, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->ssacrosstrack, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->ssalongtrack, error);

	/* deallocate memory for data structure */
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

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
int mbsys_oic_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error) {
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
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		*nbath = store->beams_bath;
		*namp = store->beams_amp;
		*nss = store->pixels_ss;
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
int mbsys_oic_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                      double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                      double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                      double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
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
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		*time_d = store->sec + 0.000001 * store->usec;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		if (store->nav_type == OIC_NAV_LONLAT) {
			*navlon = store->fish_x;
			*navlat = store->fish_y;
		}
		else {
			*navlon = 0.0;
			*navlat = 0.0;
		}

		/* get heading */
		*heading = store->fish_heading;

		/* get speed */
		*speed = 3.6 * store->ship_speed;

		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 0.2;

		/* read distance, depth, and backscatter
		    values into storage arrays */
		*nbath = store->beams_bath;
		*namp = store->beams_amp;
		*nss = store->pixels_ss;
		for (int i = 0; i < *nbath; i++) {
			beamflag[i] = store->beamflag[i];
			bath[i] = store->bath[i];
			bathacrosstrack[i] = store->bathacrosstrack[i];
			bathalongtrack[i] = store->bathalongtrack[i];
		}
		for (int i = 0; i < *namp; i++) {
			amp[i] = store->amp[i];
		}
		for (int i = 0; i < *nss; i++) {
			ss[i] = store->ss[i];
			ssacrosstrack[i] = store->ssacrosstrack[i];
			ssalongtrack[i] = store->ssalongtrack[i];
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
			fprintf(stderr, "dbg4        nss:      %d\n", *nss);
			for (int i = 0; i < *nss; i++)
				fprintf(stderr, "dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
		}

		/* done translating values */
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* copy comment */
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
    strncpy(comment, store->client, MIN(MB_COMMENT_MAXLINE, MBSYS_OIC_MAX_COMMENT) - 1);

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
		for (int i = 0; i < *nss; i++)
			fprintf(stderr, "dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
			        ssalongtrack[i]);
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
int mbsys_oic_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                     double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                     double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                     double *ssalongtrack, char *comment, int *error) {
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
		fprintf(stderr, "dbg2        nss:       %d\n", nss);
		if (verbose >= 3)
			for (int i = 0; i < nss; i++)
				fprintf(stderr, "dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;

	/* set data kind */
	store->kind = kind;

	int status = MB_SUCCESS;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		store->sec = (int)time_d;
		store->usec = (int)(1000000 * (time_d - store->sec));

		/* get navigation */
		if (navlon < 0.0)
			navlon = navlon + 360.0;
		store->nav_type = OIC_NAV_LONLAT;
		store->fish_x = navlon;
		store->fish_y = navlat;

		/* get heading */
		store->fish_heading = heading;

		/* get speed */
		store->ship_speed = speed / 3.6;

		/* set numbers of beams and sidescan */
		store->beams_bath = nbath;
		store->beams_amp = namp;
		store->pixels_ss = nss;

		/* get bath and sidescan */
		if (store->beams_bath > store->beams_bath_alloc || store->beamflag == NULL || store->bath == NULL ||
		    store->bathacrosstrack == NULL || store->bathalongtrack == NULL || store->tt == NULL || store->angle == NULL) {
			store->beams_bath_alloc = store->beams_bath;
			if (store->beamflag != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->beamflag), error);
			if (store->bath != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->bath), error);
			if (store->bathacrosstrack != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->bathacrosstrack), error);
			if (store->bathalongtrack != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->bathalongtrack), error);
			if (store->tt != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->bathalongtrack), error);
			if (store->angle != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->bathalongtrack), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(char), (void **)&(store->beamflag),
			                    error);
			status &=
			    mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(float), (void **)&(store->bath), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(float),
			                    (void **)&(store->bathacrosstrack), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(float),
			                    (void **)&(store->bathalongtrack), error);
			status &=
			    mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(float), (void **)&(store->tt), error);
			status &=
			    mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(float), (void **)&(store->angle), error);
		}
		if (store->beams_amp > store->beams_amp_alloc || store->amp == NULL) {
			store->beams_amp_alloc = store->beams_amp;
			if (store->amp != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->amp), error);
			status &=
			    mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(char), (void **)&(store->amp), error);
		}
		if (store->pixels_ss > store->pixels_ss_alloc || store->ss == NULL || store->ssacrosstrack == NULL ||
		    store->ssalongtrack == NULL) {
			store->pixels_ss_alloc = store->pixels_ss;
			if (store->ss != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->ss), error);
			if (store->ssacrosstrack != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->ssacrosstrack), error);
			if (store->ssalongtrack != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(store->ssalongtrack), error);
			status &=
			    mb_mallocd(verbose, __FILE__, __LINE__, store->pixels_ss_alloc * sizeof(float), (void **)&(store->ss), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, store->pixels_ss_alloc * sizeof(float),
			                    (void **)&(store->ssacrosstrack), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, store->pixels_ss_alloc * sizeof(float),
			                    (void **)&(store->ssalongtrack), error);
		}
		for (int i = 0; i < store->beams_bath; i++) {
			store->beamflag[i] = beamflag[i];
			store->bath[i] = bath[i];
			store->bathacrosstrack[i] = bathacrosstrack[i];
			store->bathalongtrack[i] = bathalongtrack[i];
		}
		for (int i = 0; i < store->beams_amp; i++) {
			store->amp[i] = amp[i];
		}
		for (int i = 0; i < store->pixels_ss; i++) {
			store->ss[i] = ss[i];
			store->ssacrosstrack[i] = ssacrosstrack[i];
			store->ssalongtrack[i] = ssalongtrack[i];
		}
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
		store->type = OIC_ID_COMMENT;
    memset((void *)store->client, 0, MBSYS_OIC_MAX_COMMENT);
    strncpy(store->client, comment, MIN(MBSYS_OIC_MAX_COMMENT, MB_COMMENT_MAXLINE) - 1);
    store->client_size = strlen(store->client) + 1;
	}

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
int mbsys_oic_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
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
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = store->beams_bath;

		/* initialize */
		for (int i = 0; i < store->beams_bath; i++) {
			ttimes[i] = store->tt[i];
			const double beta = store->angle[i];
			const double alpha = store->fish_pitch;
			status &= mb_rollpitch_to_takeoff(verbose, alpha, beta, &angles[i], &angles_forward[i], error);
			angles_null[i] = 0.0;
			heave[i] = 0.0;
			alongtrack_offset[i] = 0.0;
		}

		/* get ssv */
		*ssv = store->sound_velocity;

		/* get draft */
		*draft = store->fish_depth;

		if (status == MB_FAILURE) {
			fprintf(stderr, "WARNING: status is MB_FAILURE\n");
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
		fprintf(stderr, "dbg2       draft:      %f\n", *draft);
		fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
			        i, ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
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
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = store->beams_bath;

		/* get detects */
		for (int i = 0; i < *nbeams; i++) {
			detects[i] = MB_DETECT_PHASE;
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
			fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
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
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		*transducer_depth = store->fish_depth;
		*altitude = store->fish_altitude;
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
int mbsys_oic_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr, double transducer_depth, double altitude,
                              int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:            %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:         %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", altitude);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;

	int status = MB_SUCCESS;

	/* insert data into structure */
	if (store->kind == MB_DATA_DATA) {
		store->fish_depth = transducer_depth;
		store->fish_altitude = altitude;
	}

	/* deal with comment */
	else if (store->kind == MB_DATA_COMMENT) {
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
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_oic_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, double *draft, double *roll, double *pitch,
                          double *heave, int *error) {
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
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		*time_d = store->sec + 0.000001 * store->usec;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		if (store->nav_type == OIC_NAV_LONLAT) {
			*navlon = store->fish_x;
			*navlat = store->fish_y;
		}
		else {
			*navlon = 0.0;
			*navlat = 0.0;
		}

		/* get heading */
		*heading = store->fish_heading;

		/* get speed */
		*speed = 3.6 * store->ship_speed;

		/* get draft */
		*draft = store->fish_depth;

		/* get roll pitch and heave */
		*roll = store->fish_roll;
		*pitch = store->fish_pitch;
		*heave = 0.0;

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
int mbsys_oic_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon, double navlat,
                         double speed, double heading, double draft, double roll, double pitch, double heave, int *error) {
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
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		store->sec = (int)time_d;
		store->usec = (int)(1000000 * (time_d - store->sec));

		/* get navigation */
		if (navlon < 0.0)
			navlon = navlon + 360.0;
		store->nav_type = OIC_NAV_LONLAT;
		store->fish_x = navlon;
		store->fish_y = navlat;

		/* get heading */
		store->fish_heading = heading;

		/* get speed */
		store->ship_speed = speed / 3.6;

		/* get draft */
		store->fish_depth = draft;

		/* get roll pitch and heave */
		store->fish_roll = roll;
		store->fish_pitch = pitch;
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
int mbsys_oic_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
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
	struct mbsys_oic_struct *store = (struct mbsys_oic_struct *)store_ptr;
	struct mbsys_oic_struct *copy = (struct mbsys_oic_struct *)copy_ptr;

	int status = MB_SUCCESS;

	/* copy the basic header data */
	if (store != NULL && copy != NULL) {
		/* type of data record */
		copy->kind = store->kind;
		copy->type = store->type;

		/* status and size */
		copy->proc_status = store->proc_status;
		copy->data_size = store->data_size;
		copy->client_size = store->client_size;
		copy->fish_status = store->fish_status;
		copy->type = store->type;
		copy->type = store->type;

		/* nav */
		copy->nav_used = store->nav_used;
		copy->nav_type = store->nav_type;
		copy->utm_zone = store->utm_zone;
		copy->ship_x = store->ship_x;
		copy->ship_y = store->ship_y;
		copy->ship_course = store->ship_course;
		copy->ship_speed = store->ship_speed;
		copy->ship_x = store->ship_x;

		/* time stamp */
		copy->sec = store->sec;
		copy->usec = store->usec;

		/* more stuff */
		copy->spare_gain = store->spare_gain;
		copy->fish_heading = store->fish_heading;
		copy->fish_depth = store->fish_depth;
		copy->fish_range = store->fish_range;
		copy->fish_pulse_width = store->fish_pulse_width;
		copy->gain_c0 = store->gain_c0;
		copy->gain_c1 = store->gain_c1;
		copy->gain_c2 = store->gain_c2;
		copy->fish_pitch = store->fish_pitch;
		copy->fish_roll = store->fish_roll;
		copy->fish_yaw = store->fish_yaw;
		copy->fish_x = store->fish_x;
		copy->fish_y = store->fish_y;
		copy->fish_layback = store->fish_layback;
		copy->fish_altitude = store->fish_altitude;
		copy->fish_altitude_samples = store->fish_altitude_samples;
		copy->fish_ping_period = store->fish_ping_period;
		copy->sound_velocity = store->sound_velocity;

		/* numbers of beams and scaling */
		copy->num_chan = store->num_chan;
		copy->beams_bath = store->beams_bath;
		copy->beams_amp = store->beams_amp;
		copy->bath_chan_port = store->bath_chan_port;
		copy->bath_chan_stbd = store->bath_chan_stbd;
		copy->pixels_ss = store->pixels_ss;
		copy->ss_chan_port = store->ss_chan_port;
		copy->ss_chan_stbd = store->ss_chan_stbd;
	}

	/* allocate the raw data */
	if (store != NULL && copy != NULL) {
		for (int i = 0; i < copy->num_chan; i++) {
			/* copy channel info */
			copy->channel[i].offset = store->channel[i].offset;
			copy->channel[i].type = store->channel[i].type;
			copy->channel[i].side = store->channel[i].side;
			copy->channel[i].size = store->channel[i].size;
			copy->channel[i].empty = store->channel[i].empty;
			copy->channel[i].frequency = store->channel[i].frequency;
			copy->channel[i].num_samples = store->channel[i].num_samples;

			/* allocate data if needed */
			if (store->rawsize[i] > copy->rawsize[i] || copy->raw[i] == NULL) {
				if (copy->raw[i] != NULL)
					status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->raw[i]), error);
				copy->rawsize[i] = store->rawsize[i];
				status &= mb_mallocd(verbose, __FILE__, __LINE__, copy->rawsize[i], (void **)&(copy->raw[i]), error);
			}

			/* copy the raw data */
			for (int j = 0; j < copy->rawsize[i]; j++) {
				copy->raw[i][j] = store->raw[i][j];
			}
		}
	}

	/* allocate the depths and sidescan */
	if (status == MB_SUCCESS && store != NULL && copy != NULL) {
		if (store->beams_bath > copy->beams_bath_alloc || copy->beamflag == NULL || copy->bath == NULL ||
		    copy->bathacrosstrack == NULL || copy->bathalongtrack == NULL || copy->tt == NULL || copy->angle == NULL) {
			copy->beams_bath_alloc = store->beams_bath;
			if (copy->beamflag != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->beamflag), error);
			if (copy->bath != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->bath), error);
			if (copy->bathacrosstrack != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->bathacrosstrack), error);
			if (copy->bathalongtrack != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->bathalongtrack), error);
			if (copy->tt != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->tt), error);
			if (copy->angle != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->angle), error);
			status &=
			    mb_mallocd(verbose, __FILE__, __LINE__, copy->beams_bath_alloc * sizeof(char), (void **)&(copy->beamflag), error);
			status &=
			    mb_mallocd(verbose, __FILE__, __LINE__, copy->beams_bath_alloc * sizeof(float), (void **)&(copy->bath), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, copy->beams_bath_alloc * sizeof(float),
			                    (void **)&(copy->bathacrosstrack), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, copy->beams_bath_alloc * sizeof(float),
			                    (void **)&(copy->bathalongtrack), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, copy->beams_bath_alloc * sizeof(float), (void **)&(copy->tt), error);
			status &=
			    mb_mallocd(verbose, __FILE__, __LINE__, copy->beams_bath_alloc * sizeof(float), (void **)&(copy->angle), error);
		}
		if (store->beams_amp > copy->beams_amp_alloc || copy->amp == NULL) {
			copy->beams_amp_alloc = store->beams_amp;
			if (copy->amp != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->amp), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, copy->beams_bath_alloc * sizeof(char), (void **)&(copy->amp), error);
		}
		if (store->pixels_ss > copy->pixels_ss_alloc || copy->ss == NULL || copy->ssacrosstrack == NULL ||
		    copy->ssalongtrack == NULL) {
			copy->pixels_ss_alloc = store->pixels_ss;
			if (copy->ss != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->ss), error);
			if (copy->ssacrosstrack != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->ssacrosstrack), error);
			if (copy->ssalongtrack != NULL)
				status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->ssalongtrack), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, copy->pixels_ss_alloc * sizeof(float), (void **)&(copy->ss), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, copy->pixels_ss_alloc * sizeof(float),
			                    (void **)&(copy->ssacrosstrack), error);
			status &= mb_mallocd(verbose, __FILE__, __LINE__, copy->pixels_ss_alloc * sizeof(float),
			                    (void **)&(copy->ssalongtrack), error);
		}
	}

	/* copy the depths and sidescan */
	if (status == MB_SUCCESS && store != NULL && copy != NULL) {
		for (int i = 0; i < copy->beams_bath; i++) {
			copy->beamflag[i] = store->beamflag[i];
			copy->bath[i] = store->bath[i];
			copy->bathacrosstrack[i] = store->bathacrosstrack[i];
			copy->bathalongtrack[i] = store->bathalongtrack[i];
			copy->tt[i] = store->tt[i];
			copy->angle[i] = store->angle[i];
		}
		for (int i = 0; i < copy->beams_amp; i++) {
			copy->amp[i] = store->amp[i];
		}
		for (int i = 0; i < copy->pixels_ss; i++) {
			copy->ss[i] = store->ss[i];
			copy->ssacrosstrack[i] = store->ssacrosstrack[i];
			copy->ssalongtrack[i] = store->ssalongtrack[i];
		}

		/* client */
		for (int i = 0; i < store->client_size; i++)
			copy->client[i] = store->client[i];
	}

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE) {
		for (int i = 0; i < copy->num_chan; i++) {
			if (copy->raw[i] != NULL)
				status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(copy->raw[i]), error);
		}
		if (copy->beamflag != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->beamflag, error);
		if (copy->bath != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->bath, error);
		if (copy->bathacrosstrack != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->bathacrosstrack, error);
		if (copy->bathalongtrack != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->bathalongtrack, error);
		if (copy->tt != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->tt, error);
		if (copy->angle != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->angle, error);
		if (copy->amp != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->amp, error);
		if (copy->ss != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->ss, error);
		if (copy->ssacrosstrack != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->ssacrosstrack, error);
		if (copy->ssalongtrack != NULL)
			status = mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->ssalongtrack, error);
		if (copy->beamflag != NULL)
			status = MB_FAILURE;
		*error = MB_ERROR_MEMORY_FAIL;
	}

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
