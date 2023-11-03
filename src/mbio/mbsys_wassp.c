/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_wassp.c	3.00	1/27/2014
 *
 *    Copyright (c) 2014-2023 by
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
 * mbsys_wassp.c contains the MBIO functions for handling data from
 * the following data formats:
 *    MBSYS_WASSP formats (code in mbsys_wassp.c and mbsys_wassp.h):
 *      MBF_WASSPENL : MBIO ID 241 (code in mbr_wasspenl.c)
 *
 * Author:	D. W. Caress
 * Date:	January 27, 2014
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_wassp.h"

/*--------------------------------------------------------------------*/
int mbsys_wassp_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_wassp_struct), (void **)store_ptr, error);

	/* initialize allocated structure to zero */
	if (status == MB_SUCCESS) {
		memset(*store_ptr, 0, sizeof(struct mbsys_wassp_struct));
	}

	/* get data structure pointer */
	// struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)*store_ptr;
	// struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	// struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	// struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

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
int mbsys_wassp_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* get data structure pointer */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)*store_ptr;
	// struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	// struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	// struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	int status = MB_SUCCESS;

	/* deallocate any arrays or structures contained within the store data structure */
	if (rawsonar->rawdata_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(rawsonar->rawdata), error);
		rawsonar->rawdata_alloc = 0;
	}
	if (wcd_navi->wcdata_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(wcd_navi->wcdata_x), error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(wcd_navi->wcdata_y), error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(wcd_navi->wcdata_mag), error);
		wcd_navi->wcdata_alloc = 0;
	}
	if (sensprop->n_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(sensprop->sensors), error);
		sensprop->n_alloc = 0;
	}
	if (sys_cfg1->sys_cfg1_data_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(sys_cfg1->sys_cfg1_data), error);
		sys_cfg1->sys_cfg1_data_alloc = 0;
	}

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
int mbsys_wassp_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error) {
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	// struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	// struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		*nbath = corbathy->num_beams;
		*namp = corbathy->num_beams;
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
int mbsys_wassp_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)mb_io_ptr->store_data;
	// struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	// struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* extract data from structure */
	*pingnumber = corbathy->ping_number;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pingnumber: %u\n", *pingnumber);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_wassp_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
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
	// struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	// struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	// struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	// struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* get sonar type */
	*sonartype = MB_TOPOGRAPHY_TYPE_MULTIBEAM;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sonartype:  %d\n", *sonartype);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_wassp_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = corbathy->longitude;
		*navlat = corbathy->latitude;

		/* get speed */
		*speed = 1.8520 * nvupdate->sog;

		/* get heading */
		*heading = corbathy->bearing;

		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_xtrack = 4.0;
		mb_io_ptr->beamwidth_ltrack = 4.0;

		/* get coordinate scaling */
		const double headingx = sin(-(*heading) * DTR);
		const double headingy = cos(-(*heading) * DTR);

		/* read distance and depth values into storage arrays */
		*nbath = corbathy->num_beams;
		*namp = *nbath;
		for (unsigned int i = 0; i < genbathy->number_beams; i++) {
			bath[i] = 0.0;
			beamflag[i] = MB_FLAG_NULL;
			bathacrosstrack[i] = 0.0;
			bathalongtrack[i] = 0.0;
			amp[i] = 0.0;
		}
		for (int i = 0; i < *nbath; i++) {
			const int j = corbathy->beam_index[i];
			bath[j] = -corbathy->z[i];
			beamflag[j] = corbathy->empty[i];
			bathacrosstrack[j] = headingy * corbathy->x[i] + headingx * (-corbathy->y[i]);
			bathalongtrack[j] = -headingx * corbathy->x[i] + headingy * (-corbathy->y[i]);
			amp[j] = corbathy->backscatter[i];
		}

		/* extract sidescan */
		*nss = 0;

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

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = nvupdate->longitude;
		*navlat = nvupdate->latitude;

		/* get speed */
		*speed = 1.8520 * nvupdate->sog;

		/* get heading */
		*heading = nvupdate->heading;

		/* set beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

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
		}

		/* done translating values */
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* copy comment */
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
		if (mcomment->comment_length > 0) {
        strncpy(comment, mcomment->comment_message, MIN(mcomment->comment_length, MB_COMMENT_MAXLINE));
    }

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Comment extracted by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New ping values:\n");
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
			fprintf(stderr, "dbg4       comment:    %s\n", comment);
		}
	}

	/* set time for other data records */
	else {
		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		if (verbose >= 4) {
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
	}
	if (verbose >= 2 && (*kind == MB_DATA_DATA || *kind == MB_DATA_NAV)) {
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

	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_wassp_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
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
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}
	if (verbose >= 2 && (kind != MB_DATA_COMMENT)) {
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
	}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV)) {
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
				fprintf(stderr, "dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	// struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		for (int i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		corbathy->longitude = navlon;
		corbathy->latitude = navlat;

		/* get heading */
		corbathy->bearing = heading;

		/* get speed  */

		/* get coordinate scaling */
		const double headingx = sin(heading * DTR);
		const double headingy = cos(heading * DTR);

		/* read distance and depth values into storage arrays */
		for (unsigned int i = 0; i < corbathy->num_beams; i++) {
			const int j = corbathy->beam_index[i];
			corbathy->z[i] = -bath[j];
			corbathy->empty[i] = beamflag[j];
			corbathy->x[i] = headingy * bathacrosstrack[j] + headingx * bathalongtrack[j];
			corbathy->y[i] = -(-headingx * bathacrosstrack[j] + headingy * bathalongtrack[j]);
			corbathy->backscatter[i] = amp[j];
		}

		/* insert the sidescan */
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		for (int i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		nvupdate->longitude = navlon;
		nvupdate->latitude = navlat;

		/* get heading */
		nvupdate->heading = heading;

		/* get speed  */
		nvupdate->sog = speed / 1.8520;
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
    mcomment->comment_length = MIN(strlen(comment) + 1, MB_COMMENT_MAXLINE);
    if (mcomment->comment_length % 2 == 1) {
      mcomment->comment_length = MIN(mcomment->comment_length + 1, MB_COMMENT_MAXLINE);
    }
    memset((void *)mcomment->comment_message, 0, MB_COMMENT_MAXLINE);
    strncpy(mcomment->comment_message, comment, MB_COMMENT_MAXLINE - 1);
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
int mbsys_wassp_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	// struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get depth offset (heave + sonar depth) */
		*ssv = genbathy->sound_velocity;

		/* get draft */
		*draft = 0.0;

		/* get travel times, angles */
		*nbeams = genbathy->number_beams;
		for (int i = 0; i < *nbeams; i++) {
			ttimes[i] = genbathy->detection_point[i] / genbathy->sampling_frequency;

			const double alpha = nvupdate->pitch;
			const double beta = 90.0 + genbathy->rx_angle[i] - nvupdate->roll;

			double phi;
			double theta;
			mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
			angles[i] = theta;
			angles_forward[i] = phi;
			angles_null[i] = 0.0;
			heave[i] = nvupdate->heave;
			alongtrack_offset[i] = 0.0;
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
int mbsys_wassp_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	// struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	// struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get detect type for each sounding - options include:
		    MB_DETECT_UNKNOWN
		    MB_DETECT_AMPLITUDE
		    MB_DETECT_PHASE
		    MB_DETECT_LIDAR */
		*nbeams = genbathy->number_beams;
		for (int i = 0; i < *nbeams; i++) {
			if ((genbathy->flags[i] & 0xF) == 0x7)
				detects[i] = MB_DETECT_AMPLITUDE;
			else if ((genbathy->flags[i] & 0xF) == 0xB)
				detects[i] = MB_DETECT_PHASE;
			else
				detects[i] = MB_DETECT_UNKNOWN;
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
int mbsys_wassp_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                      double *receive_gain, int *error) {
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	// struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	// struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get transmit_gain (dB) */
		*transmit_gain = genbathy->tx_power;

		/* get pulse_length (usec) */
		*pulse_length = genbathy->pulse_width;

		/* get receive_gain (dB) */
		*receive_gain = 0.0;

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
		fprintf(stderr, "dbg2       transmit_gain: %f\n", *transmit_gain);
		fprintf(stderr, "dbg2       pulse_length:  %f\n", *pulse_length);
		fprintf(stderr, "dbg2       receive_gain:  %f\n", *receive_gain);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_wassp_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	// struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	// struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get transducer depth and altitude */
		*transducer_depth = -nvupdate->heave;

		/* get altitude */
		*altitude = nvupdate->nadir_depth + nvupdate->heave;

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
int mbsys_wassp_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	// struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from survey record */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = corbathy->longitude;
		*navlat = corbathy->latitude;

		/* get speed */
		*speed = 1.8520 * nvupdate->sog;

		/* get heading */
		*heading = corbathy->bearing;

		/* get draft  */
		*draft = 0.0;

		/* get attitude  */
		*roll = corbathy->roll;
		*pitch = corbathy->pitch;
		*heave = corbathy->heave;

		/* done translating values */
	}

	/* extract data from nav record */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = nvupdate->longitude;
		*navlat = nvupdate->latitude;

		/* get speed */
		*speed = 1.8520 * nvupdate->sog;

		/* get heading */
		*heading = nvupdate->heading;

		/* get draft  */
		*draft = 0.0;

		/* get attitude  */
		*roll = nvupdate->roll;
		*pitch = nvupdate->pitch;
		*heave = nvupdate->heave;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;

		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;

		/* get time */
		for (int i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:          %d\n", *kind);
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
		fprintf(stderr, "dbg2       error:         %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:        %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_wassp_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	// struct mbsys_wassp_genbathy_struct *genbathy = (struct mbsys_wassp_genbathy_struct *)&(store->genbathy);
	struct mbsys_wassp_corbathy_struct *corbathy = (struct mbsys_wassp_corbathy_struct *)&(store->corbathy);
	// struct mbsys_wassp_rawsonar_struct *rawsonar = (struct mbsys_wassp_rawsonar_struct *)&(store->rawsonar);
	// struct mbsys_wassp_gen_sens_struct *gen_sens = (struct mbsys_wassp_gen_sens_struct *)&(store->gen_sens);
	struct mbsys_wassp_nvupdate_struct *nvupdate = (struct mbsys_wassp_nvupdate_struct *)&(store->nvupdate);
	// struct mbsys_wassp_wcd_navi_struct *wcd_navi = (struct mbsys_wassp_wcd_navi_struct *)&(store->wcd_navi);
	// struct mbsys_wassp_sensprop_struct *sensprop = (struct mbsys_wassp_sensprop_struct *)&(store->sensprop);
	// struct mbsys_wassp_sys_prop_struct *sys_prop = (struct mbsys_wassp_sys_prop_struct *)&(store->sys_prop);
	// struct mbsys_wassp_sys_cfg1_struct *sys_cfg1 = (struct mbsys_wassp_sys_cfg1_struct *)&(store->sys_cfg1);
	// struct mbsys_wassp_mcomment_struct *mcomment = (struct mbsys_wassp_mcomment_struct *)&(store->mcomment);

	/* insert data in ping structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		for (int i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		corbathy->longitude = navlon;
		corbathy->latitude = navlat;

		/* get heading */
		corbathy->bearing = heading;

		/* get draft  */

		/* get roll pitch and heave */
		corbathy->heave = heave;
		corbathy->pitch = pitch;
		corbathy->roll = roll;
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		for (int i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		nvupdate->longitude = navlon;
		nvupdate->latitude = navlat;

		/* get speed  */
		nvupdate->sog = speed / 1.8520;

		/* get heading */
		nvupdate->heading = heading;

		/* get draft  */

		/* get roll pitch and heave */
		nvupdate->heave = heave;
		nvupdate->pitch = pitch;
		nvupdate->roll = roll;
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
int mbsys_wassp_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
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
	struct mbsys_wassp_struct *store = (struct mbsys_wassp_struct *)store_ptr;
	struct mbsys_wassp_struct *copy = (struct mbsys_wassp_struct *)copy_ptr;

	/* copy the data - for many formats memory must be allocated and
	    sub-structures copied separately */
	/* Type of most recently read data record */
	copy->kind = store->kind;

	/* MB-System time stamp of most recently read record */
	copy->time_d = store->time_d;
	for (int i = 0; i < 7; i++)
		copy->time_i[i] = store->time_i[i];

	/* GENBATHY record */
	copy->genbathy = store->genbathy;

	/* CORBATHY record */
	copy->corbathy = store->corbathy;

	/* RAWSONAR record */
	size_t rawdata_alloc_save = copy->rawsonar.rawdata_alloc;
	short *rawdata_save = copy->rawsonar.rawdata;
	copy->rawsonar = store->rawsonar;
	copy->rawsonar.rawdata_alloc = rawdata_alloc_save;
	copy->rawsonar.rawdata = rawdata_save;

	int status = MB_SUCCESS;

	if (status == MB_SUCCESS && copy->rawsonar.rawdata_alloc < store->rawsonar.rawdata_alloc) {
		status =
		    mb_reallocd(verbose, __FILE__, __LINE__, store->rawsonar.rawdata_alloc, (void **)&(copy->rawsonar.rawdata), error);
		if (status == MB_SUCCESS) {
			copy->rawsonar.rawdata_alloc = store->rawsonar.rawdata_alloc;
			const size_t copy_len = (size_t)(copy->rawsonar.n * copy->rawsonar.m);
			memcpy(copy->rawsonar.rawdata, store->rawsonar.rawdata, copy_len);
		}
	}

	/* GEN_SENS record */
	copy->kind = store->kind;

	/* NVUPDATE record */
	copy->nvupdate = store->nvupdate;

	/* WCD_NAVI record */
	const size_t wcdata_alloc_save = copy->wcd_navi.wcdata_alloc;
	float *wcdata_x_save = copy->wcd_navi.wcdata_x;
	float *wcdata_y_save = copy->wcd_navi.wcdata_y;
	float *wcdata_mag_save = copy->wcd_navi.wcdata_mag;
	copy->wcd_navi = store->wcd_navi;
	copy->wcd_navi.wcdata_alloc = wcdata_alloc_save;
	copy->wcd_navi.wcdata_x = wcdata_x_save;
	copy->wcd_navi.wcdata_y = wcdata_y_save;
	copy->wcd_navi.wcdata_mag = wcdata_mag_save;
	if (status == MB_SUCCESS && copy->wcd_navi.wcdata_alloc < store->wcd_navi.wcdata_alloc) {
		status =
		    mb_reallocd(verbose, __FILE__, __LINE__, store->wcd_navi.wcdata_alloc, (void **)&(copy->wcd_navi.wcdata_x), error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, store->wcd_navi.wcdata_alloc, (void **)&(copy->wcd_navi.wcdata_y),
			                     error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, store->wcd_navi.wcdata_alloc, (void **)&(copy->wcd_navi.wcdata_mag),
			                     error);
		if (status == MB_SUCCESS) {
			copy->wcd_navi.wcdata_alloc = store->wcd_navi.wcdata_alloc;
			const size_t copy_len = (size_t)(sizeof(float) * copy->wcd_navi.num_points);
			memcpy(copy->wcd_navi.wcdata_x, store->wcd_navi.wcdata_x, copy_len);
			memcpy(copy->wcd_navi.wcdata_y, store->wcd_navi.wcdata_y, copy_len);
			memcpy(copy->wcd_navi.wcdata_mag, store->wcd_navi.wcdata_mag, copy_len);
		}
	}

	/* SYS_CFG1 record */
	const size_t sys_cfg1_data_alloc_save = copy->sys_cfg1.sys_cfg1_data_alloc;
	char *sys_cfg1_data_save = copy->sys_cfg1.sys_cfg1_data;
	copy->sys_cfg1 = store->sys_cfg1;
	copy->sys_cfg1.sys_cfg1_data_alloc = sys_cfg1_data_alloc_save;
	copy->sys_cfg1.sys_cfg1_data = sys_cfg1_data_save;
	if (status == MB_SUCCESS && copy->sys_cfg1.sys_cfg1_data_alloc < store->sys_cfg1.sys_cfg1_data_alloc) {
		status = mb_reallocd(verbose, __FILE__, __LINE__, store->sys_cfg1.sys_cfg1_data_alloc,
		                     (void **)&(copy->sys_cfg1.sys_cfg1_data), error);
		if (status == MB_SUCCESS) {
			copy->sys_cfg1.sys_cfg1_data_alloc = store->sys_cfg1.sys_cfg1_data_alloc;
			const size_t copy_len = (size_t)(copy->sys_cfg1.sys_cfg1_len);
			memcpy(copy->sys_cfg1.sys_cfg1_data, store->sys_cfg1.sys_cfg1_data, copy_len);
		}
	}

	/* COMMENT_ Record */
	copy->mcomment.comment_length = store->mcomment.comment_length;
	strncpy(copy->mcomment.comment_message, store->mcomment.comment_message, MB_COMMENT_MAXLINE);

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
