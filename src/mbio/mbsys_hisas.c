/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hisas.c	11/22/2026
 *
 *    Copyright (c) 2026 by
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
 * mbsys_hisas.c includes the functions used to insert data into and extract
 * data out of the structures used to store sidescan data from Kongsberg
 * HISAS synthetic aperture sonars recorded in Kongsberg *.all files.
 * The formats associated with this i/o module are:
 *      MBF_HISASALL : MBIO ID 212
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_hisas.h"

/*--------------------------------------------------------------------*/
int mbsys_hisas_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_hisas_struct), store_ptr, error);

	/* initialize everything */
	if (status == MB_SUCCESS) {
		struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)*store_ptr;
		memset(store, 0, sizeof(struct mbsys_hisas_struct));
		store->kind = MB_DATA_NONE;
		store->sonar = MBSYS_HISAS_UNKNOWN;
		store->ss_channel[MBSYS_HISAS_PORT] = -1;
		store->ss_channel[MBSYS_HISAS_STBD] = -1;
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
int mbsys_hisas_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
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
int mbsys_hisas_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                           int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get data structure pointer */
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		*nbath = store->beams_bath;
		*namp = 0;
		*nss = store->pixels_ss;
	}
	else {
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
int mbsys_hisas_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
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

	/* get data structure pointer */
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		*time_d = store->time_d;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation (vehicle navigation, no lever arm applied) */
		*navlon = store->longitude;
		*navlat = store->latitude;
		*heading = store->heading;
		*speed = store->speed;

		/* the single bathymetry value and the processed sidescan */
		*nbath = store->beams_bath;
		*namp = 0;
		*nss = store->pixels_ss;
		for (int i = 0; i < *nbath; i++) {
			beamflag[i] = store->beamflag[i];
			bath[i] = store->bath[i];
			bathacrosstrack[i] = 0.0;
			bathalongtrack[i] = 0.0;
		}
		for (int i = 0; i < *nss; i++) {
			ss[i] = store->ss[i];
			ssacrosstrack[i] = store->ss_acrosstrack[i];
			ssalongtrack[i] = store->ss_alongtrack[i];
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
			fprintf(stderr, "dbg4        nss:      %d\n", *nss);
		}
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		memset((void *)comment, 0, MB_COMMENT_MAXLINE);
		strncpy(comment, store->comment, MIN(MB_COMMENT_MAXLINE, MBSYS_HISAS_COMMENT_LENGTH) - 1);

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
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
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
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		for (int i = 0; i < *nbath; i++)
			fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
			        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2        namp:     %d\n", *namp);
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
int mbsys_hisas_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
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
		fprintf(stderr, "dbg2       nss:        %d\n", nss);
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	/* get data structure pointer */
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure (in memory only: this format is read only) */
	if (store->kind == MB_DATA_DATA) {
		store->time_d = time_d;
		store->longitude = navlon;
		store->latitude = navlat;
		store->speed = speed;
		store->heading = heading;
		if (nbath >= 1 && nbath <= MBSYS_HISAS_MAXBEAMS) {
			store->beams_bath = nbath;
			for (int i = 0; i < nbath; i++) {
				store->beamflag[i] = beamflag[i];
				store->bath[i] = bath[i];
			}
		}
		if (nss >= 0 && nss <= MBSYS_HISAS_MAXPIXELS) {
			store->pixels_ss = nss;
			for (int i = 0; i < nss; i++) {
				store->ss[i] = ss[i];
				store->ss_acrosstrack[i] = ssacrosstrack[i];
				store->ss_alongtrack[i] = ssalongtrack[i];
			}
		}
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
		memset((void *)store->comment, 0, MBSYS_HISAS_COMMENT_LENGTH);
		strncpy(store->comment, comment, MBSYS_HISAS_COMMENT_LENGTH - 1);
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
int mbsys_hisas_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
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

	/* get data structure pointer */
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure: one vertical "beam" from the altitude */
	if (*kind == MB_DATA_DATA) {
		*nbeams = 1;
		*ssv = store->soundspeed > 0.0 ? store->soundspeed : 1500.0;
		*draft = store->sensordepth;
		ttimes[0] = store->altitude > 0.0 ? 2.0 * store->altitude / (*ssv) : 0.0;
		angles[0] = 0.0;
		angles_forward[0] = 0.0;
		angles_null[0] = 0.0;
		heave[0] = store->heave;
		alongtrack_offset[0] = 0.0;
		*error = MB_ERROR_NO_ERROR;
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
int mbsys_hisas_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                 double *altitude, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get data structure pointer */
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		*transducer_depth = store->sensordepth;
		*altitude = store->altitude;
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
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
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_hisas_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                            double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                            double *pitch, double *heave, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get data structure pointer */
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		*time_d = store->time_d;
		mb_get_date(verbose, *time_d, time_i);
		*navlon = store->longitude;
		*navlat = store->latitude;
		*heading = store->heading;
		*speed = store->speed;
		*draft = store->sensordepth;
		*roll = store->roll;
		*pitch = store->pitch;
		*heave = store->heave;

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  Extracted values:\n");
			fprintf(stderr, "dbg4       kind:       %d\n", *kind);
			fprintf(stderr, "dbg4       error:      %d\n", *error);
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
int mbsys_hisas_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
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

	/* get data structure pointer */
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;

	/* insert data in structure (in memory only: this format is read only) */
	if (store->kind == MB_DATA_DATA) {
		store->time_d = time_d;
		store->longitude = navlon;
		store->latitude = navlat;
		store->speed = speed;
		store->heading = heading;
		store->sensordepth = draft;
		store->roll = roll;
		store->pitch = pitch;
		store->heave = heave;
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
int mbsys_hisas_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
	}

	/* get data structure pointers */
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;
	struct mbsys_hisas_struct *copy = (struct mbsys_hisas_struct *)copy_ptr;

	/* copy the data (the structure holds no pointers) */
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
/* Raw sample spacing (two-way time, s) of one side, from the time duration,
   or from the slant range if the duration is missing; 0 if unknown. */
static double mbsys_hisas_sample_interval(const struct mbsys_hisas_struct *store, int side, double soundspeed) {
	const int nsamples = store->ss_numsamples[side];
	if (nsamples <= 0)
		return 0.0;
	if (store->ss_duration[side] > 0.0)
		return store->ss_duration[side] / nsamples;
	if (store->ss_slantrange[side] > 0.0)
		return 2.0 * store->ss_slantrange[side] / (soundspeed * nsamples);
	return 0.0;
}
/*--------------------------------------------------------------------*/
/* Make the processed sidescan from the raw samples of both sides, assuming a
   flat seafloor at the altitude below the sonar. Both sides are stored from
   near range to far range. The pixels run from port (negative acrosstrack)
   to starboard; pixels without data are set to MB_SIDESCAN_NULL, and gaps of
   up to pixel_int pixels between pixels with data on the same side are
   filled by linear interpolation. */
int mbsys_hisas_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                       int swath_width_set, double *swath_width, int pixel_int, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:       %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       pixel_size_set:  %d\n", pixel_size_set);
		fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
		fprintf(stderr, "dbg2       swath_width_set: %d\n", swath_width_set);
		fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
		fprintf(stderr, "dbg2       pixel_int:       %d\n", pixel_int);
	}

	/* get data structure pointer */
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;

	if (store->kind == MB_DATA_DATA) {
		store->pixels_ss = 0;
		const double soundspeed = store->soundspeed > 0.0 ? store->soundspeed : 1500.0;
		const double altitude = store->altitude > 0.0 ? store->altitude : 0.0;

		/* get the maximum ground range and the number of ground samples
		    of the side with the longer ground range */
		double groundrange = 0.0;
		int groundsamples = 0;
		for (int side = 0; side < 2; side++) {
			const int nsamples = store->ss_numsamples[side];
			const double dt = mbsys_hisas_sample_interval(store, side, soundspeed);
			if (nsamples > 1 && dt > 0.0) {
				const double rmax = 0.5 * soundspeed * dt * (nsamples - 1);
				if (rmax > altitude) {
					const double range = sqrt(rmax * rmax - altitude * altitude);
					const int pixel1 = (int)(2.0 * altitude / (soundspeed * dt)) + 1;
					if (range > groundrange && nsamples > pixel1) {
						groundrange = range;
						groundsamples = nsamples - pixel1;
					}
				}
			}
		}

		if (!swath_width_set)
			*swath_width = 2.0 * groundrange;

		if (!pixel_size_set && groundsamples > 0) {
			const double pixel_size_calc = groundrange / groundsamples;
			if ((*pixel_size) <= 0.0)
				(*pixel_size) = pixel_size_calc;
			else if (0.95 * (*pixel_size) > pixel_size_calc)
				(*pixel_size) = 0.95 * (*pixel_size);
			else if (1.05 * (*pixel_size) < pixel_size_calc)
				(*pixel_size) = 1.05 * (*pixel_size);
			else
				(*pixel_size) = pixel_size_calc;
		}

		if (groundsamples > 0 && *pixel_size > 0.0 && *swath_width > 0.0) {
			/* odd number of pixels centered on nadir */
			int nhalf = (int)(0.5 * (*swath_width) / (*pixel_size)) + 1;
			if (nhalf > (MBSYS_HISAS_MAXPIXELS - 1) / 2)
				nhalf = (MBSYS_HISAS_MAXPIXELS - 1) / 2;
			const int npixels = 2 * nhalf + 1;

			/* zero the sums; the alongtrack array counts the samples per pixel
			    while binning */
			double *ss = store->ss;
			double *ss_cnt = store->ss_alongtrack;
			for (int j = 0; j < npixels; j++) {
				ss[j] = 0.0;
				ss_cnt[j] = 0.0;
			}

			/* bin the raw samples of both sides (near range to far range) */
			for (int side = 0; side < 2; side++) {
				const int nsamples = MIN(store->ss_numsamples[side], MBSYS_HISAS_MAXRAWPIXELS);
				const double dt = mbsys_hisas_sample_interval(store, side, soundspeed);
				if (nsamples <= 0 || dt <= 0.0)
					continue;
				const int sign = (side == MBSYS_HISAS_PORT) ? -1 : 1;
				for (int i = 0; i < nsamples; i++) {
					const double range = 0.5 * soundspeed * dt * i;
					if (range <= altitude)
						continue;
					const double xtrack = sqrt(range * range - altitude * altitude);
					const int k = (int)(xtrack / (*pixel_size) + 0.5);
					if (k > nhalf)
						break;
					const int j = nhalf + sign * k;
					ss[j] += store->ssraw[side][i];
					ss_cnt[j] += 1.0;
				}
			}

			/* average, and null the pixels without data */
			for (int j = 0; j < npixels; j++) {
				if (ss_cnt[j] > 0.0)
					ss[j] /= ss_cnt[j];
				else
					ss[j] = MB_SIDESCAN_NULL;
				store->ss_acrosstrack[j] = (*pixel_size) * (j - nhalf);
			}

			/* interpolate short gaps on each side, never across nadir */
			for (int side = 0; side < 2; side++) {
				const int jstart = (side == MBSYS_HISAS_PORT) ? 0 : nhalf;
				const int jend = (side == MBSYS_HISAS_PORT) ? nhalf : npixels - 1;
				int jprev = -1;
				for (int j = jstart; j <= jend; j++) {
					if (ss_cnt[j] > 0.0) {
						if (jprev >= 0 && j - jprev > 1 && j - jprev - 1 <= pixel_int) {
							for (int jj = jprev + 1; jj < j; jj++) {
								const double factor = ((double)(jj - jprev)) / ((double)(j - jprev));
								ss[jj] = ss[jprev] + factor * (ss[j] - ss[jprev]);
							}
						}
						jprev = j;
					}
				}
			}

			/* the alongtrack distances are zero */
			for (int j = 0; j < npixels; j++)
				ss_cnt[j] = 0.0;

			store->pixel_size = *pixel_size;
			store->pixels_ss = npixels;
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Sidescan regenerated in <%s>\n", __func__);
			fprintf(stderr, "dbg2       beams_bath: %d\n", store->beams_bath);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", store->pixels_ss);
			for (int i = 0; i < store->pixels_ss; i++)
				fprintf(stderr, "dbg2       pixel:%4d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, store->ss[i],
				        store->ss_acrosstrack[i], store->ss_alongtrack[i]);
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
		fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
