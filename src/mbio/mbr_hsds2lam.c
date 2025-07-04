/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsds2raw.c	6/20/01
 *
 *    Copyright (c) 2001-2025 by
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
 * mbr_hsds2lam.c contains the functions for reading
 * multibeam data in the HSDS2LAM format.
 * These functions include:
 *   mbr_alm_hsds2lam	- allocate read/write memory
 *   mbr_dem_hsds2lam	- deallocate read/write memory
 *   mbr_rt_hsds2lam	- read and translate data
 *   mbr_wt_hsds2lam	- translate and write data
 *
 * Authors:	D. W. Caress
 * 		D. N. Chayes
 *
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_atlas.h"

/* turn on debug statements here */
/* #define MBR_HSDS2LAM_DEBUG 1 */

/*--------------------------------------------------------------------*/
int mbr_info_hsds2lam(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_ATLAS;
	*beams_bath_max = MBSYS_ATLAS_MAXBEAMS;
	*beams_amp_max = MBSYS_ATLAS_MAXBEAMS;
	*pixels_ss_max = MBSYS_ATLAS_MAXPIXELS;
	strncpy(format_name, "HSDS2LAM", MB_NAME_LENGTH);
	strncpy(system_name, "ATLAS", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_HSDS2LAM\nInformal Description: L-DEO HSDS2 processing format\nAttributes:           STN "
	        "Atlas multibeam sonars, \n                      Hydrosweep DS2, Hydrosweep MD, \n                      Fansweep 10, "
	        "Fansweep 20, \n                      bathymetry, amplitude, and sidescan,\n                      up to 1440 beams "
	        "and 4096 pixels,\n                      XDR binary, L-DEO.\n",
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
	*beamwidth_xtrack = 2.67;
	*beamwidth_ltrack = 2.67;

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
int mbr_alm_hsds2lam(int verbose, void *mbio_ptr, int *error) {
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
	const int status = mbsys_atlas_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_dem_hsds2lam(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	const int status = mbsys_atlas_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_hsds2lam_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	bool xdr_status;
	int strlength;
	int telegram_id;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbsys_atlas_struct *store = (struct mbsys_atlas_struct *)store_ptr;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read the next record (hsds2lam telegram  */
	*error = MB_ERROR_NO_ERROR;

	/* get telegram */
	xdr_status = xdr_int(mb_io_ptr->xdrs, &telegram_id);
	while (xdr_status && telegram_id != MBSYS_ATLAS_TELEGRAM_HSDS2LAM &&
	       telegram_id != MBSYS_ATLAS_TELEGRAM_COMMENTLAM) {
		xdr_status = xdr_int(mb_io_ptr->xdrs, &telegram_id);
	}

	/* read ping record */
	if (telegram_id == MBSYS_ATLAS_TELEGRAM_HSDS2LAM) {
		if (xdr_status)
			xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->start_ping_no);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_transmit_time_d);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->start_opmode, strlength);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_heave);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_roll);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_pitch);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_heading);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_ckeel);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_cmean);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_depth_min);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_depth_max);
		if (xdr_status)
			xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->tt_ping_no);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_transmit_time_d);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_beam_table_index);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_beam_cnt);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_long1);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_long2);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_long3);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_xdraught);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_double1);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_double2);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_sensdraught);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_draught);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->pr_navlon);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->pr_navlat);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->pr_speed);
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &(store->tt_lruntime[i]));
		}
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->tt_lamplitude, strlength);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->tt_lstatus, strlength);
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (xdr_status)
				xdr_status = xdr_double(mb_io_ptr->xdrs, &(store->pr_bath[i]));
		}
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (xdr_status)
				xdr_status = xdr_double(mb_io_ptr->xdrs, &(store->pr_bathacrosstrack[i]));
		}
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (xdr_status)
				xdr_status = xdr_double(mb_io_ptr->xdrs, &(store->pr_bathalongtrack[i]));
		}
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->pr_beamflag, strlength);
		if (xdr_status)
			xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->ss_ping_no);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->ss_transmit_time_d);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->ss_timedelay);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->ss_timespacing);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->ss_max_side_bb_cnt);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->ss_max_side_sb_cnt);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->ss_sidescan, strlength);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tr_transmit_time_d);
		if (xdr_status)
			xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->tr_ping_no);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tr_window_mode);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tr_no_of_win_groups);
		for (int i = 0; i < 100; i++) {
			if (xdr_status)
				xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tr_repeat_count[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->tr_start[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->tr_stop[i]);
		}
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->bs_transmit_time_d);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->bs_ping_no);
		if (xdr_status)
			xdr_status = xdr_u_short(mb_io_ptr->xdrs, &store->bs_nrActualGainSets);
		if (xdr_status)
			xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_rxGup);
		if (xdr_status)
			xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_rxGain);
		if (xdr_status)
			xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_ar);
		for (int i = 0; i < MBSYS_ATLAS_HSDS2_RX_PAR; i++) {
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_TvgRx_time[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_TvgRx_gain[i]);
		}
		if (xdr_status)
			xdr_status = xdr_u_short(mb_io_ptr->xdrs, &store->bs_nrTxSets);
		for (int i = 0; i < MBSYS_ATLAS_HSDS2_TX_PAR; i++) {
			if (xdr_status)
				xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->bs_txBeamIndex[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_txLevel[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_txBeamAngle[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_pulseLength[i]);
		}
		if (xdr_status)
			xdr_status = xdr_u_short(mb_io_ptr->xdrs, &store->bs_nrBsSets);
		for (int i = 0; i < MBSYS_ATLAS_HSDS2_PFB_NUM; i++) {
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_m_tau[i]);
		}
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->bs_eff_ampli, strlength);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->bs_nis, strlength);

		/* set kind */
		if (store->start_opmode[12] == 0)
			store->kind = MB_DATA_DATA;
		else
			store->kind = MB_DATA_CALIBRATE;
	}

	/* read comment record */
	else if (telegram_id == MBSYS_ATLAS_TELEGRAM_COMMENTLAM) {
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->comment, strlength);

		/* set kind */
		store->kind = MB_DATA_COMMENT;
	}

	int status = MB_SUCCESS;

	/* set error if required */
	if (!xdr_status) {
		*error = MB_ERROR_EOF;
		status = MB_FAILURE;
	}

	/* check for broken records - these do happen!!! */
	if (status == MB_SUCCESS && store->kind != MB_DATA_COMMENT &&
	    (store->tt_beam_cnt > MBSYS_ATLAS_MAXBEAMS || store->ss_max_side_bb_cnt > MBSYS_ATLAS_MAXPIXELS ||
	     store->ss_max_side_sb_cnt > MBSYS_ATLAS_MAXPIXELS || store->start_opmode[0] != 1)) {
		*error = MB_ERROR_UNINTELLIGIBLE;
		status = MB_FAILURE;
	}

	/* check again for broken records - these do happen!!! */
	if (status == MB_SUCCESS && store->kind != MB_DATA_COMMENT) {
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (store->tt_lruntime[i] > 20.0) {
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
			}
		}
	}

	/* check again for broken records - these do happen!!! */
	if (status == MB_SUCCESS && store->kind != MB_DATA_COMMENT) {
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (store->tt_lruntime[i] > 20.0) {
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
			}
		}
	}

