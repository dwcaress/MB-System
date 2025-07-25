/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mr1prvr2.c	3/6/2003
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
 * mbr_mr1prvr2.c contains the functions for reading and writing
 * multibeam data in the MR1PRVR2 format.
 * These functions include:
 *   mbr_alm_mr1prvr2	- allocate read/write memory
 *   mbr_dem_mr1prvr2	- deallocate read/write memory
 *   mbr_rt_mr1prvr2	- read and translate data
 *   mbr_wt_mr1prvr2	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 6, 2003
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
int mbr_info_mr1prvr2(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* set format info parameters */
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_MR1;
	*beams_bath_max = MBSYS_MR1V2001_BEAMS;
	*beams_amp_max = 0;
	*pixels_ss_max = MBSYS_MR1V2001_PIXELS;
	strncpy(format_name, "MR1PRVR2", MB_NAME_LENGTH);
	strncpy(system_name, "MR1", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_MR1PRVR2\nInformal Description: SOEST MR1 post processed format\nAttributes:           "
	        "SOEST MR1, bathymetry and sidescan,\n                      variable beams and pixels, xdr binary, \n                "
	        "      SOEST, University of Hawaii.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_XDR;
	*variable_beams = true;
	*traveltime = true;
	*beam_flagging = true;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 2.0;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       system:             %d\n", *system);
		fprintf(stderr, "dbg2       beams_bath_max:     %d\n", *beams_bath_max);
		fprintf(stderr, "dbg2       beams_amp_max:      %d\n", *beams_amp_max);
		fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", *pixels_ss_max);
		fprintf(stderr, "dbg2       format_name:        %s\n", format_name);
		fprintf(stderr, "dbg2       system_name:        %s\n", system_name);
		fprintf(stderr, "dbg2       format_description: %s\n", format_description);
		fprintf(stderr, "dbg2       numfile:            %d\n", *numfile);
		fprintf(stderr, "dbg2       filetype:           %d\n", *filetype);
		fprintf(stderr, "dbg2       variable_beams:     %d\n", *variable_beams);
		fprintf(stderr, "dbg2       traveltime:         %d\n", *traveltime);
		fprintf(stderr, "dbg2       beam_flagging:      %d\n", *beam_flagging);
		fprintf(stderr, "dbg2       platform_source:    %d\n", *platform_source);
		fprintf(stderr, "dbg2       nav_source:         %d\n", *nav_source);
		fprintf(stderr, "dbg2       sensordepth_source: %d\n", *sensordepth_source);
		fprintf(stderr, "dbg2       heading_source:     %d\n", *heading_source);
		fprintf(stderr, "dbg2       attitude_source:      %d\n", *attitude_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", *svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", *beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", *beamwidth_ltrack);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_mr1prvr2(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	const int status = mbsys_mr1v2001_alloc(verbose, mbio_ptr, (void **)&mb_io_ptr->store_data, error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* initialize everything to zeros */
	mb_io_ptr->fileheader = false;
	mb_io_ptr->hdr_comment_size = 0;
	mb_io_ptr->hdr_comment = NULL;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_mr1prvr2(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)mb_io_ptr->store_data;

	/* deallocate memory for data descriptor */
	if (store->bsbuffersize > 0 && store->bsbuffer != NULL)
		free(store->bsbuffer);
	const int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->store_data, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_mr1prvr2_rd_data(int verbose, void *mbio_ptr, int *error) {
	int read_size;
	int bs_status = BS_SUCCESS;
	char *eol;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to data */
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)mb_io_ptr->store_data;
	char *xdrs = mb_io_ptr->xdrs;

	int status = MB_SUCCESS;

	/* if first time through read file header */
	if (!mb_io_ptr->fileheader) {
		/* read the header into memory */
		bs_status = mbbs_rdbsfhdr(&(store->header), (XDR *)xdrs);
		if (bs_status == BS_SUCCESS) {
			mb_io_ptr->fileheader = true;
			status = MB_SUCCESS;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}

		/* extract the comments string */
		mb_io_ptr->hdr_comment_size = 0;
		if (mb_io_ptr->hdr_comment != NULL)
			mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->hdr_comment, error);
		if (status == MB_SUCCESS && store->header.bsf_count > 0) {
			status = mb_mallocd(verbose, __FILE__, __LINE__, strlen(store->header.bsf_log) + 1, (void **)&mb_io_ptr->hdr_comment,
			                    error);
		}
		if (status == MB_SUCCESS) {
			strcpy(mb_io_ptr->hdr_comment, store->header.bsf_log);
			if (mb_io_ptr->hdr_comment == NULL)
				mb_io_ptr->hdr_comment_size = 0;
			else
				mb_io_ptr->hdr_comment_size = strlen(mb_io_ptr->hdr_comment);
			mb_io_ptr->hdr_comment_loc = 0;
		}

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       mf_version:       %d\n", store->header.bsf_version);
			fprintf(stderr, "dbg5       mf_count:         %d\n", store->header.bsf_count);
			fprintf(stderr, "dbg5       mf_log:         \n%s\n", store->header.bsf_log);
		}
	}

	/* if comments are still held in mb_io_ptr->hdr_comment then
	    extract comment and return */
	if (mb_io_ptr->hdr_comment_size > mb_io_ptr->hdr_comment_loc) {
		eol = strchr(&mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc], '\n');
		if (eol == NULL)
			read_size = strlen(&mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc]);
		else
			read_size = strlen(&mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc]) - strlen(eol);
		if (read_size > MBSYS_MR1V2001_MAXLINE - 1) {
			read_size = MBSYS_MR1V2001_MAXLINE - 1;
			eol = NULL;
		}
		strncpy(store->comment, &mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc], MBSYS_MR1V2001_MAXLINE - 1);
		store->comment[read_size] = '\0';
		mb_io_ptr->hdr_comment_loc += read_size;
		if (eol != NULL)
			mb_io_ptr->hdr_comment_loc++;
		store->kind = MB_DATA_COMMENT;
	}

	/* else read data */
	else {
		if ((bs_status = mbbs_rdpnghdr(&(store->ping), (XDR *)xdrs, store->header.bsf_version)) != BS_SUCCESS) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		if (status == MB_SUCCESS &&
		    (bs_status = mbbs_pngrealloc(&(store->ping), &(store->bsbuffer), &(store->bsbuffersize))) != BS_SUCCESS) {
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
		}
		if (status == MB_SUCCESS &&
		    (bs_status = mbbs_rdpngdata(&(store->ping), (float *)store->bsbuffer, (XDR *)xdrs)) != BS_SUCCESS) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		if (status == MB_SUCCESS &&
		    (bs_status = mbbs_getpngdataptrs(&(store->ping), (float *)store->bsbuffer, &(store->pingdata))) != BS_SUCCESS) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		if (status == MB_SUCCESS) {
			store->kind = MB_DATA_DATA;
		}

		/* Fix case of old file with no beam flags excepting negative depths */
		if (store->ping.png_flags & PNG_BTYSSFLAGSABSENT) {
			if (store->ping.png_flags & PNG_XYZ) {
				for (int i = 0; i < store->ping.png_sides[ACP_PORT].ps_btycount; i++) {
					if (store->pingdata.pd_bty[ACP_PORT][3 * i + 2] < 0.0) {
						store->pingdata.pd_bty[ACP_PORT][3 * i + 2] *= -1.0;
						store->pingdata.pd_btyflags[ACP_PORT][i] = BTYD_EXTERNAL;
					}
				}
				for (int i = 0; i < store->ping.png_sides[ACP_STBD].ps_btycount; i++) {
					if (store->pingdata.pd_bty[ACP_STBD][3 * i + 2] < 0.0) {
						store->pingdata.pd_bty[ACP_STBD][3 * i + 2] *= -1.0;
						store->pingdata.pd_btyflags[ACP_STBD][i] = BTYD_EXTERNAL;
					}
				}
			}
			else {
				for (int i = 0; i < store->ping.png_sides[ACP_PORT].ps_btycount; i++) {
					if (store->pingdata.pd_bty[ACP_PORT][2 * i + 1] < 0.0) {
						store->pingdata.pd_bty[ACP_PORT][2 * i + 1] *= -1.0;
						store->pingdata.pd_btyflags[ACP_PORT][i] = BTYD_EXTERNAL;
					}
				}
				for (int i = 0; i < store->ping.png_sides[ACP_STBD].ps_btycount; i++) {
					if (store->pingdata.pd_bty[ACP_STBD][2 * i + 1] < 0.0) {
						store->pingdata.pd_bty[ACP_STBD][2 * i + 1] *= -1.0;
						store->pingdata.pd_btyflags[ACP_STBD][i] = BTYD_EXTERNAL;
					}
				}
			}
			store->ping.png_flags = store->ping.png_flags ^ PNG_BTYSSFLAGSABSENT;
		}

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       png_flags:        %u\n", store->ping.png_flags);
			fprintf(stderr, "dbg5       sec:              %ld\n", (long int) store->ping.png_tm.tv_sec);
			fprintf(stderr, "dbg5       usec:             %ld\n", (long int) store->ping.png_tm.tv_usec);
			fprintf(stderr, "dbg5       period:           %f\n", store->ping.png_period);
			fprintf(stderr, "dbg5       ship longitude:   %f\n", store->ping.png_slon);
			fprintf(stderr, "dbg5       ship latitude:    %f\n", store->ping.png_slat);
			fprintf(stderr, "dbg5       ship course:      %f\n", store->ping.png_scourse);
			fprintf(stderr, "dbg5       layback range:    %f\n", store->ping.png_laybackrng);
			fprintf(stderr, "dbg5       layback bearing:  %f\n", store->ping.png_laybackbrg);
			fprintf(stderr, "dbg5       towfish longitude:%f\n", store->ping.png_tlon);
			fprintf(stderr, "dbg5       towfish latitude: %f\n", store->ping.png_tlat);
			fprintf(stderr, "dbg5       towfish course:   %f\n", store->ping.png_tcourse);
			fprintf(stderr, "dbg5       compass ptr:      %p\n", (void *)store->pingdata.pd_compass);
			fprintf(stderr, "dbg5       towfish compass interval:  %f\n", store->ping.png_compass.sns_int);
			fprintf(stderr, "dbg5       towfish compass samples:   %d\n", store->ping.png_compass.sns_nsamps);
			fprintf(stderr, "dbg5       towfish compass value:     %f\n", store->ping.png_compass.sns_repval);
			fprintf(stderr, "dbg5       towfish compass  heading:\n");
			for (int i = 0; i < store->ping.png_compass.sns_nsamps; i++) {
				fprintf(stderr, "dbg5         %3d     %12.4g\n", i, store->pingdata.pd_compass[i]);
			}
			fprintf(stderr, "dbg5       depth ptr:                 %p\n", (void *)store->pingdata.pd_depth);
			fprintf(stderr, "dbg5       towfish depth interval:    %f\n", store->ping.png_depth.sns_int);
			fprintf(stderr, "dbg5       towfish depth samples:     %d\n", store->ping.png_depth.sns_nsamps);
			fprintf(stderr, "dbg5       towfish depth value:       %f\n", store->ping.png_depth.sns_repval);
			fprintf(stderr, "dbg5       towfish depth:\n");
			for (int i = 0; i < store->ping.png_depth.sns_nsamps; i++) {
				fprintf(stderr, "dbg5         %3d     %12.4g\n", i, store->pingdata.pd_depth[i]);
			}
			fprintf(stderr, "dbg5       pitch ptr:                 %p\n", (void *)store->pingdata.pd_pitch);
			fprintf(stderr, "dbg5       towfish pitch interval:    %f\n", store->ping.png_pitch.sns_int);
			fprintf(stderr, "dbg5       towfish pitch samples:     %d\n", store->ping.png_pitch.sns_nsamps);
			fprintf(stderr, "dbg5       towfish pitch value:       %f\n", store->ping.png_pitch.sns_repval);
			fprintf(stderr, "dbg5       towfish pitch:\n");
			for (int i = 0; i < store->ping.png_pitch.sns_nsamps; i++) {
				fprintf(stderr, "dbg5         %3d     %12.4g\n", i, store->pingdata.pd_pitch[i]);
			}
			fprintf(stderr, "dbg5       roll ptr:                  %p\n", (void *)store->pingdata.pd_roll);
			fprintf(stderr, "dbg5       towfish roll interval:     %f\n", store->ping.png_roll.sns_int);
			fprintf(stderr, "dbg5       towfish roll samples:      %d\n", store->ping.png_roll.sns_nsamps);
			fprintf(stderr, "dbg5       towfish roll value:        %f\n", store->ping.png_roll.sns_repval);
			fprintf(stderr, "dbg5       towfish roll:\n");
			for (int i = 0; i < store->ping.png_roll.sns_nsamps; i++) {
				fprintf(stderr, "dbg5         %3d     %12.4g\n", i, store->pingdata.pd_roll[i]);
			}
			fprintf(stderr, "dbg5       png_snspad:       %d\n", store->ping.png_snspad);
			fprintf(stderr, "dbg5       png_temp:         %f\n", store->ping.png_temp);
			fprintf(stderr, "dbg5       png_ssincr:       %f\n", store->ping.png_ssincr);
			fprintf(stderr, "dbg5       png_ssyoffsetmode:%d\n", store->ping.png_ssyoffsetmode);
			fprintf(stderr, "dbg5       png_alt:          %f\n", store->ping.png_alt);
			fprintf(stderr, "dbg5       png_magcorr:      %f\n", store->ping.png_magcorr);
			fprintf(stderr, "dbg5       png_sndvel:       %f\n", store->ping.png_sndvel);
			fprintf(stderr, "dbg5       png_cond:         %f\n", store->ping.png_cond);
			fprintf(stderr, "dbg5       png_magx:         %f\n", store->ping.png_magx);
			fprintf(stderr, "dbg5       png_magy:         %f\n", store->ping.png_magy);
			fprintf(stderr, "dbg5       png_magz:         %f\n", store->ping.png_magz);
			fprintf(stderr, "dbg5       port ps_xmitpwr:  %f\n", store->ping.png_sides[ACP_PORT].ps_xmitpwr);
			fprintf(stderr, "dbg5       port ps_gain:     %f\n", store->ping.png_sides[ACP_PORT].ps_gain);
			fprintf(stderr, "dbg5       port ps_pulse:    %f\n", store->ping.png_sides[ACP_PORT].ps_pulse);
			fprintf(stderr, "dbg5       port ps_bdrange:  %f\n", store->ping.png_sides[ACP_PORT].ps_bdrange);
			fprintf(stderr, "dbg5       port ps_btycount: %d\n", store->ping.png_sides[ACP_PORT].ps_btycount);
			fprintf(stderr, "dbg5       port ps_btypad:   %d\n", store->ping.png_sides[ACP_PORT].ps_btypad);
			fprintf(stderr, "dbg5       port bty ptr:     %p\n", (void *)store->pingdata.pd_bty[ACP_PORT]);
			fprintf(stderr, "dbg5       port btyflags ptr:%p\n", (void *)store->pingdata.pd_btyflags[ACP_PORT]);
			fprintf(stderr, "dbg5       port abi ptr:     %p\n", (void *)store->pingdata.pd_abi[ACP_PORT]);
			if (store->ping.png_flags & PNG_XYZ) {
				fprintf(stderr, "dbg5       port flag acrosstrack alongtrack bathymetry:\n");
				for (int i = 0; i < store->ping.png_sides[ACP_PORT].ps_btycount; i++) {
					fprintf(stderr, "dbg5         %3d     %d %12.4g %12.4g %12.4g\n", i, store->pingdata.pd_btyflags[ACP_PORT][i],
					        store->pingdata.pd_bty[ACP_PORT][3 * i], store->pingdata.pd_bty[ACP_PORT][3 * i + 1],
					        store->pingdata.pd_bty[ACP_PORT][3 * i + 2]);
				}
			}
			else {
				fprintf(stderr, "dbg5       port flag acrosstrack bathymetry:\n");
				for (int i = 0; i < store->ping.png_sides[ACP_PORT].ps_btycount; i++) {
					fprintf(stderr, "dbg5         %3d     %d %12.4g %12.4g\n", i, store->pingdata.pd_btyflags[ACP_PORT][i],
					        store->pingdata.pd_bty[ACP_PORT][2 * i], store->pingdata.pd_bty[ACP_PORT][2 * i + 1]);
				}
			}
			fprintf(stderr, "dbg5       port ps_ssxoffset:%f\n", store->ping.png_sides[ACP_PORT].ps_ssxoffset);
			fprintf(stderr, "dbg5       port ps_sscount:  %d\n", store->ping.png_sides[ACP_PORT].ps_sscount);
			fprintf(stderr, "dbg5       port ps_sspad:    %d\n", store->ping.png_sides[ACP_PORT].ps_sspad);
			fprintf(stderr, "dbg5       port ps_ssndrmask:%f\n", store->ping.png_sides[ACP_PORT].ps_ssndrmask);
			fprintf(stderr, "dbg5       port ps_ssyoffset:%f\n", store->ping.png_sides[ACP_PORT].ps_ssyoffset);
			fprintf(stderr, "dbg5       port ss ptr:      %p\n", (void *)store->pingdata.pd_ss[ACP_PORT]);
			fprintf(stderr, "dbg5       port ssflags ptr: %p\n", (void *)store->pingdata.pd_ssflags[ACP_PORT]);
			fprintf(stderr, "dbg5       port flag sidescan:\n");
			for (int i = 0; i < store->ping.png_sides[ACP_PORT].ps_sscount; i++) {
				fprintf(stderr, "dbg5         %3d     %d %12.4g\n", i, store->pingdata.pd_ssflags[ACP_PORT][i],
				        store->pingdata.pd_ss[ACP_PORT][i]);
			}

			fprintf(stderr, "dbg5       stbd ps_xmitpwr:  %f\n", store->ping.png_sides[ACP_STBD].ps_xmitpwr);
			fprintf(stderr, "dbg5       stbd ps_gain:     %f\n", store->ping.png_sides[ACP_STBD].ps_gain);
			fprintf(stderr, "dbg5       stbd ps_pulse:    %f\n", store->ping.png_sides[ACP_STBD].ps_pulse);
			fprintf(stderr, "dbg5       stbd ps_bdrange:  %f\n", store->ping.png_sides[ACP_STBD].ps_bdrange);
			fprintf(stderr, "dbg5       stbd ps_btycount: %d\n", store->ping.png_sides[ACP_STBD].ps_btycount);
			fprintf(stderr, "dbg5       stbd ps_btypad:   %d\n", store->ping.png_sides[ACP_STBD].ps_btypad);
			fprintf(stderr, "dbg5       stbd bty ptr:     %p\n", (void *)store->pingdata.pd_bty[ACP_STBD]);
			fprintf(stderr, "dbg5       stbd btyflags ptr:%p\n", (void *)store->pingdata.pd_btyflags[ACP_STBD]);
			fprintf(stderr, "dbg5       stbd abi ptr:     %p\n", (void *)store->pingdata.pd_abi[ACP_STBD]);
			if (store->ping.png_flags & PNG_XYZ) {
				fprintf(stderr, "dbg5       stbd flag acrosstrack alongtrack bathymetry:\n");
				for (int i = 0; i < store->ping.png_sides[ACP_STBD].ps_btycount; i++) {
					fprintf(stderr, "dbg5         %3d     %d %12.4g %12.4g %12.4g\n", i, store->pingdata.pd_btyflags[ACP_STBD][i],
					        store->pingdata.pd_bty[ACP_STBD][3 * i], store->pingdata.pd_bty[ACP_STBD][3 * i + 1],
					        store->pingdata.pd_bty[ACP_STBD][3 * i + 2]);
				}
			}
			else {
				fprintf(stderr, "dbg5       stbd flag acrosstrack bathymetry:\n");
				for (int i = 0; i < store->ping.png_sides[ACP_STBD].ps_btycount; i++) {
					fprintf(stderr, "dbg5         %3d     %d %12.4g %12.4g\n", i, store->pingdata.pd_btyflags[ACP_STBD][i],
					        store->pingdata.pd_bty[ACP_STBD][2 * i], store->pingdata.pd_bty[ACP_STBD][2 * i + 1]);
				}
			}
			fprintf(stderr, "dbg5       stbd ps_ssxoffset:%f\n", store->ping.png_sides[ACP_STBD].ps_ssxoffset);
			fprintf(stderr, "dbg5       stbd ps_sscount:  %d\n", store->ping.png_sides[ACP_STBD].ps_sscount);
			fprintf(stderr, "dbg5       stbd ps_sspad:    %d\n", store->ping.png_sides[ACP_STBD].ps_sspad);
			fprintf(stderr, "dbg5       stbd ps_ssndrmask:%f\n", store->ping.png_sides[ACP_STBD].ps_ssndrmask);
			fprintf(stderr, "dbg5       stbd ps_ssyoffset:%f\n", store->ping.png_sides[ACP_STBD].ps_ssyoffset);
			fprintf(stderr, "dbg5       stbd ss ptr:      %p\n", (void *)store->pingdata.pd_ss[ACP_STBD]);
			fprintf(stderr, "dbg5       stbd ssflags ptr: %p\n", (void *)store->pingdata.pd_ssflags[ACP_STBD]);
			fprintf(stderr, "dbg5       stbd flag sidescan:\n");
			for (int i = 0; i < store->ping.png_sides[ACP_STBD].ps_sscount; i++) {
				fprintf(stderr, "dbg5         %3d     %d %12.4g\n", i, store->pingdata.pd_ssflags[ACP_STBD][i],
				        store->pingdata.pd_ss[ACP_STBD][i]);
			}
			fprintf(stderr, "\n");
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_mr1prvr2(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;

	/* read next data from file */
	const int status = mbr_mr1prvr2_rd_data(verbose, mbio_ptr, error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_mr1prvr2_wr_data(int verbose, void *mbio_ptr, char *store_ptr, int *error) {
	int bs_status = BS_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to data */
	struct mbsys_mr1v2001_struct *store = (struct mbsys_mr1v2001_struct *)store_ptr;
	char *xdrs = mb_io_ptr->xdrs;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       png_flags:        %u\n", store->ping.png_flags);
		fprintf(stderr, "dbg5       sec:              %ld\n", (long int) store->ping.png_tm.tv_sec);
		fprintf(stderr, "dbg5       usec:             %ld\n", (long int) store->ping.png_tm.tv_usec);
		fprintf(stderr, "dbg5       period:           %f\n", store->ping.png_period);
		fprintf(stderr, "dbg5       ship longitude:   %f\n", store->ping.png_slon);
		fprintf(stderr, "dbg5       ship latitude:    %f\n", store->ping.png_slat);
		fprintf(stderr, "dbg5       ship course:      %f\n", store->ping.png_scourse);
		fprintf(stderr, "dbg5       layback range:    %f\n", store->ping.png_laybackrng);
		fprintf(stderr, "dbg5       layback bearing:  %f\n", store->ping.png_laybackbrg);
		fprintf(stderr, "dbg5       towfish longitude:%f\n", store->ping.png_tlon);
		fprintf(stderr, "dbg5       towfish latitude: %f\n", store->ping.png_tlat);
		fprintf(stderr, "dbg5       towfish course:   %f\n", store->ping.png_tcourse);
		fprintf(stderr, "dbg5       compass ptr:      %p\n", (void *)store->pingdata.pd_compass);
		fprintf(stderr, "dbg5       towfish compass interval:  %f\n", store->ping.png_compass.sns_int);
		fprintf(stderr, "dbg5       towfish compass samples:   %d\n", store->ping.png_compass.sns_nsamps);
		fprintf(stderr, "dbg5       towfish compass value:     %f\n", store->ping.png_compass.sns_repval);
		fprintf(stderr, "dbg5       towfish compass  heading:\n");
		for (int i = 0; i < store->ping.png_compass.sns_nsamps; i++) {
			fprintf(stderr, "dbg5         %3d     %12.4g\n", i, store->pingdata.pd_compass[i]);
		}
		fprintf(stderr, "dbg5       depth ptr:                 %p\n", (void *)store->pingdata.pd_depth);
		fprintf(stderr, "dbg5       towfish depth interval:    %f\n", store->ping.png_depth.sns_int);
		fprintf(stderr, "dbg5       towfish depth samples:     %d\n", store->ping.png_depth.sns_nsamps);
		fprintf(stderr, "dbg5       towfish depth value:       %f\n", store->ping.png_depth.sns_repval);
		fprintf(stderr, "dbg5       towfish depth:\n");
		for (int i = 0; i < store->ping.png_depth.sns_nsamps; i++) {
			fprintf(stderr, "dbg5         %3d     %12.4g\n", i, store->pingdata.pd_depth[i]);
		}
		fprintf(stderr, "dbg5       pitch ptr:                 %p\n", (void *)store->pingdata.pd_pitch);
		fprintf(stderr, "dbg5       towfish pitch interval:    %f\n", store->ping.png_pitch.sns_int);
		fprintf(stderr, "dbg5       towfish pitch samples:     %d\n", store->ping.png_pitch.sns_nsamps);
		fprintf(stderr, "dbg5       towfish pitch value:       %f\n", store->ping.png_pitch.sns_repval);
		fprintf(stderr, "dbg5       towfish pitch:\n");
		for (int i = 0; i < store->ping.png_pitch.sns_nsamps; i++) {
			fprintf(stderr, "dbg5         %3d     %12.4g\n", i, store->pingdata.pd_pitch[i]);
		}
		fprintf(stderr, "dbg5       roll ptr:                  %p\n", (void *)store->pingdata.pd_roll);
		fprintf(stderr, "dbg5       towfish roll interval:     %f\n", store->ping.png_roll.sns_int);
		fprintf(stderr, "dbg5       towfish roll samples:      %d\n", store->ping.png_roll.sns_nsamps);
		fprintf(stderr, "dbg5       towfish roll value:        %f\n", store->ping.png_roll.sns_repval);
		fprintf(stderr, "dbg5       towfish roll:\n");
		for (int i = 0; i < store->ping.png_roll.sns_nsamps; i++) {
			fprintf(stderr, "dbg5         %3d     %12.4g\n", i, store->pingdata.pd_roll[i]);
		}
		fprintf(stderr, "dbg5       png_snspad:       %d\n", store->ping.png_snspad);
		fprintf(stderr, "dbg5       png_temp:         %f\n", store->ping.png_temp);
		fprintf(stderr, "dbg5       png_ssincr:       %f\n", store->ping.png_ssincr);
		fprintf(stderr, "dbg5       png_ssyoffsetmode:%d\n", store->ping.png_ssyoffsetmode);
		fprintf(stderr, "dbg5       png_alt:          %f\n", store->ping.png_alt);
		fprintf(stderr, "dbg5       png_magcorr:      %f\n", store->ping.png_magcorr);
		fprintf(stderr, "dbg5       png_sndvel:       %f\n", store->ping.png_sndvel);
		fprintf(stderr, "dbg5       png_cond:         %f\n", store->ping.png_cond);
		fprintf(stderr, "dbg5       png_magx:         %f\n", store->ping.png_magx);
		fprintf(stderr, "dbg5       png_magy:         %f\n", store->ping.png_magy);
		fprintf(stderr, "dbg5       png_magz:         %f\n", store->ping.png_magz);
		fprintf(stderr, "dbg5       port ps_xmitpwr:  %f\n", store->ping.png_sides[ACP_PORT].ps_xmitpwr);
		fprintf(stderr, "dbg5       port ps_gain:     %f\n", store->ping.png_sides[ACP_PORT].ps_gain);
		fprintf(stderr, "dbg5       port ps_pulse:    %f\n", store->ping.png_sides[ACP_PORT].ps_pulse);
		fprintf(stderr, "dbg5       port ps_bdrange:  %f\n", store->ping.png_sides[ACP_PORT].ps_bdrange);
		fprintf(stderr, "dbg5       port ps_btycount: %d\n", store->ping.png_sides[ACP_PORT].ps_btycount);
		fprintf(stderr, "dbg5       port ps_btypad:   %d\n", store->ping.png_sides[ACP_PORT].ps_btypad);
		fprintf(stderr, "dbg5       port bty ptr:     %p\n", (void *)store->pingdata.pd_bty[ACP_PORT]);
		fprintf(stderr, "dbg5       port btyflags ptr:%p\n", (void *)store->pingdata.pd_btyflags[ACP_PORT]);
		fprintf(stderr, "dbg5       port abi ptr:     %p\n", (void *)store->pingdata.pd_abi[ACP_PORT]);
		if (store->ping.png_flags & PNG_XYZ) {
			fprintf(stderr, "dbg5       port flag acrosstrack alongtrack bathymetry:\n");
			for (int i = 0; i < store->ping.png_sides[ACP_PORT].ps_btycount; i++) {
				fprintf(stderr, "dbg5         %3d     %d %12.4g %12.4g %12.4g\n", i, store->pingdata.pd_btyflags[ACP_PORT][i],
				        store->pingdata.pd_bty[ACP_PORT][3 * i], store->pingdata.pd_bty[ACP_PORT][3 * i + 1],
				        store->pingdata.pd_bty[ACP_PORT][3 * i + 2]);
			}
		}
		else {
			fprintf(stderr, "dbg5       port flag acrosstrack bathymetry:\n");
			for (int i = 0; i < store->ping.png_sides[ACP_PORT].ps_btycount; i++) {
				fprintf(stderr, "dbg5         %3d     %d %12.4g %12.4g\n", i, store->pingdata.pd_btyflags[ACP_PORT][i],
				        store->pingdata.pd_bty[ACP_PORT][2 * i], store->pingdata.pd_bty[ACP_PORT][2 * i + 1]);
			}
		}
		fprintf(stderr, "dbg5       port ps_ssxoffset:%f\n", store->ping.png_sides[ACP_PORT].ps_ssxoffset);
		fprintf(stderr, "dbg5       port ps_sscount:  %d\n", store->ping.png_sides[ACP_PORT].ps_sscount);
		fprintf(stderr, "dbg5       port ps_sspad:    %d\n", store->ping.png_sides[ACP_PORT].ps_sspad);
		fprintf(stderr, "dbg5       port ps_ssndrmask:%f\n", store->ping.png_sides[ACP_PORT].ps_ssndrmask);
		fprintf(stderr, "dbg5       port ps_ssyoffset:%f\n", store->ping.png_sides[ACP_PORT].ps_ssyoffset);
		fprintf(stderr, "dbg5       port ss ptr:      %p\n", (void *)store->pingdata.pd_ss[ACP_PORT]);
		fprintf(stderr, "dbg5       port ssflags ptr: %p\n", (void *)store->pingdata.pd_ssflags[ACP_PORT]);
		fprintf(stderr, "dbg5       port flag sidescan:\n");
		for (int i = 0; i < store->ping.png_sides[ACP_PORT].ps_sscount; i++) {
			fprintf(stderr, "dbg5         %3d     %d %12.4g\n", i, store->pingdata.pd_ssflags[ACP_PORT][i],
			        store->pingdata.pd_ss[ACP_PORT][i]);
		}

		fprintf(stderr, "dbg5       stbd ps_xmitpwr:  %f\n", store->ping.png_sides[ACP_STBD].ps_xmitpwr);
		fprintf(stderr, "dbg5       stbd ps_gain:     %f\n", store->ping.png_sides[ACP_STBD].ps_gain);
		fprintf(stderr, "dbg5       stbd ps_pulse:    %f\n", store->ping.png_sides[ACP_STBD].ps_pulse);
		fprintf(stderr, "dbg5       stbd ps_bdrange:  %f\n", store->ping.png_sides[ACP_STBD].ps_bdrange);
		fprintf(stderr, "dbg5       stbd ps_btycount: %d\n", store->ping.png_sides[ACP_STBD].ps_btycount);
		fprintf(stderr, "dbg5       stbd ps_btypad:   %d\n", store->ping.png_sides[ACP_STBD].ps_btypad);
		fprintf(stderr, "dbg5       stbd bty ptr:     %p\n", (void *)store->pingdata.pd_bty[ACP_STBD]);
		fprintf(stderr, "dbg5       stbd btyflags ptr:%p\n", (void *)store->pingdata.pd_btyflags[ACP_STBD]);
		fprintf(stderr, "dbg5       stbd abi ptr:     %p\n", (void *)store->pingdata.pd_abi[ACP_STBD]);
		if (store->ping.png_flags & PNG_XYZ) {
			fprintf(stderr, "dbg5       stbd flag acrosstrack alongtrack bathymetry:\n");
			for (int i = 0; i < store->ping.png_sides[ACP_STBD].ps_btycount; i++) {
				fprintf(stderr, "dbg5         %3d     %d %12.4g %12.4g %12.4g\n", i, store->pingdata.pd_btyflags[ACP_STBD][i],
				        store->pingdata.pd_bty[ACP_STBD][3 * i], store->pingdata.pd_bty[ACP_STBD][3 * i + 1],
				        store->pingdata.pd_bty[ACP_STBD][3 * i + 2]);
			}
		}
		else {
			fprintf(stderr, "dbg5       stbd flag acrosstrack bathymetry:\n");
			for (int i = 0; i < store->ping.png_sides[ACP_STBD].ps_btycount; i++) {
				fprintf(stderr, "dbg5         %3d     %d %12.4g %12.4g\n", i, store->pingdata.pd_btyflags[ACP_STBD][i],
				        store->pingdata.pd_bty[ACP_STBD][2 * i], store->pingdata.pd_bty[ACP_STBD][2 * i + 1]);
			}
		}
		fprintf(stderr, "dbg5       stbd ps_ssxoffset:%f\n", store->ping.png_sides[ACP_STBD].ps_ssxoffset);
		fprintf(stderr, "dbg5       stbd ps_sscount:  %d\n", store->ping.png_sides[ACP_STBD].ps_sscount);
		fprintf(stderr, "dbg5       stbd ps_sspad:    %d\n", store->ping.png_sides[ACP_STBD].ps_sspad);
		fprintf(stderr, "dbg5       stbd ps_ssndrmask:%f\n", store->ping.png_sides[ACP_STBD].ps_ssndrmask);
		fprintf(stderr, "dbg5       stbd ps_ssyoffset:%f\n", store->ping.png_sides[ACP_STBD].ps_ssyoffset);
		fprintf(stderr, "dbg5       stbd ss ptr:      %p\n", (void *)store->pingdata.pd_ss[ACP_STBD]);
		fprintf(stderr, "dbg5       stbd ssflags ptr: %p\n", (void *)store->pingdata.pd_ssflags[ACP_STBD]);
		fprintf(stderr, "dbg5       stbd flag sidescan:\n");
		for (int i = 0; i < store->ping.png_sides[ACP_STBD].ps_sscount; i++) {
			fprintf(stderr, "dbg5         %3d     %d %12.4g\n", i, store->pingdata.pd_ssflags[ACP_STBD][i],
			        store->pingdata.pd_ss[ACP_STBD][i]);
		}
		fprintf(stderr, "\n");
	}

	int status = MB_SUCCESS;

	/* if comment and file header not written */
	if (!mb_io_ptr->fileheader && store->kind == MB_DATA_COMMENT) {
		/* add comment to string mb_io_ptr->hdr_comment
		    to be be written in file header */
		mb_io_ptr->hdr_comment_size += strlen(store->comment) + 2;
		status = mb_reallocd(verbose, __FILE__, __LINE__, mb_io_ptr->hdr_comment_size, (void **)&(mb_io_ptr->hdr_comment), error);
		strcat(mb_io_ptr->hdr_comment, store->comment);
		strcat(mb_io_ptr->hdr_comment, "\n");
	}

	/* if data and file header not written */
	else if (!mb_io_ptr->fileheader && store->kind != MB_DATA_COMMENT) {
		/* insert new comments into file header */
		mbbs_replacestr(&(store->header.bsf_log), mb_io_ptr->hdr_comment);

		/* write file header */
		if ((bs_status = mbbs_wrbsfhdr(&(store->header), (XDR *)xdrs)) != BS_SUCCESS) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			mb_io_ptr->fileheader = true;
	}

	/* if data and file header written */
	if (mb_io_ptr->fileheader && store->kind == MB_DATA_DATA) {
		/* write data */
		if ((bs_status = mbbs_wrpnghdr(&(store->ping), (XDR *)xdrs)) != BS_SUCCESS) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if ((bs_status = mbbs_wrpngdata(&(store->ping), (float *)store->bsbuffer, (XDR *)xdrs)) != BS_SUCCESS) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	/* if not data and file header written */
	else if (store->kind != MB_DATA_COMMENT && store->kind != MB_DATA_DATA) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Data record kind in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:       %d\n", store->kind);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_mr1prvr2(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* write next data to file */
	const int status = mbr_mr1prvr2_wr_data(verbose, mbio_ptr, store_ptr, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}


/*--------------------------------------------------------------------*/
int mbr_register_mr1prvr2(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_mr1prvr2(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_mr1prvr2;
	mb_io_ptr->mb_io_format_free = &mbr_dem_mr1prvr2;
	mb_io_ptr->mb_io_store_alloc = &mbsys_mr1v2001_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_mr1v2001_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_mr1prvr2;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_mr1prvr2;
	mb_io_ptr->mb_io_dimensions = &mbsys_mr1v2001_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_mr1v2001_extract;
	mb_io_ptr->mb_io_insert = &mbsys_mr1v2001_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_mr1v2001_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_mr1v2001_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_mr1v2001_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_mr1v2001_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_mr1v2001_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_mr1v2001_copy;
	mb_io_ptr->mb_io_extract_rawss = NULL;
	mb_io_ptr->mb_io_insert_rawss = NULL;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       system:             %d\n", mb_io_ptr->system);
		fprintf(stderr, "dbg2       beams_bath_max:     %d\n", mb_io_ptr->beams_bath_max);
		fprintf(stderr, "dbg2       beams_amp_max:      %d\n", mb_io_ptr->beams_amp_max);
		fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", mb_io_ptr->pixels_ss_max);
		fprintf(stderr, "dbg2       format_name:        %s\n", mb_io_ptr->format_name);
		fprintf(stderr, "dbg2       system_name:        %s\n", mb_io_ptr->system_name);
		fprintf(stderr, "dbg2       format_description: %s\n", mb_io_ptr->format_description);
		fprintf(stderr, "dbg2       numfile:            %d\n", mb_io_ptr->numfile);
		fprintf(stderr, "dbg2       filetype:           %d\n", mb_io_ptr->filetype);
		fprintf(stderr, "dbg2       variable_beams:     %d\n", mb_io_ptr->variable_beams);
		fprintf(stderr, "dbg2       traveltime:         %d\n", mb_io_ptr->traveltime);
		fprintf(stderr, "dbg2       beam_flagging:      %d\n", mb_io_ptr->beam_flagging);
		fprintf(stderr, "dbg2       platform_source:    %d\n", mb_io_ptr->platform_source);
		fprintf(stderr, "dbg2       nav_source:         %d\n", mb_io_ptr->nav_source);
		fprintf(stderr, "dbg2       sensordepth_source: %d\n", mb_io_ptr->nav_source);
		fprintf(stderr, "dbg2       heading_source:     %d\n", mb_io_ptr->heading_source);
		fprintf(stderr, "dbg2       attitude_source:    %d\n", mb_io_ptr->attitude_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", mb_io_ptr->svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr, "dbg2       format_alloc:       %p\n", (void *)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr, "dbg2       format_free:        %p\n", (void *)mb_io_ptr->mb_io_format_free);
		fprintf(stderr, "dbg2       store_alloc:        %p\n", (void *)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr, "dbg2       store_free:         %p\n", (void *)mb_io_ptr->mb_io_store_free);
		fprintf(stderr, "dbg2       read_ping:          %p\n", (void *)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr, "dbg2       write_ping:         %p\n", (void *)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr, "dbg2       extract:            %p\n", (void *)mb_io_ptr->mb_io_extract);
		fprintf(stderr, "dbg2       insert:             %p\n", (void *)mb_io_ptr->mb_io_insert);
		fprintf(stderr, "dbg2       extract_nav:        %p\n", (void *)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr, "dbg2       insert_nav:         %p\n", (void *)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr, "dbg2       extract_altitude:   %p\n", (void *)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr, "dbg2       insert_altitude:    %p\n", (void *)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr, "dbg2       extract_svp:        %p\n", (void *)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr, "dbg2       insert_svp:         %p\n", (void *)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr, "dbg2       ttimes:             %p\n", (void *)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr, "dbg2       detects:            %p\n", (void *)mb_io_ptr->mb_io_detects);
		fprintf(stderr, "dbg2       extract_rawss:      %p\n", (void *)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr, "dbg2       insert_rawss:       %p\n", (void *)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr, "dbg2       copyrecord:         %p\n", (void *)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
