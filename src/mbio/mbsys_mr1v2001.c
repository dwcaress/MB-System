/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_mr1v2001.c	3/6/2003
 *
 *    Copyright (c) 2003-2025 by
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
 * mbsys_mr1v2001v2001.c defines the data structures used by MBIO functions
 * to store interferometry sonard data processed by the Hawaii Mapping
 * Research Group. This includes data from the MR1, SCAMP, and WHOI
 * DSL 120
 *.
 * The data formats associated with mbsys_mr1v2001v2001 are:
 *      MBF_MR1PRVR2 : MBIO ID 64
 *
 * Author:	D. W. Caress
 * Date:	March 6, 2003
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbbs.h"
#include "mbsys_mr1v2001.h"

/*--------------------------------------------------------------------*/
int mbsys_mr1v2001_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_mr1v2001_struct), store_ptr, error);
	memset(*store_ptr, 0, sizeof(struct mbsys_mr1v2001_struct));

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
int mbsys_mr1v2001_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
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
int mbsys_mr1v2001_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
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
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;

	// BSFile *header = &(store->header);
	Ping *ping = &(store->ping);
	PingSide *pingport = &(ping->png_sides[ACP_PORT]);
	PingSide *pingstbd = &(ping->png_sides[ACP_STBD]);
	// PingData *pingdata = &(store->pingdata);
	// float *pbty = (float *)(pingdata->pd_bty[ACP_PORT]);
	// unsigned int *pbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_PORT]);
	// float *pss = (float *)(pingdata->pd_ss[ACP_PORT]);
	// unsigned char *pssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_PORT]);
	// AuxBeamInfo *pabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_PORT]);
	// float *sbty = (float *)(pingdata->pd_bty[ACP_STBD]);
	// unsigned int *sbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_STBD]);
	// float *sss = (float *)(pingdata->pd_ss[ACP_STBD]);
	// unsigned char *sssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_STBD]);
	// AuxBeamInfo *sabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_STBD]);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		*nbath = 2 * MAX(pingport->ps_btycount, pingstbd->ps_btycount);
		*namp = 0;
		*nss = 2 * MAX(pingport->ps_sscount, pingstbd->ps_sscount);
	}

	/* extract data from structure */
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
int mbsys_mr1v2001_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
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
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;

	// BSFile *header = &(store->header);
	Ping *ping = &(store->ping);
	PingSide *pingport = &(ping->png_sides[ACP_PORT]);
	PingSide *pingstbd = &(ping->png_sides[ACP_STBD]);
	PingData *pingdata = &(store->pingdata);
	float *pbty = (float *)(pingdata->pd_bty[ACP_PORT]);
	unsigned int *pbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_PORT]);
	float *pss = (float *)(pingdata->pd_ss[ACP_PORT]);
	// unsigned char *pssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_PORT]);
	// AuxBeamInfo *pabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_PORT]);
	float *sbty = (float *)(pingdata->pd_bty[ACP_STBD]);
	unsigned int *sbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_STBD]);
	float *sss = (float *)(pingdata->pd_ss[ACP_STBD]);
	// unsigned char *sssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_STBD]);
	// AuxBeamInfo *sabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_STBD]);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		*time_d = ping->png_tm.tv_sec + 0.000001 * ping->png_tm.tv_usec;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		if (!mbbs_isnand(ping->png_tlon) && !mbbs_isnand(ping->png_tlon)) {
			*navlon = ping->png_tlon;
			*navlat = ping->png_tlat;
		}
		else if (!mbbs_isnand(ping->png_slon) && !mbbs_isnand(ping->png_slon)) {
			*navlon = ping->png_slon;
			*navlat = ping->png_slat;
		}
		else {
			*navlon = 0.0;
			*navlat = 0.0;
		}

		/* get heading */
		if (!mbbs_isnand(store->ping.png_compass.sns_repval)) {
			*heading = store->ping.png_compass.sns_repval;
			if (!mbbs_isnand(store->ping.png_magcorr))
				*heading += store->ping.png_magcorr;
		}
		else if (!mbbs_isnand(ping->png_tcourse))
			*heading = ping->png_tcourse;
		if (*heading < 0.0)
			*heading += 360.0;
		if (*heading >= 360.0)
			*heading -= 360.0;

		/* set speed to zero */
		*speed = 0.0;

		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 0.1;

		/* zero data arrays */
		for (int i = 0; i < MBSYS_MR1V2001_BEAMS; i++) {
			beamflag[i] = MB_FLAG_NULL;
			bath[i] = 0.0;
			bathacrosstrack[i] = 0.0;
			bathalongtrack[i] = 0.0;
		}
		for (int i = 0; i < MBSYS_MR1V2001_PIXELS; i++) {
			ss[i] = 0.0;
			ssacrosstrack[i] = 0.0;
			ssalongtrack[i] = 0.0;
		}

		/* set up to extract beam and pixel values */
		*nbath = 2 * MAX(pingport->ps_btycount, pingstbd->ps_btycount);
		*namp = 0;
		*nss = 2 * MAX(pingport->ps_sscount, pingstbd->ps_sscount);
		if (*nss > 0)
			*nss += 3;
		const int beam_center = *nbath / 2;
		const int pixel_center = *nss / 2;

		/* extract bathymetry */
		if (store->ping.png_flags & PNG_XYZ) {
			for (int i = 0; i < pingport->ps_btycount; i++) {
				const int j = beam_center - 1 - i;
				if (pbtyflags[i] == 0) {
					beamflag[j] = MB_FLAG_NONE;
				}
				else {
					beamflag[j] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
				}
				bathacrosstrack[j] = -pbty[3 * i];
				bathalongtrack[j] = pbty[3 * i + 1];
				bath[j] = pbty[3 * i + 2];
			}
			for (int i = 0; i < pingstbd->ps_btycount; i++) {
				const int j = beam_center + i;
				if (pbtyflags[i] == 0) {
					beamflag[j] = MB_FLAG_NONE;
				}
				else {
					beamflag[j] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
				}
				bathacrosstrack[j] = sbty[3 * i];
				bathalongtrack[j] = sbty[3 * i + 1];
				bath[j] = sbty[3 * i + 2];
			}
		}
		else {
			for (int i = 0; i < pingport->ps_btycount; i++) {
				const int j = beam_center - i - 1;
				if (pbtyflags[i] == 0) {
					beamflag[j] = MB_FLAG_NONE;
				}
				else {
					beamflag[j] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
				}
				bathacrosstrack[j] = -pbty[2 * i];
				bathalongtrack[j] = 0.0;
				bath[j] = pbty[2 * i + 1];
			}
			for (int i = 0; i < pingstbd->ps_btycount; i++) {
				const int j = beam_center + i;
				if (sbtyflags[i] == 0) {
					beamflag[j] = MB_FLAG_NONE;
				}
				else {
					beamflag[j] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
				}
				bathacrosstrack[j] = sbty[2 * i];
				bathalongtrack[j] = 0.0;
				bath[j] = sbty[2 * i + 1];
			}
		}

		/* extract sidescan */
		double ssyoffset;
		if (!mbbs_isnand(pingport->ps_ssyoffset))
			ssyoffset = pingport->ps_ssyoffset;
		else
			ssyoffset = 0.0;
		for (int i = 0; i < pingport->ps_sscount; i++) {
			const int j = pixel_center - i - 1;
			ss[j] = pss[i];
			ssacrosstrack[j] = -pingport->ps_ssxoffset - i * ping->png_ssincr;
			ssalongtrack[j] = ssyoffset;
		}
		if (!mbbs_isnand(pingstbd->ps_ssyoffset))
			ssyoffset = pingstbd->ps_ssyoffset;
		else
			ssyoffset = 0.0;
		for (int i = 0; i < pingstbd->ps_sscount; i++) {
			const int j = pixel_center + i;
			ss[j] = sss[i];
			ssacrosstrack[j] = pingstbd->ps_ssxoffset + i * ping->png_ssincr;
			ssalongtrack[j] = ssyoffset;
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
				fprintf(stderr, "dbg4       beam:%d  flag:%3d  bath:%6g  acrosstrack:%6g  alongtrack:%6g\n", i, beamflag[i],
				        bath[i], bathacrosstrack[i], bathalongtrack[i]);
			fprintf(stderr, "dbg4        namp:     %d\n", *namp);
			for (int i = 0; i < *namp; i++)
				fprintf(stderr, "dbg4        beam:%d   amp:%6g  acrosstrack:%6g  alongtrack:%6g\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
			fprintf(stderr, "dbg4        nss:      %d\n", *nss);
			for (int i = 0; i < *nss; i++)
				fprintf(stderr, "dbg4        pixel:%d   ss:%6g  acrosstrack:%6g  alongtrack:%6g\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
		}

		/* done translating values */
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* copy comment */
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
		strncpy(comment, store->comment, MIN(MB_COMMENT_MAXLINE, MBSYS_MR1V2001_MAXLINE) - 1);

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
int mbsys_mr1v2001_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
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
				fprintf(stderr, "dbg3       beam:%d  flag:%3d bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
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
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;

	// BSFile *header = &(store->header);
	Ping *ping = &(store->ping);
	PingSide *pingport = &(ping->png_sides[ACP_PORT]);
	PingSide *pingstbd = &(ping->png_sides[ACP_STBD]);
	PingData *pingdata = &(store->pingdata);
	float *pbty = (float *)(pingdata->pd_bty[ACP_PORT]);
	unsigned int *pbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_PORT]);
	float *pss = (float *)(pingdata->pd_ss[ACP_PORT]);
	// unsigned char *pssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_PORT]);
	// AuxBeamInfo *pabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_PORT]);
	float *sbty = (float *)(pingdata->pd_bty[ACP_STBD]);
	unsigned int *sbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_STBD]);
	float *sss = (float *)(pingdata->pd_ss[ACP_STBD]);
	// unsigned char *sssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_STBD]);
	// AuxBeamInfo *sabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_STBD]);

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		ping->png_tm.tv_sec = (int)time_d;
		ping->png_tm.tv_usec = (int)1000000.0 * (time_d - ping->png_tm.tv_sec);

		/* get navigation */
		if (navlon < 0.0)
			navlon = navlon + 360.0;
		ping->png_tlon = navlon;
		ping->png_tlat = navlat;

		/* get heading */
		store->ping.png_compass.sns_repval = heading;
		if (!mbbs_isnand(store->ping.png_magcorr))
			store->ping.png_compass.sns_repval -= store->ping.png_magcorr;

		/* get speed */

		/* get bathymetry */
		const int beam_center = nbath / 2;
		if (store->ping.png_flags & PNG_XYZ) {
			/* get port bathymetry */
			for (int i = 0; i < pingport->ps_btycount; i++) {
				const int j = beam_center - i - 1;
				if (beamflag[j] != MB_FLAG_NULL) {
					pbty[2 * i] = -bathacrosstrack[j];
					pbty[2 * i + 1] = bathalongtrack[j];
					pbty[2 * i + 2] = bath[j];
					if (mb_beam_check_flag(beamflag[j]))
						pbtyflags[i] = BTYD_EXTERNAL;
					else
						pbtyflags[i] = 0;
				}
				else {
					pbty[2 * i] = 0.0;
					pbty[2 * i + 1] = 0.0;
					pbty[2 * i + 2] = 0.0;
					pbtyflags[i] = BTYD_EXTERNAL;
				}
			}

			/* get starboard bathymetry */
			for (int i = 0; i < pingstbd->ps_btycount; i++) {
				const int j = beam_center + i;
				if (beamflag[j] != MB_FLAG_NULL) {
					sbty[2 * i] = bathacrosstrack[j];
					sbty[2 * i + 1] = bathalongtrack[j];
					sbty[2 * i + 2] = bath[j];
					if (mb_beam_check_flag(beamflag[j]))
						sbtyflags[i] = BTYD_EXTERNAL;
					else
						sbtyflags[i] = 0;
				}
				else {
					sbty[2 * i] = 0.0;
					sbty[2 * i + 1] = 0.0;
					sbty[2 * i + 2] = 0.0;
					sbtyflags[i] = BTYD_EXTERNAL;
				}
			}
		}
		else {
			/* get port bathymetry */
			for (int i = 0; i < pingport->ps_btycount; i++) {
				const int j = beam_center - i - 1;
				if (beamflag[j] != MB_FLAG_NULL) {
					pbty[2 * i] = -bathacrosstrack[j];
					pbty[2 * i + 1] = bath[j];
					if (mb_beam_check_flag(beamflag[j]))
						pbtyflags[i] = BTYD_EXTERNAL;
					else
						pbtyflags[i] = 0;
				}
				else {
					pbty[2 * i] = 0.0;
					pbty[2 * i + 1] = 0.0;
					pbtyflags[i] = BTYD_EXTERNAL;
				}
			}

			/* get starboard bathymetry */
			for (int i = 0; i < pingstbd->ps_btycount; i++) {
				const int j = beam_center + i;
				if (beamflag[j] != MB_FLAG_NULL) {
					sbty[2 * i] = bathacrosstrack[j];
					sbty[2 * i + 1] = bath[j];
					if (mb_beam_check_flag(beamflag[j]))
						sbtyflags[i] = BTYD_EXTERNAL;
					else
						sbtyflags[i] = 0;
				}
				else {
					sbty[2 * i] = 0.0;
					sbty[2 * i + 1] = 0.0;
					sbtyflags[i] = BTYD_EXTERNAL;
				}
			}
		}

		/* get port sidescan */
		const int pixel_center = nss / 2;
		for (int i = 0; i < pingport->ps_sscount; i++) {
			const int j = pixel_center - 2 - i;
			pss[i] = ss[j];
		}

		/* get starboard sidescan */
		for (int i = 0; i < pingstbd->ps_sscount; i++) {
			const int j = pixel_center + 2 + i;
			sss[i] = ss[j];
		}
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
    memset((void *)store->comment, 0, MBSYS_MR1V2001_MAXLINE);
    strncpy(store->comment, comment, MIN(MBSYS_MR1V2001_MAXLINE, MB_COMMENT_MAXLINE) - 1);
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
int mbsys_mr1v2001_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
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
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;

	/* get pointers */
	// BSFile *header = &(store->header);
	Ping *ping = &(store->ping);
	PingSide *pingport = &(ping->png_sides[ACP_PORT]);
	PingSide *pingstbd = &(ping->png_sides[ACP_STBD]);
	PingData *pingdata = &(store->pingdata);
	float *pbty = (float *)(pingdata->pd_bty[ACP_PORT]);
	// unsigned int *pbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_PORT]);
	// float *pss = (float *)(pingdata->pd_ss[ACP_PORT]);
	// unsigned char *pssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_PORT]);
	// AuxBeamInfo *pabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_PORT]);
	float *sbty = (float *)(pingdata->pd_bty[ACP_STBD]);
	// unsigned int *sbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_STBD]);
	// float *sss = (float *)(pingdata->pd_ss[ACP_STBD]);
	// unsigned char *sssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_STBD]);
	// AuxBeamInfo *sabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_STBD]);

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get sound velocity at transducers */
		if (!mbbs_isnanf(ping->png_sndvel))
			*ssv = ping->png_sndvel;
		else
			*ssv = 1500.0;
		*draft = ping->png_depth.sns_repval;

		/* get nbeams */
		*nbeams = 2 * MAX(pingport->ps_btycount, pingstbd->ps_btycount);
		const int beam_center = *nbeams / 2;

		/* zero data arrays */
		for (int i = 0; i < *nbeams; i++) {
			ttimes[i] = 0.0;
			angles[i] = 0.0;
			angles_forward[i] = 0.0;
			angles_null[i] = 0.0;
			heave[i] = 0.0;
			alongtrack_offset[i] = 0.0;
		}

		/* get travel times and angles */
		for (int i = 0; i < pingport->ps_btycount; i++) {
			const int j = beam_center - i - 1;
			double xx;
			double yy;
			double zz;
			if (store->ping.png_flags & PNG_XYZ) {
				zz = fabs(pbty[2 * i + 2]) - store->ping.png_depth.sns_repval;
				xx = -pbty[2 * i];
				yy = pbty[2 * i + 1];
			}
			else {
				zz = fabs(pbty[2 * i + 1]) - store->ping.png_depth.sns_repval;
				xx = -pbty[2 * i];
				yy = 0.0;
			}
			const double rr = sqrt(xx * xx + yy * yy + zz * zz);
			ttimes[j] = 2.0 * rr / ping->png_sndvel;
			mb_xyz_to_takeoff(verbose, xx, yy, zz, &angles[j], &angles_forward[j], error);
			heave[j] = 0.0;
			angles_null[j] = MBSYS_MR1V2001_XDUCER_ANGLE;
		}
		for (int i = 0; i < pingstbd->ps_btycount; i++) {
			const int j = beam_center + i;
			double xx;
			double yy;
			double zz;
			if (store->ping.png_flags & PNG_XYZ) {
				zz = fabs(sbty[2 * i + 2]) - store->ping.png_depth.sns_repval;
				xx = sbty[2 * i];
				yy = sbty[2 * i + 1];
			}
			else {
				zz = fabs(sbty[2 * i + 1]) - store->ping.png_depth.sns_repval;
				xx = sbty[2 * i];
				yy = 0.0;
			}
			const double rr = sqrt(xx * xx + yy * yy + zz * zz);
			ttimes[j] = 2.0 * rr / ping->png_sndvel;
			mb_xyz_to_takeoff(verbose, xx, yy, zz, &angles[j], &angles_forward[j], error);
			heave[j] = 0.0;
			angles_null[j] = MBSYS_MR1V2001_XDUCER_ANGLE;
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
int mbsys_mr1v2001_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
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
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;

	/* get pointers */
	// BSFile *header = &(store->header);
	Ping *ping = &(store->ping);
	PingSide *pingport = &(ping->png_sides[ACP_PORT]);
	PingSide *pingstbd = &(ping->png_sides[ACP_STBD]);
	// PingData *pingdata = &(store->pingdata);
	// float *pbty = (float *)(pingdata->pd_bty[ACP_PORT]);
	// unsigned int *pbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_PORT]);
	// float *pss = (float *)(pingdata->pd_ss[ACP_PORT]);
	// unsigned char *pssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_PORT]);
	// AuxBeamInfo *pabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_PORT]);
	// float *sbty = (float *)(pingdata->pd_bty[ACP_STBD]);
	// unsigned int *sbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_STBD]);
	// float *sss = (float *)(pingdata->pd_ss[ACP_STBD]);
	// unsigned char *sssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_STBD]);
	// AuxBeamInfo *sabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_STBD]);

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = 2 * MAX(pingport->ps_btycount, pingstbd->ps_btycount);

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
int mbsys_mr1v2001_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
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
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;

	/* get pointers */
	// BSFile *header = &(store->header);
	Ping *ping = &(store->ping);
	PingSide *pingport = &(ping->png_sides[ACP_PORT]);
	PingSide *pingstbd = &(ping->png_sides[ACP_STBD]);
	PingData *pingdata = &(store->pingdata);
	float *pbty = (float *)(pingdata->pd_bty[ACP_PORT]);
	unsigned int *pbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_PORT]);
	// float *pss = (float *)(pingdata->pd_ss[ACP_PORT]);
	// unsigned char *pssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_PORT]);
	// AuxBeamInfo *pabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_PORT]);
	float *sbty = (float *)(pingdata->pd_bty[ACP_STBD]);
	unsigned int *sbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_STBD]);
	// float *sss = (float *)(pingdata->pd_ss[ACP_STBD]);
	// unsigned char *sssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_STBD]);
	// AuxBeamInfo *sabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_STBD]);

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		if (!mbbs_isnanf(ping->png_depth.sns_repval))
			*transducer_depth = fabs(ping->png_depth.sns_repval);
		else
			*transducer_depth = 0.0;
		if (!mbbs_isnanf(ping->png_alt) && ping->png_alt > 0.0)
			*altitude = ping->png_alt;
		else {
			double bestxtrack = 10000.0;
			double bestxtrackflagged = 10000.0;
			double bestdepth = 0.0;
			double bestdepthflagged = 0.0;
			bool found = false;
			bool foundflagged = false;

			/* loop over port bathymetry */
			for (int i = 0; i < pingport->ps_btycount; i++) {
				double depth;
				double xtrack;
				if (store->ping.png_flags & PNG_XYZ) {
					xtrack = pbty[3 * i];
					depth = pbty[3 * i + 2];
				}
				else {
					xtrack = pbty[2 * i];
					depth = pbty[2 * i + 1];
				}
				if (pbtyflags[i] == 0 && xtrack < bestxtrack) {
					bestdepth = depth;
					bestxtrack = xtrack;
					found = true;
				}
				else if (xtrack < bestxtrackflagged) {
					bestdepthflagged = depth;
					bestxtrackflagged = xtrack;
					foundflagged = true;
				}
			}

			/* loop over starboard bathymetry */
			for (int i = 0; i < pingstbd->ps_btycount; i++) {
				double depth;
				double xtrack;
				if (store->ping.png_flags & PNG_XYZ) {
					xtrack = sbty[3 * i];
					depth = sbty[3 * i + 2];
				}
				else {
					xtrack = sbty[2 * i];
					depth = sbty[2 * i + 1];
				}
				if (sbtyflags[i] == 0 && xtrack < bestxtrack) {
					bestdepth = depth;
					bestxtrack = xtrack;
					found = true;
				}
				else if (xtrack < bestxtrackflagged) {
					bestdepthflagged = depth;
					bestxtrackflagged = xtrack;
					foundflagged = true;
				}
			}
			if (found)
				*altitude = bestdepth - *transducer_depth;
			else if (foundflagged)
				*altitude = bestdepthflagged - *transducer_depth;
			else
				*altitude = 0.0;
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
int mbsys_mr1v2001_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
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
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;

	/* get pointers */
	// BSFile *header = &(store->header);
	Ping *ping = &(store->ping);
	// PingSide *pingport = &(ping->png_sides[ACP_PORT]);
	// PingSide *pingstbd = &(ping->png_sides[ACP_STBD]);
	// PingData *pingdata = &(store->pingdata);
	// float *pbty = (float *)(pingdata->pd_bty[ACP_PORT]);
	// unsigned int *pbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_PORT]);
	// float *pss = (float *)(pingdata->pd_ss[ACP_PORT]);
	// unsigned char *pssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_PORT]);
	// AuxBeamInfo *pabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_PORT]);
	// float *sbty = (float *)(pingdata->pd_bty[ACP_STBD]);
	// unsigned int *sbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_STBD]);
	// float *sss = (float *)(pingdata->pd_ss[ACP_STBD]);
	// unsigned char *sssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_STBD]);
	// AuxBeamInfo *sabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_STBD]);

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		*time_d = ping->png_tm.tv_sec + 0.000001 * ping->png_tm.tv_usec;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		if (!mbbs_isnand(ping->png_tlon) && !mbbs_isnand(ping->png_tlon)) {
			*navlon = ping->png_tlon;
			*navlat = ping->png_tlat;
		}
		else if (!mbbs_isnand(ping->png_slon) && !mbbs_isnand(ping->png_slon)) {
			*navlon = ping->png_slon;
			*navlat = ping->png_slat;
		}
		else {
			*navlon = 0.0;
			*navlat = 0.0;
		}

		/* get heading */
		if (!mbbs_isnand(store->ping.png_compass.sns_repval)) {
			*heading = store->ping.png_compass.sns_repval;
			if (!mbbs_isnand(store->ping.png_magcorr))
				*heading += store->ping.png_magcorr;
		}
		else if (!mbbs_isnand(ping->png_tcourse))
			*heading = ping->png_tcourse;
		if (*heading < 0.0)
			*heading += 360.0;
		if (*heading >= 360.0)
			*heading -= 360.0;

		/* set speed to zero */
		*speed = 0.0;

		/* get draft */
		*draft = ping->png_depth.sns_repval;

		/* get roll pitch and heave */
		*roll = ping->png_roll.sns_repval;
		*pitch = ping->png_pitch.sns_repval;
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
int mbsys_mr1v2001_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
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
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;

	// BSFile *header = &(store->header);
	Ping *ping = &(store->ping);
	// PingSide *pingport = &(ping->png_sides[ACP_PORT]);
	// PingSide *pingstbd = &(ping->png_sides[ACP_STBD]);
	// PingData *pingdata = &(store->pingdata);
	// float *pbty = (float *)(pingdata->pd_bty[ACP_PORT]);
	// unsigned int *pbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_PORT]);
	// float *pss = (float *)(pingdata->pd_ss[ACP_PORT]);
	// unsigned char *pssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_PORT]);
	// AuxBeamInfo *pabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_PORT]);
	// float *sbty = (float *)(pingdata->pd_bty[ACP_STBD]);
	// unsigned int *sbtyflags = (unsigned int *)(pingdata->pd_btyflags[ACP_STBD]);
	// float *sss = (float *)(pingdata->pd_ss[ACP_STBD]);
	// unsigned char *sssflags = (unsigned char *)(pingdata->pd_ssflags[ACP_STBD]);
	// AuxBeamInfo *sabi = (AuxBeamInfo *)(pingdata->pd_abi[ACP_STBD]);

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		ping->png_tm.tv_sec = (int)time_d;
		ping->png_tm.tv_usec = (int)1000000.0 * (time_d - ping->png_tm.tv_sec);

		/* get navigation */
		if (navlon < 0.0)
			navlon = navlon + 360.0;
		ping->png_tlon = navlon;
		ping->png_tlat = navlat;

		/* get heading */
		store->ping.png_compass.sns_repval = heading;
		if (!mbbs_isnand(store->ping.png_magcorr))
			store->ping.png_compass.sns_repval -= store->ping.png_magcorr;

		/* get speed */

		/* get draft */
		ping->png_depth.sns_repval = draft;

		/* get roll pitch and heave */
		ping->png_roll.sns_repval = roll;
		ping->png_pitch.sns_repval = pitch;
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
int mbsys_mr1v2001_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
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
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;
	struct mbsys_mr1v2001_struct *copy = (struct mbsys_mr1v2001_struct *)copy_ptr;

	/* copy the data */
	copy->kind = store->kind;
	copy->header.bsf_version = store->header.bsf_version;
	copy->header.bsf_count = store->header.bsf_count;
	if (copy->header.bsf_log != NULL)
		free(copy->header.bsf_log);

	int status = MB_SUCCESS;

	if (copy->header.bsf_count > 0) {
		copy->header.bsf_log = (char *)calloc((MemSizeType)(copy->header.bsf_count + 1), sizeof(char));
		if (copy->header.bsf_log != NULL) {
			memcpy(copy->header.bsf_log, store->header.bsf_log, copy->header.bsf_count);
			copy->header.bsf_log[copy->header.bsf_count] = '\0';
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
		}
	}
	copy->ping = store->ping;
	copy->bsbuffersize = store->bsbuffersize;
	if (mbbs_pngrealloc(&(copy->ping), &(copy->bsbuffer), &(copy->bsbuffersize)) == BS_SUCCESS) {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		memcpy(copy->bsbuffer, store->bsbuffer, copy->bsbuffersize);
		mbbs_getpngdataptrs(&(copy->ping), copy->bsbuffer, &(copy->pingdata));
	}
	else {
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