#ifndef MBR_HSDS2LAM_DEBUG
	if (verbose >= 5)
#endif
	{
		fprintf(stderr, "\ndbg5  HSDS2LAM telegram read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       xdr_status:              %d\n", xdr_status);
		fprintf(stderr, "dbg5       kind:                    %d\n", store->kind);
		if (store->kind == MB_DATA_COMMENT) {
			fprintf(stderr, "dbg5       comment:                 %s\n", store->comment);
		}
		else {
			fprintf(stderr, "dbg5       start_ping_no:           %d\n", store->start_ping_no);
			fprintf(stderr, "dbg5       start_transmit_time_d:   %f\n", store->start_transmit_time_d);
			fprintf(stderr, "dbg5       start_opmode:            ");
			for (int i = 0; i < 32; i++)
				fprintf(stderr, " %d", store->start_opmode[i]);
			fprintf(stderr, "\n");
			fprintf(stderr, "dbg5       start_heave:             %f\n", store->start_heave);
			fprintf(stderr, "dbg5       start_roll:              %f\n", store->start_roll);
			fprintf(stderr, "dbg5       start_pitch:             %f\n", store->start_pitch);
			fprintf(stderr, "dbg5       start_heading:           %f\n", store->start_heading);
			fprintf(stderr, "dbg5       start_ckeel:             %f\n", store->start_ckeel);
			fprintf(stderr, "dbg5       start_cmean:             %f\n", store->start_cmean);
			fprintf(stderr, "dbg5       start_depth_min:         %f\n", store->start_depth_min);
			fprintf(stderr, "dbg5       start_depth_max:         %f\n", store->start_depth_max);
			fprintf(stderr, "dbg5       tt_ping_no:              %d\n", store->tt_ping_no);
			fprintf(stderr, "dbg5       tt_transmit_time_d:      %f\n", store->tt_transmit_time_d);
			fprintf(stderr, "dbg5       tt_beam_table_index:     %d\n", store->tt_beam_table_index);
			fprintf(stderr, "dbg5       tt_beam_cnt:             %d\n", store->tt_beam_cnt);
			fprintf(stderr, "dbg5       tt_long1:                %d\n", store->tt_long1);
			fprintf(stderr, "dbg5       tt_long2:                %d\n", store->tt_long2);
			fprintf(stderr, "dbg5       tt_long3:                %d\n", store->tt_long3);
			fprintf(stderr, "dbg5       tt_xdraught:             %d\n", store->tt_xdraught);
			fprintf(stderr, "dbg5       tt_double1:              %f\n", store->tt_double1);
			fprintf(stderr, "dbg5       tt_double2:              %f\n", store->tt_double2);
			fprintf(stderr, "dbg5       tt_sensdraught:          %f\n", store->tt_sensdraught);
			fprintf(stderr, "dbg5       tt_draught:              %f\n", store->tt_draught);
			fprintf(stderr, "dbg5       beam bath xtrack lttrack tt amp stat flag:\n");
			for (int i = 0; i < store->tt_beam_cnt; i++)
				fprintf(stderr, "dbg5       %4d %12f %12f %12f %12f %3d %3d %3d\n", i, store->pr_bath[i],
				        store->pr_bathacrosstrack[i], store->pr_bathalongtrack[i], store->tt_lruntime[i], store->tt_lamplitude[i],
				        store->tt_lstatus[i], store->pr_beamflag[i]);
			fprintf(stderr, "dbg5       ss_ping_no:              %d\n", store->ss_ping_no);
			fprintf(stderr, "dbg5       ss_transmit_time_d:      %f\n", store->ss_transmit_time_d);
			fprintf(stderr, "dbg5       ss_timedelay:            %f\n", store->ss_timedelay);
			fprintf(stderr, "dbg5       ss_timespacing:          %f\n", store->ss_timespacing);
			fprintf(stderr, "dbg5       ss_max_side_bb_cnt:      %d\n", store->ss_max_side_bb_cnt);
			fprintf(stderr, "dbg5       ss_max_side_sb_cnt:      %d\n", store->ss_max_side_sb_cnt);
			for (int i = 0; i < (store->ss_max_side_bb_cnt + store->ss_max_side_sb_cnt); i++)
				fprintf(stderr, "dbg5       pixel[%d] ss:            %d\n", i, store->ss_sidescan[i]);
			fprintf(stderr, "dbg5       tr_ping_no:              %d\n", store->tr_ping_no);
			fprintf(stderr, "dbg5       tr_transmit_time_d:      %f\n", store->tr_transmit_time_d);
			fprintf(stderr, "dbg5       tr_window_mode:          %d\n", store->tr_window_mode);
			fprintf(stderr, "dbg5       tr_no_of_win_groups:     %d\n", store->tr_no_of_win_groups);
			for (int i = 0; i < MBSYS_ATLAS_MAXWINDOWS; i++) {
				fprintf(stderr, "dbg5       window[%d]:cnt start stop: %d %f %f\n", i, store->tr_repeat_count[i],
				        store->tr_start[i], store->tr_stop[i]);
			}
			fprintf(stderr, "dbg5       bs_ping_no:              %d\n", store->bs_ping_no);
			fprintf(stderr, "dbg5       bs_transmit_time_d:      %f\n", store->bs_transmit_time_d);
			fprintf(stderr, "dbg5       bs_nrActualGainSets:     %d\n", store->bs_nrActualGainSets);
			fprintf(stderr, "dbg5       bs_rxGup:                %f\n", store->bs_rxGup);
			fprintf(stderr, "dbg5       bs_rxGain:               %f\n", store->bs_rxGain);
			fprintf(stderr, "dbg5       bs_ar:                   %f\n", store->bs_ar);
			for (int i = 0; i < MBSYS_ATLAS_HSDS2_RX_PAR; i++) {
				fprintf(stderr, "dbg5       tvgrx[%d]: time gain: %f %f\n", i, store->bs_TvgRx_time[i], store->bs_TvgRx_gain[i]);
			}
			fprintf(stderr, "dbg5       bs_nrTxSets:             %d\n", store->bs_nrTxSets);
			for (int i = 0; i < MBSYS_ATLAS_HSDS2_TX_PAR; i++) {
				fprintf(stderr, "dbg5       tx[%d]: # gain ang len:    %d %f %f %f\n", i, store->bs_txBeamIndex[i],
				        store->bs_txLevel[i], store->bs_txBeamAngle[i], store->bs_pulseLength[i]);
			}
			fprintf(stderr, "dbg5       bs_nrBsSets:             %d\n", store->bs_nrBsSets);
			for (int i = 0; i < MBSYS_ATLAS_HSDS2_TX_PAR; i++) {
				fprintf(stderr, "dbg5       bs[%d]: # tau amp nis:   %f %d %d\n", i, store->bs_m_tau[i], store->bs_eff_ampli[i],
				        store->bs_nis[i]);
			}
		}
	}

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

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
int mbr_rt_hsds2lam(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* read next data from file */
	const int status = mbr_hsds2lam_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* get pointers to data structures */
	struct mbsys_atlas_struct *store = (struct mbsys_atlas_struct *)store_ptr;

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
int mbr_hsds2lam_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int xdr_status;
	int strlength;
	int telegram_id;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbsys_atlas_struct *store = (struct mbsys_atlas_struct *)store_ptr;

#ifndef MBR_HSDS2LAM_DEBUG
	if (verbose >= 5)
#endif
	{
		fprintf(stderr, "\ndbg5  HSDS2LAM telegram to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:                    %d\n", store->kind);
		if (store->kind == MB_DATA_COMMENT) {
			fprintf(stderr, "dbg5       comment:                 %s\n", store->comment);
		}
		else {
			fprintf(stderr, "dbg5       start_ping_no:           %d\n", store->start_ping_no);
			fprintf(stderr, "dbg5       start_transmit_time_d:   %f\n", store->start_transmit_time_d);
			fprintf(stderr, "dbg5       start_opmode:            ");
			for (int i = 0; i < 32; i++)
				fprintf(stderr, " %d", store->start_opmode[i]);
			fprintf(stderr, "\n");
			fprintf(stderr, "dbg5       start_heave:             %f\n", store->start_heave);
			fprintf(stderr, "dbg5       start_roll:              %f\n", store->start_roll);
			fprintf(stderr, "dbg5       start_pitch:             %f\n", store->start_pitch);
			fprintf(stderr, "dbg5       start_heading:           %f\n", store->start_heading);
			fprintf(stderr, "dbg5       start_ckeel:             %f\n", store->start_ckeel);
			fprintf(stderr, "dbg5       start_cmean:             %f\n", store->start_cmean);
			fprintf(stderr, "dbg5       start_depth_min:         %f\n", store->start_depth_min);
			fprintf(stderr, "dbg5       start_depth_max:         %f\n", store->start_depth_max);
			fprintf(stderr, "dbg5       tt_ping_no:              %d\n", store->tt_ping_no);
			fprintf(stderr, "dbg5       tt_transmit_time_d:      %f\n", store->tt_transmit_time_d);
			fprintf(stderr, "dbg5       tt_beam_table_index:     %d\n", store->tt_beam_table_index);
			fprintf(stderr, "dbg5       tt_beam_cnt:             %d\n", store->tt_beam_cnt);
			fprintf(stderr, "dbg5       tt_long1:                %d\n", store->tt_long1);
			fprintf(stderr, "dbg5       tt_long2:                %d\n", store->tt_long2);
			fprintf(stderr, "dbg5       tt_long3:                %d\n", store->tt_long3);
			fprintf(stderr, "dbg5       tt_xdraught:             %d\n", store->tt_xdraught);
			fprintf(stderr, "dbg5       tt_double1:              %f\n", store->tt_double1);
			fprintf(stderr, "dbg5       tt_double2:              %f\n", store->tt_double2);
			fprintf(stderr, "dbg5       tt_sensdraught:          %f\n", store->tt_sensdraught);
			fprintf(stderr, "dbg5       tt_draught:              %f\n", store->tt_draught);
			fprintf(stderr, "dbg5       beam bath xtrack lttrack tt amp stat flag:\n");
			for (int i = 0; i < store->tt_beam_cnt; i++)
				fprintf(stderr, "dbg5       %4d %12f %12f %12f %12f %3d %3d %3d\n", i, store->pr_bath[i],
				        store->pr_bathacrosstrack[i], store->pr_bathalongtrack[i], store->tt_lruntime[i], store->tt_lamplitude[i],
				        store->tt_lstatus[i], store->pr_beamflag[i]);
			fprintf(stderr, "dbg5       ss_ping_no:              %d\n", store->ss_ping_no);
			fprintf(stderr, "dbg5       ss_transmit_time_d:      %f\n", store->ss_transmit_time_d);
			fprintf(stderr, "dbg5       ss_timedelay:            %f\n", store->ss_timedelay);
			fprintf(stderr, "dbg5       ss_timespacing:          %f\n", store->ss_timespacing);
			fprintf(stderr, "dbg5       ss_max_side_bb_cnt:      %d\n", store->ss_max_side_bb_cnt);
			fprintf(stderr, "dbg5       ss_max_side_sb_cnt:      %d\n", store->ss_max_side_sb_cnt);
			for (int i = 0; i < (store->ss_max_side_bb_cnt + store->ss_max_side_sb_cnt); i++)
				fprintf(stderr, "dbg5       pixel[%d] ss:            %d\n", i, store->ss_sidescan[i]);
			fprintf(stderr, "dbg5       tr_ping_no:              %d\n", store->tr_ping_no);
			fprintf(stderr, "dbg5       tr_transmit_time_d:      %f\n", store->tr_transmit_time_d);
			fprintf(stderr, "dbg5       tr_window_mode:          %d\n", store->tr_window_mode);
			fprintf(stderr, "dbg5       tr_no_of_win_groups:     %d\n", store->tr_no_of_win_groups);
			for (int i = 0; i < MBSYS_ATLAS_MAXWINDOWS; i++) {
				fprintf(stderr, "dbg5       window[%d]:cnt start stop: %d %f %f\n", i, store->tr_repeat_count[i],
				        store->tr_start[i], store->tr_stop[i]);
			}
			fprintf(stderr, "dbg5       bs_ping_no:              %d\n", store->bs_ping_no);
			fprintf(stderr, "dbg5       bs_transmit_time_d:      %f\n", store->bs_transmit_time_d);
			fprintf(stderr, "dbg5       bs_nrActualGainSets:     %d\n", store->bs_nrActualGainSets);
			fprintf(stderr, "dbg5       bs_rxGup:                %f\n", store->bs_rxGup);
			fprintf(stderr, "dbg5       bs_rxGain:               %f\n", store->bs_rxGain);
			fprintf(stderr, "dbg5       bs_ar:                   %f\n", store->bs_ar);
			for (int i = 0; i < MBSYS_ATLAS_HSDS2_RX_PAR; i++) {
				fprintf(stderr, "dbg5       tvgrx[%d]: time gain: %f %f\n", i, store->bs_TvgRx_time[i], store->bs_TvgRx_gain[i]);
			}
			fprintf(stderr, "dbg5       bs_nrTxSets:             %d\n", store->bs_nrTxSets);
			for (int i = 0; i < MBSYS_ATLAS_HSDS2_TX_PAR; i++) {
				fprintf(stderr, "dbg5       tx[%d]: # gain ang len:    %d %f %f %f\n", i, store->bs_txBeamIndex[i],
				        store->bs_txLevel[i], store->bs_txBeamAngle[i], store->bs_pulseLength[i]);
			}
			fprintf(stderr, "dbg5       bs_nrBsSets:             %d\n", store->bs_nrBsSets);
			for (int i = 0; i < MBSYS_ATLAS_HSDS2_TX_PAR; i++) {
				fprintf(stderr, "dbg5       bs[%d]: # tau amp nis:   %f %d %d\n", i, store->bs_m_tau[i], store->bs_eff_ampli[i],
				        store->bs_nis[i]);
			}
		}
	}

	/* write the next record (hsds2lam telegram)  */
	*error = MB_ERROR_NO_ERROR;

	/* write ping record */
	if (store->kind == MB_DATA_DATA || store->kind == MB_DATA_CALIBRATE) {
		telegram_id = MBSYS_ATLAS_TELEGRAM_HSDS2LAM;
		xdr_status = xdr_int(mb_io_ptr->xdrs, &telegram_id);
		if (xdr_status)
			xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->start_ping_no);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_transmit_time_d);
		strlength = 32;
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->start_opmode, strlength);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_heave);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_roll);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_pitch);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_heading);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_ckeel);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_cmean);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_depth_min);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->start_depth_max);
		if (xdr_status)
			xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->tt_ping_no);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_transmit_time_d);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_beam_table_index);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_beam_cnt);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_long1);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_long2);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_long3);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tt_xdraught);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_double1);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_double2);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_sensdraught);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tt_draught);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->pr_navlon);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->pr_navlat);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->pr_speed);
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &(store->tt_lruntime[i]));
		}
		if (store->tt_beam_cnt % 4 == 0)
			strlength = store->tt_beam_cnt;
		else
			strlength = 4 * ((store->tt_beam_cnt / 4) + 1);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->tt_lamplitude, strlength);
		if (store->tt_beam_cnt % 4 == 0)
			strlength = store->tt_beam_cnt;
		else
			strlength = 4 * ((store->tt_beam_cnt / 4) + 1);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->tt_lstatus, strlength);
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (xdr_status)
				xdr_status = xdr_double(mb_io_ptr->xdrs, &(store->pr_bath[i]));
		}
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (xdr_status)
				xdr_status = xdr_double(mb_io_ptr->xdrs, &(store->pr_bathacrosstrack[i]));
		}
		for (int i = 0; i < store->tt_beam_cnt; i++) {
			if (xdr_status)
				xdr_status = xdr_double(mb_io_ptr->xdrs, &(store->pr_bathalongtrack[i]));
		}
		if (store->tt_beam_cnt % 4 == 0)
			strlength = store->tt_beam_cnt;
		else
			strlength = 4 * ((store->tt_beam_cnt / 4) + 1);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->pr_beamflag, strlength);
		if (xdr_status)
			xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->ss_ping_no);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->ss_transmit_time_d);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->ss_timedelay);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->ss_timespacing);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->ss_max_side_bb_cnt);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->ss_max_side_sb_cnt);
		if ((store->ss_max_side_bb_cnt + store->ss_max_side_sb_cnt) % 4 == 0)
			strlength = store->ss_max_side_bb_cnt + store->ss_max_side_sb_cnt;
		else
			strlength = 4 * (((store->ss_max_side_bb_cnt + store->ss_max_side_sb_cnt) / 4) + 1);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->ss_sidescan, strlength);
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->tr_transmit_time_d);
		if (xdr_status)
			xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->tr_ping_no);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tr_window_mode);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tr_no_of_win_groups);
		for (int i = 0; i < 100; i++) {
			if (xdr_status)
				xdr_status = xdr_int(mb_io_ptr->xdrs, &store->tr_repeat_count[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->tr_start[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->tr_stop[i]);
		}
		if (xdr_status)
			xdr_status = xdr_double(mb_io_ptr->xdrs, &store->bs_transmit_time_d);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &store->bs_ping_no);
		if (xdr_status)
			xdr_status = xdr_u_short(mb_io_ptr->xdrs, &store->bs_nrActualGainSets);
		if (xdr_status)
			xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_rxGup);
		if (xdr_status)
			xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_rxGain);
		if (xdr_status)
			xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_ar);
		for (int i = 0; i < MBSYS_ATLAS_HSDS2_RX_PAR; i++) {
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_TvgRx_time[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_TvgRx_gain[i]);
		}
		if (xdr_status)
			xdr_status = xdr_u_short(mb_io_ptr->xdrs, &store->bs_nrTxSets);
		for (int i = 0; i < MBSYS_ATLAS_HSDS2_TX_PAR; i++) {
			if (xdr_status)
				xdr_status = xdr_u_int(mb_io_ptr->xdrs, &store->bs_txBeamIndex[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_txLevel[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_txBeamAngle[i]);
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_pulseLength[i]);
		}
		if (xdr_status)
			xdr_status = xdr_u_short(mb_io_ptr->xdrs, &store->bs_nrBsSets);
		for (int i = 0; i < MBSYS_ATLAS_HSDS2_PFB_NUM; i++) {
			if (xdr_status)
				xdr_status = xdr_float(mb_io_ptr->xdrs, &store->bs_m_tau[i]);
		}
		strlength = MBSYS_ATLAS_HSDS2_PFB_NUM;
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->bs_eff_ampli, strlength);
		strlength = MBSYS_ATLAS_HSDS2_PFB_NUM;
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->bs_nis, strlength);
	}

	/* write comment record */
	else if (store->kind == MB_DATA_COMMENT) {
		telegram_id = MBSYS_ATLAS_TELEGRAM_COMMENTLAM;
		xdr_status = xdr_int(mb_io_ptr->xdrs, &telegram_id);
		if ((strlen(store->comment) + 1) % 4 == 0)
			strlength = strlen(store->comment) + 1;
		else
			strlength = 4 * (((strlen(store->comment) + 1) / 4) + 1);
		if (xdr_status)
			xdr_status = xdr_int(mb_io_ptr->xdrs, &strlength);
		if (xdr_status)
			xdr_status = xdr_opaque(mb_io_ptr->xdrs, (char *)store->comment, strlength);
	}

	else
		xdr_status = false;

	int status = MB_SUCCESS;

	/* set error if required */
	if (!xdr_status) {
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
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
int mbr_wt_hsds2lam(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* write next data to file */
	const int status = mbr_hsds2lam_wr_data(verbose, mbio_ptr, store_ptr, error);

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
int mbr_register_hsds2lam(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_hsds2lam(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hsds2lam;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hsds2lam;
	mb_io_ptr->mb_io_store_alloc = &mbsys_atlas_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_atlas_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hsds2lam;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hsds2lam;
	mb_io_ptr->mb_io_dimensions = &mbsys_atlas_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_atlas_extract;
	mb_io_ptr->mb_io_insert = &mbsys_atlas_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_atlas_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_atlas_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_atlas_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_atlas_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_atlas_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_atlas_copy;
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
