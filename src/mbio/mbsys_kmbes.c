/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_kmbes.c	3.00	1/27/2014
 *	$Id: mbsys_kmbes.c 2308 2017-06-04 19:55:48Z caress $
 *
 *    Copyright (c) 2014-2017 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_kmbes.c contains the MBIO functions for handling data from
 * the following data formats:
 *    MBSYS_KMBES formats (code in mbsys_kmbes.c and mbsys_kmbes.h):
 *      MBF_TEMPFORM : MBIO ID ??? (code in mbr_kmbes.c)
 *
 * Author:	D. W. Caress
 * Date:	January 27, 2014
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_kmbes.h"

static char version_id[] = "$Id: mbsys_kmbes.c 2308 2017-06-04 19:55:48Z caress $";

/*--------------------------------------------------------------------*/
int mbsys_kmbes_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	char *function_name = "mbsys_kmbes_alloc";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_kmbes_struct), (void **)store_ptr, error);

    /* initialize allocated structure to zero */
    if (status == MB_SUCCESS) {
        memset(*store_ptr, 0, sizeof(struct mbsys_wassp_struct));
    }

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)*store_ptr;

	/* initialize data record kind */
	store->kind = MB_DATA_NONE;

	/* initialize data struct pointers to NULL */
    for (int i=0; i<MBSYS_KMBES_MAX_NUM_MWC_DGMS; i++)
        store->mwc[i].beamData_p = NULL;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	char *function_name = "mbsys_kmbes_deall";
	int status = MB_SUCCESS;
	struct mbsys_kmbes_struct *store;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)*store_ptr;

	/* deallocate any arrays or structures contained within the store data structure */
    for (int i=0; i<MBSYS_KMBES_MAX_NUM_MWC_DGMS; i++) {
        if (store->mwc[i].beamData_p != NULL) {

            for (int k=0; k<store->mwc[i].rxInfo.numBeams; k++) {
                status = mb_freed(verbose, __FILE__, __LINE__,
                		(void **)(&store->mwc[i].beamData_p[k].sampleAmplitude05dB_p), error);
            };

            status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&store->mwc[i].beamData_p), error);
            store->mwc[i].beamData_p = NULL;
        }
    }

	/* deallocate memory for data structure */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_kmbes_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                                    int *error) {
	char *function_name = "mbsys_kmbes_dimensions";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		*nbath = store->num_soundings;
		*namp = store->num_soundings;
		*nss = store->num_sidescan_samples;
	}
	else {
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		fprintf(stderr, "dbg2        namp:      %d\n", *namp);
		fprintf(stderr, "dbg2        nss:       %d\n", *nss);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_pingnumber(int verbose, void *mbio_ptr, int *pingnumber, int *error) {
	char *function_name = "mbsys_kmbes_pingnumber";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_index *dgm_index;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)mb_io_ptr->store_data;
	dgm_index = (struct mbsys_kmbes_index *)&(store->dgm_index[0]);

	/* extract data from structure */
	*pingnumber = dgm_index->ping_num;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pingnumber: %d\n", *pingnumber);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
	char *function_name = "mbsys_kmbes_sonartype";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get sonar type */
	*sonartype = MB_TOPOGRAPHY_TYPE_ECHOSOUNDER;  // TODO: review this setting

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sonartype:  %d\n", *sonartype);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error) {
	char *function_name = "mbsys_kmbes_sidescantype";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get sidescan type */
	*ss_type = MB_SIDESCAN_LINEAR; // TODO: review this setting

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ss_type:    %d\n", *ss_type);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                                 double *navlon, double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss,
                                 char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                                 double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
	char *function_name = "mbsys_kmbes_extract";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
    struct mbsys_kmbes_mwc *mwc;
	struct mbsys_kmbes_mrz *mrz;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];

	/* get data kind */
	*kind = store->kind;

	int mrz_count = 0;
	for(i = 0; i < store->dgm_count; i++) {
		if( store->dgm_index[i].emdgm_type == MRZ ) mrz_count++;
	}

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = mrz->pingInfo.longitude_deg;
		*navlat = mrz->pingInfo.latitude_deg;

		/* get speed */
		// TODO: *speed = store->speed;

		/* get heading */
		*heading = mrz->pingInfo.headingVessel_deg;

		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_xtrack = store->beam_width_xtrack_degrees;
		mb_io_ptr->beamwidth_ltrack = store->beam_width_ltrack_degrees;

		/* TODO: read distance and depth values into storage arrays */
		*nbath = store->num_soundings;
		*namp = *nbath;

		int soundings_count = 0;
		for(int j = 0; j < mrz_count; j++) {

			mrz = (struct mbsys_kmbes_mrz *)&store->mrz[j];

			int numSoundings = mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections;

			for (i = 0; i < numSoundings; i++) {
				bath[soundings_count] = mrz->sounding[i].z_reRefPoint_m + mrz->pingInfo.z_waterLevelReRefPoint_m;  // TODO: correct for RefPoint?
				beamflag[soundings_count] = mrz->sounding[i].beamflag; // set in
				bathacrosstrack[soundings_count] = mrz->sounding[i].x_reRefPoint_m;
				bathalongtrack[soundings_count] = mrz->sounding[i].y_reRefPoint_m;
				amp[soundings_count] = mrz->sounding[i].reflectivity1_dB;

				soundings_count++;
			}

			/* TODO: extract sidescan */
			*nss = 0;

			*nss = store->num_sidescan_samples;
			pixel_size = ping->png_pixel_size;
			for (i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++) {
				if (ping->png_ss[i] == EM3_INVALID_SS || (ping->png_ss[i] == EM3_INVALID_AMP && ping->png_ssalongtrack[i] == 0)) {
					ss[i] = MB_SIDESCAN_NULL;
					ssacrosstrack[i] = pixel_size * (i - MBSYS_SIMRAD3_MAXPIXELS / 2);
					ssalongtrack[i] = 0.0;
				}
				else {
					ss[i] = 0.01 * ping->png_ss[i];
					ssacrosstrack[i] = pixel_size * (i - MBSYS_SIMRAD3_MAXPIXELS / 2);
					ssalongtrack[i] = 0.01 * ping->png_ssalongtrack[i];
				}
			}

		}


		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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
			for (i = 0; i < *nbath; i++)
				fprintf(stderr, "dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
			fprintf(stderr, "dbg4        namp:     %d\n", *namp);
			for (i = 0; i < *namp; i++)
				fprintf(stderr, "dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
			fprintf(stderr, "dbg4        nss:      %d\n", *nss);
			for (i = 0; i < *nss; i++)
				fprintf(stderr, "dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
		}


		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = store->longitude;
		*navlat = store->latitude;

		/* get speed */
		*speed = store->speed;

		/* get heading */
		*heading = store->heading;

		/* set beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

		/* print debug statements */
		if (verbose >= 5) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* copy comment */
		if (store->comment > 0)
			strncpy(comment, store->comment, MB_COMMENT_MAXLINE);
		else
			comment[0] = '\0';

		/* print debug statements */
		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Comment extracted by MBIO function <%s>\n", function_name);
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
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* print debug statements */
		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
		for (i = 0; i < *nbath; i++)
			fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
			        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2        namp:     %d\n", *namp);
		for (i = 0; i < *namp; i++)
			fprintf(stderr, "dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
			        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:      %d\n", *nss);
		for (i = 0; i < *nss; i++)
			fprintf(stderr, "dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
			        ssalongtrack[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d,
                                double navlon, double navlat, double speed, double heading, int nbath, int namp, int nss,
                                char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                                double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
	char *function_name = "mbsys_kmbes_insert";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
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
			for (i = 0; i < nbath; i++)
				fprintf(stderr, "dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2       namp:       %d\n", namp);
		if (verbose >= 3)
			for (i = 0; i < namp; i++)
				fprintf(stderr, "dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:       %d\n", nss);
		if (verbose >= 3)
			for (i = 0; i < nss; i++)
				fprintf(stderr, "dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		store->longitude = navlon;
		store->latitude = navlat;

		/* get heading */
		store->heading = heading;

		/* get speed  */
		store->speed = speed;

		/* read distance and depth values into storage arrays */
		store->number_beams = nbath;
		for (i = 0; i < bathymetry->number_beams; i++) {
			store->depth[i] = bath[i];
			store->beamflag[i] = beamflag[i];
			store->acrosstrack[i] = bathacrosstrack[i];
			store->alongtrack[i] = bathalongtrack[i];
			store->amplitude[i] = amp[i];
		}

		/* insert the sidescan */
		store->number_pixels = nss;
		for (i = 0; i < store->number_pixels; i++) {
			store->sidescan[i] = ss[i];
			store->acrosstrack[i] = store->acrosstrack[i];
			store->alongtrack[i] = store->alongtrack[i];
		}
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		store->longitude = navlon;
		store->latitude = navlat;

		/* get heading */
		store->heading = heading;

		/* get speed  */
		store->speed = speed;
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
		strncpy(store->comment, comment, MB_COMMENT_MAXLINE);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes,
                                double *angles, double *angles_forward, double *angles_null, double *heave,
                                double *alongtrack_offset, double *draft, double *ssv, int *error) {
	char *function_name = "mbsys_kmbes_ttimes";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	double heave_use, roll, pitch;
	double alpha, beta, theta, phi;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
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
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get depth offset (heave + sonar depth) */
		*ssv = store->ssv;

		/* get draft */
		*draft = store->static_draft + store->dynamic_draft;

		/* get travel times, angles */
		*nbeams = bathymetry->number_beams;
		for (i = 0; i < bathymetry->number_beams; i++) {
			ttimes[i] = store->ttimes[i];
			angles[i] = store->vertical_angle[i];
			angles_forward[i] = store->azimuthal_angle[i];
			angles_null[i] = store->angles_null[i];
			heave[i] = store->heave_beam[i];
			alongtrack_offset[i] = store->alongtrack_offset;
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

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       draft:      %f\n", *draft);
		fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
			        i, ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
	char *function_name = "mbsys_kmbes_detects";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get detect type for each sounding - options include:
		    MB_DETECT_UNKNOWN
		    MB_DETECT_AMPLITUDE
		    MB_DETECT_PHASE
		    MB_DETECT_UNKNOWN */
		*nbeams = bathymetry->number_beams;
		for (i = 0; i < *nbeams; i++) {
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

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain,
                               double *pulse_length, double *receive_gain, int *error) {
	char *function_name = "mbsys_kmbes_gains";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get transmit_gain (dB) */
		*transmit_gain = store->transmit_gain;

		/* get pulse_length (usec) */
		*pulse_length = store->pulse_width;

		/* get receive_gain (dB) */
		*receive_gain = store->receive_gain;

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

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                          double *altitudev, int *error) {
	char *function_name = "mbsys_kmbes_extract_altitude";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	double heave, roll, pitch;
	double xtrackmin;
	int altitude_found;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get transducer depth and altitude */
		*transducer_depth = store->static_draft + store->dynamic_draft + store->heave;

		/* get altitude */
		*altitude = store->altitude;

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

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitudev);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                                     double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                                     double *pitch, double *heave, int *error) {
	char *function_name = "mbsys_kmbes_extract_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from survey record */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = store->longitude;
		*navlat = store->latitude;

		/* get speed */
		*speed = store->speed;

		/* get heading */
		*heading = store->heading;

		/* get draft  */
		*draft = store->static_draft + store->dynamic_draft;
		;

		/* get attitude  */
		*roll = store->roll;
		*pitch = store->pitch;
		*heave = store->heave;

		/* done translating values */
	}

	/* extract data from nav record */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;

		/* get navigation */
		*navlon = store->longitude;
		*navlat = store->latitude;

		/* get speed */
		*speed = store->speed;

		/* get heading */
		*heading = store->heading;

		/* get draft  */
		*draft = store->static_draft + store->dynamic_draft;
		;

		/* get attitude  */
		*roll = store->roll;
		*pitch = store->pitch;
		*heave = store->heave;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		*time_d = store->time_d;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i,
                                      double *time_d, double *navlon, double *navlat, double *speed, double *heading,
                                      double *draft, double *roll, double *pitch, double *heave, int *error) {
	char *function_name = "mbsys_kmbes_extract_nnav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	int i, inav;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from survey record */
	if (*kind == MB_DATA_DATA) {
		/* just one navigation value */
		*n = 1;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;

		/* get navigation */
		navlon[0] = store->longitude;
		navlat = store->latitude;

		/* get speed */
		speed[0] = store->speed;

		/* get heading */
		heading[0] = store->heading;

		/* get draft  */
		draft[0] = store->static_draft + store->dynamic_draft;
		;

		/* get attitude  */
		roll[0] = store->roll;
		pitch[0] = store->pitch;
		heave[0] = store->heave;

		/* done translating values */
	}

	/* extract data from nav record */
	else if (*kind == MB_DATA_NAV) {
		/* just one navigation value - in some formats there
		    are multiple values in nav records to loop over */
		*n = 1;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;

		/* get navigation */
		navlon[0] = store->longitude;
		navlat[0] = store->latitude;

		/* get speed */
		speed[0] = store->speed;

		/* get heading */
		heading[0] = store->heading;

		/* get draft  */
		draft[0] = store->static_draft + store->dynamic_draft;
		;

		/* get attitude  */
		roll[0] = store->roll;
		pitch[0] = store->pitch;
		heave[0] = store->heave;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;

		/* get time */
		for (i = 0; i < 7; i++)
			time_i[i] = store->time_i[i];
		time_d[0] = store->time_d;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       n:          %d\n", *n);
		for (inav = 0; inav < *n; inav++) {
			for (i = 0; i < 7; i++)
				fprintf(stderr, "dbg2       %d time_i[%d]:     %d\n", inav, i, time_i[inav * 7 + i]);
			fprintf(stderr, "dbg2       %d time_d:        %f\n", inav, time_d[inav]);
			fprintf(stderr, "dbg2       %d longitude:     %f\n", inav, navlon[inav]);
			fprintf(stderr, "dbg2       %d latitude:      %f\n", inav, navlat[inav]);
			fprintf(stderr, "dbg2       %d speed:         %f\n", inav, speed[inav]);
			fprintf(stderr, "dbg2       %d heading:       %f\n", inav, heading[inav]);
			fprintf(stderr, "dbg2       %d draft:         %f\n", inav, draft[inav]);
			fprintf(stderr, "dbg2       %d roll:          %f\n", inav, roll[inav]);
			fprintf(stderr, "dbg2       %d pitch:         %f\n", inav, pitch[inav]);
			fprintf(stderr, "dbg2       %d heave:         %f\n", inav, heave[inav]);
		}
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                                    double navlat, double speed, double heading, double draft, double roll, double pitch,
                                    double heave, int *error) {
	char *function_name = "mbsys_kmbes_insert_nav";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
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
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* insert data in ping structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		store->longitude = navlon;
		store->latitude = navlat;

		/* get speed  */
		store->speed = speed;

		/* get heading */
		store->heading = heading;

		/* get draft  */
		*draft = store->static_draft + store->dynamic_draft;
		;
		store->dynamic_draft = *draft - store->static_draft;

		/* get roll pitch and heave */
		store->heave = heave;
		store->pitch = pitch;
		store->roll = roll;
	}

	/* insert data in nav structure */
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		for (i = 0; i < 7; i++)
			store->time_i[i] = time_i[i];
		store->time_d = time_d;

		/* get navigation */
		store->longitude = navlon;
		store->latitude = navlat;

		/* get speed  */
		store->speed = speed;

		/* get heading */
		store->heading = heading;

		/* get draft  */
		*draft = store->static_draft + store->dynamic_draft;
		;
		store->dynamic_draft = *draft - store->static_draft;

		/* get roll pitch and heave */
		store->heave = heave;
		store->pitch = pitch;
		store->roll = roll;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth,
                                     double *velocity, int *error) {
	char *function_name = "mbsys_kmbes_extract_svp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		*nsvp = store->number_svp;

		/* get profile */
		for (i = 0; i < *nsvp; i++) {
			depth[i] = store->svp_depth[i];
			velocity[i] = store->svp_sv[i];
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

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
		for (i = 0; i < *nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
                                    int *error) {
	char *function_name = "mbsys_kmbes_insert_svp";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	s7kr_svp *svp;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
		for (i = 0; i < nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE) {
		/* get profile */
		if (status == MB_SUCCESS) {
			store->number_svp = MIN(nsvp, MBSYS_KMBES_NUMBER_SVP_MAX);
			for (i = 0; i < store->number_svp; i++) {
				store->svp_depth[i] = depth[i];
				store->svp_sv[i] = velocity[i];
			}
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
	char *function_name = "mbsys_kmbes_copy";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	struct mbsys_kmbes_struct *copy;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", version_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
	}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_kmbes_struct *)store_ptr;
	copy = (struct mbsys_kmbes_struct *)copy_ptr;

	/* copy the data - for many formats memory must be allocated and
	    sub-structures copied separately */
	*copy = *store;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
int mbsys_kmbes_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
						 int swath_width_set, double *swath_width, int pixel_int, int *error) {
	char *function_name = "mbsys_kmbes_makess";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_kmbes_struct *store;
	struct mbsys_simrad3_ping_struct *ping;
	double ss[MBSYS_KMBES_MAXPIXELS];
	int ss_cnt[MBSYS_KMBES_MAXPIXELS];
	double ssacrosstrack[MBSYS_KMBES_MAXPIXELS];
	double ssalongtrack[MBSYS_KMBES_MAXPIXELS];
	short *beam_ss;
	int nbathsort;
	double bathsort[MBSYS_KMBES_MAXPIXELS];
	double depthoffset;
	double reflscale;
	double pixel_size_calc;
	double pixel_size_max_swath;
	double ss_spacing, ss_spacing_use;
	int pixel_int_use;
	double angle, xtrackss;
	double range, beam_foot, beamwidth, sint;
	int time_i[7];
	double bath_time_d, ss_time_d;
	int ss_ok;
	int first, last, k1, k2;
	int i, k, kk;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
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

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_kmbes_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get survey data structure */
		ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

		/* zero the sidescan */
		for (i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++) {
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
			ss_cnt[i] = 0;
		}

		/* set scaling parameters */
		depthoffset = ping->png_xducer_depth;
		reflscale = 0.1;

		/* get raw pixel size */
		ss_spacing = 750.0 / ping->png_sample_rate;

		/* get beam angle size */
		if (store->sonar == MBSYS_SIMRAD3_EM1000) {
			beamwidth = 2.5;
		}
		else if (ping->png_tx > 0) {
			beamwidth = 0.1 * ping->png_tx;
		}
		else if (store->run_tran_beam > 0) {
			beamwidth = 0.1 * store->run_tran_beam;
		}

		/* get median depth */
		nbathsort = 0;
		for (i = 0; i < ping->png_nbeams; i++) {
			if (mb_beam_ok(ping->png_beamflag[i])) {
				bathsort[nbathsort] = ping->png_depth[i] + depthoffset;
				nbathsort++;
			}
		}

		/* get sidescan pixel size */
		if (swath_width_set == MB_NO) {
			if (store->run_swath_angle > 0)
				*swath_width = (double)store->run_swath_angle;
			else
				*swath_width = 2.5 + MAX(90.0 - ping->png_depression[0], 90.0 - ping->png_depression[ping->png_nbeams - 1]);
		}
		if (pixel_size_set == MB_NO && nbathsort > 0) {
			qsort((char *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
			pixel_size_calc = 2 * tan(DTR * (*swath_width)) * bathsort[nbathsort / 2] / MBSYS_SIMRAD3_MAXPIXELS;
			if (store->run_max_swath > 0) {
				pixel_size_max_swath = 2 * ((double)store->run_max_swath) / ((double)MBSYS_SIMRAD3_MAXPIXELS);
				if (pixel_size_max_swath < pixel_size_calc)
					pixel_size_calc = pixel_size_max_swath;
			}
			pixel_size_calc = MAX(pixel_size_calc, bathsort[nbathsort / 2] * sin(DTR * 0.1));
			if ((*pixel_size) <= 0.0)
				(*pixel_size) = pixel_size_calc;
			else if (0.95 * (*pixel_size) > pixel_size_calc)
				(*pixel_size) = 0.95 * (*pixel_size);
			else if (1.05 * (*pixel_size) < pixel_size_calc)
				(*pixel_size) = 1.05 * (*pixel_size);
			else
				(*pixel_size) = pixel_size_calc;
		}
		/* get pixel interpolation */
		pixel_int_use = pixel_int + 1;

		/* check that sidescan can be used */
		/* get times of bath and sidescan records */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &bath_time_d);
		time_i[0] = ping->png_ss_date / 10000;
		time_i[1] = (ping->png_ss_date % 10000) / 100;
		time_i[2] = ping->png_ss_date % 100;
		time_i[3] = ping->png_ss_msec / 3600000;
		time_i[4] = (ping->png_ss_msec % 3600000) / 60000;
		time_i[5] = (ping->png_ss_msec % 60000) / 1000;
		time_i[6] = (ping->png_ss_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ss_time_d);
		ss_ok = MB_YES;
		if (ping->png_nbeams < ping->png_nbeams_ss || ping->png_nbeams > ping->png_nbeams_ss + 1) {
			ss_ok = MB_NO;
			if (verbose > 0)
				fprintf(stderr,
						"%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: num bath beams != num ss beams: %d %d\n",
						function_name, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
						ping->png_nbeams, ping->png_nbeams_ss);
		}
		/*fprintf(stderr,"%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d   bathbeams:%d   ssbeams:%d\n",
		function_name, time_i[0], time_i[1], time_i[2],
		time_i[3], time_i[4], time_i[5], time_i[6],
		ping->png_nbeams, ping->png_nbeams_ss);*/

		/* loop over raw sidescan, putting each raw pixel into
		    the binning arrays */
		if (ss_ok == MB_YES)
			for (i = 0; i < ping->png_nbeams_ss; i++) {
				beam_ss = &ping->png_ssraw[ping->png_start_sample[i]];
				if (mb_beam_ok(ping->png_beamflag[i])) {
					if (ping->png_beam_samples[i] > 0) {
						range =
								sqrt(ping->png_depth[i] * ping->png_depth[i] + ping->png_acrosstrack[i] * ping->png_acrosstrack[i]);
						angle = 90.0 - ping->png_depression[i];
						beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
						sint = fabs(sin(DTR * angle));
						if (sint < ping->png_beam_samples[i] * ss_spacing / beam_foot)
							ss_spacing_use = beam_foot / ping->png_beam_samples[i];
						else
							ss_spacing_use = ss_spacing / sint;
						/* fprintf(stderr, "spacing: %f %f n:%d sint:%f beamwidth:%f angle:%f range:%f foot:%f factor:%f\n",
						ss_spacing, ss_spacing_use,
						ping->png_beam_samples[i], sint, beamwidth, angle, range, beam_foot,
						ping->png_beam_samples[i] * ss_spacing / beam_foot);*/
					}
					for (k = 0; k < ping->png_beam_samples[i]; k++) {
						if (beam_ss[k] != EM3_INVALID_AMP) {
							xtrackss = ping->png_acrosstrack[i] + ss_spacing_use * (k - ping->png_center_sample[i]);
							kk = MBSYS_SIMRAD3_MAXPIXELS / 2 + (int)(xtrackss / (*pixel_size));
							if (kk > 0 && kk < MBSYS_SIMRAD3_MAXPIXELS) {
								ss[kk] += reflscale * ((double)beam_ss[k]);
								ssalongtrack[kk] += ping->png_alongtrack[i];
								ss_cnt[kk]++;
							}
						}
					}
				}
			}

		/* average the sidescan */
		first = MBSYS_SIMRAD3_MAXPIXELS;
		last = -1;
		for (k = 0; k < MBSYS_SIMRAD3_MAXPIXELS; k++) {
			if (ss_cnt[k] > 0) {
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				ssacrosstrack[k] = (k - MBSYS_SIMRAD3_MAXPIXELS / 2) * (*pixel_size);
				first = MIN(first, k);
				last = k;
			}
			else
				ss[k] = MB_SIDESCAN_NULL;
		}

		/* interpolate the sidescan */
		k1 = first;
		k2 = first;
		for (k = first + 1; k < last; k++) {
			if (ss_cnt[k] <= 0) {
				if (k2 <= k) {
					k2 = k + 1;
					while (ss_cnt[k2] <= 0 && k2 < last)
						k2++;
				}
				if (k2 - k1 <= pixel_int_use) {
					ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
					ssacrosstrack[k] = (k - MBSYS_SIMRAD3_MAXPIXELS / 2) * (*pixel_size);
					ssalongtrack[k] =
							ssalongtrack[k1] + (ssalongtrack[k2] - ssalongtrack[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
				}
			}
			else {
				k1 = k;
			}
		}

		/* insert the new sidescan into store */
		ping->png_pixel_size = *pixel_size;
		if (last > first)
			ping->png_pixels_ss = MBSYS_SIMRAD3_MAXPIXELS;
		else
			ping->png_pixels_ss = 0;
		for (i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++) {
			if (ss[i] > MB_SIDESCAN_NULL) {
				ping->png_ss[i] = (short)(100 * ss[i]);
				ping->png_ssalongtrack[i] = (short)(100 * ssalongtrack[i]);
			}
			else {
				ping->png_ss[i] = EM3_INVALID_SS;
				ping->png_ssalongtrack[i] = 0;
			}
		}

		/* print debug statements */
		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Sidescan regenerated in <%s>\n", function_name);
			fprintf(stderr, "dbg2       png_nbeams_ss: %d\n", ping->png_nbeams_ss);
			for (i = 0; i < ping->png_nbeams_ss; i++)
				fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  amp:%d  acrosstrack:%f  alongtrack:%f\n", i,
						ping->png_beamflag[i], ping->png_depth[i], ping->png_amp[i], ping->png_acrosstrack[i],
						ping->png_alongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", MBSYS_SIMRAD3_MAXPIXELS);
			for (i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, ss_cnt[i], ss[i],
						ssacrosstrack[i], ssalongtrack[i]);
			fprintf(stderr, "dbg2       pixels_ss:  %d\n", ping->png_pixels_ss);
			for (i = 0; i < MBSYS_SIMRAD3_MAXPIXELS; i++)
				fprintf(stderr, "dbg2       pixel:%4d  ss:%8d  ltrack:%8d\n", i, ping->png_ss[i], ping->png_ssalongtrack[i]);
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
		fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/