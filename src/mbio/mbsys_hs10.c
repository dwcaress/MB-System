/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hs10.c	12/4/00
 *
 *    Copyright (c) 2000-2025 by
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
 * mbsys_hs10.c contains the functions for handling the data structure
 * used by MBIO functions to store data from the 45-beam Furuno HS-10
 * multibeam sonar system.
 * The data formats which are commonly used to store HS-10
 * data in files include
 *      MBF_HS10JAMS : MBIO ID 171
 *
 * Author:	D. W. Caress
 * Date:	December 4, 2000
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_hs10.h"

/*--------------------------------------------------------------------*/
int mbsys_hs10_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_hs10_struct), store_ptr, error);

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
int mbsys_hs10_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* deallocate memory for data structure */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

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
int mbsys_hs10_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error) {
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
	struct mbsys_hs10_struct *store = (struct mbsys_hs10_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		*nbath = MBSYS_HS10_BEAMS;
		*namp = MBSYS_HS10_BEAMS;
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
int mbsys_hs10_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
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
	struct mbsys_hs10_struct *store = (struct mbsys_hs10_struct *)store_ptr;

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
		time_i[5] = store->tenth_second / 10;
		time_i[6] = 100000 * (store->tenth_second - 10 * time_i[5]);
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		*navlon = ((double)store->londeg) + (store->lonmin / 60000.0);
		if (store->EorW == 'W')
			*navlon *= -1.0;
		*navlat = ((double)store->latdeg) + (store->latmin / 60000.0);
		if (store->NorS == 'S')
			*navlat *= -1.0;

		/* get heading */
		*heading = (0.1 * (double)store->heading);

		/* get speed */
		*speed = 0.0;

		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 3.0;
		mb_io_ptr->beamwidth_xtrack = 3.0;

		/* read distance and depth values into storage arrays */
		*nbath = MBSYS_HS10_BEAMS;
		*namp = MBSYS_HS10_BEAMS;
		*nss = 0;
		for (int i = 0; i < *nbath; i++) {
			if (store->depth[i] >= 29999) {
				beamflag[i] = MB_FLAG_NULL;
				bath[i] = 0.0;
				bathacrosstrack[i] = 0.0;
			}
			else if (store->depth[i] > 0) {
				beamflag[i] = MB_FLAG_NONE;
				bath[i] = (double)store->depth[i];
				bathacrosstrack[i] = (double)store->acrosstrack[i];
			}
			else if (store->depth[i] < 0) {
				beamflag[i] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
				bath[i] = (double)-store->depth[i];
				bathacrosstrack[i] = (double)store->acrosstrack[i];
			}
			amp[i] = store->amplitude[i];
			bathalongtrack[i] = 0.0;
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

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* copy comment */
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
    strncpy(comment, store->comment, MIN(MB_COMMENT_MAXLINE, MBSYS_HS10_COMMENT) - 1);

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
int mbsys_hs10_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
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
	struct mbsys_hs10_struct *store = (struct mbsys_hs10_struct *)store_ptr;

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
		store->tenth_second = (10 * time_i[5]) + (time_i[6] / 100000);

		/* get navigation */
		if (navlon < -180.0)
			navlon = navlon + 360.0;
		if (navlon > 180.0)
			navlon = navlon - 360.0;
		if (navlon < 0.0)
			store->EorW = 'W';
		else
			store->EorW = 'E';
		store->londeg = (int)fabs(navlon);
		store->lonmin = (int)(60000.0 * (fabs(navlon) - store->londeg));
		if (navlat < 0.0)
			store->NorS = 'S';
		else
			store->NorS = 'N';
		store->latdeg = (int)fabs(navlat);
		store->latmin = (int)(60000.0 * (fabs(navlat) - store->latdeg));

		/* get heading */
		store->heading = (int)(10 * heading);

		/* get speed */

		/* put distance and depth values
		    into data structure */
		for (int i = 0; i < nbath; i++) {
			if (mb_beam_check_flag_null(beamflag[i])) {
				store->depth[i] = 29999;
				store->acrosstrack[i] = 99999;
			}
			else if (mb_beam_check_flag(beamflag[i])) {
				store->depth[i] = (int)-bath[i];
				store->acrosstrack[i] = (int)bathacrosstrack[i];
			}
			else {
				store->depth[i] = (int)bath[i];
				store->acrosstrack[i] = (int)bathacrosstrack[i];
			}
			store->amplitude[i] = (int)amp[i];
		}
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
    memset((void *)store->comment, 0, MBSYS_HS10_COMMENT);
    strncpy(store->comment, comment, MIN(MBSYS_HS10_COMMENT, MB_COMMENT_MAXLINE) - 1);
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
int mbsys_hs10_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
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
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_hs10_struct *store = (struct mbsys_hs10_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA || *kind == MB_DATA_CALIBRATE) {
		/* get nbeams */
		*nbeams = MBSYS_HS10_BEAMS;

		/* get travel times, angles */
		for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			ttimes[i] = 0.0;
			angles[i] = (i - MBSYS_HS10_BEAMS / 2) * MBSYS_HS10_BEAMSPACING;
			if (angles[i] < 0.0) {
				angles[i] = -angles[i];
				angles_forward[i] = 180.0;
			}
			else
				angles_forward[i] = 0.0;
			angles_null[i] = 0.0;
			heave[i] = 0.0;
			alongtrack_offset[i] = 0.0;
		}

		/* get ssv */
		*ssv = 0.0;
		*draft = 0.0;

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
			fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
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
int mbsys_hs10_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
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
	struct mbsys_hs10_struct *store = (struct mbsys_hs10_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = MBSYS_HS10_BEAMS;

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
int mbsys_hs10_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                double *altitude, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_hs10_struct *store = (struct mbsys_hs10_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA || *kind == MB_DATA_CALIBRATE) {
		*transducer_depth = 0.0;
		*altitude = store->center_depth;

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
int mbsys_hs10_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
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
	struct mbsys_hs10_struct *store = (struct mbsys_hs10_struct *)store_ptr;

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
		time_i[5] = store->tenth_second / 10;
		time_i[6] = 100000 * (store->tenth_second - 10 * time_i[5]);
		mb_get_time(verbose, time_i, time_d);

		/* get navigation */
		*navlon = ((double)store->londeg) + (store->lonmin / 60000.0);
		if (store->EorW == 'W')
			*navlon *= -1.0;
		*navlat = ((double)store->latdeg) + (store->latmin / 60000.0);
		if (store->NorS == 'S')
			*navlat *= -1.0;

		/* get heading */
		*heading = (0.1 * (double)store->heading);

		/* get speed */
		*speed = 0.0;

		/* get draft */
		*draft = 0.0;

		/* get roll pitch and heave */
		*roll = 0.0;
		*pitch = 0.0;
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
int mbsys_hs10_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                          double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                          int *error) {
	int status = MB_SUCCESS;

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
	struct mbsys_hs10_struct *store = (struct mbsys_hs10_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		mb_unfix_y2k(verbose, time_i[0], &store->year);
		store->month = time_i[1];
		store->day = time_i[2];
		store->hour = time_i[3];
		store->minute = time_i[4];
		store->tenth_second = (10 * time_i[5]) + (time_i[6] / 100000);

		/* get navigation */
		if (navlon < -180.0)
			navlon = navlon + 360.0;
		if (navlon > 180.0)
			navlon = navlon - 360.0;
		if (navlon < 0.0)
			store->EorW = 'W';
		else
			store->EorW = 'E';
		store->londeg = (int)fabs(navlon);
		store->lonmin = (int)(60000.0 * (fabs(navlon) - store->londeg));
		if (navlat < 0.0)
			store->NorS = 'S';
		else
			store->NorS = 'N';
		store->latdeg = (int)fabs(navlat);
		store->latmin = (int)(60000.0 * (fabs(navlat) - store->latdeg));

		/* get heading */
		store->heading = (int)(10 * heading);

		/* get speed */

		/* get draft */

		/* get roll pitch and heave */
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
int mbsys_hs10_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
	int status = MB_SUCCESS;

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
	struct mbsys_hs10_struct *store = (struct mbsys_hs10_struct *)store_ptr;
	struct mbsys_hs10_struct *copy = (struct mbsys_hs10_struct *)copy_ptr;

	/* copy the data */
	*copy = *store;

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
