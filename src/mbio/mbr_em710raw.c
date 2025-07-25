/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em710raw.c	2/26/2008
 *
 *    Copyright (c) 2008-2025 by
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
 * mbr_em710raw.c contains the functions for reading and writing
 * multibeam data in the EM710RAW format.
 * These functions include:
 *   mbr_alm_em710raw	- allocate read/write memory
 *   mbr_dem_em710raw	- deallocate read/write memory
 *   mbr_rt_em710raw	- read and translate data
 *   mbr_wt_em710raw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 26, 2008
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mb_swap.h"
#include "mbsys_simrad3.h"

/* get NaN detector */
#if defined(isnanf)
#define check_fnan(x) isnanf((x))
#elif defined(isnan)
#define check_fnan(x) isnan((double)(x))
#elif HAVE_ISNANF == 1
#define check_fnan(x) isnanf(x)
extern int isnanf(float x);
#elif HAVE_ISNAN == 1
#define check_fnan(x) isnan((double)(x))
#elif HAVE_ISNAND == 1
#define check_fnan(x) isnand((double)(x))
#else
#define check_fnan(x) ((x) != (x))
#endif

/* control method of estimating range and angles for bathymetry recalculation
    - by default the code solves for the angles and heave offsets that come
      close to matching the original reported bathymetry by raytracing through
      the original water sound speed model.
    - if the define below is uncommented, then the code will instead adjust
      the angles and ranges to match the original bathymetry - this will
      generally achieve bathymetry closer to that reported by the sonar, but
      requires modifying the travel times (an unsavory and unsatisfactory
      approach). */
/* #define MBR_EM710RAW_BATH_RECALC_TWEAK_ANGLE_RANGE 1 */

/* turn on debug statements here */
// #define MBR_EM710RAW_DEBUG 1
// #define MBR_EM710RAW_DEBUG2 1
// #define MBR_EM710RAW_DEBUG3 1

/*--------------------------------------------------------------------*/
int mbr_info_em710raw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_SIMRAD3;
	*beams_bath_max = 400;
	*beams_amp_max = 400;
	*pixels_ss_max = 1024;
	strncpy(format_name, "EM710RAW", MB_NAME_LENGTH);
	strncpy(system_name, "SIMRAD3", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_EM710RAW\nInformal Description: Kongsberg 3rd generation multibeam vendor format\nAttributes:    "
	        "       Kongsberg EM122, EM302, EM710,\n                      bathymetry, amplitude, and sidescan,\n                 "
	        "     up to 400 beams, variable pixels, binary, Kongsberg.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_SINGLE;
	*variable_beams = true;
	*traveltime = true;
	*beam_flagging = false;
	*platform_source = MB_DATA_START;
	*nav_source = MB_DATA_NAV;
	*sensordepth_source = MB_DATA_HEIGHT;
	*heading_source = MB_DATA_NAV;
	*attitude_source = MB_DATA_ATTITUDE;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 2.0;
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
int mbr_alm_em710raw(int verbose, void *mbio_ptr, int *error) {
	int *databyteswapped;
	double *pixel_size;
	double *swath_width;

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
	const int status = mbsys_simrad3_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* initialize saved values */
	databyteswapped = (int *)&mb_io_ptr->save1;
	pixel_size = &mb_io_ptr->saved1;
	swath_width = &mb_io_ptr->saved2;
	*databyteswapped = -1;
	*pixel_size = 0.0;
	*swath_width = 0.0;

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
int mbr_dem_em710raw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	const int status = mbsys_simrad3_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_em710raw_chk_label(int verbose, void *mbio_ptr, char *label, short *type, short *sonar) {
	mb_u_char startbyte;
	mb_u_char typebyte;
	short *sonar_save;
	short sonarunswap;
	short sonarswap;
	int *databyteswapped;
	bool typegood;
	bool sonargood;
	bool sonarswapgood;
	bool sonarunswapgood;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       label:      %x%x%x%x\n", label[0], label[1], label[2], label[3]);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	sonar_save = (short *)(&mb_io_ptr->save11);
	databyteswapped = (int *)&mb_io_ptr->save1;

#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "Check label: %x|%x|%x|%x\n", label[0], label[1], label[2], label[3]);
#endif

	/* check for valid start byte and type */
	startbyte = label[0];
	typebyte = label[1];
	if (startbyte == EM3_START_BYTE &&
	    (typebyte == EM3_ID_PU_ID || typebyte == EM3_ID_PU_STATUS || typebyte == EM3_ID_PU_BIST ||
	     typebyte == EM3_ID_EXTRAPARAMETERS || typebyte == EM3_ID_ATTITUDE || typebyte == EM3_ID_NETATTITUDE ||
	     typebyte == EM3_ID_CLOCK || typebyte == EM3_ID_BATH || typebyte == EM3_ID_SBDEPTH || typebyte == EM3_ID_RAWBEAM ||
	     typebyte == EM3_ID_SSV || typebyte == EM3_ID_HEADING || typebyte == EM3_ID_START || typebyte == EM3_ID_TILT ||
	     typebyte == EM3_ID_CBECHO || typebyte == EM3_ID_RAWBEAM4 || typebyte == EM3_ID_QUALITY || typebyte == EM3_ID_POS ||
	     typebyte == EM3_ID_RUN_PARAMETER || typebyte == EM3_ID_SS || typebyte == EM3_ID_TIDE || typebyte == EM3_ID_SVP2 ||
	     typebyte == EM3_ID_SVP || typebyte == EM3_ID_SSPINPUT || typebyte == EM3_ID_BATH2 || typebyte == EM3_ID_SS2 ||
	     typebyte == EM3_ID_RAWBEAM2 || typebyte == EM3_ID_RAWBEAM3 || typebyte == EM3_ID_HEIGHT || typebyte == EM3_ID_STOP ||
	     typebyte == EM3_ID_WATERCOLUMN || typebyte == EM3_ID_REMOTE || typebyte == EM3_ID_SSP || typebyte == EM3_ID_BATH_MBA ||
	     typebyte == EM3_ID_SS_MBA || typebyte == EM3_ID_BATH2_MBA || typebyte == EM3_ID_SS2_MBA)) {
		typegood = true;
	}
	else {
		typegood = false;
	}

	/* check for data byte swapping if necessary */
	sonarswapgood = false;
	sonarunswapgood = false;
	if (typegood && *databyteswapped == -1) {
		sonarunswap = *((short *)&label[2]);
		sonarswap = mb_swap_short(sonarunswap);

		/* check for valid sonarunswap */
		if (sonarunswap == MBSYS_SIMRAD3_M3
		    || sonarunswap == MBSYS_SIMRAD3_EM2045
		    || sonarunswap == MBSYS_SIMRAD3_EM2040
		    || sonarunswap == MBSYS_SIMRAD3_EM850
		    || sonarunswap == MBSYS_SIMRAD3_EM710
		    || sonarunswap == MBSYS_SIMRAD3_EM712
		    || sonarunswap == MBSYS_SIMRAD3_EM302
		    || sonarunswap == MBSYS_SIMRAD3_EM304
		    || sonarunswap == MBSYS_SIMRAD3_EM122
		    || sonarunswap == MBSYS_SIMRAD3_EM124
		    || sonarunswap == MBSYS_SIMRAD3_EM120
		    || sonarunswap == MBSYS_SIMRAD3_EM300
		    || sonarunswap == MBSYS_SIMRAD3_EM1002
		    || sonarunswap == MBSYS_SIMRAD3_EM2000
		    || sonarunswap == MBSYS_SIMRAD3_EM3000
		    || sonarunswap == MBSYS_SIMRAD3_EM3000D_1
		    || sonarunswap == MBSYS_SIMRAD3_EM3000D_2
		    || sonarunswap == MBSYS_SIMRAD3_EM3000D_3
		    || sonarunswap == MBSYS_SIMRAD3_EM3000D_4
		    || sonarunswap == MBSYS_SIMRAD3_EM3000D_5
		    || sonarunswap == MBSYS_SIMRAD3_EM3000D_6
		    || sonarunswap == MBSYS_SIMRAD3_EM3000D_7
		    || sonarunswap == MBSYS_SIMRAD3_EM3000D_8
		    || sonarunswap == MBSYS_SIMRAD3_EM3002) {
			sonarunswapgood = true;
		}
		else {
			sonarunswapgood = false;
		}

		/* check for valid sonarswap */
		if (sonarswap == MBSYS_SIMRAD3_M3
		    || sonarswap == MBSYS_SIMRAD3_EM2045
		    || sonarswap == MBSYS_SIMRAD3_EM2040
		    || sonarswap == MBSYS_SIMRAD3_EM850
		    || sonarswap == MBSYS_SIMRAD3_EM710
		    || sonarswap == MBSYS_SIMRAD3_EM712
		    || sonarswap == MBSYS_SIMRAD3_EM302
		    || sonarswap == MBSYS_SIMRAD3_EM304
		    || sonarswap == MBSYS_SIMRAD3_EM122
		    || sonarswap == MBSYS_SIMRAD3_EM124
		    || sonarswap == MBSYS_SIMRAD3_EM120
		    || sonarswap == MBSYS_SIMRAD3_EM300
		    || sonarswap == MBSYS_SIMRAD3_EM1002
		    || sonarswap == MBSYS_SIMRAD3_EM2000
		    || sonarswap == MBSYS_SIMRAD3_EM3000
		    || sonarswap == MBSYS_SIMRAD3_EM3000D_1
		    || sonarswap == MBSYS_SIMRAD3_EM3000D_2
		    || sonarswap == MBSYS_SIMRAD3_EM3000D_3
		    || sonarswap == MBSYS_SIMRAD3_EM3000D_4
		    || sonarswap == MBSYS_SIMRAD3_EM3000D_5
		    || sonarswap == MBSYS_SIMRAD3_EM3000D_6
		    || sonarswap == MBSYS_SIMRAD3_EM3000D_7
		    || sonarswap == MBSYS_SIMRAD3_EM3000D_8
		    || sonarswap == MBSYS_SIMRAD3_EM3002) {
			sonarswapgood = true;
		}
		else {
			sonarswapgood = false;
		}
		if (sonarunswapgood && !sonarswapgood) {
			if (mb_io_ptr->byteswapped)
				*databyteswapped = true;
			else
				*databyteswapped = false;
		}
		else if (!sonarunswapgood && sonarswapgood) {
			if (mb_io_ptr->byteswapped)
				*databyteswapped = false;
			else
				*databyteswapped = true;
		}
	}

	/* set flag to swap bytes if necessary */
	// int swap = *databyteswapped;

	*type = *((short *)&label[0]);
	*sonar = *((short *)&label[2]);
	if (mb_io_ptr->byteswapped)
		*type = mb_swap_short(*type);
	if (*databyteswapped != mb_io_ptr->byteswapped) {
		*sonar = mb_swap_short(*sonar);
	}

#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr,
	        "typegood:%d mb_io_ptr->byteswapped:%d sonarunswapgood:%d sonarswapgood:%d *databyteswapped:%d *type:%d *sonar:%d\n",
	        typegood, mb_io_ptr->byteswapped, sonarunswapgood, sonarswapgood, *databyteswapped, *type, *sonar);
#endif

	/* check for valid sonar */
	if (*sonar == MBSYS_SIMRAD3_M3
	    || *sonar == MBSYS_SIMRAD3_EM2045
	    || *sonar == MBSYS_SIMRAD3_EM2040
	    || *sonar == MBSYS_SIMRAD3_EM850
	    || *sonar == MBSYS_SIMRAD3_EM710
	    || *sonar == MBSYS_SIMRAD3_EM712
	    || *sonar == MBSYS_SIMRAD3_EM302
	    || *sonar == MBSYS_SIMRAD3_EM304
	    || *sonar == MBSYS_SIMRAD3_EM122
	    || *sonar == MBSYS_SIMRAD3_EM124
	    || *sonar == MBSYS_SIMRAD3_EM120
	    || *sonar == MBSYS_SIMRAD3_EM300
	    || *sonar == MBSYS_SIMRAD3_EM1002
	    || *sonar == MBSYS_SIMRAD3_EM2000
	    || *sonar == MBSYS_SIMRAD3_EM3000
	    || *sonar == MBSYS_SIMRAD3_EM3000D_1
	    || *sonar == MBSYS_SIMRAD3_EM3000D_2
	    || *sonar == MBSYS_SIMRAD3_EM3000D_3
	    || *sonar == MBSYS_SIMRAD3_EM3000D_4
	    || *sonar == MBSYS_SIMRAD3_EM3000D_5
	    || *sonar == MBSYS_SIMRAD3_EM3000D_6
	    || *sonar == MBSYS_SIMRAD3_EM3000D_7
	    || *sonar == MBSYS_SIMRAD3_EM3000D_8
	    || *sonar == MBSYS_SIMRAD3_EM3002) {
		sonargood = true;
	}
	else {
		sonargood = false;
	}

	if (startbyte == EM3_START_BYTE && !typegood && sonargood) {
		mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_BAD_DATAGRAM);
		if (verbose >= 1)
			fprintf(stderr, "Bad datagram type: %4.4hX %4.4hX | %d %d\n", *type, *sonar, *type, *sonar);
	}

	int status = MB_SUCCESS;

	if (typegood != true || sonargood != true) {
		status = MB_FAILURE;
	}

	/* save sonar if successful */
	if (status == MB_SUCCESS)
		*sonar_save = *sonar;

	/* allow exception found in some data */
	if (*type == EM3_SSV && *sonar == 0 && *sonar_save != 0) {
		status = MB_SUCCESS;
		*sonar = *sonar_save;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       type:       %d\n", *type);
		fprintf(stderr, "dbg2       sonar:      %d\n", *sonar);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_puid(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short type, short sonar,
                           int *goodend, int *error) {
	(void)type; // unused

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_STATUS;
	store->type = EM3_PU_ID;
	store->sonar = sonar;

	/* read binary values into char array */
	size_t read_len = (size_t)(EM3_PU_ID_SIZE - 4);
	char line[EM3_PU_ID_SIZE];
	const int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->pid_date);
		if (store->sts_date != 0)
			store->date = store->sts_date;
		mb_get_binary_int(swap, &line[4], &store->pid_msec);
		if (store->sts_date != 0)
			store->msec = store->pid_msec;
		unsigned short ushort_val = 0;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		store->pid_byte_order_flag = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		store->pid_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		store->pid_udp_port_1 = (int)(ushort_val);
		mb_get_binary_short(swap, &line[14], &ushort_val);
		store->pid_udp_port_2 = (int)(ushort_val);
		mb_get_binary_short(swap, &line[16], &ushort_val);
		store->pid_udp_port_3 = (int)(ushort_val);
		mb_get_binary_short(swap, &line[18], &ushort_val);
		store->pid_udp_port_4 = (int)(ushort_val);
		mb_get_binary_int(swap, &line[20], &store->pid_sys_descriptor);
    for (int i=0; i<16; i++) {
      store->pid_pu_sw_version[i] = line[24+i];
    }
    for (int i=0; i<16; i++) {
      store->pid_bsp_sw_version[i] = line[40+i];
    }
    for (int i=0; i<16; i++) {
      store->pid_head1_version[i] = line[56+i];
    }
    for (int i=0; i<16; i++) {
      store->pid_head2_version[i] = line[72+i];
    }
		mb_get_binary_int(swap, &line[88], &store->pid_host_ip);
    store->pid_tx_opening_angle = line[92];
    store->pid_rx_opening_angle = line[93];
    for (int i=0; i<7; i++) {
      store->pid_spare[i] = line[94+i];
    }
		if (line[EM3_PU_ID_SIZE - 7] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM3_PU_ID_SIZE - 7],
		        line[EM3_PU_ID_SIZE - 7], line[EM3_PU_ID_SIZE - 6], line[EM3_PU_ID_SIZE - 6],
		        line[EM3_PU_ID_SIZE - 5], line[EM3_PU_ID_SIZE - 5]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:                %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:               %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                %d\n", store->msec);
		fprintf(stderr, "dbg5       pid_date:            %d\n", store->pid_date);
		fprintf(stderr, "dbg5       pid_msec:            %d\n", store->pid_msec);
		fprintf(stderr, "dbg5       pid_byte_order_flag: %d\n", store->pid_byte_order_flag);
		fprintf(stderr, "dbg5       pid_serial:          %d\n", store->pid_serial);
		fprintf(stderr, "dbg5       pid_udp_port_1:      %d\n", store->pid_udp_port_1);
		fprintf(stderr, "dbg5       pid_udp_port_2:      %d\n", store->pid_udp_port_2);
		fprintf(stderr, "dbg5       pid_udp_port_3:      %d\n", store->pid_udp_port_3);
		fprintf(stderr, "dbg5       pid_udp_port_4:      %d\n", store->pid_udp_port_4);
		fprintf(stderr, "dbg5       pid_pu_sw_version:   ");
    for (int i=0; i<16; i++) {
      fprintf(stderr, "%c", store->pid_pu_sw_version[i]);
    }
    fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       pid_bsp_sw_version:   ");
    for (int i=0; i<16; i++) {
      fprintf(stderr, "%c", store->pid_bsp_sw_version[i]);
    }
    fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       pid_head1_version:   ");
    for (int i=0; i<16; i++) {
      fprintf(stderr, "%c", store->pid_head1_version[i]);
    }
    fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       pid_head2_version:   ");
    for (int i=0; i<16; i++) {
      fprintf(stderr, "%c", store->pid_head2_version[i]);
    }
    fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       pid_host_ip:         %d\n", store->pid_host_ip);
		fprintf(stderr, "dbg5       pid_tx_opening_angle:%d\n", store->pid_tx_opening_angle);
		fprintf(stderr, "dbg5       pid_rx_opening_angle:%d\n", store->pid_rx_opening_angle);
		fprintf(stderr, "dbg5       pid_spare:           ");
    for (int i=0; i<7; i++) {
      fprintf(stderr, "%c", store->pid_spare[i]);
    }
    fprintf(stderr, "\n");
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_status(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short type, short sonar,
                           int *goodend, int *error) {
	(void)type; // unused

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_STATUS;
	store->type = EM3_PU_STATUS;
	store->sonar = sonar;

	/* read binary values into char array */
	size_t read_len = (size_t)(EM3_PU_STATUS_SIZE - 4);
	char line[EM3_PU_STATUS_SIZE];
	const int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->sts_date);
		if (store->sts_date != 0)
			store->date = store->sts_date;
		mb_get_binary_int(swap, &line[4], &store->sts_msec);
		if (store->sts_date != 0)
			store->msec = store->sts_msec;
		short short_val = 0;
		mb_get_binary_short(swap, &line[8], &short_val);
		store->sts_status_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		store->sts_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		store->sts_pingrate = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[14], &short_val);
		store->sts_ping_count = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[16], &store->sts_load);
		mb_get_binary_int(swap, &line[20], &store->sts_udp_status);
		mb_get_binary_int(swap, &line[24], &store->sts_serial1_status);
		mb_get_binary_int(swap, &line[28], &store->sts_serial2_status);
		mb_get_binary_int(swap, &line[32], &store->sts_serial3_status);
		mb_get_binary_int(swap, &line[36], &store->sts_serial4_status);
		store->sts_pps_status = (mb_u_char)line[40];
		store->sts_position_status = (mb_s_char)line[41];
		store->sts_attitude_status = (mb_s_char)line[42];
		store->sts_clock_status = (mb_s_char)line[43];
		store->sts_heading_status = (mb_s_char)line[44];
		store->sts_pu_status = (mb_u_char)line[45];
		mb_get_binary_short(swap, &line[46], &short_val);
		store->sts_last_heading = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[48], &short_val);
		store->sts_last_roll = (int)((short)short_val);
		mb_get_binary_short(swap, &line[50], &short_val);
		store->sts_last_pitch = (int)((short)short_val);
		mb_get_binary_short(swap, &line[52], &short_val);
		store->sts_last_heave = (int)((short)short_val);
		mb_get_binary_short(swap, &line[54], &short_val);
		store->sts_last_ssv = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[56], &store->sts_last_depth);
		mb_get_binary_int(swap, &line[60], &store->sts_spare);
		store->sts_bso = (mb_s_char)line[64];
		store->sts_bsn = (mb_s_char)line[65];
		store->sts_gain = (mb_s_char)line[66];
		store->sts_dno = (mb_u_char)line[67];
		mb_get_binary_short(swap, &line[68], &short_val);
		store->sts_rno = (int)((unsigned short)short_val);
		store->sts_port = (mb_s_char)line[70];
		store->sts_stbd = (mb_u_char)line[71];
		mb_get_binary_short(swap, &line[72], &short_val);
		store->sts_ssp = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[74], &short_val);
		store->sts_yaw = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[76], &short_val);
		store->sts_port2 = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[78], &short_val);
		store->sts_stbd2 = (int)((unsigned short)short_val);
		store->sts_spare2 = (mb_u_char)line[80];
		if (line[EM3_PU_STATUS_SIZE - 7] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM3_PU_STATUS_SIZE - 7],
		        line[EM3_PU_STATUS_SIZE - 7], line[EM3_PU_STATUS_SIZE - 6], line[EM3_PU_STATUS_SIZE - 6],
		        line[EM3_PU_STATUS_SIZE - 5], line[EM3_PU_STATUS_SIZE - 5]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:                %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:               %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                %d\n", store->msec);
		fprintf(stderr, "dbg5       sts_date:            %d\n", store->sts_date);
		fprintf(stderr, "dbg5       sts_msec:            %d\n", store->sts_msec);
		fprintf(stderr, "dbg5       sts_status_count:    %d\n", store->sts_status_count);
		fprintf(stderr, "dbg5       sts_serial:          %d\n", store->sts_serial);
		fprintf(stderr, "dbg5       sts_pingrate:        %d\n", store->sts_pingrate);
		fprintf(stderr, "dbg5       sts_ping_count:      %d\n", store->sts_ping_count);
		fprintf(stderr, "dbg5       sts_load:            %d\n", store->sts_load);
		fprintf(stderr, "dbg5       sts_udp_status:      %d\n", store->sts_udp_status);
		fprintf(stderr, "dbg5       sts_serial1_status:  %d\n", store->sts_serial1_status);
		fprintf(stderr, "dbg5       sts_serial2_status:  %d\n", store->sts_serial2_status);
		fprintf(stderr, "dbg5       sts_serial3_status:  %d\n", store->sts_serial3_status);
		fprintf(stderr, "dbg5       sts_serial4_status:  %d\n", store->sts_serial4_status);
		fprintf(stderr, "dbg5       sts_pps_status:      %d\n", store->sts_pps_status);
		fprintf(stderr, "dbg5       sts_position_status: %d\n", store->sts_position_status);
		fprintf(stderr, "dbg5       sts_attitude_status: %d\n", store->sts_attitude_status);
		fprintf(stderr, "dbg5       sts_clock_status:    %d\n", store->sts_clock_status);
		fprintf(stderr, "dbg5       sts_heading_status:  %d\n", store->sts_heading_status);
		fprintf(stderr, "dbg5       sts_pu_status:       %d\n", store->sts_pu_status);
		fprintf(stderr, "dbg5       sts_last_heading:    %d\n", store->sts_last_heading);
		fprintf(stderr, "dbg5       sts_last_roll:       %d\n", store->sts_last_roll);
		fprintf(stderr, "dbg5       sts_last_pitch:      %d\n", store->sts_last_pitch);
		fprintf(stderr, "dbg5       sts_last_heave:      %d\n", store->sts_last_heave);
		fprintf(stderr, "dbg5       sts_last_ssv:        %d\n", store->sts_last_ssv);
		fprintf(stderr, "dbg5       sts_last_heave:      %d\n", store->sts_last_heave);
		fprintf(stderr, "dbg5       sts_last_depth:      %d\n", store->sts_last_depth);
		fprintf(stderr, "dbg5       sts_spare:           %d\n", store->sts_spare);
		fprintf(stderr, "dbg5       sts_bso:             %d\n", store->sts_bso);
		fprintf(stderr, "dbg5       sts_bsn:             %d\n", store->sts_bsn);
		fprintf(stderr, "dbg5       sts_gain:            %d\n", store->sts_gain);
		fprintf(stderr, "dbg5       sts_dno:             %d\n", store->sts_dno);
		fprintf(stderr, "dbg5       sts_rno:             %d\n", store->sts_rno);
		fprintf(stderr, "dbg5       sts_port:            %d\n", store->sts_port);
		fprintf(stderr, "dbg5       sts_stbd:            %d\n", store->sts_stbd);
		fprintf(stderr, "dbg5       sts_ssp:             %d\n", store->sts_ssp);
		fprintf(stderr, "dbg5       sts_yaw:             %d\n", store->sts_yaw);
		fprintf(stderr, "dbg5       sts_port2:           %d\n", store->sts_port2);
		fprintf(stderr, "dbg5       sts_stbd2:           %d\n", store->sts_stbd2);
		fprintf(stderr, "dbg5       sts_spare2:          %d\n", store->sts_spare2);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_start(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short type, short sonar,
                          int *version, int *num_sonars, int *goodend, int *error) {
	char line[MBSYS_SIMRAD3_BUFFER_SIZE];
	short short_val = 0;
	size_t read_len;
	int len;
	char *comma_ptr;
	int i1, i2, i3;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       type:       %d\n", type);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get sensordepth_only flag */
	bool *sensordepth_only = (bool *)&mb_io_ptr->save5;

	/* set goodend false until a good end is found */
	*goodend = false;

	/* make sure comment is initialized */
	store->par_com[0] = '\0';

	/* set type value */
	store->type = type;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = (size_t)(EM3_START_HEADER_SIZE);
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->par_date);
		store->date = store->par_date;
		mb_get_binary_int(swap, &line[4], &store->par_msec);
		store->msec = store->par_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		store->par_line_num = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		store->par_serial_1 = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		store->par_serial_2 = (int)((unsigned short)short_val);

		/* set the number of sonars */
		if (store->par_serial_2 != 0)
			*num_sonars = 2;
		else
			*num_sonars = 1;
	}

	/* now loop over reading individual characters to
	    handle ASCII parameter values */
	bool done = false;
	len = 0;
	while (status == MB_SUCCESS && !done) {
		read_len = (size_t)1;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)&line[len], &read_len, error);
		if (status == MB_SUCCESS) {
			len++;
		}
		else {
			done = true;
		}

		if (status == MB_SUCCESS && (((mb_u_char)(line[len - 1])) < 32 || ((mb_u_char)(line[len - 1])) > 127) &&
		    ((mb_u_char)(line[len - 1])) != '\r' && ((mb_u_char)(line[len - 1])) != '\n') {
			done = true;
			if (len > 1)
				line[0] = line[len - 1];
		}
		else if (status == MB_SUCCESS && line[len - 1] == ',' && len > 5) {
			line[len] = 0;
			if (strncmp("WLZ=", line, 4) == 0)
				mb_get_double(&(store->par_wlz), &line[4], len - 5);
			else if (strncmp("SMH=", line, 4) == 0)
				mb_get_int(&(store->par_smh), &line[4], len - 5);

			else if (strncmp("HUN=", line, 4) == 0)
				mb_get_int(&(store->par_hun), &line[4], len - 5);
			else if (strncmp("HUT=", line, 4) == 0)
				mb_get_double(&(store->par_hut), &line[4], len - 5);
			else if (strncmp("TXS=", line, 4) == 0)
				mb_get_int(&(store->par_txs), &line[4], len - 5);
			else if (strncmp("T2X=", line, 4) == 0)
				mb_get_int(&(store->par_t2x), &line[4], len - 5);
			else if (strncmp("R1S=", line, 4) == 0)
				mb_get_int(&(store->par_r1s), &line[4], len - 5);
			else if (strncmp("R2S=", line, 4) == 0)
				mb_get_int(&(store->par_r2s), &line[4], len - 5);
			else if (strncmp("STC=", line, 4) == 0)
				mb_get_int(&(store->par_stc), &line[4], len - 5);

			else if (strncmp("S0Z=", line, 4) == 0)
				mb_get_double(&(store->par_s0z), &line[4], len - 5);
			else if (strncmp("S0X=", line, 4) == 0)
				mb_get_double(&(store->par_s0x), &line[4], len - 5);
			else if (strncmp("S0Y=", line, 4) == 0)
				mb_get_double(&(store->par_s0y), &line[4], len - 5);
			else if (strncmp("S0H=", line, 4) == 0)
				mb_get_double(&(store->par_s0h), &line[4], len - 5);
			else if (strncmp("S0R=", line, 4) == 0)
				mb_get_double(&(store->par_s0r), &line[4], len - 5);
			else if (strncmp("S0P=", line, 4) == 0)
				mb_get_double(&(store->par_s0p), &line[4], len - 5);

			else if (strncmp("S1Z=", line, 4) == 0)
				mb_get_double(&(store->par_s1z), &line[4], len - 5);
			else if (strncmp("S1X=", line, 4) == 0)
				mb_get_double(&(store->par_s1x), &line[4], len - 5);
			else if (strncmp("S1Y=", line, 4) == 0)
				mb_get_double(&(store->par_s1y), &line[4], len - 5);
			else if (strncmp("S1H=", line, 4) == 0)
				mb_get_double(&(store->par_s1h), &line[4], len - 5);
			else if (strncmp("S1R=", line, 4) == 0)
				mb_get_double(&(store->par_s1r), &line[4], len - 5);
			else if (strncmp("S1P=", line, 4) == 0)
				mb_get_double(&(store->par_s1p), &line[4], len - 5);
			else if (strncmp("S1N=", line, 4) == 0)
				mb_get_int(&(store->par_s1n), &line[4], len - 5);

			else if (strncmp("S2Z=", line, 4) == 0)
				mb_get_double(&(store->par_s2z), &line[4], len - 5);
			else if (strncmp("S2X=", line, 4) == 0)
				mb_get_double(&(store->par_s2x), &line[4], len - 5);
			else if (strncmp("S2Y=", line, 4) == 0)
				mb_get_double(&(store->par_s2y), &line[4], len - 5);
			else if (strncmp("S2H=", line, 4) == 0)
				mb_get_double(&(store->par_s2h), &line[4], len - 5);
			else if (strncmp("S2R=", line, 4) == 0)
				mb_get_double(&(store->par_s2r), &line[4], len - 5);
			else if (strncmp("S2P=", line, 4) == 0)
				mb_get_double(&(store->par_s2p), &line[4], len - 5);
			else if (strncmp("S2N=", line, 4) == 0)
				mb_get_int(&(store->par_s2n), &line[4], len - 5);

			else if (strncmp("S3Z=", line, 4) == 0)
				mb_get_double(&(store->par_s3z), &line[4], len - 5);
			else if (strncmp("S3X=", line, 4) == 0)
				mb_get_double(&(store->par_s3x), &line[4], len - 5);
			else if (strncmp("S3Y=", line, 4) == 0)
				mb_get_double(&(store->par_s3y), &line[4], len - 5);
			else if (strncmp("S3H=", line, 4) == 0)
				mb_get_double(&(store->par_s3h), &line[4], len - 5);
			else if (strncmp("S3R=", line, 4) == 0)
				mb_get_double(&(store->par_s3r), &line[4], len - 5);
			else if (strncmp("S3P=", line, 4) == 0)
				mb_get_double(&(store->par_s3p), &line[4], len - 5);

			else if (strncmp("S1S=", line, 4) == 0)
				mb_get_int(&(store->par_s1s), &line[4], len - 5);
			else if (strncmp("S2S=", line, 4) == 0)
				mb_get_int(&(store->par_s2s), &line[4], len - 5);

			else if (strncmp("GO1=", line, 4) == 0)
				mb_get_double(&(store->par_go1), &line[4], len - 5);
			else if (strncmp("GO2=", line, 4) == 0)
				mb_get_double(&(store->par_go2), &line[4], len - 5);
			else if (strncmp("OBO=", line, 4) == 0)
				mb_get_double(&(store->par_obo), &line[4], len - 5);
			else if (strncmp("FGD=", line, 4) == 0)
				mb_get_double(&(store->par_fgd), &line[4], len - 5);

			else if (strncmp("TSV=", line, 4) == 0)
				strncpy(store->par_tsv, &line[4], MIN(len - 5, 15));
			else if (strncmp("RSV=", line, 4) == 0)
				strncpy(store->par_rsv, &line[4], MIN(len - 5, 15));
			else if (strncmp("BSV=", line, 4) == 0)
				strncpy(store->par_bsv, &line[4], MIN(len - 5, 15));
			else if (strncmp("PSV=", line, 4) == 0) {
				/* save the processor software version to use
				   in tracking changes to the data format */
				strncpy(store->par_psv, &line[4], MIN(len - 5, 15));
				if (sscanf(store->par_psv, "%d.%d.%d", &i1, &i2, &i3) == 3)
					*version = i3 + 100 * i2 + 10000 * i1;
			}
			else if (strncmp("DDS=", line, 4) == 0)
				strncpy(store->par_dds, &line[4], MIN(len - 5, 15));
			else if (strncmp("OSV=", line, 4) == 0)
				strncpy(store->par_osv, &line[4], MIN(len - 5, 15));
			else if (strncmp("DSV=", line, 4) == 0)
				strncpy(store->par_dsv, &line[4], MIN(len - 5, 15));
			else if (strncmp("DSX=", line, 4) == 0)
				mb_get_double(&(store->par_dsx), &line[4], len - 5);
			else if (strncmp("DSY=", line, 4) == 0)
				mb_get_double(&(store->par_dsy), &line[4], len - 5);
			else if (strncmp("DSZ=", line, 4) == 0)
				mb_get_double(&(store->par_dsz), &line[4], len - 5);

			else if (strncmp("DSD=", line, 4) == 0)
				mb_get_int(&(store->par_dsd), &line[4], len - 5);
			else if (strncmp("DSO=", line, 4) == 0)
				mb_get_double(&(store->par_dso), &line[4], len - 5);
			else if (strncmp("DSF=", line, 4) == 0)
				mb_get_double(&(store->par_dsf), &line[4], len - 5);
			else if (strncmp("DSH=", line, 4) == 0) {
				store->par_dsh[0] = line[4];
				store->par_dsh[1] = line[5];
			}
			else if (strncmp("APS=", line, 4) == 0)
				mb_get_int(&(store->par_aps), &line[4], len - 5);
			else if (strncmp("P1Q=", line, 4) == 0)
				mb_get_int(&(store->par_p1q), &line[4], len - 5);
			else if (strncmp("P1M=", line, 4) == 0)
				mb_get_int(&(store->par_p1m), &line[4], len - 5);
			else if (strncmp("P1T=", line, 4) == 0)
				mb_get_int(&(store->par_p1t), &line[4], len - 5);
			else if (strncmp("P1Z=", line, 4) == 0)
				mb_get_double(&(store->par_p1z), &line[4], len - 5);
			else if (strncmp("P1X=", line, 4) == 0)
				mb_get_double(&(store->par_p1x), &line[4], len - 5);
			else if (strncmp("P1Y=", line, 4) == 0)
				mb_get_double(&(store->par_p1y), &line[4], len - 5);
			else if (strncmp("P1D=", line, 4) == 0)
				mb_get_double(&(store->par_p1d), &line[4], len - 5);
			else if (strncmp("P1G=", line, 4) == 0)
				strncpy(store->par_p1g, &line[4], MIN(len - 5, 15));
			else if (strncmp("P2Q=", line, 4) == 0)
				mb_get_int(&(store->par_p2q), &line[4], len - 5);
			else if (strncmp("P2M=", line, 4) == 0)
				mb_get_int(&(store->par_p2m), &line[4], len - 5);
			else if (strncmp("P2T=", line, 4) == 0)
				mb_get_int(&(store->par_p2t), &line[4], len - 5);
			else if (strncmp("P2Z=", line, 4) == 0)
				mb_get_double(&(store->par_p2z), &line[4], len - 5);
			else if (strncmp("P2X=", line, 4) == 0)
				mb_get_double(&(store->par_p2x), &line[4], len - 5);
			else if (strncmp("P2Y=", line, 4) == 0)
				mb_get_double(&(store->par_p2y), &line[4], len - 5);
			else if (strncmp("P2D=", line, 4) == 0)
				mb_get_double(&(store->par_p2d), &line[4], len - 5);
			else if (strncmp("P2G=", line, 4) == 0)
				strncpy(store->par_p2g, &line[4], MIN(len - 5, 15));
			else if (strncmp("P3Q=", line, 4) == 0)
				mb_get_int(&(store->par_p3q), &line[4], len - 5);
			else if (strncmp("P3M=", line, 4) == 0)
				mb_get_int(&(store->par_p3m), &line[4], len - 5);
			else if (strncmp("P3T=", line, 4) == 0)
				mb_get_int(&(store->par_p3t), &line[4], len - 5);
			else if (strncmp("P3Z=", line, 4) == 0)
				mb_get_double(&(store->par_p3z), &line[4], len - 5);
			else if (strncmp("P3X=", line, 4) == 0)
				mb_get_double(&(store->par_p3x), &line[4], len - 5);
			else if (strncmp("P3Y=", line, 4) == 0)
				mb_get_double(&(store->par_p3y), &line[4], len - 5);
			else if (strncmp("P3D=", line, 4) == 0)
				mb_get_double(&(store->par_p3d), &line[4], len - 5);
			else if (strncmp("P3G=", line, 4) == 0)
				strncpy(store->par_p3g, &line[4], MIN(len - 5, 15));
			else if (strncmp("P3S=", line, 4) == 0)
				mb_get_int(&(store->par_p3s), &line[4], len - 5);

			else if (strncmp("MSZ=", line, 4) == 0)
				mb_get_double(&(store->par_msz), &line[4], len - 5);
			else if (strncmp("MSX=", line, 4) == 0)
				mb_get_double(&(store->par_msx), &line[4], len - 5);
			else if (strncmp("MSY=", line, 4) == 0)
				mb_get_double(&(store->par_msy), &line[4], len - 5);
			else if (strncmp("MRP=", line, 4) == 0) {
				store->par_mrp[0] = line[4];
				store->par_mrp[1] = line[5];
			}
			else if (strncmp("MSD=", line, 4) == 0)
				mb_get_double(&(store->par_msd), &line[4], len - 5);
			else if (strncmp("MSR=", line, 4) == 0)
				mb_get_double(&(store->par_msr), &line[4], len - 5);
			else if (strncmp("MSP=", line, 4) == 0)
				mb_get_double(&(store->par_msp), &line[4], len - 5);
			else if (strncmp("MSG=", line, 4) == 0)
				mb_get_double(&(store->par_msg), &line[4], len - 5);

			else if (strncmp("NSZ=", line, 4) == 0)
				mb_get_double(&(store->par_nsz), &line[4], len - 5);
			else if (strncmp("NSX=", line, 4) == 0)
				mb_get_double(&(store->par_nsx), &line[4], len - 5);
			else if (strncmp("NSY=", line, 4) == 0)
				mb_get_double(&(store->par_nsy), &line[4], len - 5);
			else if (strncmp("NRP=", line, 4) == 0) {
				store->par_nrp[0] = line[4];
				store->par_nrp[1] = line[5];
			}
			else if (strncmp("NSD=", line, 4) == 0)
				mb_get_double(&(store->par_nsd), &line[4], len - 5);
			else if (strncmp("NSR=", line, 4) == 0)
				mb_get_double(&(store->par_nsr), &line[4], len - 5);
			else if (strncmp("NSP=", line, 4) == 0)
				mb_get_double(&(store->par_nsp), &line[4], len - 5);
			else if (strncmp("NSG=", line, 4) == 0)
				mb_get_double(&(store->par_nsg), &line[4], len - 5);

			else if (strncmp("GCG=", line, 4) == 0)
				mb_get_double(&(store->par_gcg), &line[4], len - 5);
			else if (strncmp("MAS=", line, 4) == 0)
				mb_get_double(&(store->par_mas), &line[4], len - 5);
			else if (strncmp("SHC=", line, 4) == 0)
				mb_get_int(&(store->par_shc), &line[4], len - 5);
			else if (strncmp("PPS=", line, 4) == 0)
				mb_get_int(&(store->par_pps), &line[4], len - 5);
			else if (strncmp("CLS=", line, 4) == 0)
				mb_get_int(&(store->par_cls), &line[4], len - 5);
			else if (strncmp("CLO=", line, 4) == 0)
				mb_get_int(&(store->par_clo), &line[4], len - 5);
			else if (strncmp("VSN=", line, 4) == 0)
				mb_get_int(&(store->par_vsn), &line[4], len - 5);
			else if (strncmp("VSU=", line, 4) == 0)
				mb_get_int(&(store->par_vsu), &line[4], len - 5);
			else if (strncmp("VSE=", line, 4) == 0)
				mb_get_int(&(store->par_vse), &line[4], len - 5);
			else if (strncmp("VTU=", line, 4) == 0)
				mb_get_int(&(store->par_vtu), &line[4], len - 5);
			else if (strncmp("VTE=", line, 4) == 0)
				mb_get_int(&(store->par_vte), &line[4], len - 5);
			else if (strncmp("ARO=", line, 4) == 0)
				mb_get_int(&(store->par_aro), &line[4], len - 5);
			else if (strncmp("AHE=", line, 4) == 0)
				mb_get_int(&(store->par_ahe), &line[4], len - 5);
			else if (strncmp("AHS=", line, 4) == 0)
				mb_get_int(&(store->par_ahs), &line[4], len - 5);
			else if (strncmp("VSI=", line, 4) == 0)
				strncpy(store->par_vsi, &line[4], MIN(len - 5, 15));
			else if (strncmp("VSM=", line, 4) == 0)
				strncpy(store->par_vsm, &line[4], MIN(len - 5, 15));

			else if (strncmp("MCA1=", line, 5) == 0)
				strncpy(store->par_mca1, &line[5], MIN(len - 6, 15));
			else if (strncmp("MCU1=", line, 5) == 0)
				mb_get_int(&(store->par_mcu1), &line[5], len - 6);
			else if (strncmp("MCI1=", line, 5) == 0)
				strncpy(store->par_mci1, &line[5], MIN(len - 6, 15));
			else if (strncmp("MCP1=", line, 5) == 0)
				mb_get_int(&(store->par_mcp1), &line[5], len - 6);

			else if (strncmp("MCA2=", line, 5) == 0)
				strncpy(store->par_mca2, &line[5], MIN(len - 6, 15));
			else if (strncmp("MCU2=", line, 5) == 0)
				mb_get_int(&(store->par_mcu2), &line[5], len - 6);
			else if (strncmp("MCI2=", line, 5) == 0)
				strncpy(store->par_mci2, &line[5], MIN(len - 6, 15));
			else if (strncmp("MCP2=", line, 5) == 0)
				mb_get_int(&(store->par_mcp2), &line[5], len - 6);

			else if (strncmp("MCA3=", line, 5) == 0)
				strncpy(store->par_mca3, &line[5], MIN(len - 6, 15));
			else if (strncmp("MCU3=", line, 5) == 0)
				mb_get_int(&(store->par_mcu3), &line[5], len - 6);
			else if (strncmp("MCI3=", line, 5) == 0)
				strncpy(store->par_mci3, &line[5], MIN(len - 6, 15));
			else if (strncmp("MCP3=", line, 5) == 0)
				mb_get_int(&(store->par_mcp3), &line[5], len - 6);

			else if (strncmp("MCA4=", line, 5) == 0)
				strncpy(store->par_mca4, &line[5], MIN(len - 6, 15));
			else if (strncmp("MCU4=", line, 5) == 0)
				mb_get_int(&(store->par_mcu4), &line[5], len - 6);
			else if (strncmp("MCI4=", line, 5) == 0)
				strncpy(store->par_mci4, &line[5], MIN(len - 6, 15));
			else if (strncmp("MCP4=", line, 5) == 0)
				mb_get_int(&(store->par_mcp4), &line[5], len - 6);
			else if (strncmp("SNL=", line, 4) == 0)
				mb_get_int(&(store->par_snl), &line[4], len - 5);

			else if (strncmp("CPR=", line, 4) == 0)
				strncpy(store->par_cpr, &line[4], MIN(len - 5, 3));
			else if (strncmp("ROP=", line, 4) == 0)
				strncpy(store->par_rop, &line[4], MIN(len - 5, MBSYS_SIMRAD3_COMMENT_LENGTH - 1));
			else if (strncmp("SID=", line, 4) == 0)
				strncpy(store->par_sid, &line[4], MIN(len - 5, MBSYS_SIMRAD3_COMMENT_LENGTH - 1));
			else if (strncmp("RFN=", line, 4) == 0)
				strncpy(store->par_rfn, &line[4], MIN(len - 5, MBSYS_SIMRAD3_COMMENT_LENGTH - 1));
			else if (strncmp("PLL=", line, 4) == 0)
				strncpy(store->par_pll, &line[4], MIN(len - 5, MBSYS_SIMRAD3_COMMENT_LENGTH - 1));
			else if (strncmp("COM=", line, 4) == 0) {
				strncpy(store->par_com, &line[4], MIN(len - 5, MBSYS_SIMRAD3_COMMENT_LENGTH - 1));
				store->par_com[MIN(len - 5, MBSYS_SIMRAD3_COMMENT_LENGTH - 1)] = 0;
				/* replace caret (^) values with commas (,) to circumvent
				   the format's inability to store commas in comments */
				while ((comma_ptr = strchr(store->par_com, '^')) != NULL) {
					comma_ptr[0] = ',';
				}
			}
			len = 0;
		}
		else if (status == MB_SUCCESS && line[len - 1] == ',' && len <= 5) {
			len = 0;
		}
	}

	/* if specified from mbpreprocess then reset sensor depth mode */
	if (*sensordepth_only) {
		store->par_dsh[0] = 'I';
		store->par_dsh[1] = 'N';
	}

	/* now set the data kind */
	if (status == MB_SUCCESS) {
		if (store->type == EM3_START && store->par_date == 0)
			store->kind = MB_DATA_COMMENT;
		else if (store->type == EM3_START)
			store->kind = MB_DATA_START;
		else if (store->type == EM3_STOP)
			store->kind = MB_DATA_STOP;
	}

	/* read end of record and last two check sum bytes */
	if (status == MB_SUCCESS) {
		/* if EM3_END not yet found then the
		next byte should be EM3_END */
		if (line[0] != EM3_END) {
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)&line[0], &read_len, error);
		}

		/* if EM3_END not yet found then the
		next byte should be EM3_END */
		if (line[0] != EM3_END) {
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)&line[0], &read_len, error);
		}

		/* if we got the end byte then get check sum bytes */
		if (line[0] == EM3_END) {
			if (line[0] == EM3_END)
				*goodend = true;
			read_len = (size_t)2;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)&line[1], &read_len, error);
/* don't check success of read
	- return success here even if read fails
	because all of the
important information in this record has
already been read - next attempt to read
file will return error */
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[0], line[0], line[1], line[1], line[2],
			        line[2]);
#endif
		}
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "\n");
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       par_date:        %d\n", store->par_date);
		fprintf(stderr, "dbg5       par_msec:        %d\n", store->par_msec);
		fprintf(stderr, "dbg5       par_line_num:    %d\n", store->par_line_num);
		fprintf(stderr, "dbg5       par_serial_1:    %d\n", store->par_serial_1);
		fprintf(stderr, "dbg5       par_serial_2:    %d\n", store->par_serial_2);
		fprintf(stderr, "dbg5       par_wlz:         %f\n", store->par_wlz);
		fprintf(stderr, "dbg5       par_smh:         %d\n", store->par_smh);
		fprintf(stderr, "dbg5       par_hun:         %d\n", store->par_hun);
		fprintf(stderr, "dbg5       par_hut:         %f\n", store->par_hut);
		fprintf(stderr, "dbg5       par_txs:         %d\n", store->par_txs);
		fprintf(stderr, "dbg5       par_t2x:         %d\n", store->par_t2x);
		fprintf(stderr, "dbg5       par_r1s:         %d\n", store->par_r1s);
		fprintf(stderr, "dbg5       par_r2s:         %d\n", store->par_r2s);
		fprintf(stderr, "dbg5       par_stc:         %d\n", store->par_stc);
		fprintf(stderr, "dbg5       par_s0z:         %f\n", store->par_s0z);
		fprintf(stderr, "dbg5       par_s0x:         %f\n", store->par_s0x);
		fprintf(stderr, "dbg5       par_s0y:         %f\n", store->par_s0y);
		fprintf(stderr, "dbg5       par_s0h:         %f\n", store->par_s0h);
		fprintf(stderr, "dbg5       par_s0r:         %f\n", store->par_s0r);
		fprintf(stderr, "dbg5       par_s0p:         %f\n", store->par_s0p);
		fprintf(stderr, "dbg5       par_s1z:         %f\n", store->par_s1z);
		fprintf(stderr, "dbg5       par_s1x:         %f\n", store->par_s1x);
		fprintf(stderr, "dbg5       par_s1y:         %f\n", store->par_s1y);
		fprintf(stderr, "dbg5       par_s1h:         %f\n", store->par_s1h);
		fprintf(stderr, "dbg5       par_s1r:         %f\n", store->par_s1r);
		fprintf(stderr, "dbg5       par_s1p:         %f\n", store->par_s1p);
		fprintf(stderr, "dbg5       par_s1n:         %d\n", store->par_s1n);
		fprintf(stderr, "dbg5       par_s2z:         %f\n", store->par_s2z);
		fprintf(stderr, "dbg5       par_s2x:         %f\n", store->par_s2x);
		fprintf(stderr, "dbg5       par_s2y:         %f\n", store->par_s2y);
		fprintf(stderr, "dbg5       par_s2h:         %f\n", store->par_s2h);
		fprintf(stderr, "dbg5       par_s2r:         %f\n", store->par_s2r);
		fprintf(stderr, "dbg5       par_s2p:         %f\n", store->par_s2p);
		fprintf(stderr, "dbg5       par_s2n:         %d\n", store->par_s2n);
		fprintf(stderr, "dbg5       par_s3z:         %f\n", store->par_s3z);
		fprintf(stderr, "dbg5       par_s3x:         %f\n", store->par_s3x);
		fprintf(stderr, "dbg5       par_s3y:         %f\n", store->par_s3y);
		fprintf(stderr, "dbg5       par_s3h:         %f\n", store->par_s3h);
		fprintf(stderr, "dbg5       par_s3r:         %f\n", store->par_s3r);
		fprintf(stderr, "dbg5       par_s3p:         %f\n", store->par_s3p);
		fprintf(stderr, "dbg5       par_s1s:         %d\n", store->par_s1s);
		fprintf(stderr, "dbg5       par_s2s:         %d\n", store->par_s2s);
		fprintf(stderr, "dbg5       par_go1:         %f\n", store->par_go1);
		fprintf(stderr, "dbg5       par_go2:         %f\n", store->par_go2);
		fprintf(stderr, "dbg5       par_obo:         %f\n", store->par_obo);
		fprintf(stderr, "dbg5       par_fgd:         %f\n", store->par_fgd);
		fprintf(stderr, "dbg5       par_tsv:         %s\n", store->par_tsv);
		fprintf(stderr, "dbg5       par_rsv:         %s\n", store->par_rsv);
		fprintf(stderr, "dbg5       par_bsv:         %s\n", store->par_bsv);
		fprintf(stderr, "dbg5       par_psv:         %s\n", store->par_psv);
		fprintf(stderr, "dbg5       par_dds:         %s\n", store->par_dds);
		fprintf(stderr, "dbg5       par_osv:         %s\n", store->par_osv);
		fprintf(stderr, "dbg5       par_dsv:         %s\n", store->par_dsv);
		fprintf(stderr, "dbg5       par_dsx:         %f\n", store->par_dsx);
		fprintf(stderr, "dbg5       par_dsy:         %f\n", store->par_dsy);
		fprintf(stderr, "dbg5       par_dsz:         %f\n", store->par_dsz);
		fprintf(stderr, "dbg5       par_dsd:         %d\n", store->par_dsd);
		fprintf(stderr, "dbg5       par_dso:         %f\n", store->par_dso);
		fprintf(stderr, "dbg5       par_dsf:         %f\n", store->par_dsf);
		fprintf(stderr, "dbg5       par_dsh:         %c%c\n", store->par_dsh[0], store->par_dsh[1]);
		fprintf(stderr, "dbg5       par_aps:         %d\n", store->par_aps);
		fprintf(stderr, "dbg5       par_p1q:         %d\n", store->par_p1q);
		fprintf(stderr, "dbg5       par_p1m:         %d\n", store->par_p1m);
		fprintf(stderr, "dbg5       par_p1t:         %d\n", store->par_p1t);
		fprintf(stderr, "dbg5       par_p1z:         %f\n", store->par_p1z);
		fprintf(stderr, "dbg5       par_p1x:         %f\n", store->par_p1x);
		fprintf(stderr, "dbg5       par_p1y:         %f\n", store->par_p1y);
		fprintf(stderr, "dbg5       par_p1d:         %f\n", store->par_p1d);
		fprintf(stderr, "dbg5       par_p1g:         %s\n", store->par_p1g);
		fprintf(stderr, "dbg5       par_p2q:         %d\n", store->par_p2q);
		fprintf(stderr, "dbg5       par_p2m:         %d\n", store->par_p2m);
		fprintf(stderr, "dbg5       par_p2t:         %d\n", store->par_p2t);
		fprintf(stderr, "dbg5       par_p2z:         %f\n", store->par_p2z);
		fprintf(stderr, "dbg5       par_p2x:         %f\n", store->par_p2x);
		fprintf(stderr, "dbg5       par_p2y:         %f\n", store->par_p2y);
		fprintf(stderr, "dbg5       par_p2d:         %f\n", store->par_p2d);
		fprintf(stderr, "dbg5       par_p2g:         %s\n", store->par_p2g);
		fprintf(stderr, "dbg5       par_p3q:         %d\n", store->par_p3q);
		fprintf(stderr, "dbg5       par_p3m:         %d\n", store->par_p3m);
		fprintf(stderr, "dbg5       par_p3t:         %d\n", store->par_p3t);
		fprintf(stderr, "dbg5       par_p3z:         %f\n", store->par_p3z);
		fprintf(stderr, "dbg5       par_p3x:         %f\n", store->par_p3x);
		fprintf(stderr, "dbg5       par_p3y:         %f\n", store->par_p3y);
		fprintf(stderr, "dbg5       par_p3d:         %f\n", store->par_p3d);
		fprintf(stderr, "dbg5       par_p3g:         %s\n", store->par_p3g);
		fprintf(stderr, "dbg5       par_p3s:         %d\n", store->par_p3s);
		fprintf(stderr, "dbg5       par_msz:         %f\n", store->par_msz);
		fprintf(stderr, "dbg5       par_msx:         %f\n", store->par_msx);
		fprintf(stderr, "dbg5       par_msy:         %f\n", store->par_msy);
		fprintf(stderr, "dbg5       par_mrp:         %c%c\n", store->par_mrp[0], store->par_mrp[1]);
		fprintf(stderr, "dbg5       par_msd:         %f\n", store->par_msd);
		fprintf(stderr, "dbg5       par_msr:         %f\n", store->par_msr);
		fprintf(stderr, "dbg5       par_msp:         %f\n", store->par_msp);
		fprintf(stderr, "dbg5       par_msg:         %f\n", store->par_msg);
		fprintf(stderr, "dbg5       par_nsz:         %f\n", store->par_nsz);
		fprintf(stderr, "dbg5       par_nsx:         %f\n", store->par_nsx);
		fprintf(stderr, "dbg5       par_nsy:         %f\n", store->par_nsy);
		fprintf(stderr, "dbg5       par_nrp:         %c%c\n", store->par_nrp[0], store->par_nrp[1]);
		fprintf(stderr, "dbg5       par_nsd:         %f\n", store->par_nsd);
		fprintf(stderr, "dbg5       par_nsr:         %f\n", store->par_nsr);
		fprintf(stderr, "dbg5       par_nsp:         %f\n", store->par_nsp);
		fprintf(stderr, "dbg5       par_nsg:         %f\n", store->par_nsg);
		fprintf(stderr, "dbg5       par_gcg:         %f\n", store->par_gcg);
		fprintf(stderr, "dbg5       par_mas:         %f\n", store->par_mas);
		fprintf(stderr, "dbg5       par_shc:         %d\n", store->par_shc);
		fprintf(stderr, "dbg5       par_pps:         %d\n", store->par_pps);
		fprintf(stderr, "dbg5       par_cls:         %d\n", store->par_cls);
		fprintf(stderr, "dbg5       par_clo:         %d\n", store->par_clo);
		fprintf(stderr, "dbg5       par_vsn:         %d\n", store->par_vsn);
		fprintf(stderr, "dbg5       par_vsu:         %d\n", store->par_vsu);
		fprintf(stderr, "dbg5       par_vse:         %d\n", store->par_vse);
		fprintf(stderr, "dbg5       par_vtu:         %d\n", store->par_vtu);
		fprintf(stderr, "dbg5       par_vte:         %d\n", store->par_vte);
		fprintf(stderr, "dbg5       par_aro:         %d\n", store->par_aro);
		fprintf(stderr, "dbg5       par_ahe:         %d\n", store->par_ahe);
		fprintf(stderr, "dbg5       par_ahs:         %d\n", store->par_ahs);
		fprintf(stderr, "dbg5       par_vsi:         %s\n", store->par_vsi);
		fprintf(stderr, "dbg5       par_vsm:         %s\n", store->par_vsm);
		fprintf(stderr, "dbg5       par_mca1:        %s\n", store->par_mca1);
		fprintf(stderr, "dbg5       par_mcu1:        %d\n", store->par_mcu1);
		fprintf(stderr, "dbg5       par_mci1:        %s\n", store->par_mci1);
		fprintf(stderr, "dbg5       par_mcp1:        %d\n", store->par_mcp1);
		fprintf(stderr, "dbg5       par_mca2:        %s\n", store->par_mca2);
		fprintf(stderr, "dbg5       par_mcu2:        %d\n", store->par_mcu2);
		fprintf(stderr, "dbg5       par_mci2:        %s\n", store->par_mci2);
		fprintf(stderr, "dbg5       par_mcp2:        %d\n", store->par_mcp2);
		fprintf(stderr, "dbg5       par_mca3:        %s\n", store->par_mca3);
		fprintf(stderr, "dbg5       par_mcu3:        %d\n", store->par_mcu3);
		fprintf(stderr, "dbg5       par_mci3:        %s\n", store->par_mci3);
		fprintf(stderr, "dbg5       par_mcp3:        %d\n", store->par_mcp3);
		fprintf(stderr, "dbg5       par_mca4:        %s\n", store->par_mca4);
		fprintf(stderr, "dbg5       par_mcu4:        %d\n", store->par_mcu4);
		fprintf(stderr, "dbg5       par_mci4:        %s\n", store->par_mci4);
		fprintf(stderr, "dbg5       par_mcp4:        %d\n", store->par_mcp4);
		fprintf(stderr, "dbg5       par_snl:         %d\n", store->par_snl);
		fprintf(stderr, "dbg5       par_cpr:         %s\n", store->par_cpr);
		fprintf(stderr, "dbg5       par_rop:         %s\n", store->par_rop);
		fprintf(stderr, "dbg5       par_sid:         %s\n", store->par_sid);
		fprintf(stderr, "dbg5       par_rfn:         %s\n", store->par_rfn);
		fprintf(stderr, "dbg5       par_pll:         %s\n", store->par_pll);
		fprintf(stderr, "dbg5       par_com:         %s\n", store->par_com);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       version:    %d\n", *version);
		fprintf(stderr, "dbg2       num_sonars: %d\n", *num_sonars);
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_run_parameter(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar,
                                  int *goodend, int *error) {
	char line[EM3_RUN_PARAMETER_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_RUN_PARAMETER;
	store->type = EM3_RUN_PARAMETER;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = (size_t)(EM3_RUN_PARAMETER_SIZE - 4);
	const int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->run_date);
		if (store->run_date != 0)
			store->date = store->run_date;
		mb_get_binary_int(swap, &line[4], &store->run_msec);
		if (store->run_date != 0)
			store->msec = store->run_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		store->run_ping_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		store->run_serial = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[12], &store->run_status);
		store->run_mode = (mb_u_char)line[16];
		store->run_filter_id = (mb_u_char)line[17];
		mb_get_binary_short(swap, &line[18], &short_val);
		store->run_min_depth = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[20], &short_val);
		store->run_max_depth = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[22], &short_val);
		store->run_absorption = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[24], &short_val);
		store->run_tran_pulse = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[26], &short_val);
		store->run_tran_beam = (int)((unsigned short)short_val);
		store->run_tran_pow = (mb_u_char)line[28];
		store->run_rec_beam = (mb_u_char)line[29];
		store->run_rec_band = (mb_u_char)line[30];
		store->run_rec_gain = (mb_u_char)line[31];
		store->run_tvg_cross = (mb_u_char)line[32];
		store->run_ssv_source = (mb_u_char)line[33];
		mb_get_binary_short(swap, &line[34], &short_val);
		store->run_max_swath = (int)((unsigned short)short_val);
		store->run_beam_space = (mb_u_char)line[36];
		store->run_swath_angle = (mb_u_char)line[37];
		store->run_stab_mode = (mb_u_char)line[38];
		for (int i = 0; i < 6; i++)
			store->run_spare[i] = line[39 + i];
		if (line[EM3_RUN_PARAMETER_SIZE - 7] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM3_RUN_PARAMETER_SIZE - 7],
		        line[EM3_RUN_PARAMETER_SIZE - 7], line[EM3_RUN_PARAMETER_SIZE - 6], line[EM3_RUN_PARAMETER_SIZE - 6],
		        line[EM3_RUN_PARAMETER_SIZE - 5], line[EM3_RUN_PARAMETER_SIZE - 5]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       run_date:        %d\n", store->run_date);
		fprintf(stderr, "dbg5       run_msec:        %d\n", store->run_msec);
		fprintf(stderr, "dbg5       run_ping_count:  %d\n", store->run_ping_count);
		fprintf(stderr, "dbg5       run_serial:      %d\n", store->run_serial);
		fprintf(stderr, "dbg5       run_status:      %d\n", store->run_status);
		fprintf(stderr, "dbg5       run_mode:        %d\n", store->run_mode);
		fprintf(stderr, "dbg5       run_filter_id:   %d\n", store->run_filter_id);
		fprintf(stderr, "dbg5       run_min_depth:   %d\n", store->run_min_depth);
		fprintf(stderr, "dbg5       run_max_depth:   %d\n", store->run_max_depth);
		fprintf(stderr, "dbg5       run_absorption:  %d\n", store->run_absorption);
		fprintf(stderr, "dbg5       run_tran_pulse:  %d\n", store->run_tran_pulse);
		fprintf(stderr, "dbg5       run_tran_beam:   %d\n", store->run_tran_beam);
		fprintf(stderr, "dbg5       run_tran_pow:    %d\n", store->run_tran_pow);
		fprintf(stderr, "dbg5       run_rec_beam:    %d\n", store->run_rec_beam);
		fprintf(stderr, "dbg5       run_rec_band:    %d\n", store->run_rec_band);
		fprintf(stderr, "dbg5       run_rec_gain:    %d\n", store->run_rec_gain);
		fprintf(stderr, "dbg5       run_tvg_cross:   %d\n", store->run_tvg_cross);
		fprintf(stderr, "dbg5       run_ssv_source:  %d\n", store->run_ssv_source);
		fprintf(stderr, "dbg5       run_max_swath:   %d\n", store->run_max_swath);
		fprintf(stderr, "dbg5       run_beam_space:  %d\n", store->run_beam_space);
		fprintf(stderr, "dbg5       run_swath_angle: %d\n", store->run_swath_angle);
		fprintf(stderr, "dbg5       run_stab_mode:   %d\n", store->run_stab_mode);
		for (int i = 0; i < 6; i++)
			fprintf(stderr, "dbg5       run_spare[%d]:    %d\n", i, store->run_spare[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_clock(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                          int *error) {
	char line[EM3_CLOCK_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_CLOCK;
	store->type = EM3_CLOCK;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = (size_t)(EM3_CLOCK_SIZE - 4);
	const int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->clk_date);
		store->date = store->clk_date;
		mb_get_binary_int(swap, &line[4], &store->clk_msec);
		store->msec = store->clk_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		store->clk_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		store->clk_serial = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[12], &store->clk_origin_date);
		mb_get_binary_int(swap, &line[16], &store->clk_origin_msec);
		store->clk_1_pps_use = (mb_u_char)line[20];
		if (line[EM3_CLOCK_SIZE - 7] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM3_CLOCK_SIZE - 7], line[EM3_CLOCK_SIZE - 7],
		        line[EM3_CLOCK_SIZE - 6], line[EM3_CLOCK_SIZE - 6], line[EM3_CLOCK_SIZE - 5], line[EM3_CLOCK_SIZE - 5]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       clk_date:        %d\n", store->clk_date);
		fprintf(stderr, "dbg5       clk_msec:        %d\n", store->clk_msec);
		fprintf(stderr, "dbg5       clk_count:       %d\n", store->clk_count);
		fprintf(stderr, "dbg5       clk_serial:      %d\n", store->clk_serial);
		fprintf(stderr, "dbg5       clk_origin_date: %d\n", store->clk_origin_date);
		fprintf(stderr, "dbg5       clk_origin_msec: %d\n", store->clk_origin_msec);
		fprintf(stderr, "dbg5       clk_1_pps_use:   %d\n", store->clk_1_pps_use);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_tide(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                         int *error) {
	char line[EM3_TIDE_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_TIDE;
	store->type = EM3_TIDE;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = (size_t)(EM3_TIDE_SIZE - 4);
	const int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->tid_date);
		store->date = store->tid_date;
		mb_get_binary_int(swap, &line[4], &store->tid_msec);
		store->msec = store->tid_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		store->tid_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		store->tid_serial = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[12], &store->tid_origin_date);
		mb_get_binary_int(swap, &line[16], &store->tid_origin_msec);
		mb_get_binary_short(swap, &line[20], &short_val);
		store->tid_tide = (int)short_val;
		if (line[EM3_TIDE_SIZE - 7] == 0x03)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM3_TIDE_SIZE - 7], line[EM3_TIDE_SIZE - 7],
		        line[EM3_TIDE_SIZE - 6], line[EM3_TIDE_SIZE - 6], line[EM3_TIDE_SIZE - 5], line[EM3_TIDE_SIZE - 5]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       tid_date:        %d\n", store->tid_date);
		fprintf(stderr, "dbg5       tid_msec:        %d\n", store->tid_msec);
		fprintf(stderr, "dbg5       tid_count:       %d\n", store->tid_count);
		fprintf(stderr, "dbg5       tid_serial:      %d\n", store->tid_serial);
		fprintf(stderr, "dbg5       tid_origin_date: %d\n", store->tid_origin_date);
		fprintf(stderr, "dbg5       tid_origin_msec: %d\n", store->tid_origin_msec);
		fprintf(stderr, "dbg5       tid_tide:        %d\n", store->tid_tide);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_height(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                           int *error) {
	char line[EM3_HEIGHT_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_HEIGHT;
	store->type = EM3_HEIGHT;
	store->sonar = sonar;

	/* read binary values into char array */
	read_len = (size_t)(EM3_HEIGHT_SIZE - 4);
	const int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->hgt_date);
		store->date = store->hgt_date;
		mb_get_binary_int(swap, &line[4], &store->hgt_msec);
		store->msec = store->hgt_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		store->hgt_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		store->hgt_serial = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[12], &store->hgt_height);
		store->hgt_type = (mb_u_char)line[16];
		if (line[EM3_HEIGHT_SIZE - 7] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM3_HEIGHT_SIZE - 7], line[EM3_HEIGHT_SIZE - 7],
		        line[EM3_HEIGHT_SIZE - 6], line[EM3_HEIGHT_SIZE - 6], line[EM3_HEIGHT_SIZE - 5], line[EM3_HEIGHT_SIZE - 5]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       hgt_date:        %d\n", store->hgt_date);
		fprintf(stderr, "dbg5       hgt_msec:        %d\n", store->hgt_msec);
		fprintf(stderr, "dbg5       hgt_count:       %d\n", store->hgt_count);
		fprintf(stderr, "dbg5       hgt_serial:      %d\n", store->hgt_serial);
		fprintf(stderr, "dbg5       hgt_height:      %d\n", store->hgt_height);
		fprintf(stderr, "dbg5       hgt_type:        %d\n", store->hgt_type);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_heading(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                            int *error) {
	struct mbsys_simrad3_heading_struct *heading;
	char line[EM3_HEADING_HEADER_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	heading = (struct mbsys_simrad3_heading_struct *)store->heading;

	/* set kind and type values */
	store->kind = MB_DATA_HEADING;
	store->type = EM3_HEADING;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)(EM3_HEADING_HEADER_SIZE);
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &heading->hed_date);
		store->date = heading->hed_date;
		mb_get_binary_int(swap, &line[4], &heading->hed_msec);
		store->msec = heading->hed_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		heading->hed_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		heading->hed_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		heading->hed_ndata = (int)((unsigned short)short_val);
	}

	/* read binary heading values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < heading->hed_ndata && status == MB_SUCCESS; i++) {
			read_len = (size_t)(EM3_HEADING_SLICE_SIZE);
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXHEADING) {
				mb_get_binary_short(swap, &line[0], &short_val);
				heading->hed_time[i] = (int)((unsigned short)short_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				heading->hed_heading[i] = (int)((unsigned short)short_val);
			}
		}
		heading->hed_ndata = MIN(heading->hed_ndata, MBSYS_SIMRAD3_MAXHEADING);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (status == MB_SUCCESS) {
			heading->hed_heading_status = (mb_u_char)line[0];
		}
		else {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       hed_date:        %d\n", heading->hed_date);
		fprintf(stderr, "dbg5       hed_msec:        %d\n", heading->hed_msec);
		fprintf(stderr, "dbg5       hed_count:       %d\n", heading->hed_count);
		fprintf(stderr, "dbg5       hed_serial:      %d\n", heading->hed_serial);
		fprintf(stderr, "dbg5       hed_ndata:       %d\n", heading->hed_ndata);
		fprintf(stderr, "dbg5       count    time (msec)    heading (0.01 deg)\n");
		fprintf(stderr, "dbg5       -----    -----------    ------------------\n");
		for (int i = 0; i < heading->hed_ndata; i++)
			fprintf(stderr, "dbg5        %4d      %7d          %7d\n", i, heading->hed_time[i], heading->hed_heading[i]);
		fprintf(stderr, "dbg5       hed_heading_status: %d\n", heading->hed_heading_status);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_ssv(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                        int *error) {
	struct mbsys_simrad3_ssv_struct *ssv;
	char line[EM3_SSV_HEADER_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	ssv = (struct mbsys_simrad3_ssv_struct *)store->ssv;

	/* set kind and type values */
	store->kind = MB_DATA_SSV;
	store->type = EM3_SSV;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_SSV_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ssv->ssv_date);
		store->date = ssv->ssv_date;
		mb_get_binary_int(swap, &line[4], &ssv->ssv_msec);
		store->msec = ssv->ssv_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		ssv->ssv_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		ssv->ssv_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		ssv->ssv_ndata = (int)((unsigned short)short_val);
	}

	/* read binary ssv values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ssv->ssv_ndata && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_SSV_SLICE_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXSSV) {
				mb_get_binary_short(swap, &line[0], &short_val);
				ssv->ssv_time[i] = (int)((unsigned short)short_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				ssv->ssv_ssv[i] = (int)((unsigned short)short_val);
			}
		}
		ssv->ssv_ndata = MIN(ssv->ssv_ndata, MBSYS_SIMRAD3_MAXSSV);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (status != MB_SUCCESS) {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       ssv_date:        %d\n", ssv->ssv_date);
		fprintf(stderr, "dbg5       ssv_msec:        %d\n", ssv->ssv_msec);
		fprintf(stderr, "dbg5       ssv_count:       %d\n", ssv->ssv_count);
		fprintf(stderr, "dbg5       ssv_serial:      %d\n", ssv->ssv_serial);
		fprintf(stderr, "dbg5       ssv_ndata:       %d\n", ssv->ssv_ndata);
		fprintf(stderr, "dbg5       count    time (msec)    ssv (0.1 m/s)\n");
		fprintf(stderr, "dbg5       -----    -----------    ------------------\n");
		for (int i = 0; i < ssv->ssv_ndata; i++)
			fprintf(stderr, "dbg5        %4d      %7d          %7d\n", i, ssv->ssv_time[i], ssv->ssv_ssv[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_tilt(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                         int *error) {
	struct mbsys_simrad3_tilt_struct *tilt;
	char line[EM3_TILT_HEADER_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	tilt = (struct mbsys_simrad3_tilt_struct *)store->tilt;

	/* set kind and type values */
	store->kind = MB_DATA_TILT;
	store->type = EM3_TILT;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_TILT_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &tilt->tlt_date);
		store->date = tilt->tlt_date;
		mb_get_binary_int(swap, &line[4], &tilt->tlt_msec);
		store->msec = tilt->tlt_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		tilt->tlt_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		tilt->tlt_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		tilt->tlt_ndata = (int)((unsigned short)short_val);
	}

	/* read binary tilt values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < tilt->tlt_ndata && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_TILT_SLICE_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXTILT) {
				mb_get_binary_short(swap, &line[0], &short_val);
				tilt->tlt_time[i] = (int)((unsigned short)short_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				tilt->tlt_tilt[i] = (int)((unsigned short)short_val);
			}
		}
		tilt->tlt_ndata = MIN(tilt->tlt_ndata, MBSYS_SIMRAD3_MAXTILT);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (status != MB_SUCCESS) {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       tlt_date:        %d\n", tilt->tlt_date);
		fprintf(stderr, "dbg5       tlt_msec:        %d\n", tilt->tlt_msec);
		fprintf(stderr, "dbg5       tlt_count:       %d\n", tilt->tlt_count);
		fprintf(stderr, "dbg5       tlt_serial:      %d\n", tilt->tlt_serial);
		fprintf(stderr, "dbg5       tlt_ndata:       %d\n", tilt->tlt_ndata);
		fprintf(stderr, "dbg5       count    time (msec)    tilt (0.01 deg)\n");
		fprintf(stderr, "dbg5       -----    -----------    ------------------\n");
		for (int i = 0; i < tilt->tlt_ndata; i++)
			fprintf(stderr, "dbg5        %4d      %7d          %7d\n", i, tilt->tlt_time[i], tilt->tlt_tilt[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_extraparameters(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar,
                                    int *goodend, int *error) {
	struct mbsys_simrad3_extraparameters_struct *extraparameters;
	char line[EM3_EXTRAPARAMETERS_HEADER_SIZE];
	short short_val = 0;
	size_t read_len;
	int *record_size_save;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get  storage structure */
	extraparameters = (struct mbsys_simrad3_extraparameters_struct *)store->extraparameters;

	/* set kind and type values */
	store->kind = MB_DATA_PARAMETER;
	store->type = EM3_EXTRAPARAMETERS;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_EXTRAPARAMETERS_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &extraparameters->xtr_date);
		store->date = extraparameters->xtr_date;
		mb_get_binary_int(swap, &line[4], &extraparameters->xtr_msec);
		store->msec = extraparameters->xtr_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		extraparameters->xtr_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		extraparameters->xtr_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		extraparameters->xtr_id = (int)((unsigned short)short_val);
	}

	/* calculate length of data array */
	if (status == MB_SUCCESS) {
		record_size_save = (int *)&mb_io_ptr->save2;
		extraparameters->xtr_data_size = *record_size_save - 22;
	}

	/* allocate memory if necessary */
	if (status == MB_SUCCESS && extraparameters->xtr_data_size > extraparameters->xtr_nalloc) {
		status =
		    mb_reallocd(verbose, __FILE__, __LINE__, extraparameters->xtr_data_size, (void **)&extraparameters->xtr_data, error);
		if (status == MB_SUCCESS)
			extraparameters->xtr_nalloc = extraparameters->xtr_data_size;
		else
			extraparameters->xtr_nalloc = 0;
	}

	/* read data */
	if (status == MB_SUCCESS) {
		read_len = (size_t)extraparameters->xtr_data_size;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)extraparameters->xtr_data, &read_len, error);
	}

	/* parse data if possible */
	if (status == MB_SUCCESS && extraparameters->xtr_id == 2) {
		index = 0;
		mb_get_binary_int(swap, &(extraparameters->xtr_data[index]), &extraparameters->xtr_pqf_activepositioning);
		for (int i = 0; i < 3; i++) {
			mb_get_binary_short(swap, &(extraparameters->xtr_data[index]), &extraparameters->xtr_pqf_qfsetting[i]);
			index += 2;
		}
		for (int i = 0; i < 3; i++) {
			mb_get_binary_int(swap, &(extraparameters->xtr_data[index]), &extraparameters->xtr_pqf_nqualityfactors[i]);
			index += 4;
		}
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < extraparameters->xtr_pqf_nqualityfactors[i]; j++) {
				mb_get_binary_int(swap, &(extraparameters->xtr_data[index]), &extraparameters->xtr_pqf_qfvalues[i][j]);
				index += 4;
				mb_get_binary_int(swap, &(extraparameters->xtr_data[index]), &extraparameters->xtr_pqf_qflimits[i][j]);
				index += 4;
			}
		}
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (status != MB_SUCCESS) {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       xtr_date:        %d\n", extraparameters->xtr_date);
		fprintf(stderr, "dbg5       xtr_msec:        %d\n", extraparameters->xtr_msec);
		fprintf(stderr, "dbg5       xtr_count:       %d\n", extraparameters->xtr_count);
		fprintf(stderr, "dbg5       xtr_serial:      %d\n", extraparameters->xtr_serial);
		fprintf(stderr, "dbg5       xtr_id:          %d\n", extraparameters->xtr_id);
		fprintf(stderr, "dbg5       xtr_data_size:   %d\n", extraparameters->xtr_data_size);
		fprintf(stderr, "dbg5       xtr_nalloc:      %d\n", extraparameters->xtr_nalloc);
		if (extraparameters->xtr_id == 2) {
			fprintf(stderr, "dbg5       xtr_pqf_activepositioning:          %d\n", extraparameters->xtr_pqf_activepositioning);
			for (int i = 0; i < 3; i++) {
				fprintf(stderr, "dbg5       positioning system:%d qfsetting:%d nqf:%d\n", i,
				        extraparameters->xtr_pqf_qfsetting[i], extraparameters->xtr_pqf_nqualityfactors[i]);
				for (int j = 0; j < extraparameters->xtr_pqf_nqualityfactors[i]; j++)
					fprintf(stderr, "dbg5       quality factor:%d value:%d limit:%d\n", j,
					        extraparameters->xtr_pqf_qfvalues[i][j], extraparameters->xtr_pqf_qflimits[i][j]);
			}
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_attitude(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                             int *error) {
	struct mbsys_simrad3_attitude_struct *attitude;
	char line[EM3_ATTITUDE_HEADER_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	attitude = (struct mbsys_simrad3_attitude_struct *)store->attitude;

	/* set type values
	    - kind has to wait for the sensor descriptor value at the end of the record */
	store->type = EM3_ATTITUDE;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_ATTITUDE_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &attitude->att_date);
		store->date = attitude->att_date;
		mb_get_binary_int(swap, &line[4], &attitude->att_msec);
		store->msec = attitude->att_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		attitude->att_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		attitude->att_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		attitude->att_ndata = (int)((unsigned short)short_val);
	}

	/* read binary attitude values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < attitude->att_ndata && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_ATTITUDE_SLICE_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXATTITUDE) {
				mb_get_binary_short(swap, &line[0], &short_val);
				attitude->att_time[i] = (int)((unsigned short)short_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				attitude->att_sensor_status[i] = (int)((unsigned short)short_val);
				mb_get_binary_short(swap, &line[4], &short_val);
				attitude->att_roll[i] = (int)short_val;
				mb_get_binary_short(swap, &line[6], &short_val);
				attitude->att_pitch[i] = (int)short_val;
				mb_get_binary_short(swap, &line[8], &short_val);
				attitude->att_heave[i] = (int)short_val;
				mb_get_binary_short(swap, &line[10], &short_val);
				attitude->att_heading[i] = (int)((unsigned short)short_val);
			}
		}
		attitude->att_ndata = MIN(attitude->att_ndata, MBSYS_SIMRAD3_MAXATTITUDE);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (status == MB_SUCCESS) {
			attitude->att_sensordescriptor = (mb_u_char)line[0];
		}
		else {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	/* Set data kind */
	if (status == MB_SUCCESS) {
		/* set data kind */
		if ((attitude->att_sensordescriptor & 48) == 0)
			store->kind = MB_DATA_ATTITUDE;
		else if ((attitude->att_sensordescriptor & 48) == 16)
			store->kind = MB_DATA_ATTITUDE1;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:            %d\n", store->kind);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       att_date:        %d\n", attitude->att_date);
		fprintf(stderr, "dbg5       att_msec:        %d\n", attitude->att_msec);
		fprintf(stderr, "dbg5       att_count:       %d\n", attitude->att_count);
		fprintf(stderr, "dbg5       att_serial:      %d\n", attitude->att_serial);
		fprintf(stderr, "dbg5       att_ndata:       %d\n", attitude->att_ndata);
		fprintf(stderr, "dbg5       cnt   time   roll pitch heave heading\n");
		fprintf(stderr, "dbg5       -------------------------------------\n");
		for (int i = 0; i < attitude->att_ndata; i++)
			fprintf(stderr, "dbg5        %3d  %d  %d %d %d %d\n", i, attitude->att_time[i], attitude->att_roll[i],
			        attitude->att_pitch[i], attitude->att_heave[i], attitude->att_heading[i]);
		fprintf(stderr, "dbg5       att_sensordescriptor: %d\n", attitude->att_sensordescriptor);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_netattitude(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar,
                                int *goodend, int *error) {
	struct mbsys_simrad3_netattitude_struct *netattitude;
	char line[MBSYS_SIMRAD3_BUFFER_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	netattitude = (struct mbsys_simrad3_netattitude_struct *)store->netattitude;

	/* set type values */
	store->type = EM3_NETATTITUDE;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_NETATTITUDE_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &netattitude->nat_date);
		store->date = netattitude->nat_date;
		mb_get_binary_int(swap, &line[4], &netattitude->nat_msec);
		store->msec = netattitude->nat_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		netattitude->nat_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		netattitude->nat_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		netattitude->nat_ndata = (int)((unsigned short)short_val);
		netattitude->nat_sensordescriptor = line[14];
	}

	/* Set data kind */
	if (status == MB_SUCCESS) {
		/* set data kind */
		store->kind = MB_DATA_ATTITUDE2;
	}

	/* read binary netattitude values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < netattitude->nat_ndata && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_NETATTITUDE_SLICE_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXATTITUDE) {
				mb_get_binary_short(swap, &line[0], &short_val);
				netattitude->nat_time[i] = (int)((unsigned short)short_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				netattitude->nat_roll[i] = (int)short_val;
				mb_get_binary_short(swap, &line[4], &short_val);
				netattitude->nat_pitch[i] = (int)short_val;
				mb_get_binary_short(swap, &line[6], &short_val);
				netattitude->nat_heave[i] = (int)short_val;
				mb_get_binary_short(swap, &line[8], &short_val);
				netattitude->nat_heading[i] = (int)((unsigned short)short_val);
				netattitude->nat_nbyte_raw[i] = (mb_u_char)line[10];
				if (netattitude->nat_nbyte_raw[i] <= MBSYS_SIMRAD3_BUFFER_SIZE) {
					read_len = (size_t)(netattitude->nat_nbyte_raw[i]);
					status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
					if (status == MB_SUCCESS) {
						for (int j = 0; j < netattitude->nat_nbyte_raw[i]; j++)
							netattitude->nat_raw[i * MBSYS_SIMRAD3_BUFFER_SIZE + j] = line[j];
					}
				}
				else {
					for (int j = 0; j < netattitude->nat_nbyte_raw[i]; j++) {
						read_len = (size_t)1;
						status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
					}
					netattitude->nat_nbyte_raw[i] = 0;
				}
			}
		}
		netattitude->nat_ndata = MIN(netattitude->nat_ndata, MBSYS_SIMRAD3_MAXATTITUDE);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)1;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (line[0] != EM3_END) {
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)&line[1], &read_len, error);
		}
		else
			line[1] = EM3_END;
		read_len = (size_t)2;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)&line[2], &read_len, error);
		if (status != MB_SUCCESS) {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:                 %d\n", store->kind);
		fprintf(stderr, "dbg5       type:                 %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:                %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                 %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                 %d\n", store->msec);
		fprintf(stderr, "dbg5       nat_date:             %d\n", netattitude->nat_date);
		fprintf(stderr, "dbg5       nat_msec:             %d\n", netattitude->nat_msec);
		fprintf(stderr, "dbg5       nat_count:            %d\n", netattitude->nat_count);
		fprintf(stderr, "dbg5       nat_serial:           %d\n", netattitude->nat_serial);
		fprintf(stderr, "dbg5       nat_ndata:            %d\n", netattitude->nat_ndata);
		fprintf(stderr, "dbg5       nat_sensordescriptor: %d\n", netattitude->nat_sensordescriptor);
		fprintf(stderr, "dbg5       cnt   time   roll pitch heave heading nraw\n");
		fprintf(stderr, "dbg5       -------------------------------------\n");
		for (int i = 0; i < netattitude->nat_ndata; i++) {
			fprintf(stderr, "dbg5        %3d  %d  %d %d %d %d %d\n", i, netattitude->nat_time[i], netattitude->nat_roll[i],
			        netattitude->nat_pitch[i], netattitude->nat_heave[i], netattitude->nat_heading[i],
			        netattitude->nat_nbyte_raw[i]);
			fprintf(stderr, "dbg5        nat_raw[%d]: ", netattitude->nat_nbyte_raw[i]);
			for (int j = 0; j < netattitude->nat_nbyte_raw[i]; j++)
				fprintf(stderr, "%x", netattitude->nat_raw[i * MBSYS_SIMRAD3_BUFFER_SIZE + j]);
			fprintf(stderr, "\n");
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_pos(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                        int *error) {
	char line[MBSYS_SIMRAD3_COMMENT_LENGTH];
	short short_val = 0;
	size_t read_len;
	int navchannel;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_NAV;
	store->type = EM3_POS;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_POS_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->pos_date);
		store->date = store->pos_date;
		mb_get_binary_int(swap, &line[4], &store->pos_msec);
		store->msec = store->pos_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		store->pos_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		store->pos_serial = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[12], &store->pos_latitude);
		mb_get_binary_int(swap, &line[16], &store->pos_longitude);
		mb_get_binary_short(swap, &line[20], &short_val);
		store->pos_quality = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[22], &short_val);
		store->pos_speed = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[24], &short_val);
		store->pos_course = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[26], &short_val);
		store->pos_heading = (int)((unsigned short)short_val);
		store->pos_system = (mb_u_char)line[28];
		store->pos_input_size = (mb_u_char)line[29];
	}

	/* read input position string */
	if (status == MB_SUCCESS && store->pos_input_size < 256) {
		read_len = (size_t)store->pos_input_size;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)store->pos_input, &read_len, error);
		if (status == MB_SUCCESS) {
			store->pos_input[store->pos_input_size] = '\0';
		}
	}

	/* now loop over reading individual characters to
	    get last bytes of record */
	if (status == MB_SUCCESS) {
		bool done = false;
		while (!done) {
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && line[0] == EM3_END) {
				done = true;

				/* get last two check sum bytes */
				read_len = (size_t)2;
				status = mb_fileio_get(verbose, mbio_ptr, (char *)&line[1], &read_len, error);

				*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
				fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[0], line[0], line[1], line[1], line[2],
				        line[2]);
#endif
			}
			else if (status != MB_SUCCESS) {
				done = true;
				/* return success here because all of the
				    important information in this record has
				    already been read - next attempt to read
				    file will return error */
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
		}
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "\n");
#endif
	}

	/* check for navigation source */
	if (status == MB_SUCCESS) {
		/* "active" nav system has first bit set in store->pos_system */
		if (store->pos_system & 128) {
			store->kind = MB_DATA_NAV;
		}

		/* otherwise its from a secondary nav system */
		else {
			navchannel = (store->pos_system & 0x03);
			if (navchannel == 1) {
				store->kind = MB_DATA_NAV1;
			}
			else if (navchannel == 2) {
				store->kind = MB_DATA_NAV2;
			}
			else if (navchannel == 3) {
				store->kind = MB_DATA_NAV3;
			}

			/* otherwise its an error */
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       pos_date:        %d\n", store->pos_date);
		fprintf(stderr, "dbg5       pos_msec:        %d\n", store->pos_msec);
		fprintf(stderr, "dbg5       pos_count:       %d\n", store->pos_count);
		fprintf(stderr, "dbg5       pos_serial:      %d\n", store->pos_serial);
		fprintf(stderr, "dbg5       pos_latitude:    %d\n", store->pos_latitude);
		fprintf(stderr, "dbg5       pos_longitude:   %d\n", store->pos_longitude);
		fprintf(stderr, "dbg5       pos_quality:     %d\n", store->pos_quality);
		fprintf(stderr, "dbg5       pos_speed:       %d\n", store->pos_speed);
		fprintf(stderr, "dbg5       pos_course:      %d\n", store->pos_course);
		fprintf(stderr, "dbg5       pos_heading:     %d\n", store->pos_heading);
		fprintf(stderr, "dbg5       pos_system:      %d\n", store->pos_system);
		fprintf(stderr, "dbg5       pos_input_size:  %d\n", store->pos_input_size);
		fprintf(stderr, "dbg5       pos_input:\ndbg5            %s\n", store->pos_input);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_svp(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                        int *error) {
	char line[EM3_SVP_HEADER_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_VELOCITY_PROFILE;
	store->type = EM3_SVP;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_SVP_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->svp_use_date);
		store->date = store->svp_use_date;
		mb_get_binary_int(swap, &line[4], &store->svp_use_msec);
		store->msec = store->svp_use_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		store->svp_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		store->svp_serial = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[12], &store->svp_origin_date);
		mb_get_binary_int(swap, &line[16], &store->svp_origin_msec);
		mb_get_binary_short(swap, &line[20], &short_val);
		store->svp_num = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[22], &short_val);
		store->svp_depth_res = (int)((unsigned short)short_val);
	}

	/* read binary svp values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < store->svp_num && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_SVP_SLICE_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXSVP) {
				mb_get_binary_short(swap, &line[0], &short_val);
				store->svp_depth[i] = (int)((unsigned short)short_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				store->svp_vel[i] = (int)((unsigned short)short_val);
			}
		}
		store->svp_num = MIN(store->svp_num, MBSYS_SIMRAD3_MAXSVP);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (status != MB_SUCCESS) {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       svp_use_date:    %d\n", store->svp_use_date);
		fprintf(stderr, "dbg5       svp_use_msec:    %d\n", store->svp_use_msec);
		fprintf(stderr, "dbg5       svp_count:       %d\n", store->svp_count);
		fprintf(stderr, "dbg5       svp_serial:      %d\n", store->svp_serial);
		fprintf(stderr, "dbg5       svp_origin_date: %d\n", store->svp_origin_date);
		fprintf(stderr, "dbg5       svp_origin_msec: %d\n", store->svp_origin_msec);
		fprintf(stderr, "dbg5       svp_num:         %d\n", store->svp_num);
		fprintf(stderr, "dbg5       svp_depth_res:   %d\n", store->svp_depth_res);
		fprintf(stderr, "dbg5       count    depth    speed\n");
		fprintf(stderr, "dbg5       -----------------------\n");
		for (int i = 0; i < store->svp_num; i++)
			fprintf(stderr, "dbg5        %d   %d  %d\n", i, store->svp_depth[i], store->svp_vel[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_svp2(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                         int *error) {
	char line[EM3_SVP2_HEADER_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_VELOCITY_PROFILE;
	store->type = EM3_SVP2;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_SVP2_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->svp_use_date);
		store->date = store->svp_use_date;
		mb_get_binary_int(swap, &line[4], &store->svp_use_msec);
		store->msec = store->svp_use_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		store->svp_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		store->svp_serial = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[12], &store->svp_origin_date);
		mb_get_binary_int(swap, &line[16], &store->svp_origin_msec);
		mb_get_binary_short(swap, &line[20], &short_val);
		store->svp_num = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[22], &short_val);
		store->svp_depth_res = (int)((unsigned short)short_val);
	}

	/* read binary svp values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < store->svp_num && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_SVP2_SLICE_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXSVP) {
				mb_get_binary_int(swap, &line[0], &store->svp_depth[i]);
				mb_get_binary_int(swap, &line[4], &store->svp_vel[i]);
			}
		}
		store->svp_num = MIN(store->svp_num, MBSYS_SIMRAD3_MAXSVP);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (status != MB_SUCCESS) {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       svp_use_date:    %d\n", store->svp_use_date);
		fprintf(stderr, "dbg5       svp_use_msec:    %d\n", store->svp_use_msec);
		fprintf(stderr, "dbg5       svp_count:       %d\n", store->svp_count);
		fprintf(stderr, "dbg5       svp_serial:      %d\n", store->svp_serial);
		fprintf(stderr, "dbg5       svp_origin_date: %d\n", store->svp_origin_date);
		fprintf(stderr, "dbg5       svp_origin_msec: %d\n", store->svp_origin_msec);
		fprintf(stderr, "dbg5       svp_num:         %d\n", store->svp_num);
		fprintf(stderr, "dbg5       svp_depth_res:   %d\n", store->svp_depth_res);
		fprintf(stderr, "dbg5       count    depth    speed\n");
		fprintf(stderr, "dbg5       -----------------------\n");
		for (int i = 0; i < store->svp_num; i++)
			fprintf(stderr, "dbg5        %d   %d  %d\n", i, store->svp_depth[i], store->svp_vel[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_bath2(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int version,
                          int *goodend, int *error) {
	struct mbsys_simrad3_ping_struct *ping;
	char line[EM3_BATH2_HEADER_SIZE];
	short short_val = 0;
	float float_val = 0.0;
	int int_val = 0;
	size_t read_len;
	int png_count;
	int serial;
	int oldest_ping;
	int oldest_ping_index;
	int found;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
		fprintf(stderr, "dbg2       version:    %d\n", version);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_DATA;
	store->type = EM3_BATH;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_BATH2_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* figure out which storage structure to use */
	mb_get_binary_short(swap, &line[8], &short_val);
	png_count = (int)((unsigned short)short_val);
	mb_get_binary_short(swap, &line[10], &short_val);
	serial = (int)((unsigned short)short_val);
	found = false;
	oldest_ping = 999999999;
	oldest_ping_index = -1;
	for (int i = 0; i < MBSYS_SIMRAD3_NUM_PING_STRUCTURES && !found; i++) {
		/* look for this ping by ping number and sonar serial number - if we already read
		 * a record from this ping it has to be stored in one of the structures */
		if (store->pings[i].read_status > 0 && png_count == store->pings[i].count && serial == store->pings[i].serial) {
			found = true;
			store->ping_index = i;
		}

		/* figure out which structure is holding the oldest ping in case we need to drop one
		 * from memory to make room for the new ping. */
		else if (store->pings[i].read_status > 0) {
			if (png_count < oldest_ping) {
				oldest_ping = png_count;
				oldest_ping_index = i;
			}
		}

		/* if one of the ping structures is unused, set it to be used if we need to store
		 * this ping */
		else if (oldest_ping > 0) {
			oldest_ping = 0;
			oldest_ping_index = i;
		}
	}
	if (!found) {
		store->ping_index = oldest_ping_index;
	}
	ping = (struct mbsys_simrad3_ping_struct *)&store->pings[store->ping_index];
	ping->count = png_count;
	ping->serial = serial;

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ping->png_date);
		store->date = ping->png_date;
		mb_get_binary_int(swap, &line[4], &ping->png_msec);
		store->msec = ping->png_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		ping->png_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		ping->png_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		ping->png_heading = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[14], &short_val);
		ping->png_ssv = (int)((unsigned short)short_val);
		mb_get_binary_float(swap, &line[16], &float_val);
		ping->png_xducer_depth = float_val;
		mb_get_binary_short(swap, &line[20], &short_val);
		ping->png_nbeams = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[22], &short_val);
		ping->png_nbeams_valid = (int)((unsigned short)short_val);
		mb_get_binary_float(swap, &line[24], &float_val);
		ping->png_sample_rate = float_val;
		mb_get_binary_int(swap, &line[28], &int_val);
		ping->png_spare = int_val;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(
		    stderr,
		    "mbr_em710raw_rd_bath2:    ping->png_date:%d     ping->png_msec:%d     ping->png_count:%d     ping->png_nbeams:%d\n",
		    ping->png_date, ping->png_msec, ping->png_count, ping->png_nbeams);
#endif
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_nbeams_valid > ping->png_nbeams || ping->png_nbeams < 0 || ping->png_nbeams_valid < 0 ||
		    ping->png_nbeams > MBSYS_SIMRAD3_MAXBEAMS || ping->png_nbeams_valid > MBSYS_SIMRAD3_MAXBEAMS) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr,
			        "mbr_em710raw_rd_bath2: ERROR SET: ping->png_nbeams:%d ping->png_nbeams_valid:%d MBSYS_SIMRAD3_MAXBEAMS:%d\n",
			        ping->png_nbeams, ping->png_nbeams_valid, MBSYS_SIMRAD3_MAXBEAMS);
#endif
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_nbeams && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_BATH2_BEAM_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXBEAMS) {
				mb_get_binary_float(swap, &line[0], &float_val);
				ping->png_depth[i] = float_val;
				mb_get_binary_float(swap, &line[4], &float_val);
				ping->png_acrosstrack[i] = float_val;
				mb_get_binary_float(swap, &line[8], &float_val);
				ping->png_alongtrack[i] = float_val;
				mb_get_binary_short(swap, &line[12], &short_val);
				ping->png_window[i] = (int)((unsigned short)short_val);
				ping->png_quality[i] = (int)((mb_u_char)line[14]);
				ping->png_iba[i] = (int)((mb_s_char)line[15]);
				ping->png_detection[i] = (int)((mb_u_char)line[16]);
				ping->png_clean[i] = (int)((mb_s_char)line[17]);
				mb_get_binary_short(swap, &line[18], &short_val);
				ping->png_amp[i] = (int)short_val;
			}
		}
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:                  %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:                 %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                  %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                  %d\n", store->msec);
		fprintf(stderr, "dbg5       png_date:              %d\n", ping->png_date);
		fprintf(stderr, "dbg5       png_msec:              %d\n", ping->png_msec);
		fprintf(stderr, "dbg5       png_count:             %d\n", ping->png_count);
		fprintf(stderr, "dbg5       png_serial:            %d\n", ping->png_serial);
		fprintf(stderr, "dbg5       png_heading:           %d\n", ping->png_heading);
		fprintf(stderr, "dbg5       png_ssv:               %d\n", ping->png_ssv);
		fprintf(stderr, "dbg5       png_xducer_depth:      %f\n", ping->png_xducer_depth);
		fprintf(stderr, "dbg5       png_nbeams:            %d\n", ping->png_nbeams);
		fprintf(stderr, "dbg5       png_nbeams_valid:      %d\n", ping->png_nbeams_valid);
		fprintf(stderr, "dbg5       png_sample_rate:       %f\n", ping->png_sample_rate);
		fprintf(stderr, "dbg5       png_spare:             %d\n", ping->png_spare);
		fprintf(stderr, "dbg5       cnt  depth xtrack ltrack win  qual  iba det cln amp\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %7.2f %7.2f %7.2f %5d %5d %5d %4d %3d %3d\n", i, ping->png_depth[i],
			        ping->png_acrosstrack[i], ping->png_alongtrack[i], ping->png_window[i], ping->png_quality[i],
			        ping->png_iba[i], ping->png_detection[i], ping->png_clean[i], ping->png_amp[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_rawbeam4(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                             int *error) {
	struct mbsys_simrad3_ping_struct *ping;
	char line[EM3_RAWBEAM4_HEADER_SIZE];
	short short_val = 0;
	int int_val = 0;
	float float_val = 0.0;
	size_t read_len;
	int png_count;
	int serial;
	int oldest_ping;
	int oldest_ping_index;
	int found;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* read binary header values into char array */
	read_len = (size_t)EM3_RAWBEAM4_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* figure out which storage structure to use */
	mb_get_binary_short(swap, &line[8], &short_val);
	png_count = (int)((unsigned short)short_val);
	mb_get_binary_short(swap, &line[10], &short_val);
	serial = (int)((unsigned short)short_val);
	found = false;
	oldest_ping = 999999999;
	oldest_ping_index = -1;
	for (int i = 0; i < MBSYS_SIMRAD3_NUM_PING_STRUCTURES && !found; i++) {
		/* look for this ping by ping number and sonar serial number - if we already read
		 * a record from this ping it has to be stored in one of the structures */
		if (store->pings[i].read_status > 0 && png_count == store->pings[i].count && serial == store->pings[i].serial) {
			found = true;
			store->ping_index = i;
		}

		/* figure out which structure is holding the oldest ping in case we need to drop one
		 * from memory to make room for the new ping. */
		else if (store->pings[i].read_status > 0) {
			if (png_count < oldest_ping) {
				oldest_ping = png_count;
				oldest_ping_index = i;
			}
		}

		/* if one of the ping structures is unused, set it to be used if we need to store
		 * this ping */
		else if (oldest_ping > 0) {
			oldest_ping = 0;
			oldest_ping_index = i;
		}
	}
	if (!found) {
		store->ping_index = oldest_ping_index;
	}
	ping = (struct mbsys_simrad3_ping_struct *)&store->pings[store->ping_index];
	ping->count = png_count;
	ping->serial = serial;

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ping->png_raw_date);
		store->date = ping->png_raw_date;
		mb_get_binary_int(swap, &line[4], &ping->png_raw_msec);
		store->msec = ping->png_raw_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		ping->png_raw_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		ping->png_raw_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		ping->png_raw_ssv = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[14], &short_val);
		ping->png_raw_ntx = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[16], &short_val);
		ping->png_raw_nbeams = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[18], &short_val);
		ping->png_raw_detections = (int)((unsigned short)short_val);
		mb_get_binary_float(swap, &line[20], &float_val);
		ping->png_raw_sample_rate = float_val;
		mb_get_binary_int(swap, &line[24], &int_val);
		ping->png_raw_spare = (int)(int_val);
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_raw_detections > ping->png_raw_nbeams || ping->png_raw_nbeams < 0 || ping->png_raw_detections < 0 ||
		    ping->png_raw_nbeams > MBSYS_SIMRAD3_MAXBEAMS || ping->png_raw_detections > MBSYS_SIMRAD3_MAXBEAMS ||
		    ping->png_raw_ntx > MBSYS_SIMRAD3_MAXTX) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary tx values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_raw_ntx && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_RAWBEAM4_TX_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXTX) {
				mb_get_binary_short(swap, &line[0], &short_val);
				ping->png_raw_txtiltangle[i] = (int)short_val;
				mb_get_binary_short(swap, &line[2], &short_val);
				ping->png_raw_txfocus[i] = (int)((unsigned short)short_val);
				mb_get_binary_float(swap, &line[4], &float_val);
				ping->png_raw_txsignallength[i] = float_val;
				mb_get_binary_float(swap, &line[8], &float_val);
				ping->png_raw_txoffset[i] = float_val;
				mb_get_binary_float(swap, &line[12], &float_val);
				ping->png_raw_txcenter[i] = float_val;
				mb_get_binary_short(swap, &line[16], &short_val);
				ping->png_raw_txabsorption[i] = (int)((unsigned short)short_val);
				ping->png_raw_txwaveform[i] = (int)line[18];
				ping->png_raw_txsector[i] = (int)line[19];
				mb_get_binary_float(swap, &line[20], &float_val);
				ping->png_raw_txbandwidth[i] = float_val;
			}
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_raw_nbeams && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_RAWBEAM4_BEAM_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXBEAMS) {
				mb_get_binary_short(swap, &line[0], &short_val);
				ping->png_raw_rxpointangle[i] = (int)short_val;
				ping->png_raw_rxsector[i] = (mb_u_char)line[2];
				ping->png_raw_rxdetection[i] = (mb_u_char)line[3];
				mb_get_binary_short(swap, &line[4], &short_val);
				ping->png_raw_rxwindow[i] = (int)((unsigned short)short_val);
				ping->png_raw_rxquality[i] = (mb_u_char)line[6];
				ping->png_raw_rxspare1[i] = (mb_s_char)line[7];
				mb_get_binary_float(swap, &line[8], &float_val);
				ping->png_raw_rxrange[i] = float_val;
				mb_get_binary_short(swap, &line[12], &short_val);
				ping->png_raw_rxamp[i] = (int)((short)short_val);
				ping->png_raw_rxcleaning[i] = (mb_s_char)line[14];
				ping->png_raw_rxspare2[i] = (mb_u_char)line[15];
			}
		}

		/* zero out ranges that are NaN values - unfortunately this has actually
		    happened in some R/V Langseth EM122 data from 20100616 */
		for (int i = 0; i < ping->png_raw_nbeams; i++) {
			if (check_fnan(ping->png_raw_rxrange[i]))
				ping->png_raw_rxrange[i] = 0.0;
		}
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	/* check for some other indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_raw_nbeams > 0 && ping->png_raw_detections > ping->png_raw_nbeams) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       png_raw_read:                %d\n", ping->png_raw_read);
		fprintf(stderr, "dbg5       png_raw_date:                %d\n", ping->png_raw_date);
		fprintf(stderr, "dbg5       png_raw_msec:                %d\n", ping->png_raw_msec);
		fprintf(stderr, "dbg5       png_raw_count:               %d\n", ping->png_raw_count);
		fprintf(stderr, "dbg5       png_raw_serial:              %d\n", ping->png_raw_serial);
		fprintf(stderr, "dbg5       png_raw_ssv:                 %d\n", ping->png_raw_ssv);
		fprintf(stderr, "dbg5       png_raw_ntx:                 %d\n", ping->png_raw_ntx);
		fprintf(stderr, "dbg5       png_raw_nbeams:              %d\n", ping->png_raw_nbeams);
		fprintf(stderr, "dbg5       png_raw_detections:          %d\n", ping->png_raw_detections);
		fprintf(stderr, "dbg5       png_raw_sample_rate:         %f\n", ping->png_raw_sample_rate);
		fprintf(stderr, "dbg5       png_raw_spare:               %d\n", ping->png_raw_spare);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       transmit pulse values:\n");
		fprintf(stderr, "dbg5       tiltangle focus length offset center bandwidth waveform sector\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_ntx; i++)
			fprintf(stderr, "dbg5       %3d %5d %5d %f %f %f %4d %4d %4d %f\n", i, ping->png_raw_txtiltangle[i],
			        ping->png_raw_txfocus[i], ping->png_raw_txsignallength[i], ping->png_raw_txoffset[i],
			        ping->png_raw_txcenter[i], ping->png_raw_txabsorption[i], ping->png_raw_txwaveform[i],
			        ping->png_raw_txsector[i], ping->png_raw_txbandwidth[i]);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       beam values:\n");
		fprintf(stderr, "dbg5       beam angle sector detection window quality spare1 range amp clean spare2\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %5d %3d %3d %4d %3d %5d %f %5d %5d %5d\n", i, ping->png_raw_rxpointangle[i],
			        ping->png_raw_rxsector[i], ping->png_raw_rxdetection[i], ping->png_raw_rxwindow[i],
			        ping->png_raw_rxquality[i], ping->png_raw_rxspare1[i], ping->png_raw_rxrange[i], ping->png_raw_rxamp[i],
			        ping->png_raw_rxcleaning[i], ping->png_raw_rxspare2[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_quality(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                            int *error) {
	struct mbsys_simrad3_ping_struct *ping;
	char line[EM3_QUALITY_HEADER_SIZE];
	short short_val = 0;
	float float_val = 0.0;
	size_t read_len;
	int png_count;
	int serial;
	int oldest_ping;
	int oldest_ping_index;
	int found;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_DATA;
	store->type = EM3_QUALITY;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_QUALITY_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* figure out which storage structure to use */
	mb_get_binary_short(swap, &line[8], &short_val);
	png_count = (int)((unsigned short)short_val);
	mb_get_binary_short(swap, &line[10], &short_val);
	serial = (int)((unsigned short)short_val);
	found = false;
	oldest_ping = 999999999;
	oldest_ping_index = -1;
	for (int i = 0; i < MBSYS_SIMRAD3_NUM_PING_STRUCTURES && !found; i++) {
		/* look for this ping by ping number and sonar serial number - if we already read
		 * a record from this ping it has to be stored in one of the structures */
		if (store->pings[i].read_status > 0 && png_count == store->pings[i].count && serial == store->pings[i].serial) {
			found = true;
			store->ping_index = i;
		}

		/* figure out which structure is holding the oldest ping in case we need to drop one
		 * from memory to make room for the new ping. */
		else if (store->pings[i].read_status > 0) {
			if (png_count < oldest_ping) {
				oldest_ping = png_count;
				oldest_ping_index = i;
			}
		}

		/* if one of the ping structures is unused, set it to be used if we need to store
		 * this ping */
		else if (oldest_ping > 0) {
			oldest_ping = 0;
			oldest_ping_index = i;
		}
	}
	if (!found) {
		store->ping_index = oldest_ping_index;
	}
	ping = (struct mbsys_simrad3_ping_struct *)&store->pings[store->ping_index];
	ping->count = png_count;
	ping->serial = serial;

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ping->png_quality_date);
		store->date = ping->png_quality_date;
		mb_get_binary_int(swap, &line[4], &ping->png_quality_msec);
		store->msec = ping->png_quality_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		ping->png_quality_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		ping->png_quality_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		ping->png_quality_nbeams = (int)((unsigned short)short_val);
		ping->png_quality_nparameters = (int)line[14];
		ping->png_quality_spare = (int)line[15];

#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr,
		        "mbr_em710raw_rd_quality:    ping->png_quality_date:%d     ping->png_quality_msec:%d     "
		        "ping->png_quality_count:%d     ping->png_quality_nbeams:%d\n",
		        ping->png_quality_date, ping->png_quality_msec, ping->png_quality_count, ping->png_quality_nbeams);
#endif
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_quality_nbeams && status == MB_SUCCESS; i++) {
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXBEAMS) {
				read_len = (size_t)(ping->png_quality_nparameters * sizeof(float));
				status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
				for (int j = 0; j < ping->png_quality_nparameters; j++) {
					mb_get_binary_float(swap, &line[j * sizeof(float)], &float_val);
					ping->png_quality_parameters[i][j] = float_val;
				}
			}
		}
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = (size_t)4;
		status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		if (line[1] == EM3_END)
			*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3],
		        line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:                  %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:                 %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                  %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                  %d\n", store->msec);
		fprintf(stderr, "dbg5       png_quality_date:              %d\n", ping->png_quality_date);
		fprintf(stderr, "dbg5       png_quality_msec:              %d\n", ping->png_quality_msec);
		fprintf(stderr, "dbg5       png_quality_count:             %d\n", ping->png_quality_count);
		fprintf(stderr, "dbg5       png_quality_serial:            %d\n", ping->png_quality_serial);
		fprintf(stderr, "dbg5       png_quality_nbeams:            %d\n", ping->png_quality_nbeams);
		fprintf(stderr, "dbg5       png_quality_nparameters:       %d\n", ping->png_quality_nparameters);
		fprintf(stderr, "dbg5       png_quality_spare:             %d\n", ping->png_quality_spare);
		fprintf(stderr, "dbg5       cnt  quality parameters\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_quality_nbeams; i++) {
			fprintf(stderr, "dbg5       %3d ", i);
			for (int j = 0; j < ping->png_quality_nparameters; j++)
				fprintf(stderr, "%f", ping->png_quality_parameters[i][j]);
			fprintf(stderr, "\n");
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_ss2(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                        int *error) {
	struct mbsys_simrad3_ping_struct *ping;
	char line[EM3_SS2_HEADER_SIZE];
	short short_val = 0;
	float float_val = 0.0;
	size_t read_len;
	int junk_bytes;
	int png_count;
	int serial;
	int oldest_ping;
	int oldest_ping_index;
	int found;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_DATA;
	store->type = EM3_SS2;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_SS2_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* figure out which storage structure to use */
	mb_get_binary_short(swap, &line[8], &short_val);
	png_count = (int)((unsigned short)short_val);
	mb_get_binary_short(swap, &line[10], &short_val);
	serial = (int)((unsigned short)short_val);
	found = false;
	oldest_ping = 999999999;
	oldest_ping_index = -1;
	for (int i = 0; i < MBSYS_SIMRAD3_NUM_PING_STRUCTURES && !found; i++) {
		/* look for this ping by ping number and sonar serial number - if we already read
		 * a record from this ping it has to be stored in one of the structures */
		if (store->pings[i].read_status > 0 && png_count == store->pings[i].count && serial == store->pings[i].serial) {
			found = true;
			store->ping_index = i;
		}

		/* figure out which structure is holding the oldest ping in case we need to drop one
		 * from memory to make room for the new ping. */
		else if (store->pings[i].read_status > 0) {
			if (png_count < oldest_ping) {
				oldest_ping = png_count;
				oldest_ping_index = i;
			}
		}

		/* if one of the ping structures is unused, set it to be used if we need to store
		 * this ping */
		else if (oldest_ping > 0) {
			oldest_ping = 0;
			oldest_ping_index = i;
		}
	}
	if (!found) {
		store->ping_index = oldest_ping_index;
	}
	ping = (struct mbsys_simrad3_ping_struct *)&store->pings[store->ping_index];
	ping->count = png_count;
	ping->serial = serial;

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ping->png_ss_date);
		store->date = ping->png_ss_date;
		mb_get_binary_int(swap, &line[4], &ping->png_ss_msec);
		store->msec = ping->png_ss_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		ping->png_ss_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		ping->png_ss_serial = (int)((unsigned short)short_val);
		mb_get_binary_float(swap, &line[12], &float_val);
		ping->png_ss_sample_rate = float_val;
		mb_get_binary_short(swap, &line[16], &short_val);
		ping->png_r_zero = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[18], &short_val);
		ping->png_bsn = (int)((short)short_val);
		mb_get_binary_short(swap, &line[20], &short_val);
		ping->png_bso = (int)((short)short_val);
		mb_get_binary_short(swap, &line[22], &short_val);
		ping->png_tx = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[24], &short_val);
		ping->png_tvg_crossover = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[26], &short_val);
		ping->png_nbeams_ss = (int)((unsigned short)short_val);
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_nbeams_ss < 0 || ping->png_nbeams_ss > MBSYS_SIMRAD3_MAXBEAMS) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		ping->png_npixels = 0;
		for (int i = 0; i < ping->png_nbeams_ss && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_SS2_BEAM_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXBEAMS) {
				status = MB_SUCCESS;
				ping->png_sort_direction[i] = (mb_s_char)line[0];
				ping->png_ssdetection[i] = (mb_u_char)line[1];
				mb_get_binary_short(swap, &line[2], &short_val);
				ping->png_beam_samples[i] = (int)((unsigned short)short_val);
				mb_get_binary_short(swap, &line[4], &short_val);
				ping->png_center_sample[i] = (int)((unsigned short)short_val);

				ping->png_start_sample[i] = ping->png_npixels;
				ping->png_npixels += ping->png_beam_samples[i];
				if (ping->png_npixels > MBSYS_SIMRAD3_MAXRAWPIXELS) {
					ping->png_beam_samples[i] -= (ping->png_npixels - MBSYS_SIMRAD3_MAXRAWPIXELS);
					if (ping->png_beam_samples[i] < 0)
						ping->png_beam_samples[i] = 0;
				}
			}
		}

		/* check for too much pixel data */
		if (ping->png_npixels > MBSYS_SIMRAD3_MAXRAWPIXELS) {
			if (verbose > 0)
				fprintf(stderr, "WARNING: Simrad multibeam sidescan pixels %d exceed maximum %d!\n", ping->png_npixels,
				        MBSYS_SIMRAD3_MAXRAWPIXELS);
			junk_bytes = ping->png_npixels - MBSYS_SIMRAD3_MAXRAWPIXELS;
			ping->png_npixels = MBSYS_SIMRAD3_MAXRAWPIXELS;
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
		else
			junk_bytes = 0;
	}

	/* read binary sidescan values */
	if (status == MB_SUCCESS) {
		read_len = (size_t)(2 * ping->png_npixels);
		status = mb_fileio_get(verbose, mbio_ptr, (char *)ping->png_ssraw, &read_len, error);
	}

	/* read any leftover binary sidescan values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < junk_bytes; i++) {
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
		}
	}

	/* now loop over reading individual characters to
	    get last bytes of record */
	if (status == MB_SUCCESS) {
		bool done = false;
		while (!done) {
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && line[0] == EM3_END) {
				done = true;

				/* get last two check sum bytes */
				read_len = (size_t)2;
				status = mb_fileio_get(verbose, mbio_ptr, (char *)&line[1], &read_len, error);

				*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
				fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[0], line[0], line[1], line[1], line[2],
				        line[2]);
#endif
			}
			else if (status != MB_SUCCESS) {
				done = true;
				/* return success here because all of the
				    important information in this record has
				    already been read - next attempt to read
				    file will return error */
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
		}
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "\n");
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:               %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:              %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:               %d\n", store->date);
		fprintf(stderr, "dbg5       msec:               %d\n", store->msec);
		fprintf(stderr, "dbg5       png_date:           %d\n", ping->png_date);
		fprintf(stderr, "dbg5       png_msec:           %d\n", ping->png_msec);
		fprintf(stderr, "dbg5       png_date:              %d\n", ping->png_date);
		fprintf(stderr, "dbg5       png_msec:              %d\n", ping->png_msec);
		fprintf(stderr, "dbg5       png_count:             %d\n", ping->png_count);
		fprintf(stderr, "dbg5       png_serial:            %d\n", ping->png_serial);
		fprintf(stderr, "dbg5       png_heading:           %d\n", ping->png_heading);
		fprintf(stderr, "dbg5       png_ssv:               %d\n", ping->png_ssv);
		fprintf(stderr, "dbg5       png_xducer_depth:      %f\n", ping->png_xducer_depth);
		fprintf(stderr, "dbg5       png_nbeams:            %d\n", ping->png_nbeams);
		fprintf(stderr, "dbg5       png_nbeams_valid:      %d\n", ping->png_nbeams_valid);
		fprintf(stderr, "dbg5       png_sample_rate:       %f\n", ping->png_sample_rate);
		fprintf(stderr, "dbg5       png_spare:             %d\n", ping->png_spare);
		fprintf(stderr, "dbg5       cnt  depth   xtrack   ltrack   wndw quality iba det clean amp\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %7.2f %7.2f %7.2f %5d %5d %5d %4d %3d %3d\n", i, ping->png_depth[i],
			        ping->png_acrosstrack[i], ping->png_alongtrack[i], ping->png_window[i], ping->png_quality[i],
			        ping->png_iba[i], ping->png_detection[i], ping->png_clean[i], ping->png_amp[i]);

		fprintf(stderr, "dbg5       png_ss_date:        %d\n", ping->png_ss_date);
		fprintf(stderr, "dbg5       png_ss_msec:        %d\n", ping->png_ss_msec);
		fprintf(stderr, "dbg5       png_ss_count:       %d\n", ping->png_ss_count);
		fprintf(stderr, "dbg5       png_ss_serial:      %d\n", ping->png_ss_serial);
		fprintf(stderr, "dbg5       png_ss_sample_rate: %f\n", ping->png_ss_sample_rate);
		fprintf(stderr, "dbg5       png_r_zero:         %d\n", ping->png_r_zero);
		fprintf(stderr, "dbg5       png_bsn:            %d\n", ping->png_bsn);
		fprintf(stderr, "dbg5       png_bso:            %d\n", ping->png_bso);
		fprintf(stderr, "dbg5       png_tx:             %d\n", ping->png_tx);
		fprintf(stderr, "dbg5       png_tvg_crossover:  %d\n", ping->png_tvg_crossover);
		fprintf(stderr, "dbg5       png_nbeams_ss:      %d\n", ping->png_nbeams_ss);
		fprintf(stderr, "dbg5       png_npixels:        %d\n", ping->png_npixels);
		fprintf(stderr, "dbg5       cnt  index sort samples start center\n");
		fprintf(stderr, "dbg5       --------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams_ss; i++)
			fprintf(stderr, "dbg5        %4d %2d %4d %4d %4d %4d\n", i, ping->png_sort_direction[i], ping->png_ssdetection[i],
			        ping->png_beam_samples[i], ping->png_start_sample[i], ping->png_center_sample[i]);
		fprintf(stderr, "dbg5       cnt  ss\n");
		fprintf(stderr, "dbg5       --------------------------------------------------\n");
		for (int i = 0; i < ping->png_npixels; i++)
			fprintf(stderr, "dbg5        %d %d\n", i, ping->png_ssraw[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_makenull_ss2(int verbose, void *mbio_ptr, struct mbsys_simrad3_struct *store, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

  int status = MB_SUCCESS;

  /* fill out the sidescan data as if an SS2 record has been read but it had no
      valid backscatter snippets */
  if (store != NULL && store->pings[store->ping_index].png_bath_read
      && store->pings[store->ping_index].png_raw_read
      && store->pings[store->ping_index].png_count == store->pings[store->ping_index].png_raw_count) {
	  struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

    ping->png_ss_read = MB_YES;
    ping->png_ss_date = ping->png_date;
    ping->png_ss_msec = ping->png_msec;
    ping->png_ss_count = ping->png_count;
    ping->png_ss_serial = ping->png_serial;
    ping->png_ss_sample_rate = ping->png_raw_sample_rate;
    // ping->png_r_zero
    // ping->png_bsn
    // ping->png_bso
    // ping->png_tx
    // ping->png_tvg_crossover
    ping->png_nbeams_ss = ping->png_nbeams;
    ping->png_npixels = 0;
    memset(ping->png_sort_direction, 0, MBSYS_SIMRAD3_MAXBEAMS * sizeof(int));
    memset(ping->png_ssdetection, 0, MBSYS_SIMRAD3_MAXBEAMS * sizeof(int));
    memset(ping->png_beam_samples, 0, MBSYS_SIMRAD3_MAXBEAMS * sizeof(int));
    memset(ping->png_start_sample, 0, MBSYS_SIMRAD3_MAXBEAMS * sizeof(int));
    memset(ping->png_center_sample, 0, MBSYS_SIMRAD3_MAXBEAMS * sizeof(int));
    memset(ping->png_ssraw, 0, MBSYS_SIMRAD3_MAXRAWPIXELS * sizeof(short));
    ping->png_pixel_size = 0.0;
    ping->png_pixels_ss = 0;
    memset(ping->png_ss, 0, MBSYS_SIMRAD3_MAXPIXELS * sizeof(short));
    memset(ping->png_ssalongtrack, 0, MBSYS_SIMRAD3_MAXPIXELS * sizeof(short));

  	if (verbose >= 5) {
  		fprintf(stderr, "\ndbg5  Values modified in MBIO function <%s>\n", __func__);
  		fprintf(stderr, "dbg5       type:               %d\n", store->type);
  		fprintf(stderr, "dbg5       sonar:              %d\n", store->sonar);
  		fprintf(stderr, "dbg5       date:               %d\n", store->date);
  		fprintf(stderr, "dbg5       msec:               %d\n", store->msec);
  		fprintf(stderr, "dbg5       png_date:           %d\n", ping->png_date);
  		fprintf(stderr, "dbg5       png_msec:           %d\n", ping->png_msec);
  		fprintf(stderr, "dbg5       png_date:              %d\n", ping->png_date);
  		fprintf(stderr, "dbg5       png_msec:              %d\n", ping->png_msec);
  		fprintf(stderr, "dbg5       png_count:             %d\n", ping->png_count);
  		fprintf(stderr, "dbg5       png_serial:            %d\n", ping->png_serial);
  		fprintf(stderr, "dbg5       png_heading:           %d\n", ping->png_heading);
  		fprintf(stderr, "dbg5       png_ssv:               %d\n", ping->png_ssv);
  		fprintf(stderr, "dbg5       png_xducer_depth:      %f\n", ping->png_xducer_depth);
  		fprintf(stderr, "dbg5       png_nbeams:            %d\n", ping->png_nbeams);
  		fprintf(stderr, "dbg5       png_nbeams_valid:      %d\n", ping->png_nbeams_valid);
  		fprintf(stderr, "dbg5       png_sample_rate:       %f\n", ping->png_sample_rate);
  		fprintf(stderr, "dbg5       png_spare:             %d\n", ping->png_spare);
  		fprintf(stderr, "dbg5       cnt  depth   xtrack   ltrack   wndw quality iba det clean amp\n");
  		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
  		for (int i = 0; i < ping->png_nbeams; i++)
  			fprintf(stderr, "dbg5       %3d %7.2f %7.2f %7.2f %5d %5d %5d %4d %3d %3d\n", i, ping->png_depth[i],
  			        ping->png_acrosstrack[i], ping->png_alongtrack[i], ping->png_window[i], ping->png_quality[i],
  			        ping->png_iba[i], ping->png_detection[i], ping->png_clean[i], ping->png_amp[i]);

  		fprintf(stderr, "dbg5       png_ss_date:        %d\n", ping->png_ss_date);
  		fprintf(stderr, "dbg5       png_ss_msec:        %d\n", ping->png_ss_msec);
  		fprintf(stderr, "dbg5       png_ss_count:       %d\n", ping->png_ss_count);
  		fprintf(stderr, "dbg5       png_ss_serial:      %d\n", ping->png_ss_serial);
  		fprintf(stderr, "dbg5       png_ss_sample_rate: %f\n", ping->png_ss_sample_rate);
  		fprintf(stderr, "dbg5       png_r_zero:         %d\n", ping->png_r_zero);
  		fprintf(stderr, "dbg5       png_bsn:            %d\n", ping->png_bsn);
  		fprintf(stderr, "dbg5       png_bso:            %d\n", ping->png_bso);
  		fprintf(stderr, "dbg5       png_tx:             %d\n", ping->png_tx);
  		fprintf(stderr, "dbg5       png_tvg_crossover:  %d\n", ping->png_tvg_crossover);
  		fprintf(stderr, "dbg5       png_nbeams_ss:      %d\n", ping->png_nbeams_ss);
  		fprintf(stderr, "dbg5       png_npixels:        %d\n", ping->png_npixels);
  		fprintf(stderr, "dbg5       cnt  index sort samples start center\n");
  		fprintf(stderr, "dbg5       --------------------------------------------------\n");
  		for (int i = 0; i < ping->png_nbeams_ss; i++)
  			fprintf(stderr, "dbg5        %4d %2d %4d %4d %4d %4d\n", i, ping->png_sort_direction[i], ping->png_ssdetection[i],
  			        ping->png_beam_samples[i], ping->png_start_sample[i], ping->png_center_sample[i]);
  		fprintf(stderr, "dbg5       cnt  ss\n");
  		fprintf(stderr, "dbg5       --------------------------------------------------\n");
  		for (int i = 0; i < ping->png_npixels; i++)
  			fprintf(stderr, "dbg5        %d %d\n", i, ping->png_ssraw[i]);
			fprintf(stderr, "dbg5       png_pixel_size:     %f\n", ping->png_pixel_size);
			fprintf(stderr, "dbg5       png_pixels_ss:      %d\n", ping->png_pixels_ss);
			for (int i = 0; i < ping->png_pixels_ss; i++)
				fprintf(stderr, "dbg5       pixel:%4d  ss:%8d  ltrack:%8d\n", i, ping->png_ss[i], ping->png_ssalongtrack[i]);
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
int mbr_em710raw_rd_wc(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, short sonar, int *goodend,
                       int *error) {
	struct mbsys_simrad3_watercolumn_struct *wc;
	char line[EM3_WC_HEADER_SIZE];
	short short_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	wc = (struct mbsys_simrad3_watercolumn_struct *)store->wc;

	/* set kind and type values */
	store->kind = MB_DATA_WATER_COLUMN;
	store->type = EM3_WATERCOLUMN;
	store->sonar = sonar;

	/* read binary header values into char array */
	read_len = (size_t)EM3_WC_HEADER_SIZE;
	int status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &wc->wtc_date);
		store->date = wc->wtc_date;
		mb_get_binary_int(swap, &line[4], &wc->wtc_msec);
		store->msec = wc->wtc_msec;
		mb_get_binary_short(swap, &line[8], &short_val);
		wc->wtc_count = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[10], &short_val);
		wc->wtc_serial = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[12], &short_val);
		wc->wtc_ndatagrams = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[14], &short_val);
		wc->wtc_datagram = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[16], &short_val);
		wc->wtc_ntx = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[18], &short_val);
		wc->wtc_nrx = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[20], &short_val);
		wc->wtc_nbeam = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[22], &short_val);
		wc->wtc_ssv = (int)((unsigned short)short_val);
		mb_get_binary_int(swap, &line[24], &(wc->wtc_sfreq));
		mb_get_binary_short(swap, &line[28], &short_val);
		wc->wtc_heave = (int)((short)short_val);
		mb_get_binary_short(swap, &line[30], &short_val);
		wc->wtc_spare1 = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[32], &short_val);
		wc->wtc_spare2 = (int)((unsigned short)short_val);
		mb_get_binary_short(swap, &line[34], &short_val);
		wc->wtc_spare3 = (int)((unsigned short)short_val);
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (wc->wtc_nbeam < 0 || wc->wtc_nbeam > MBSYS_SIMRAD3_MAXBEAMS || wc->wtc_ntx < 0 || wc->wtc_ntx > MBSYS_SIMRAD3_MAXTX) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < wc->wtc_ntx && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_WC_TX_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXTX) {
				mb_get_binary_short(swap, &line[0], &short_val);
				wc->wtc_txtiltangle[i] = (int)(short_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				wc->wtc_txcenter[i] = (int)(short_val);
				wc->wtc_txsector[i] = (int)((mb_u_char)line[4]);
			}
		}
		for (int i = 0; i < wc->wtc_nbeam && status == MB_SUCCESS; i++) {
			read_len = (size_t)EM3_WC_BEAM_SIZE;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && i < MBSYS_SIMRAD3_MAXBEAMS) {
				mb_get_binary_short(swap, &line[0], &short_val);
				wc->beam[i].wtc_rxpointangle = (int)(short_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				wc->beam[i].wtc_start_sample = (int)(short_val);
				mb_get_binary_short(swap, &line[4], &short_val);
				wc->beam[i].wtc_beam_samples = (int)(unsigned short)(short_val);
				mb_get_binary_short(swap, &line[6], &short_val);
				wc->beam[i].wtc_beam_spare = (int)(unsigned short)(short_val);
				wc->beam[i].wtc_sector = (int)(mb_u_char)(line[8]);
				wc->beam[i].wtc_beam = (int)(mb_u_char)(line[9]);
			}
			read_len = (size_t)wc->beam[i].wtc_beam_samples;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)wc->beam[i].wtc_amp, &read_len, error);
		}
	}

	/* now loop over reading individual characters to
	    get last bytes of record */
	if (status == MB_SUCCESS) {
		bool done = false;
		while (!done) {
			read_len = (size_t)1;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)line, &read_len, error);
			if (status == MB_SUCCESS && line[0] == EM3_END) {
				done = true;

				/* get last two check sum bytes */
				read_len = (size_t)2;
				status = mb_fileio_get(verbose, mbio_ptr, (char *)&line[1], &read_len, error);
				*goodend = true;
#ifdef MBR_EM710RAW_DEBUG
				fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[0], line[0], line[1], line[1], line[2],
				        line[2]);
#endif
			}
			else if (status != MB_SUCCESS) {
				done = true;
				/* return success here because all of the
				    important information in this record has
				    already been read - next attempt to read
				    file will return error */
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
		}
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "\n");
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       wtc_date:        %d\n", wc->wtc_date);
		fprintf(stderr, "dbg5       wtc_msec:        %d\n", wc->wtc_msec);
		fprintf(stderr, "dbg5       wtc_count:       %d\n", wc->wtc_count);
		fprintf(stderr, "dbg5       wtc_serial:      %d\n", wc->wtc_serial);
		fprintf(stderr, "dbg5       wtc_ndatagrams:  %d\n", wc->wtc_ndatagrams);
		fprintf(stderr, "dbg5       wtc_datagram:    %d\n", wc->wtc_datagram);
		fprintf(stderr, "dbg5       wtc_ntx:         %d\n", wc->wtc_ntx);
		fprintf(stderr, "dbg5       wtc_nrx:         %d\n", wc->wtc_nrx);
		fprintf(stderr, "dbg5       wtc_nbeam:       %d\n", wc->wtc_nbeam);
		fprintf(stderr, "dbg5       wtc_ssv:         %d\n", wc->wtc_ssv);
		fprintf(stderr, "dbg5       wtc_sfreq:       %d\n", wc->wtc_sfreq);
		fprintf(stderr, "dbg5       wtc_heave:       %d\n", wc->wtc_heave);
		fprintf(stderr, "dbg5       wtc_spare1:      %d\n", wc->wtc_spare1);
		fprintf(stderr, "dbg5       wtc_spare2:      %d\n", wc->wtc_spare2);
		fprintf(stderr, "dbg5       wtc_spare3:      %d\n", wc->wtc_spare3);
		fprintf(stderr, "dbg5       ---------------------------\n");
		fprintf(stderr, "dbg5       cnt  tilt center sector\n");
		fprintf(stderr, "dbg5       ---------------------------\n");
		for (int i = 0; i < wc->wtc_ntx; i++)
			fprintf(stderr, "dbg5       %3d %6d %6d %6d\n", i, wc->wtc_txtiltangle[i], wc->wtc_txcenter[i], wc->wtc_txsector[i]);
		for (int i = 0; i < wc->wtc_nbeam; i++) {
			fprintf(stderr, "dbg5       --------------------------------------------------\n");
			fprintf(stderr, "dbg5       cnt  angle start samples unknown sector beam\n");
			fprintf(stderr, "dbg5       --------------------------------------------------\n");
			fprintf(stderr, "dbg5        %4d %3d %2d %4d %4d %4d %4d\n", i, wc->beam[i].wtc_rxpointangle,
			        wc->beam[i].wtc_start_sample, wc->beam[i].wtc_beam_samples, wc->beam[i].wtc_beam_spare,
			        wc->beam[i].wtc_sector, wc->beam[i].wtc_beam);
			/*			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			            fprintf(stderr,"dbg5       beam[%d]: sample amplitude\n",i);
			            fprintf(stderr,"dbg5       --------------------------------------------------\n");
			            for (j=0;j<wc->beam[i].wtc_beam_samples;j++)
			                fprintf(stderr,"dbg5        %4d %4d\n",
			                    j, wc->beam[i].wtc_amp[j]);*/
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em710raw_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int *databyteswapped;
	int record_size;
	int *record_size_save;
	int bytes_read;
	char *label;
	int *label_save_flag;
	char *record_size_char;
  bool *ignore_snippets;
  bool *sensordepth_only;
	short type;
	short sonar;
	int *version;
	short *typelast;
	short *sonarlast;
	int *nbadrec;
	int good_end_bytes;
	size_t read_len;
	int skip = 0;
	int *num_sonars;
	char junk;

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
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;
	FILE *mbfp = mb_io_ptr->mbfp;

	/* get saved values */
	databyteswapped = (int *)&mb_io_ptr->save1;
	record_size_save = (int *)&mb_io_ptr->save2;
	label = (char *)mb_io_ptr->save_label;
	version = (int *)(&mb_io_ptr->save3);
	label_save_flag = (int *)&mb_io_ptr->save_label_flag;
	typelast = (short *)&mb_io_ptr->save6;
	sonarlast = (short *)&mb_io_ptr->save7;
	nbadrec = (int *)&mb_io_ptr->save8;
	num_sonars = (int *)&mb_io_ptr->save10;
	record_size_char = (char *)&record_size;
  ignore_snippets = (bool *)&mb_io_ptr->save4;
	sensordepth_only = (bool *)&mb_io_ptr->save5;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* set flag to swap bytes if necessary */
	int swap = *databyteswapped;

	/* if a ping structure was previously flagged as complete then reset the structure to empty */
	for (int i = 0; i < MBSYS_SIMRAD3_NUM_PING_STRUCTURES; i++) {
		if (store->pings[i].read_status == MBSYS_SIMRAD3_PING_COMPLETE) {
			store->pings[i].read_status = MBSYS_SIMRAD3_PING_NO_DATA;
			store->pings[i].png_bath_read = false;
			store->pings[i].png_raw_read = false;
			store->pings[i].png_quality_read = false;
			store->pings[i].png_ss_read = false;
		}
	}

	int status = MB_SUCCESS;

	/* loop over reading data until a record is ready for return */
	*error = MB_ERROR_NO_ERROR;
	bool done = false;
	while (!done) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "\nabove mbr_em710raw_rd_data loop:\n");
		fprintf(stderr, "label_save_flag:%d status:%d\n", *label_save_flag, status);
#endif
		/* if no label saved get next record label */
		if (!*label_save_flag) {
			/* read four byte record size */
			read_len = 4;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)&record_size, &read_len, error);
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "read record size:%d status:%d\n", record_size, status);
#endif

			/* read label */
			read_len = 4;
			status = mb_fileio_get(verbose, mbio_ptr, (char *)label, &read_len, error);

			/* check label - if not a good label read a byte
			    at a time until a good label is found */
			skip = 0;
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "read label:%x%x%x%x skip:%d status:%d\n", label[0], label[1], label[2], label[3], skip, status);
#endif
			while (status == MB_SUCCESS && mbr_em710raw_chk_label(verbose, mbio_ptr, label, &type, &sonar) != MB_SUCCESS) {
				/* get next byte */
				for (int i = 0; i < 3; i++)
					record_size_char[i] = record_size_char[i + 1];
				record_size_char[3] = label[0];
				for (int i = 0; i < 3; i++)
					label[i] = label[i + 1];
				read_len = 1;
				status = mb_fileio_get(verbose, mbio_ptr, (char *)(&label[3]), &read_len, error);
				skip++;
#ifdef MBR_EM710RAW_DEBUG
				fprintf(stderr, "read label:%x%x%x%x skip:%d status:%d\n", label[0], label[1], label[2], label[3], skip, status);
#endif
			}

			/* report problem */
			if (skip > 0 && verbose > 0) {
				if (*nbadrec == 0)
					fprintf(stderr, "\nThe MBF_EM710RAW module skipped data between identified\n\
data records. Something is broken, most probably the data...\n\
However, the data may include a data record type that we\n\
haven't seen yet, or there could be an error in the code.\n\
If skipped data are reported multiple times, \n\
we recommend you send a data sample and problem \n\
description to the MB-System team \n\
(caress@mbari.org and dale@ldeo.columbia.edu)\n\
Have a nice day...\n");
				fprintf(stderr, "MBF_EM710RAW skipped %d bytes between records %4.4hX:%d and %4.4hX:%d\n", skip, *typelast,
				        *typelast, type, type);
				(*nbadrec)++;
			}
			*typelast = type;
			*sonarlast = sonar;

			/* set flag to swap bytes if necessary */
			swap = *databyteswapped;

			/* get record_size */
			if (*databyteswapped != mb_io_ptr->byteswapped)
				record_size = mb_swap_int(record_size);
			*record_size_save = record_size;
		}

		/* else use saved label */
		else {
			*label_save_flag = false;
			type = *typelast;
			sonar = *sonarlast;
			record_size = *record_size_save;
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "use previously read label:%x%x%x%x skip:%d status:%d\n", label[0], label[1], label[2], label[3],
			        skip, status);
#endif
		}

#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "\nstart of mbr_em710raw_rd_data loop:\n");
		fprintf(stderr, "skip:%d type:%x sonar:%d recsize:%u done:%d\n", skip, type, sonar, *record_size_save, done);
#endif
#ifdef MBR_EM710RAW_DEBUG3
		if (skip > 0)
			fprintf(stderr, "SKIPPED BYTES: %d\n", skip);
		fprintf(stderr, "type:%x sonar:%d recsize:%u done:%d   ", type, sonar, *record_size_save, done);
#endif

		/* allocate secondary data structure for
		    extraparameters data if needed */
		if (status == MB_SUCCESS && (type == EM3_EXTRAPARAMETERS) && store->extraparameters == NULL) {
			status = mbsys_simrad3_extraparameters_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    heading data if needed */
		if (status == MB_SUCCESS && (type == EM3_HEADING) && store->heading == NULL) {
			status = mbsys_simrad3_heading_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    attitude data if needed */
		if (status == MB_SUCCESS && (type == EM3_ATTITUDE) && store->attitude == NULL) {
			status = mbsys_simrad3_attitude_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    netattitude data if needed */
		if (status == MB_SUCCESS && (type == EM3_NETATTITUDE) && store->netattitude == NULL) {
			status = mbsys_simrad3_netattitude_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    ssv data if needed */
		if (status == MB_SUCCESS && (type == EM3_SSV) && store->ssv == NULL) {
			status = mbsys_simrad3_ssv_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    tilt data if needed */
		if (status == MB_SUCCESS && (type == EM3_TILT) && store->tilt == NULL) {
			status = mbsys_simrad3_tilt_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    water column data if needed */
		if (status == MB_SUCCESS && (type == EM3_WATERCOLUMN)) {
			if (store->wc == NULL)
				status = mbsys_simrad3_wc_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* read the appropriate data records */
		if (status == MB_FAILURE) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call nothing, read failure\n");
#endif
			done = true;
			record_size = 0;
			*record_size_save = record_size;
		}
		else if (type != EM3_PU_ID && type != EM3_PU_STATUS && type != EM3_PU_BIST && type != EM3_EXTRAPARAMETERS &&
		         type != EM3_ATTITUDE && type != EM3_CLOCK && type != EM3_BATH && type != EM3_SBDEPTH && type != EM3_RAWBEAM &&
		         type != EM3_SSV && type != EM3_HEADING && type != EM3_START && type != EM3_TILT && type != EM3_CBECHO &&
		         type != EM3_RAWBEAM4 && type != EM3_QUALITY && type != EM3_POS && type != EM3_RUN_PARAMETER && type != EM3_SS &&
		         type != EM3_TIDE && type != EM3_SVP2 && type != EM3_SVP && type != EM3_SSPINPUT && type != EM3_BATH2 &&
		         type != EM3_SS2 && type != EM3_RAWBEAM2 && type != EM3_RAWBEAM3 && type != EM3_HEIGHT && type != EM3_STOP &&
		         type != EM3_WATERCOLUMN && type != EM3_NETATTITUDE && type != EM3_REMOTE && type != EM3_SSP &&
		         type != EM3_BATH_MBA && type != EM3_SS_MBA && type != EM3_BATH2_MBA && type != EM3_SS2_MBA &&
		         type != EM3_BATH3_MBA) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call nothing, try again\n");
#endif
			done = false;
		}
		else if (type == EM3_PU_ID) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_puid type %x\n", type);
#endif

			status = mbr_em710raw_rd_puid(verbose, mbio_ptr, swap, store, type, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_puid\n");
#endif
    }
		else if (type == EM3_PU_STATUS) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_status type %x\n", type);
#endif

			status = mbr_em710raw_rd_status(verbose, mbio_ptr, swap, store, type, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_status\n");
#endif
		}
		else if (type == EM3_START || type == EM3_STOP) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_start type %x\n", type);
#endif
			status =
			    mbr_em710raw_rd_start(verbose, mbio_ptr, swap, store, type, sonar, version, num_sonars, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_start\n");
#endif
		}
		else if (type == EM3_RUN_PARAMETER) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_run_parameter type %x\n", type);
#endif
			status = mbr_em710raw_rd_run_parameter(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_run_parameter\n");
#endif
		}
		else if (type == EM3_CLOCK) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_clock type %x\n", type);
#endif
			status = mbr_em710raw_rd_clock(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_clock\n");
#endif
		}
		else if (type == EM3_TIDE) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_tide type %x\n", type);
#endif
			status = mbr_em710raw_rd_tide(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_tide\n");
#endif
		}
		else if (type == EM3_HEIGHT) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_height type %x\n", type);
#endif
			status = mbr_em710raw_rd_height(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_height\n");
#endif
		}
		else if (type == EM3_HEADING) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_heading type %x\n", type);
#endif
			status = mbr_em710raw_rd_heading(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_heading\n");
#endif
		}
		else if (type == EM3_SSV) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_ssv type %x\n", type);
#endif
			status = mbr_em710raw_rd_ssv(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_ssv\n");
#endif
		}
		else if (type == EM3_TILT) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_tilt type %x\n", type);
#endif
			status = mbr_em710raw_rd_tilt(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_tilt\n");
#endif
		}
		else if (type == EM3_EXTRAPARAMETERS) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_extraparameters type %x\n", type);
#endif
			status = mbr_em710raw_rd_extraparameters(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_extraparameters\n");
#endif
		}
		else if (type == EM3_ATTITUDE) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_attitude type %x\n", type);
#endif
			status = mbr_em710raw_rd_attitude(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_attitude\n");
#endif
		}
		else if (type == EM3_NETATTITUDE) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_netattitude type %x\n", type);
#endif
			status = mbr_em710raw_rd_netattitude(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_netattitude\n");
#endif
		}
		else if (type == EM3_POS) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_pos type %x\n", type);
#endif
			status = mbr_em710raw_rd_pos(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_pos\n");
#endif
		}
		else if (type == EM3_SVP) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_svp type %x\n", type);
#endif
			status = mbr_em710raw_rd_svp(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_svp\n");
#endif
		}
		else if (type == EM3_SVP2) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_svp2 type %x\n", type);
#endif
			status = mbr_em710raw_rd_svp2(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_svp2\n");
#endif
		}
		else if (type == EM3_BATH2) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_bath2 type %x\n", type);
#endif
			status = mbr_em710raw_rd_bath2(verbose, mbio_ptr, swap, store, sonar, *version, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				store->pings[store->ping_index].read_status = MBSYS_SIMRAD3_PING_PARTIAL;
				store->pings[store->ping_index].png_bath_read = true;
				done = false;
			}
/*
fprintf(stderr, "%s:%d:%s: ignore_snippets:%d index:%d    %d %d %d    %d %d %d\n",
__FILE__, __LINE__, __FUNCTION__,
*ignore_snippets, store->ping_index,
store->pings[store->ping_index].png_bath_read,
store->pings[store->ping_index].png_raw_read,
store->pings[store->ping_index].png_ss_read,
store->pings[store->ping_index].png_count,
store->pings[store->ping_index].png_raw_count,
store->pings[store->ping_index].png_ss_count);
*/

			if (status == MB_SUCCESS && sonar == MBSYS_SIMRAD3_M3) {
				if (store->pings[store->ping_index].png_bath_read &&
				    store->pings[store->ping_index].png_raw_read &&
				    store->pings[store->ping_index].png_count == store->pings[store->ping_index].png_raw_count) {
					store->pings[store->ping_index].read_status = MBSYS_SIMRAD3_PING_COMPLETE;
					done = true;
				}
			}
			else if (status == MB_SUCCESS && *ignore_snippets) {
				if (store->pings[store->ping_index].png_bath_read &&
				    store->pings[store->ping_index].png_raw_read &&
				    store->pings[store->ping_index].png_count == store->pings[store->ping_index].png_raw_count) {
          status = mbr_em710raw_makenull_ss2(verbose, mbio_ptr, store, error);
				  store->pings[store->ping_index].png_ss_read = true;
					store->pings[store->ping_index].read_status = MBSYS_SIMRAD3_PING_COMPLETE;
					done = true;
				}
			}
			else if (status == MB_SUCCESS) {
				if (store->pings[store->ping_index].png_bath_read &&
				    store->pings[store->ping_index].png_ss_read &&
				    store->pings[store->ping_index].png_count == store->pings[store->ping_index].png_ss_count) {
					store->pings[store->ping_index].read_status = MBSYS_SIMRAD3_PING_COMPLETE;
					done = true;
				}
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_bath2: ping_index:%d ping:%d serial:%d\n", store->ping_index,
			        store->pings[store->ping_index].png_count, store->pings[store->ping_index].png_serial);
#endif
		}
		else if (type == EM3_RAWBEAM4) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_rawbeam4 type %x\n", type);
#endif
			status = mbr_em710raw_rd_rawbeam4(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				store->pings[store->ping_index].read_status = MBSYS_SIMRAD3_PING_PARTIAL;
				store->pings[store->ping_index].png_raw_read = true;
				done = false;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_rawbeam4: ping_index:%d ping:%d serial:%d\n", store->ping_index,
			        store->pings[store->ping_index].png_raw_count, store->pings[store->ping_index].png_raw_serial);
#endif
		}
		else if (type == EM3_QUALITY) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_quality type %x\n", type);
#endif
			status = mbr_em710raw_rd_quality(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				store->pings[store->ping_index].read_status = MBSYS_SIMRAD3_PING_PARTIAL;
				store->pings[store->ping_index].png_quality_read = true;
				done = false;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_quality: ping_index:%d ping:%d serial:%d\n", store->ping_index,
			        store->pings[store->ping_index].png_quality_count, store->pings[store->ping_index].png_quality_serial);
#endif
		}
		else if (type == EM3_SS2) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_ss2 type %x\n", type);
#endif
			status = mbr_em710raw_rd_ss2(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
        if (!(*ignore_snippets)) {
  				store->pings[store->ping_index].read_status = MBSYS_SIMRAD3_PING_PARTIAL;
  				store->pings[store->ping_index].png_ss_read = true;
  				done = false;
        }
			}
			if (status == MB_SUCCESS) {
				if (store->pings[store->ping_index].png_bath_read &&
				    store->pings[store->ping_index].png_ss_read &&
				    store->pings[store->ping_index].png_count == store->pings[store->ping_index].png_ss_count) {
					store->pings[store->ping_index].read_status = MBSYS_SIMRAD3_PING_COMPLETE;
					done = true;
				}
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_ss2: ping_index:%d ping:%d serial:%d\n", store->ping_index,
			        store->pings[store->ping_index].png_ss_count, store->pings[store->ping_index].png_ss_serial);
#endif
		}
		else if (type == EM3_WATERCOLUMN) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_rd_wc type %x\n", type);
#endif
			status = mbr_em710raw_rd_wc(verbose, mbio_ptr, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "mbr_em710raw_rd_wc ping:%d\n", store->wc->wtc_count);
#endif
		}
		else {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "skip over %d bytes of unsupported datagram type %x\n", *record_size_save, type);
#endif
			for (int i = 0; i < *record_size_save - 4; i++) {
				read_len = 1;
				status = mb_fileio_get(verbose, mbio_ptr, (char *)&junk, &read_len, error);
			}
			if (status == MB_FAILURE) {
				done = true;
				good_end_bytes = false;
			}
			else {
				done = false;
				good_end_bytes = true;
			}
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "skip over %d bytes of unsupported datagram type %x\n", *record_size_save, type);
#endif
		}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done = true;

		/* if necessary read over unread but expected bytes */
        if(mbfp != NULL){
            // reading from file
            bytes_read = ftell(mbfp) - mb_io_ptr->file_bytes - 4;
        }
//        else if(mbsp != NULL) {
//            // reading from socket
//            bytes_read = 0;
//        } else {
//            // error: neither file nor socket set
//        }

		if (!*label_save_flag && !good_end_bytes && bytes_read < record_size) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "skip over %d unread bytes of supported datagram type %x\n", record_size - bytes_read, type);
#endif
			for (int i = 0; i < record_size - bytes_read; i++) {
				read_len = 1;
				status = mb_fileio_get(verbose, mbio_ptr, (char *)&junk, &read_len, error);
			}
		}

#ifdef MBR_EM710RAW_DEBUG
        if(mbfp != NULL)
            fprintf(stderr, "record_size:%d bytes read:%ld file_pos old:%ld new:%ld\n", record_size, ftell(mbfp) - mb_io_ptr->file_bytes, mb_io_ptr->file_bytes, ftell(mbfp));
		fprintf(stderr, "done:%d status:%d error:%d\n", done, status, *error);
		fprintf(stderr, "end of mbr_em710raw_rd_data loop:\n\n");
#endif
#ifdef MBR_EM710RAW_DEBUG3
		if (done)
			fprintf(stderr, "DONE! type:%x kind:%d status:%d error:%d\n\n", type, store->kind, status, *error);
#endif

		/* get file position */
        if (*label_save_flag) {
            if(mbfp != NULL) {
                mb_io_ptr->file_bytes = ftell(mbfp) - 2;
            }
        } else {
            if(mbfp != NULL){
                mb_io_ptr->file_bytes = ftell(mbfp);
            }
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
int mbr_rt_em710raw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int time_i[7];
	double ntime_d, ptime_d, atime_d, btime_d;
	double rawspeed, pheading;
	double plon, plat, pspeed, roll, pitch, heave;
	double att_time_d[MBSYS_SIMRAD3_MAXATTITUDE];
	double att_roll[MBSYS_SIMRAD3_MAXATTITUDE];
	double att_pitch[MBSYS_SIMRAD3_MAXATTITUDE];
	double att_heave[MBSYS_SIMRAD3_MAXATTITUDE];

	double transmit_time_d, transmit_heading, transmit_heave, transmit_roll, transmit_pitch;
	double receive_time_d, receive_heading, receive_heave, receive_roll, receive_pitch;

	/* variables for beam angle calculation */
	mb_3D_orientation tx_align;
	mb_3D_orientation tx_orientation;
	double tx_steer;
	mb_3D_orientation rx_align;
	mb_3D_orientation rx_orientation;
	double rx_steer;
	double reference_heading;
	double beamAzimuth;
	double beamDepression;
	mb_u_char detection_mask;
  int interp_error = MB_ERROR_NO_ERROR;

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
	int status = mbr_em710raw_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* get pointers to data structures */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;
	struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);
	struct mbsys_simrad3_attitude_struct *attitude = (struct mbsys_simrad3_attitude_struct *)store->attitude;
	struct mbsys_simrad3_netattitude_struct *netattitude = (struct mbsys_simrad3_netattitude_struct *)store->netattitude;
	double *pixel_size = (double *)&mb_io_ptr->saved1;
	double *swath_width = (double *)&mb_io_ptr->saved2;

	/* save fix and heading if nav data from the active position system */
	if (status == MB_SUCCESS &&
	    (store->kind == MB_DATA_NAV || store->kind == MB_DATA_NAV1 || store->kind == MB_DATA_NAV2 ||
	     store->kind == MB_DATA_NAV3) &&
	    store->pos_system & 128) {
		/* get nav time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ntime_d);

		/* add latest fix */
		if (store->pos_longitude != EM3_INVALID_INT && store->pos_latitude != EM3_INVALID_INT)
			mb_navint_add(verbose, mbio_ptr, ntime_d, (double)(0.0000001 * store->pos_longitude),
			              (double)(0.00000005 * store->pos_latitude), error);

		/* add latest heading */
		if (store->pos_heading != EM3_INVALID_INT)
			mb_hedint_add(verbose, mbio_ptr, ntime_d, (double)(0.01 * store->pos_heading), error);
	}

	/* save attitude if "active" attitude data
	    - note that the mb_io_ptr->attitude_source value will be set dynamically
	        to reflect the actual attitude source used in realtime
	        by the sonar.
	    - attitude records from the sensors are
	        set at kinds MB_DATA_ATTITUDE (serial port 1),
	        MB_DATA_ATTITUDE1 (serial port 2),
	        or MB_DATA_ATTITUDE2 (network attitude). */
	if (status == MB_SUCCESS && store->type == EM3_ATTITUDE) {
		/* save for interpolation if this record comes
		    from the active attitude sensor */
		if ((attitude->att_sensordescriptor & 14) == 0) {
			/* set the attitude_source */
			mb_io_ptr->attitude_source = store->kind;

			/* get attitude time */
			time_i[0] = attitude->att_date / 10000;
			time_i[1] = (attitude->att_date % 10000) / 100;
			time_i[2] = attitude->att_date % 100;
			time_i[3] = attitude->att_msec / 3600000;
			time_i[4] = (attitude->att_msec % 3600000) / 60000;
			time_i[5] = (attitude->att_msec % 60000) / 1000;
			time_i[6] = (attitude->att_msec % 1000) * 1000;
			mb_get_time(verbose, time_i, &atime_d);

			/* add latest attitude samples */
			attitude->att_ndata = MIN(attitude->att_ndata, MBSYS_SIMRAD3_MAXATTITUDE);
			for (int i = 0; i < attitude->att_ndata; i++) {
				att_time_d[i] = (double)(atime_d + 0.001 * attitude->att_time[i]);
				att_heave[i] = (double)(0.01 * attitude->att_heave[i]);
				att_roll[i] = (double)(0.01 * attitude->att_roll[i]);
				att_pitch[i] = (double)(0.01 * attitude->att_pitch[i]);
			}
			mb_attint_nadd(verbose, mbio_ptr, attitude->att_ndata, att_time_d, att_heave, att_roll, att_pitch, error);
		}

		/* else this record is not from the active vru sensor
		    - make sure the attitude_source does not point to this sensor */
		else if (mb_io_ptr->attitude_source == store->kind) {
			if (store->kind == MB_DATA_ATTITUDE)
				mb_io_ptr->attitude_source = MB_DATA_ATTITUDE1;
			else
				mb_io_ptr->attitude_source = MB_DATA_ATTITUDE;
		}
	}

	/* save netattitude if "active" attitude data
	    - note that the mb_io_ptr->attitude_source value will be set dynamically
	        to reflect the actual attitude source used in realtime
	        by the sonar.
	    - attitude records from the sensors are
	        set at kinds MB_DATA_ATTITUDE (serial port 1),
	        MB_DATA_ATTITUDE1 (serial port 2),
	        or MB_DATA_ATTITUDE2 (network attitude). */
	if (status == MB_SUCCESS && store->type == EM3_NETATTITUDE) {
		/* save for interpolation if this record comes
		    from the active attitude sensor */
		if ((netattitude->nat_sensordescriptor & 14) == 0) {
			/* set the attitude_source */
			mb_io_ptr->attitude_source = store->kind;

			/* get attitude time */
			time_i[0] = netattitude->nat_date / 10000;
			time_i[1] = (netattitude->nat_date % 10000) / 100;
			time_i[2] = netattitude->nat_date % 100;
			time_i[3] = netattitude->nat_msec / 3600000;
			time_i[4] = (netattitude->nat_msec % 3600000) / 60000;
			time_i[5] = (netattitude->nat_msec % 60000) / 1000;
			time_i[6] = (netattitude->nat_msec % 1000) * 1000;
			mb_get_time(verbose, time_i, &atime_d);

			/* add latest attitude samples */
			netattitude->nat_ndata = MIN(netattitude->nat_ndata, MBSYS_SIMRAD3_MAXATTITUDE);
			for (int i = 0; i < netattitude->nat_ndata; i++) {
				att_time_d[i] = (double)(atime_d + 0.001 * netattitude->nat_time[i]);
				att_heave[i] = (double)(0.01 * netattitude->nat_heave[i]);
				att_roll[i] = (double)(0.01 * netattitude->nat_roll[i]);
				att_pitch[i] = (double)(0.01 * netattitude->nat_pitch[i]);
			}
			mb_attint_nadd(verbose, mbio_ptr, netattitude->nat_ndata, att_time_d, att_heave, att_roll, att_pitch, error);
		}

		/* else this record is not from the active vru sensor
		    - make sure the attitude_source does not point to this sensor */
		else if (mb_io_ptr->attitude_source == store->kind) {
			mb_io_ptr->attitude_source = MB_DATA_ATTITUDE;
		}
	}

	/* save sonar depth value if survey data */
	if (status == MB_SUCCESS && store->kind == MB_DATA_HEIGHT) {
		time_i[0] = store->hgt_date / 10000;
		time_i[1] = (store->hgt_date % 10000) / 100;
		time_i[2] = store->hgt_date % 100;
		time_i[3] = store->hgt_msec / 3600000;
		time_i[4] = (store->hgt_msec % 3600000) / 60000;
		time_i[5] = (store->hgt_msec % 60000) / 1000;
		time_i[6] = (store->hgt_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &btime_d);
		mb_depint_add(verbose, mbio_ptr, btime_d, (0.01 * ((double)store->hgt_height)), error);
	}

	/* interpolate attitude data into navigation records */
	if (status == MB_SUCCESS && (store->kind == MB_DATA_NAV || store->kind == MB_DATA_NAV1 || store->kind == MB_DATA_NAV2 ||
	                             store->kind == MB_DATA_NAV3)) {
		/* get nav time */
		time_i[0] = store->pos_date / 10000;
		time_i[1] = (store->pos_date % 10000) / 100;
		time_i[2] = store->pos_date % 100;
		time_i[3] = store->pos_msec / 3600000;
		time_i[4] = (store->pos_msec % 3600000) / 60000;
		time_i[5] = (store->pos_msec % 60000) / 1000;
		time_i[6] = (store->pos_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ntime_d);

		/* interpolate from saved attitude */
		mb_attint_interp(verbose, mbio_ptr, ntime_d, &heave, &roll, &pitch, &interp_error);
		store->pos_roll = (int)rint(roll / 0.01);
		store->pos_pitch = (int)rint(pitch / 0.01);
		store->pos_heave = (int)rint(heave / 0.01);
	}

	/* if no sidescan read then zero sidescan data */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && !ping->png_ss_read) {
		status = mbsys_simrad3_zero_ss(verbose, store_ptr, error);
	}

	/* else check that bath and sidescan data record time stamps
	   match for survey data - we can have bath without
	   sidescan but not sidescan without bath */
	else if (status == MB_SUCCESS && store->kind == MB_DATA_DATA) {
		
		/* make sure bathymetry, raw beam and snippet records have the same timestamp */
		if (ping->png_count == ping->png_raw_count && (ping->png_date != ping->png_raw_date || ping->png_msec != ping->png_raw_msec)) {
			ping->png_raw_date = ping->png_date;
			ping->png_raw_msec = ping->png_msec;
		}
		if (ping->png_count == ping->png_ss_count && (ping->png_date != ping->png_ss_date || ping->png_msec != ping->png_ss_msec)) {
			ping->png_ss_date = ping->png_date;
			ping->png_ss_msec = ping->png_msec;
		}

		/* check for ping count match - if not
		   then zero sidescan,  if ok then
		   check that beam numbers are the same */
		if (ping->png_ss_date == 0 || ping->png_nbeams_ss == 0) {
			status = mbsys_simrad3_zero_ss(verbose, store_ptr, error);
		}
		else if (ping->png_count != ping->png_ss_count) {
			if (verbose > 0) {
				time_i[0] = ping->png_date / 10000;
				time_i[1] = (ping->png_date % 10000) / 100;
				time_i[2] = ping->png_date % 100;
				time_i[3] = ping->png_msec / 3600000;
				time_i[4] = (ping->png_msec % 3600000) / 60000;
				time_i[5] = (ping->png_msec % 60000) / 1000;
				time_i[6] = (ping->png_msec % 1000) * 1000;
				fprintf(stderr, "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan zeroed, png_count:%d != png_ss_count:%d\n",
				        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], 
						ping->png_count, ping->png_ss_count);
			}
			status = mbsys_simrad3_zero_ss(verbose, store_ptr, error);
		}
		else {
			/* check for some indicators of broken records */
			if (ping->png_nbeams < ping->png_nbeams_ss || ping->png_nbeams > ping->png_nbeams_ss + 1) {
				if (verbose > 1)
					fprintf(
					    stderr,
					    "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: num bath beams != num ss beams: %d %d\n",
					    __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
					    ping->png_nbeams, ping->png_nbeams_ss);
			}
		}
	}

	// transmit and receive array offsets
	// double tx_x, tx_y, tx_z;
	double tx_h, tx_r, tx_p;

	// double rx_x, rx_y, rx_z;
	double rx_h, rx_r, rx_p;
	/* add some calculated data to survey records */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA) {
		/* get transducer offsets */
		if (store->par_stc == 0) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s2x;
			// rx_y = store->par_s2y;
			// rx_z = store->par_s2z;
			rx_h = store->par_s2h;
			rx_r = store->par_s2r;
			rx_p = store->par_s2p;
		}
		else if (store->par_stc == 1) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s1x;
			// rx_y = store->par_s1y;
			// rx_z = store->par_s1z;
			rx_h = store->par_s1h;
			rx_r = store->par_s1r;
			rx_p = store->par_s1p;
		}
		else if (store->par_stc == 2 && ping->png_serial == store->par_serial_1) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s1x;
			// rx_y = store->par_s1y;
			// rx_z = store->par_s1z;
			rx_h = store->par_s1h;
			rx_r = store->par_s1r;
			rx_p = store->par_s1p;
		}
		else if (store->par_stc == 2 && ping->png_serial == store->par_serial_2) {
			// tx_x = store->par_s2x;
			// tx_y = store->par_s2y;
			// tx_z = store->par_s2z;
			tx_h = store->par_s2h;
			tx_r = store->par_s2r;
			tx_p = store->par_s2p;
			// rx_x = store->par_s2x;
			// rx_y = store->par_s2y;
			// rx_z = store->par_s2z;
			rx_h = store->par_s2h;
			rx_r = store->par_s2r;
			rx_p = store->par_s2p;
		}
		else if (store->par_stc == 3 && ping->png_serial == store->par_serial_1) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s2x;
			// rx_y = store->par_s2y;
			// rx_z = store->par_s2z;
			rx_h = store->par_s2h;
			rx_r = store->par_s2r;
			rx_p = store->par_s2p;
		}
		else if (store->par_stc == 3 && ping->png_serial == store->par_serial_2) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s3x;
			// rx_y = store->par_s3y;
			// rx_z = store->par_s3z;
			rx_h = store->par_s3h;
			rx_r = store->par_s3r;
			rx_p = store->par_s3p;
		}
		else if (store->par_stc == 4 && ping->png_serial == store->par_serial_1) {
			// tx_x = store->par_s0x;
			// tx_y = store->par_s0y;
			// tx_z = store->par_s0z;
			tx_h = store->par_s0h;
			tx_r = store->par_s0r;
			tx_p = store->par_s0p;
			// rx_x = store->par_s2x;
			// rx_y = store->par_s2y;
			// rx_z = store->par_s2z;
			rx_h = store->par_s2h;
			rx_r = store->par_s2r;
			rx_p = store->par_s2p;
		}
		else if (store->par_stc == 4 && ping->png_serial == store->par_serial_2) {
			// tx_x = store->par_s1x;
			// tx_y = store->par_s1y;
			// tx_z = store->par_s1z;
			tx_h = store->par_s1h;
			tx_r = store->par_s1r;
			tx_p = store->par_s1p;
			// rx_x = store->par_s3x;
			// rx_y = store->par_s3y;
			// rx_z = store->par_s3z;
			rx_h = store->par_s3h;
			rx_r = store->par_s3r;
			rx_p = store->par_s3p;
		}

		// position sensor offsets
		// double position_off_x, position_off_y, position_off_z;

		/* get active sensor offsets */

		if (store->par_aps == 0) {
			// position_off_x = store->par_p1x;
			// position_off_y = store->par_p1y;
			// position_off_z = store->par_p1z;
		}
		else if (store->par_aps == 1) {
			// position_off_x = store->par_p2x;
			// position_off_y = store->par_p2y;
			// position_off_z = store->par_p2z;
		}
		else if (store->par_aps == 2) {
			// position_off_x = store->par_p3x;
			// position_off_y = store->par_p3y;
			// position_off_z = store->par_p3z;
		}
		/* roll and pitch sensor offsets */
		// double rollpitch_off_x, rollpitch_off_y, rollpitch_off_z, rollpitch_off_h, rollpitch_off_r, rollpitch_off_p;
		if (store->par_aro == 2) {
			// rollpitch_off_x = store->par_msx;
			// rollpitch_off_y = store->par_msy;
			// rollpitch_off_z = store->par_msz;
			// rollpitch_off_h = store->par_msg;
			// rollpitch_off_r = store->par_msr;
			// rollpitch_off_p = store->par_msp;
		}
		else if (store->par_aro == 3) {
			// rollpitch_off_x = store->par_nsx;
			// rollpitch_off_y = store->par_nsy;
			// rollpitch_off_z = store->par_nsz;
			// rollpitch_off_h = store->par_nsg;
			// rollpitch_off_r = store->par_nsr;
			// rollpitch_off_p = store->par_nsp;
		}
		/* heave sensor offsets */
		// double heave_off_x, heave_off_y, heave_off_z, heave_off_h, heave_off_r, heave_off_p;
		if (store->par_ahe == 2) {
			// heave_off_x = store->par_msx;
			// heave_off_y = store->par_msy;
			// heave_off_z = store->par_msz;
			// heave_off_h = store->par_msg;
			// heave_off_r = store->par_msr;
			// heave_off_p = store->par_msp;
		}
		else if (store->par_ahe == 3) {
			// heave_off_x = store->par_nsx;
			// heave_off_y = store->par_nsy;
			// heave_off_z = store->par_nsz;
			// heave_off_h = store->par_nsg;
			// heave_off_r = store->par_nsr;
			// heave_off_p = store->par_nsp;
		}
		/* heading sensor offset */
		// double heading_off_x, heading_off_y, heading_off_z, heading_off_h, heading_off_r, heading_off_p;
		if (store->par_ahs == 0 || store->par_ahs == 4) {
			// heading_off_x = store->par_p3x;
			// heading_off_y = store->par_p3y;
			// heading_off_z = store->par_p3z;
			// heading_off_h = store->par_gcg;
			// heading_off_r = 0.0;
			// heading_off_p = 0.0;
		}
		else if (store->par_ahs == 1) {
			// heading_off_x = store->par_p1x;
			// heading_off_y = store->par_p1y;
			// heading_off_z = store->par_p1z;
			// heading_off_h = store->par_gcg;
			// heading_off_r = 0.0;
			// heading_off_p = 0.0;
		}
		else if (store->par_ahs == 2) {
			// heading_off_x = store->par_msx;
			// heading_off_y = store->par_msy;
			// heading_off_z = store->par_msz;
			// heading_off_h = store->par_msg + store->par_gcg;
			// heading_off_r = store->par_msr;
			// heading_off_p = store->par_msp;
		}
		else if (store->par_ahs == 3 && store->par_nsz != 0.0) {
			// heading_off_x = store->par_nsx;
			// heading_off_y = store->par_nsy;
			// heading_off_z = store->par_nsz;
			// heading_off_h = store->par_nsg + store->par_gcg;
			// heading_off_r = store->par_nsr;
			// heading_off_p = store->par_nsp;
		}
		else if (store->par_ahs == 3) {
			// heading_off_x = store->par_p2x;
			// heading_off_y = store->par_p2y;
			// heading_off_z = store->par_p2z;
			// heading_off_h = store->par_gcg;
			// heading_off_r = 0.0;
			// heading_off_p = 0.0;
		}

		/* get ping time */
		time_i[0] = ping->png_date / 10000;
		time_i[1] = (ping->png_date % 10000) / 100;
		time_i[2] = ping->png_date % 100;
		time_i[3] = ping->png_msec / 3600000;
		time_i[4] = (ping->png_msec % 3600000) / 60000;
		time_i[5] = (ping->png_msec % 60000) / 1000;
		time_i[6] = (ping->png_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &ptime_d);

		/* interpolate from saved nav */
		if (store->pos_speed == 0 || store->pos_speed == EM3_INVALID_SHORT)
			rawspeed = 0.0;
		else
			rawspeed = 0.036 * store->pos_speed;
		pheading = 0.01 * ping->png_heading;
		mb_navint_interp(verbose, mbio_ptr, ptime_d, pheading, rawspeed, &plon, &plat, &pspeed,  &interp_error);
		if (plon == 0.0 && plat == 0.0) {
			ping->png_longitude = (int)EM3_INVALID_INT;
			ping->png_latitude = (int)EM3_INVALID_INT;
		}
		else {
			ping->png_longitude = (int)rint(10000000 * plon);
			ping->png_latitude = (int)rint(20000000 * plat);
		}
		ping->png_speed = (int)rint(pspeed / 0.036);

		/* interpolate from saved attitude */
		mb_attint_interp(verbose, mbio_ptr, ptime_d, &heave, &roll, &pitch, &interp_error);
		ping->png_roll = (int)rint(roll / 0.01);
		ping->png_pitch = (int)rint(pitch / 0.01);
		ping->png_heave = (int)rint(heave / 0.01);

		/* make first cut at angles */
		/* calculate corrected ranges, angles, and bathymetry */
		// const double theta_nadir = 90.0;
		// int inadir = 0;
		for (int i = 0; i < ping->png_nbeams; i++) {
			/* calculate time of transmit and receive */
			transmit_time_d = ptime_d + (double)ping->png_raw_txoffset[ping->png_raw_rxsector[i]];
			receive_time_d = transmit_time_d + ping->png_raw_rxrange[i];

			/* get attitude and heave at ping and receive time */
			mb_hedint_interp(verbose, mbio_ptr, transmit_time_d, &transmit_heading, &interp_error);
			mb_attint_interp(verbose, mbio_ptr, transmit_time_d, &transmit_heave, &transmit_roll, &transmit_pitch, &interp_error);
			mb_hedint_interp(verbose, mbio_ptr, receive_time_d, &receive_heading, &interp_error);
			mb_attint_interp(verbose, mbio_ptr, receive_time_d, &receive_heave, &receive_roll, &receive_pitch, &interp_error);

			/* alongtrack offset distance */
			// const double transmit_alongtrack =
			//     (0.01 * ((double)ping->png_speed)) * ((double)ping->png_raw_txoffset[ping->png_raw_rxsector[i]]);

			/* get corrected range */
			if (ping->png_ssv <= 0)
				ping->png_ssv = 150;
			// const double soundspeed = 0.1 * ((double)ping->png_ssv);
			ping->png_range[i] = ping->png_raw_rxrange[i];
			/* ping->png_bheave[i] is the difference between the heave at the ping timestamp time that is factored
			 * into the ping->png_xducer_depth value and the average heave at the sector transmit time and the beam receive time
			 */
			ping->png_bheave[i] = 0.5 * (receive_heave + transmit_heave) - heave;

			/* calculate beam angles for raytracing using Jon Beaudoin's code based on:
			    Beaudoin, J., Hughes Clarke, J., and Bartlett, J. Application of
			    Surface Sound Speed Measurements in Post-Processing for Multi-Sector
			    Multibeam Echosounders : International Hydrographic Review, v.5, no.3,
			    p.26-31.
			    (http://www.omg.unb.ca/omg/papers/beaudoin_IHR_nov2004.pdf).
			   note complexity if transducer arrays are reverse mounted, as determined
			   by a mount heading angle of about 180 degrees rather than about 0 degrees.
			   If a receive array or a transmit array are reverse mounted then:
			    1) subtract 180 from the heading mount angle of the array
			    2) flip the sign of the pitch and roll mount offsets of the array
			    3) flip the sign of the beam steering angle from that array
			        (reverse TX means flip sign of TX steer, reverse RX
			        means flip sign of RX steer) */
			if (tx_h <= 90.0 || tx_h >= 270.0) {
				tx_align.roll = tx_r;
				tx_align.pitch = tx_p;
				tx_align.heading = tx_h;
				tx_steer = (0.01 * (double)ping->png_raw_txtiltangle[ping->png_raw_rxsector[i]]);
			}
			else {
				tx_align.roll = -tx_r;
				tx_align.pitch = -tx_p;
				tx_align.heading = tx_h - 180.0;
				tx_steer = -(0.01 * (double)ping->png_raw_txtiltangle[ping->png_raw_rxsector[i]]);
			}
			tx_orientation.roll = transmit_roll;
			tx_orientation.pitch = transmit_pitch;
			tx_orientation.heading = transmit_heading;
			if (rx_h <= 90.0 || rx_h >= 270.0) {
				rx_align.roll = rx_r;
				rx_align.pitch = rx_p;
				rx_align.heading = rx_h;
				rx_steer = (0.01 * (double)ping->png_raw_rxpointangle[i]);
			}
			else {
				rx_align.roll = -rx_r;
				rx_align.pitch = -rx_p;
				rx_align.heading = rx_h - 180.0;
				rx_steer = -(0.01 * (double)ping->png_raw_rxpointangle[i]);
			}
			rx_orientation.roll = receive_roll;
			rx_orientation.pitch = receive_pitch;
			rx_orientation.heading = receive_heading;
			reference_heading = pheading;

			status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
			                     reference_heading, &beamAzimuth, &beamDepression, error);
			ping->png_depression[i] = 90.0 - beamDepression;
			ping->png_azimuth[i] = 90.0 + beamAzimuth;
			if (ping->png_azimuth[i] < 0.0)
				ping->png_azimuth[i] += 360.0;

			/* calculate beamflag */
			detection_mask = (mb_u_char)ping->png_raw_rxdetection[i];
			if (store->sonar == MBSYS_SIMRAD3_M3 && (ping->png_detection[i] & 128) == 128) {
				ping->png_beamflag[i] = MB_FLAG_NULL;
				ping->png_raw_rxdetection[i] = ping->png_raw_rxdetection[i] | 128;
			}
			else if ((detection_mask & 128) == 128) {
				if ((detection_mask & 15) == 0) {
					ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
				}
				else if ((detection_mask & 15) == 1) {
					ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_INTERPOLATE;
				}
				else if ((detection_mask & 15) == 2) {
					ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_INTERPOLATE;
				}
				else if ((detection_mask & 15) == 3) {
					ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
					;
				}
				else if ((detection_mask & 15) == 4) {
					ping->png_beamflag[i] = MB_FLAG_NULL;
				}
			}
			else if (ping->png_clean[i] != 0) {
				ping->png_beamflag[i] = MB_FLAG_FLAG + MB_FLAG_SONAR;
			}
			else {
				ping->png_beamflag[i] = MB_FLAG_NONE;
			}

			/* check for NaN value */
			if (isnan(ping->png_depth[i])
          || isnan(ping->png_acrosstrack[i])
          || isnan(ping->png_alongtrack[i])) {
				ping->png_beamflag[i] = MB_FLAG_NULL;
			}
		}

		/* generate processed sidescan */
		ping->png_pixel_size = 0;
		ping->png_pixels_ss = 0;
    status = mbsys_simrad3_makess(verbose, mbio_ptr, store_ptr, false, pixel_size, false, swath_width, 1, error);
	}

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
int mbr_em710raw_wr_start(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[MBSYS_SIMRAD3_BUFFER_SIZE], *buff;
	int buff_len;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	char *comma_ptr;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       par_date:        %d\n", store->par_date);
		fprintf(stderr, "dbg5       par_msec:        %d\n", store->par_msec);
		fprintf(stderr, "dbg5       par_line_num:    %d\n", store->par_line_num);
		fprintf(stderr, "dbg5       par_serial_1:    %d\n", store->par_serial_1);
		fprintf(stderr, "dbg5       par_serial_2:    %d\n", store->par_serial_2);
		fprintf(stderr, "dbg5       par_wlz:         %f\n", store->par_wlz);
		fprintf(stderr, "dbg5       par_smh:         %d\n", store->par_smh);
		fprintf(stderr, "dbg5       par_hun:         %d\n", store->par_hun);
		fprintf(stderr, "dbg5       par_hut:         %f\n", store->par_hut);
		fprintf(stderr, "dbg5       par_txs:         %d\n", store->par_txs);
		fprintf(stderr, "dbg5       par_t2x:         %d\n", store->par_t2x);
		fprintf(stderr, "dbg5       par_r1s:         %d\n", store->par_r1s);
		fprintf(stderr, "dbg5       par_r2s:         %d\n", store->par_r2s);
		fprintf(stderr, "dbg5       par_stc:         %d\n", store->par_stc);
		fprintf(stderr, "dbg5       par_s0z:         %f\n", store->par_s0z);
		fprintf(stderr, "dbg5       par_s0x:         %f\n", store->par_s0x);
		fprintf(stderr, "dbg5       par_s0y:         %f\n", store->par_s0y);
		fprintf(stderr, "dbg5       par_s0h:         %f\n", store->par_s0h);
		fprintf(stderr, "dbg5       par_s0r:         %f\n", store->par_s0r);
		fprintf(stderr, "dbg5       par_s0p:         %f\n", store->par_s0p);
		fprintf(stderr, "dbg5       par_s1z:         %f\n", store->par_s1z);
		fprintf(stderr, "dbg5       par_s1x:         %f\n", store->par_s1x);
		fprintf(stderr, "dbg5       par_s1y:         %f\n", store->par_s1y);
		fprintf(stderr, "dbg5       par_s1h:         %f\n", store->par_s1h);
		fprintf(stderr, "dbg5       par_s1r:         %f\n", store->par_s1r);
		fprintf(stderr, "dbg5       par_s1p:         %f\n", store->par_s1p);
		fprintf(stderr, "dbg5       par_s1n:         %d\n", store->par_s1n);
		fprintf(stderr, "dbg5       par_s2z:         %f\n", store->par_s2z);
		fprintf(stderr, "dbg5       par_s2x:         %f\n", store->par_s2x);
		fprintf(stderr, "dbg5       par_s2y:         %f\n", store->par_s2y);
		fprintf(stderr, "dbg5       par_s2h:         %f\n", store->par_s2h);
		fprintf(stderr, "dbg5       par_s2r:         %f\n", store->par_s2r);
		fprintf(stderr, "dbg5       par_s2p:         %f\n", store->par_s2p);
		fprintf(stderr, "dbg5       par_s2n:         %d\n", store->par_s2n);
		fprintf(stderr, "dbg5       par_s3z:         %f\n", store->par_s3z);
		fprintf(stderr, "dbg5       par_s3x:         %f\n", store->par_s3x);
		fprintf(stderr, "dbg5       par_s3y:         %f\n", store->par_s3y);
		fprintf(stderr, "dbg5       par_s3h:         %f\n", store->par_s3h);
		fprintf(stderr, "dbg5       par_s3r:         %f\n", store->par_s3r);
		fprintf(stderr, "dbg5       par_s3p:         %f\n", store->par_s3p);
		fprintf(stderr, "dbg5       par_s1s:         %d\n", store->par_s1s);
		fprintf(stderr, "dbg5       par_s2s:         %d\n", store->par_s2s);
		fprintf(stderr, "dbg5       par_go1:         %f\n", store->par_go1);
		fprintf(stderr, "dbg5       par_go2:         %f\n", store->par_go2);
		fprintf(stderr, "dbg5       par_obo:         %f\n", store->par_obo);
		fprintf(stderr, "dbg5       par_fgd:         %f\n", store->par_fgd);
		fprintf(stderr, "dbg5       par_tsv:         %s\n", store->par_tsv);
		fprintf(stderr, "dbg5       par_rsv:         %s\n", store->par_rsv);
		fprintf(stderr, "dbg5       par_bsv:         %s\n", store->par_bsv);
		fprintf(stderr, "dbg5       par_psv:         %s\n", store->par_psv);
		fprintf(stderr, "dbg5       par_dds:         %s\n", store->par_dds);
		fprintf(stderr, "dbg5       par_osv:         %s\n", store->par_osv);
		fprintf(stderr, "dbg5       par_dsv:         %s\n", store->par_dsv);
		fprintf(stderr, "dbg5       par_dsx:         %f\n", store->par_dsx);
		fprintf(stderr, "dbg5       par_dsy:         %f\n", store->par_dsy);
		fprintf(stderr, "dbg5       par_dsz:         %f\n", store->par_dsz);
		fprintf(stderr, "dbg5       par_dsd:         %d\n", store->par_dsd);
		fprintf(stderr, "dbg5       par_dso:         %f\n", store->par_dso);
		fprintf(stderr, "dbg5       par_dsf:         %f\n", store->par_dsf);
		fprintf(stderr, "dbg5       par_dsh:         %c%c\n", store->par_dsh[0], store->par_dsh[1]);
		fprintf(stderr, "dbg5       par_aps:         %d\n", store->par_aps);
		fprintf(stderr, "dbg5       par_p1q:         %d\n", store->par_p1q);
		fprintf(stderr, "dbg5       par_p1m:         %d\n", store->par_p1m);
		fprintf(stderr, "dbg5       par_p1t:         %d\n", store->par_p1t);
		fprintf(stderr, "dbg5       par_p1z:         %f\n", store->par_p1z);
		fprintf(stderr, "dbg5       par_p1x:         %f\n", store->par_p1x);
		fprintf(stderr, "dbg5       par_p1y:         %f\n", store->par_p1y);
		fprintf(stderr, "dbg5       par_p1d:         %f\n", store->par_p1d);
		fprintf(stderr, "dbg5       par_p1g:         %s\n", store->par_p1g);
		fprintf(stderr, "dbg5       par_p2q:         %d\n", store->par_p2q);
		fprintf(stderr, "dbg5       par_p2m:         %d\n", store->par_p2m);
		fprintf(stderr, "dbg5       par_p2t:         %d\n", store->par_p2t);
		fprintf(stderr, "dbg5       par_p2z:         %f\n", store->par_p2z);
		fprintf(stderr, "dbg5       par_p2x:         %f\n", store->par_p2x);
		fprintf(stderr, "dbg5       par_p2y:         %f\n", store->par_p2y);
		fprintf(stderr, "dbg5       par_p2d:         %f\n", store->par_p2d);
		fprintf(stderr, "dbg5       par_p2g:         %s\n", store->par_p2g);
		fprintf(stderr, "dbg5       par_p3q:         %d\n", store->par_p3q);
		fprintf(stderr, "dbg5       par_p3m:         %d\n", store->par_p3m);
		fprintf(stderr, "dbg5       par_p3t:         %d\n", store->par_p3t);
		fprintf(stderr, "dbg5       par_p3z:         %f\n", store->par_p3z);
		fprintf(stderr, "dbg5       par_p3x:         %f\n", store->par_p3x);
		fprintf(stderr, "dbg5       par_p3y:         %f\n", store->par_p3y);
		fprintf(stderr, "dbg5       par_p3d:         %f\n", store->par_p3d);
		fprintf(stderr, "dbg5       par_p3g:         %s\n", store->par_p3g);
		fprintf(stderr, "dbg5       par_p3s:         %d\n", store->par_p3s);
		fprintf(stderr, "dbg5       par_msz:         %f\n", store->par_msz);
		fprintf(stderr, "dbg5       par_msx:         %f\n", store->par_msx);
		fprintf(stderr, "dbg5       par_msy:         %f\n", store->par_msy);
		fprintf(stderr, "dbg5       par_mrp:         %c%c\n", store->par_mrp[0], store->par_mrp[1]);
		fprintf(stderr, "dbg5       par_msd:         %f\n", store->par_msd);
		fprintf(stderr, "dbg5       par_msr:         %f\n", store->par_msr);
		fprintf(stderr, "dbg5       par_msp:         %f\n", store->par_msp);
		fprintf(stderr, "dbg5       par_msg:         %f\n", store->par_msg);
		fprintf(stderr, "dbg5       par_nsz:         %f\n", store->par_nsz);
		fprintf(stderr, "dbg5       par_nsx:         %f\n", store->par_nsx);
		fprintf(stderr, "dbg5       par_nsy:         %f\n", store->par_nsy);
		fprintf(stderr, "dbg5       par_nrp:         %c%c\n", store->par_nrp[0], store->par_nrp[1]);
		fprintf(stderr, "dbg5       par_nsd:         %f\n", store->par_nsd);
		fprintf(stderr, "dbg5       par_nsr:         %f\n", store->par_nsr);
		fprintf(stderr, "dbg5       par_nsp:         %f\n", store->par_nsp);
		fprintf(stderr, "dbg5       par_nsg:         %f\n", store->par_nsg);
		fprintf(stderr, "dbg5       par_gcg:         %f\n", store->par_gcg);
		fprintf(stderr, "dbg5       par_mas:         %f\n", store->par_mas);
		fprintf(stderr, "dbg5       par_shc:         %d\n", store->par_shc);
		fprintf(stderr, "dbg5       par_pps:         %d\n", store->par_pps);
		fprintf(stderr, "dbg5       par_cls:         %d\n", store->par_cls);
		fprintf(stderr, "dbg5       par_clo:         %d\n", store->par_clo);
		fprintf(stderr, "dbg5       par_vsn:         %d\n", store->par_vsn);
		fprintf(stderr, "dbg5       par_vsu:         %d\n", store->par_vsu);
		fprintf(stderr, "dbg5       par_vse:         %d\n", store->par_vse);
		fprintf(stderr, "dbg5       par_vtu:         %d\n", store->par_vtu);
		fprintf(stderr, "dbg5       par_vte:         %d\n", store->par_vte);
		fprintf(stderr, "dbg5       par_aro:         %d\n", store->par_aro);
		fprintf(stderr, "dbg5       par_ahe:         %d\n", store->par_ahe);
		fprintf(stderr, "dbg5       par_ahs:         %d\n", store->par_ahs);
		fprintf(stderr, "dbg5       par_vsi:         %s\n", store->par_vsi);
		fprintf(stderr, "dbg5       par_vsm:         %s\n", store->par_vsm);
		fprintf(stderr, "dbg5       par_mca1:        %s\n", store->par_mca1);
		fprintf(stderr, "dbg5       par_mcu1:        %d\n", store->par_mcu1);
		fprintf(stderr, "dbg5       par_mci1:        %s\n", store->par_mci1);
		fprintf(stderr, "dbg5       par_mcp1:        %d\n", store->par_mcp1);
		fprintf(stderr, "dbg5       par_mca2:        %s\n", store->par_mca2);
		fprintf(stderr, "dbg5       par_mcu2:        %d\n", store->par_mcu2);
		fprintf(stderr, "dbg5       par_mci2:        %s\n", store->par_mci2);
		fprintf(stderr, "dbg5       par_mcp2:        %d\n", store->par_mcp2);
		fprintf(stderr, "dbg5       par_mca3:        %s\n", store->par_mca3);
		fprintf(stderr, "dbg5       par_mcu3:        %d\n", store->par_mcu3);
		fprintf(stderr, "dbg5       par_mci3:        %s\n", store->par_mci3);
		fprintf(stderr, "dbg5       par_mcp3:        %d\n", store->par_mcp3);
		fprintf(stderr, "dbg5       par_mca4:        %s\n", store->par_mca4);
		fprintf(stderr, "dbg5       par_mcu4:        %d\n", store->par_mcu4);
		fprintf(stderr, "dbg5       par_mci4:        %s\n", store->par_mci4);
		fprintf(stderr, "dbg5       par_mcp4:        %d\n", store->par_mcp4);
		fprintf(stderr, "dbg5       par_snl:         %d\n", store->par_snl);
		fprintf(stderr, "dbg5       par_cpr:         %s\n", store->par_cpr);
		fprintf(stderr, "dbg5       par_rop:         %s\n", store->par_rop);
		fprintf(stderr, "dbg5       par_sid:         %s\n", store->par_sid);
		fprintf(stderr, "dbg5       par_rfn:         %s\n", store->par_rfn);
		fprintf(stderr, "dbg5       par_pll:         %s\n", store->par_pll);
		fprintf(stderr, "dbg5       par_com:         %s\n", store->par_com);
	}

	/* zero checksum */
	checksum = 0;

	/* if data type not set - use start */
	if (store->type == EM3_NONE)
		store->type = EM3_START;

	/* if sonar not set use EM710 */
	if (store->sonar == 0)
		store->sonar = MBSYS_SIMRAD3_EM710;

	/* set up start of output buffer - we handle this
	   record differently because of the ascii data */
	memset(line, 0, MBSYS_SIMRAD3_BUFFER_SIZE);

	/* put binary header data into buffer */
	line[4] = EM3_START_BYTE;
  if (store->type == EM3_START)
	  line[5] = EM3_ID_START;
  else
	  line[5] = EM3_ID_STOP;
	mb_put_binary_short(swap, (unsigned short)store->sonar, (void *)&line[6]);
	mb_put_binary_int(swap, (int)store->par_date, (void *)&line[8]);
	mb_put_binary_int(swap, (int)store->par_msec, (void *)&line[12]);
	mb_put_binary_short(swap, (unsigned short)store->par_line_num, (void *)&line[16]);
	mb_put_binary_short(swap, (unsigned short)store->par_serial_1, (void *)&line[18]);
	mb_put_binary_short(swap, (unsigned short)store->par_serial_2, (void *)&line[20]);

	/* construct ascii parameter buffer */
	buff = &line[22];
	buff_len = 0;

	if (store->par_wlz != 0.0) {
		sprintf(&buff[buff_len], "WLZ=%.3f,", store->par_wlz);
		buff_len = strlen(buff);
	}
	if (store->par_smh != 0) {
		sprintf(&buff[buff_len], "SMH=%d,", store->par_smh);
		buff_len = strlen(buff);
	}
	if (store->par_hut != 0.0) {
		sprintf(&buff[buff_len], "HUN=%d,", store->par_hun);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "HUT=%f,", store->par_hut);
		buff_len = strlen(buff);
	}
	if (store->par_txs != 0) {
		sprintf(&buff[buff_len], "TXS=%d,", store->par_txs);
		buff_len = strlen(buff);
	}
	if (store->par_t2x != 0) {
		sprintf(&buff[buff_len], "T2X=%d,", store->par_t2x);
		buff_len = strlen(buff);
	}
	if (store->par_r1s != 0) {
		sprintf(&buff[buff_len], "R1S=%d,", store->par_r1s);
		buff_len = strlen(buff);
	}
	if (store->par_r2s != 0) {
		sprintf(&buff[buff_len], "R2S=%d,", store->par_r2s);
		buff_len = strlen(buff);
	}
	sprintf(&buff[buff_len], "STC=%d,", store->par_stc);
	buff_len = strlen(buff);
	if (store->par_stc == 4) {
		sprintf(&buff[buff_len], "S0Z=%.3f,", store->par_s0z);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S0X=%.3f,", store->par_s0x);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S0Y=%.3f,", store->par_s0y);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S0H=%.3f,", store->par_s0h);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S0R=%.3f,", store->par_s0r);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S0P=%.3f,", store->par_s0p);
		buff_len = strlen(buff);
	}
	sprintf(&buff[buff_len], "S1Z=%.3f,", store->par_s1z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1X=%.3f,", store->par_s1x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1Y=%.3f,", store->par_s1y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1H=%.3f,", store->par_s1h);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1R=%.3f,", store->par_s1r);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1P=%.3f,", store->par_s1p);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1S=%d,", store->par_s1s);
	buff_len = strlen(buff);
	if (store->par_stc != 1) {
		sprintf(&buff[buff_len], "S2Z=%.3f,", store->par_s2z);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S2X=%.3f,", store->par_s2x);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S2Y=%.3f,", store->par_s2y);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S2H=%.3f,", store->par_s2h);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S2R=%.3f,", store->par_s2r);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S2P=%.3f,", store->par_s2p);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S2S=%d,", store->par_s2s);
		buff_len = strlen(buff);
	}
	if (store->par_stc >= 3) {
		sprintf(&buff[buff_len], "S3Z=%.3f,", store->par_s3z);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S3X=%.3f,", store->par_s3x);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S3Y=%.3f,", store->par_s3y);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S3H=%.3f,", store->par_s3h);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S3R=%.3f,", store->par_s3r);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "S3P=%.3f,", store->par_s3p);
		buff_len = strlen(buff);
	}
	if (store->par_go1 != 0.0) {
		sprintf(&buff[buff_len], "GO1=%.3f,", store->par_go1);
		buff_len = strlen(buff);
	}
	if (store->par_go2 != 0.0) {
		sprintf(&buff[buff_len], "GO2=%.3f,", store->par_go1);
		buff_len = strlen(buff);
	}
	if (store->par_obo != 0.0) {
		sprintf(&buff[buff_len], "OBO=%.3f,", store->par_obo);
		buff_len = strlen(buff);
	}
	if (store->par_fgd != 0.0) {
		sprintf(&buff[buff_len], "FGD=%.3f,", store->par_fgd);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_tsv) > 0) {
		sprintf(&buff[buff_len], "TSV=%s,", store->par_tsv);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_rsv) > 0) {
		sprintf(&buff[buff_len], "RSV=%s,", store->par_rsv);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_bsv) > 0) {
		sprintf(&buff[buff_len], "BSV=%s,", store->par_bsv);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_psv) > 0) {
		sprintf(&buff[buff_len], "PSV=%s,", store->par_psv);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_dds) > 0) {
		sprintf(&buff[buff_len], "DDS=%s,", store->par_dds);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_osv) > 0) {
		sprintf(&buff[buff_len], "OSV=%s,", store->par_osv);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_dsv) > 0) {
		sprintf(&buff[buff_len], "DSV=%s,", store->par_dsv);
		buff_len = strlen(buff);
	}

	sprintf(&buff[buff_len], "DSX=%.6f,", store->par_dsx);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSY=%.6f,", store->par_dsy);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSZ=%.6f,", store->par_dsz);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSD=%d,", store->par_dsd);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSO=%.6f,", store->par_dso);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSF=%.6f,", store->par_dsf);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSH=%c%c,", store->par_dsh[0], store->par_dsh[1]);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "APS=%d,", store->par_aps);
	buff_len = strlen(buff);

	if (store->par_p1q) {
		sprintf(&buff[buff_len], "P1Q=%d,", store->par_p1q);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P1M=%d,", store->par_p1m);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P1T=%d,", store->par_p1t);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P1Z=%.3f,", store->par_p1z);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P1X=%.3f,", store->par_p1x);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P1Y=%.3f,", store->par_p1y);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P1D=%.1f,", store->par_p1d);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P1G=%s,", store->par_p1g);
		buff_len = strlen(buff);
	}
	if (store->par_p2q) {
		sprintf(&buff[buff_len], "P2Q=%d,", store->par_p2q);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P2M=%d,", store->par_p2m);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P2T=%d,", store->par_p2t);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P2Z=%.3f,", store->par_p2z);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P2X=%.3f,", store->par_p2x);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P2Y=%.3f,", store->par_p2y);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P2D=%.3f,", store->par_p2d);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P2G=%s,", store->par_p2g);
		buff_len = strlen(buff);
	}
	if (store->par_p3q) {
		sprintf(&buff[buff_len], "P3Q=%d,", store->par_p3q);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P3M=%d,", store->par_p3m);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P3T=%d,", store->par_p3t);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P3Z=%.3f,", store->par_p3z);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P3X=%.3f,", store->par_p3x);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P3Y=%.3f,", store->par_p3y);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P3D=%.3f,", store->par_p3d);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P3G=%s,", store->par_p3g);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "P3S=%d,", store->par_p3s);
		buff_len = strlen(buff);
	}

	sprintf(&buff[buff_len], "MSZ=%.3f,", store->par_msz);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSX=%.3f,", store->par_msx);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSY=%.3f,", store->par_msy);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MRP=%c%c,", store->par_mrp[0], store->par_mrp[1]);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSD=%.3f,", store->par_msd);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSR=%.3f,", store->par_msr);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSP=%.3f,", store->par_msp);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSG=%.3f,", store->par_msg);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "NSZ=%.3f,", store->par_nsz);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "NSX=%.3f,", store->par_nsx);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "NSY=%.3f,", store->par_nsy);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "NRP=%c%c,", store->par_nrp[0], store->par_nrp[1]);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "NSD=%.3f,", store->par_nsd);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "NSR=%.3f,", store->par_nsr);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "NSP=%.3f,", store->par_nsp);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "NSG=%.3f,", store->par_nsg);
	buff_len = strlen(buff);

	sprintf(&buff[buff_len], "GCG=%.3f,", store->par_gcg);
	buff_len = strlen(buff);
	if (store->par_mas != 0.0) {
		sprintf(&buff[buff_len], "MAS=%.3f,", store->par_mas);
		buff_len = strlen(buff);
	}
	sprintf(&buff[buff_len], "SHC=%d,", store->par_shc);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "PPS=%d,", store->par_pps);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "CLS=%d,", store->par_cls);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "CLO=%d,", store->par_clo);
	buff_len = strlen(buff);

	sprintf(&buff[buff_len], "VSN=%d,", store->par_vsn);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "VSU=%d,", store->par_vsu);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "VSE=%d,", store->par_vse);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "VTU=%d,", store->par_vtu);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "VTE=%d,", store->par_vte);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "ARO=%d,", store->par_aro);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "AHE=%d,", store->par_ahe);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "AHS=%d,", store->par_ahs);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "VSI=%s,", store->par_vsi);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "VSM=%s,", store->par_vsm);
	buff_len = strlen(buff);

	if (store->par_mcp1 > 0) {
		sprintf(&buff[buff_len], "MCA1=%s,", store->par_mca1);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCU1=%d,", store->par_mcu1);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCI1=%s,", store->par_mci1);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCP1=%d,", store->par_mcp1);
		buff_len = strlen(buff);
	}
	if (store->par_mcp2 > 0) {
		sprintf(&buff[buff_len], "MCA2=%s,", store->par_mca2);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCU2=%d,", store->par_mcu2);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCI2=%s,", store->par_mci2);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCP2=%d,", store->par_mcp2);
		buff_len = strlen(buff);
	}
	if (store->par_mcp3 > 0) {
		sprintf(&buff[buff_len], "MCA3=%s,", store->par_mca3);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCU3=%d,", store->par_mcu3);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCI3=%s,", store->par_mci3);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCP3=%d,", store->par_mcp3);
		buff_len = strlen(buff);
	}
	if (store->par_mcp4 > 0) {
		sprintf(&buff[buff_len], "MCA4=%s,", store->par_mca4);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCU4=%d,", store->par_mcu4);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCI4=%s,", store->par_mci4);
		buff_len = strlen(buff);
		sprintf(&buff[buff_len], "MCP4=%d,", store->par_mcp4);
		buff_len = strlen(buff);
	}
	sprintf(&buff[buff_len], "SNL=%d,", store->par_snl);
	buff_len = strlen(buff);

	if (strlen(store->par_cpr) > 0) {
		sprintf(&buff[buff_len], "CPR=%s,", store->par_cpr);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_rop) > 0) {
		sprintf(&buff[buff_len], "ROP=%s,", store->par_rop);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_sid) > 0) {
		sprintf(&buff[buff_len], "SID=%s,", store->par_sid);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_rfn) > 0) {
		sprintf(&buff[buff_len], "RFN=%s,", store->par_rfn);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_pll) > 0) {
		sprintf(&buff[buff_len], "PLL=%s,", store->par_pll);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_com) > 0) {
		/* replace commas (,) with caret (^) values to circumvent
		   the format's inability to store commas in comments */
		while ((comma_ptr = strchr(store->par_com, ',')) != NULL) {
			comma_ptr[0] = '^';
		}
		sprintf(&buff[buff_len], "COM=%s,", store->par_com);
		buff_len = strlen(buff);
	}
	buff[buff_len] = ',';
	buff_len++;
	if (buff_len % 2 == 0)
		buff_len++;

	/* put end of record in buffer */
	line[buff_len + 22] = EM3_END;

	/* get size of record */
	write_size = 25 + buff_len;
	mb_put_binary_int(swap, (int)(write_size - 4), (void *)&line[0]);

	/* compute checksum */
	uchar_ptr = (mb_u_char *)line;
	for (int j = 5; j < write_size - 3; j++)
		checksum += uchar_ptr[j];

	/* set checksum */
	mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[buff_len + 23]);

	/* finally write out the data */
	write_len = write_size;
	const int status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);

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
int mbr_em710raw_wr_puid(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[EM3_PU_ID_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:                %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:               %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                %d\n", store->msec);
		fprintf(stderr, "dbg5       pid_date:            %d\n", store->pid_date);
		fprintf(stderr, "dbg5       pid_msec:            %d\n", store->pid_msec);
		fprintf(stderr, "dbg5       pid_byte_order_flag: %d\n", store->pid_byte_order_flag);
		fprintf(stderr, "dbg5       pid_serial:          %d\n", store->pid_serial);
		fprintf(stderr, "dbg5       pid_udp_port_1:      %d\n", store->pid_udp_port_1);
		fprintf(stderr, "dbg5       pid_udp_port_2:      %d\n", store->pid_udp_port_2);
		fprintf(stderr, "dbg5       pid_udp_port_3:      %d\n", store->pid_udp_port_3);
		fprintf(stderr, "dbg5       pid_udp_port_4:      %d\n", store->pid_udp_port_4);
		fprintf(stderr, "dbg5       pid_pu_sw_version:   ");
    for (int i=0; i<16; i++) {
      fprintf(stderr, "%c", store->pid_pu_sw_version[i]);
    }
    fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       pid_bsp_sw_version:   ");
    for (int i=0; i<16; i++) {
      fprintf(stderr, "%c", store->pid_bsp_sw_version[i]);
    }
    fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       pid_head1_version:   ");
    for (int i=0; i<16; i++) {
      fprintf(stderr, "%c", store->pid_head1_version[i]);
    }
    fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       pid_head2_version:   ");
    for (int i=0; i<16; i++) {
      fprintf(stderr, "%c", store->pid_head2_version[i]);
    }
    fprintf(stderr, "\n");
		fprintf(stderr, "dbg5       pid_host_ip:         %d\n", store->pid_host_ip);
		fprintf(stderr, "dbg5       pid_tx_opening_angle:%d\n", store->pid_tx_opening_angle);
		fprintf(stderr, "dbg5       pid_rx_opening_angle:%d\n", store->pid_rx_opening_angle);
		fprintf(stderr, "dbg5       pid_spare:           ");
    for (int i=0; i<7; i++) {
      fprintf(stderr, "%c", store->pid_spare[i]);
    }
    fprintf(stderr, "\n");
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_PU_ID_SIZE), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);

	/* write the record label */
	labelchar = (char *)&label;
	labelchar[0] = EM3_START_BYTE;
	labelchar[1] = EM3_ID_PU_ID;
	write_len = 2;
	int status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

	/* compute checksum */
	uchar_ptr = (mb_u_char *)&label;
	checksum += uchar_ptr[1];

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* construct binary data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)store->pid_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)store->pid_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)store->pid_byte_order_flag, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)store->pid_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)store->pid_udp_port_1, (void *)&line[12]);
		mb_put_binary_short(swap, (unsigned short)store->pid_udp_port_2, (void *)&line[14]);
		mb_put_binary_short(swap, (unsigned short)store->pid_udp_port_3, (void *)&line[16]);
		mb_put_binary_short(swap, (unsigned short)store->pid_udp_port_4, (void *)&line[18]);
		mb_put_binary_int(swap, (int)store->pid_sys_descriptor, (void *)&line[20]);
    for (int i=0; i<16; i++){
      line[24+i] = store->pid_pu_sw_version[i];
    }
    for (int i=0; i<16; i++){
      line[40+i] = store->pid_bsp_sw_version[i];
    }
    for (int i=0; i<16; i++){
      line[56+i] = store->pid_head1_version[i];
    }
    for (int i=0; i<16; i++){
      line[72+i] = store->pid_head2_version[i];
    }
		mb_put_binary_int(swap, (int)store->pid_sys_descriptor, (void *)&line[88]);
    line[92] = store->pid_tx_opening_angle;
    line[93] = store->pid_tx_opening_angle;
    for (int i=0; i<7; i++){
      line[94+i] = store->pid_spare[i];
    }
		line[EM3_PU_ID_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_PU_ID_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM3_PU_ID_SIZE - 6]);

		/* write out data */
		write_len = EM3_PU_ID_SIZE - 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_status(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[EM3_PU_STATUS_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:                %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:               %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                %d\n", store->msec);
		fprintf(stderr, "dbg5       sts_date:            %d\n", store->sts_date);
		fprintf(stderr, "dbg5       sts_msec:            %d\n", store->sts_msec);
		fprintf(stderr, "dbg5       sts_status_count:    %d\n", store->sts_status_count);
		fprintf(stderr, "dbg5       sts_serial:          %d\n", store->sts_serial);
		fprintf(stderr, "dbg5       sts_pingrate:        %d\n", store->sts_pingrate);
		fprintf(stderr, "dbg5       sts_ping_count:      %d\n", store->sts_ping_count);
		fprintf(stderr, "dbg5       sts_load:            %d\n", store->sts_load);
		fprintf(stderr, "dbg5       sts_udp_status:      %d\n", store->sts_udp_status);
		fprintf(stderr, "dbg5       sts_serial1_status:  %d\n", store->sts_serial1_status);
		fprintf(stderr, "dbg5       sts_serial2_status:  %d\n", store->sts_serial2_status);
		fprintf(stderr, "dbg5       sts_serial3_status:  %d\n", store->sts_serial3_status);
		fprintf(stderr, "dbg5       sts_serial4_status:  %d\n", store->sts_serial4_status);
		fprintf(stderr, "dbg5       sts_pps_status:      %d\n", store->sts_pps_status);
		fprintf(stderr, "dbg5       sts_position_status: %d\n", store->sts_position_status);
		fprintf(stderr, "dbg5       sts_attitude_status: %d\n", store->sts_attitude_status);
		fprintf(stderr, "dbg5       sts_clock_status:    %d\n", store->sts_clock_status);
		fprintf(stderr, "dbg5       sts_heading_status:  %d\n", store->sts_heading_status);
		fprintf(stderr, "dbg5       sts_pu_status:       %d\n", store->sts_pu_status);
		fprintf(stderr, "dbg5       sts_last_heading:    %d\n", store->sts_last_heading);
		fprintf(stderr, "dbg5       sts_last_roll:       %d\n", store->sts_last_roll);
		fprintf(stderr, "dbg5       sts_last_pitch:      %d\n", store->sts_last_pitch);
		fprintf(stderr, "dbg5       sts_last_heave:      %d\n", store->sts_last_heave);
		fprintf(stderr, "dbg5       sts_last_ssv:        %d\n", store->sts_last_ssv);
		fprintf(stderr, "dbg5       sts_last_heave:      %d\n", store->sts_last_heave);
		fprintf(stderr, "dbg5       sts_last_depth:      %d\n", store->sts_last_depth);
		fprintf(stderr, "dbg5       sts_spare:           %d\n", store->sts_spare);
		fprintf(stderr, "dbg5       sts_bso:             %d\n", store->sts_bso);
		fprintf(stderr, "dbg5       sts_bsn:             %d\n", store->sts_bsn);
		fprintf(stderr, "dbg5       sts_gain:            %d\n", store->sts_gain);
		fprintf(stderr, "dbg5       sts_dno:             %d\n", store->sts_dno);
		fprintf(stderr, "dbg5       sts_rno:             %d\n", store->sts_rno);
		fprintf(stderr, "dbg5       sts_port:            %d\n", store->sts_port);
		fprintf(stderr, "dbg5       sts_stbd:            %d\n", store->sts_stbd);
		fprintf(stderr, "dbg5       sts_ssp:             %d\n", store->sts_ssp);
		fprintf(stderr, "dbg5       sts_yaw:             %d\n", store->sts_yaw);
		fprintf(stderr, "dbg5       sts_port2:           %d\n", store->sts_port2);
		fprintf(stderr, "dbg5       sts_stbd2:           %d\n", store->sts_stbd2);
		fprintf(stderr, "dbg5       sts_spare2:          %d\n", store->sts_spare2);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_PU_STATUS_SIZE), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);

	/* write the record label */
	labelchar = (char *)&label;
	labelchar[0] = EM3_START_BYTE;
	labelchar[1] = EM3_ID_PU_STATUS;
	write_len = 2;
	int status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

	/* compute checksum */
	uchar_ptr = (mb_u_char *)&label;
	checksum += uchar_ptr[1];

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* construct binary data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)store->sts_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)store->sts_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)store->sts_status_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)store->run_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)store->sts_pingrate, (void *)&line[12]);
		mb_put_binary_short(swap, (unsigned short)store->sts_ping_count, (void *)&line[14]);
		mb_put_binary_int(swap, (int)store->sts_load, (void *)&line[16]);
		mb_put_binary_int(swap, (int)store->sts_udp_status, (void *)&line[20]);
		mb_put_binary_int(swap, (int)store->sts_serial1_status, (void *)&line[24]);
		mb_put_binary_int(swap, (int)store->sts_serial2_status, (void *)&line[28]);
		mb_put_binary_int(swap, (int)store->sts_serial3_status, (void *)&line[32]);
		mb_put_binary_int(swap, (int)store->sts_serial3_status, (void *)&line[36]);
		line[40] = store->sts_pps_status;
		line[41] = store->sts_position_status;
		line[42] = store->sts_attitude_status;
		line[43] = store->sts_clock_status;
		line[44] = store->sts_heading_status;
		line[45] = store->sts_pu_status;
		mb_put_binary_short(swap, (unsigned short)store->sts_last_heading, (void *)&line[46]);
		mb_put_binary_short(swap, (short)store->sts_last_roll, (void *)&line[48]);
		mb_put_binary_short(swap, (short)store->sts_last_pitch, (void *)&line[50]);
		mb_put_binary_short(swap, (short)store->sts_last_heave, (void *)&line[52]);
		mb_put_binary_short(swap, (unsigned short)store->sts_last_ssv, (void *)&line[54]);
		mb_put_binary_int(swap, (int)store->sts_last_depth, (void *)&line[56]);
		mb_put_binary_int(swap, (int)store->sts_spare, (void *)&line[60]);
		line[64] = store->sts_bso;
		line[65] = store->sts_bsn;
		line[66] = store->sts_gain;
		line[67] = store->sts_dno;
		mb_put_binary_short(swap, (unsigned short)store->sts_rno, (void *)&line[68]);
		line[70] = store->sts_port;
		line[71] = store->sts_stbd;
		mb_put_binary_short(swap, (unsigned short)store->sts_ssp, (void *)&line[72]);
		mb_put_binary_short(swap, (unsigned short)store->sts_yaw, (void *)&line[74]);
		mb_put_binary_short(swap, (unsigned short)store->sts_port2, (void *)&line[76]);
		mb_put_binary_short(swap, (unsigned short)store->sts_stbd2, (void *)&line[78]);
		line[80] = store->sts_spare2;
		line[EM3_PU_STATUS_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_PU_STATUS_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM3_PU_STATUS_SIZE - 6]);

		/* write out data */
		write_len = EM3_PU_STATUS_SIZE - 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_run_parameter(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[EM3_RUN_PARAMETER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       run_date:        %d\n", store->run_date);
		fprintf(stderr, "dbg5       run_msec:        %d\n", store->run_msec);
		fprintf(stderr, "dbg5       run_ping_count:  %d\n", store->run_ping_count);
		fprintf(stderr, "dbg5       run_serial:      %d\n", store->run_serial);
		fprintf(stderr, "dbg5       run_status:      %d\n", store->run_status);
		fprintf(stderr, "dbg5       run_mode:        %d\n", store->run_mode);
		fprintf(stderr, "dbg5       run_filter_id:   %d\n", store->run_filter_id);
		fprintf(stderr, "dbg5       run_min_depth:   %d\n", store->run_min_depth);
		fprintf(stderr, "dbg5       run_max_depth:   %d\n", store->run_max_depth);
		fprintf(stderr, "dbg5       run_absorption:  %d\n", store->run_absorption);
		fprintf(stderr, "dbg5       run_tran_pulse:  %d\n", store->run_tran_pulse);
		fprintf(stderr, "dbg5       run_tran_beam:   %d\n", store->run_tran_beam);
		fprintf(stderr, "dbg5       run_tran_pow:    %d\n", store->run_tran_pow);
		fprintf(stderr, "dbg5       run_rec_beam:    %d\n", store->run_rec_beam);
		fprintf(stderr, "dbg5       run_rec_band:    %d\n", store->run_rec_band);
		fprintf(stderr, "dbg5       run_rec_gain:    %d\n", store->run_rec_gain);
		fprintf(stderr, "dbg5       run_tvg_cross:   %d\n", store->run_tvg_cross);
		fprintf(stderr, "dbg5       run_ssv_source:  %d\n", store->run_ssv_source);
		fprintf(stderr, "dbg5       run_max_swath:   %d\n", store->run_max_swath);
		fprintf(stderr, "dbg5       run_beam_space:  %d\n", store->run_beam_space);
		fprintf(stderr, "dbg5       run_swath_angle: %d\n", store->run_swath_angle);
		fprintf(stderr, "dbg5       run_stab_mode:   %d\n", store->run_stab_mode);
		for (int i = 0; i < 6; i++)
			fprintf(stderr, "dbg5       run_spare[%d]:    %d\n", i, store->run_spare[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_RUN_PARAMETER_SIZE), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_RUN_PARAMETER;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* construct binary data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)store->run_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)store->run_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)store->run_ping_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)store->run_serial, (void *)&line[10]);
		mb_put_binary_int(swap, (int)store->run_status, (void *)&line[12]);
		line[16] = store->run_mode;
		line[17] = store->run_filter_id;
		mb_put_binary_short(swap, (unsigned short)store->run_min_depth, (void *)&line[18]);
		mb_put_binary_short(swap, (unsigned short)store->run_max_depth, (void *)&line[20]);
		mb_put_binary_short(swap, (unsigned short)store->run_absorption, (void *)&line[22]);
		mb_put_binary_short(swap, (unsigned short)store->run_tran_pulse, (void *)&line[24]);
		mb_put_binary_short(swap, (unsigned short)store->run_tran_beam, (void *)&line[26]);
		line[28] = store->run_tran_pow;
		line[29] = store->run_rec_beam;
		line[30] = store->run_rec_band;
		line[31] = store->run_rec_gain;
		line[32] = store->run_tvg_cross;
		line[33] = store->run_ssv_source;
		mb_put_binary_short(swap, (unsigned short)store->run_max_swath, (void *)&line[34]);
		line[36] = store->run_beam_space;
		line[37] = store->run_swath_angle;
		line[38] = store->run_stab_mode;
		for (int i = 0; i < 6; i++)
			line[39 + i] = store->run_spare[i];
		line[EM3_RUN_PARAMETER_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_RUN_PARAMETER_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM3_RUN_PARAMETER_SIZE - 6]);

		/* write out data */
		write_len = EM3_RUN_PARAMETER_SIZE - 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_clock(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[EM3_CLOCK_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       clk_date:        %d\n", store->clk_date);
		fprintf(stderr, "dbg5       clk_msec:        %d\n", store->clk_msec);
		fprintf(stderr, "dbg5       clk_count:       %d\n", store->clk_count);
		fprintf(stderr, "dbg5       clk_serial:      %d\n", store->clk_serial);
		fprintf(stderr, "dbg5       clk_origin_date: %d\n", store->clk_origin_date);
		fprintf(stderr, "dbg5       clk_origin_msec: %d\n", store->clk_origin_msec);
		fprintf(stderr, "dbg5       clk_1_pps_use:   %d\n", store->clk_1_pps_use);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_CLOCK_SIZE), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_CLOCK;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* construct binary data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)store->clk_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)store->clk_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)store->clk_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)store->clk_serial, (void *)&line[10]);
		mb_put_binary_int(swap, (int)store->clk_origin_date, (void *)&line[12]);
		mb_put_binary_int(swap, (int)store->clk_origin_msec, (void *)&line[16]);
		line[20] = store->clk_1_pps_use;
		line[EM3_CLOCK_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_CLOCK_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM3_CLOCK_SIZE - 6]);

		/* write out data */
		write_len = EM3_CLOCK_SIZE - 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_tide(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[EM3_TIDE_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       tid_date:        %d\n", store->tid_date);
		fprintf(stderr, "dbg5       tid_msec:        %d\n", store->tid_msec);
		fprintf(stderr, "dbg5       tid_count:       %d\n", store->tid_count);
		fprintf(stderr, "dbg5       tid_serial:      %d\n", store->tid_serial);
		fprintf(stderr, "dbg5       tid_origin_date: %d\n", store->tid_origin_date);
		fprintf(stderr, "dbg5       tid_origin_msec: %d\n", store->tid_origin_msec);
		fprintf(stderr, "dbg5       tid_tide:        %d\n", store->tid_tide);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_TIDE_SIZE), (void *)&write_size);
	write_len = 4;
	int status = mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_TIDE;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* construct binary data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)store->tid_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)store->tid_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)store->tid_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)store->tid_serial, (void *)&line[10]);
		mb_put_binary_int(swap, (int)store->tid_origin_date, (void *)&line[12]);
		mb_put_binary_int(swap, (int)store->tid_origin_msec, (void *)&line[16]);
		mb_put_binary_short(swap, (short)store->tid_tide, (void *)&line[20]);
		line[EM3_TIDE_SIZE - 8] = '\0';
		line[EM3_TIDE_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_TIDE_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM3_TIDE_SIZE - 6]);

		/* write out data */
		write_len = EM3_TIDE_SIZE - 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_height(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[EM3_HEIGHT_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       hgt_date:        %d\n", store->hgt_date);
		fprintf(stderr, "dbg5       hgt_msec:        %d\n", store->hgt_msec);
		fprintf(stderr, "dbg5       hgt_count:       %d\n", store->hgt_count);
		fprintf(stderr, "dbg5       hgt_serial:      %d\n", store->hgt_serial);
		fprintf(stderr, "dbg5       hgt_height:      %d\n", store->hgt_height);
		fprintf(stderr, "dbg5       hgt_type:        %d\n", store->hgt_type);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_HEIGHT_SIZE), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_HEIGHT;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* construct binary data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)store->hgt_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)store->hgt_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)store->hgt_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)store->hgt_serial, (void *)&line[10]);
		mb_put_binary_int(swap, (int)store->hgt_height, (void *)&line[12]);
		line[16] = (mb_u_char)store->hgt_type;
		line[EM3_HEIGHT_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_HEIGHT_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM3_HEIGHT_SIZE - 6]);

		/* write out data */
		write_len = EM3_HEIGHT_SIZE - 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_heading(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_heading_struct *heading;
	char line[EM3_HEADING_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	heading = (struct mbsys_simrad3_heading_struct *)store->heading;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       hed_date:        %d\n", heading->hed_date);
		fprintf(stderr, "dbg5       hed_msec:        %d\n", heading->hed_msec);
		fprintf(stderr, "dbg5       hed_count:       %d\n", heading->hed_count);
		fprintf(stderr, "dbg5       hed_serial:      %d\n", heading->hed_serial);
		fprintf(stderr, "dbg5       hed_ndata:       %d\n", heading->hed_ndata);
		fprintf(stderr, "dbg5       count    time (msec)    heading (0.01 deg)\n");
		fprintf(stderr, "dbg5       -----    -----------    ------------------\n");
		for (int i = 0; i < heading->hed_ndata; i++)
			fprintf(stderr, "dbg5        %4d      %7d          %7d\n", i, heading->hed_time[i], heading->hed_heading[i]);
		fprintf(stderr, "dbg5       hed_heading_status: %d\n", heading->hed_heading_status);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_HEADING_HEADER_SIZE + EM3_HEADING_SLICE_SIZE * heading->hed_ndata + 8),
	                  (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_HEADING;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)heading->hed_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)heading->hed_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)heading->hed_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)heading->hed_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)heading->hed_ndata, (void *)&line[12]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_HEADING_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_HEADING_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary heading data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < heading->hed_ndata; i++) {
			mb_put_binary_short(swap, (unsigned short)heading->hed_time[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)heading->hed_heading[i], (void *)&line[2]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_HEADING_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_HEADING_SLICE_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = (mb_u_char)heading->hed_heading_status;
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_ssv(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_ssv_struct *ssv;
	char line[EM3_SSV_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	ssv = (struct mbsys_simrad3_ssv_struct *)store->ssv;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       ssv_date:        %d\n", ssv->ssv_date);
		fprintf(stderr, "dbg5       ssv_msec:        %d\n", ssv->ssv_msec);
		fprintf(stderr, "dbg5       ssv_count:       %d\n", ssv->ssv_count);
		fprintf(stderr, "dbg5       ssv_serial:      %d\n", ssv->ssv_serial);
		fprintf(stderr, "dbg5       ssv_ndata:       %d\n", ssv->ssv_ndata);
		fprintf(stderr, "dbg5       count    time (msec)    ssv (0.1 m/s)\n");
		fprintf(stderr, "dbg5       -----    -----------    ------------------\n");
		for (int i = 0; i < ssv->ssv_ndata; i++)
			fprintf(stderr, "dbg5        %4d      %7d          %7d\n", i, ssv->ssv_time[i], ssv->ssv_ssv[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_SSV_HEADER_SIZE + EM3_SSV_SLICE_SIZE * ssv->ssv_ndata + 8), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_SSV;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)ssv->ssv_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)ssv->ssv_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)ssv->ssv_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)ssv->ssv_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)ssv->ssv_ndata, (void *)&line[12]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_SSV_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_SSV_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary ssv data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ssv->ssv_ndata; i++) {
			mb_put_binary_short(swap, (unsigned short)ssv->ssv_time[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)ssv->ssv_ssv[i], (void *)&line[2]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_SSV_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_SSV_SLICE_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = 0;
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_tilt(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_tilt_struct *tilt;
	char line[EM3_TILT_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	tilt = (struct mbsys_simrad3_tilt_struct *)store->tilt;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       tlt_date:        %d\n", tilt->tlt_date);
		fprintf(stderr, "dbg5       tlt_msec:        %d\n", tilt->tlt_msec);
		fprintf(stderr, "dbg5       tlt_count:       %d\n", tilt->tlt_count);
		fprintf(stderr, "dbg5       tlt_serial:      %d\n", tilt->tlt_serial);
		fprintf(stderr, "dbg5       tlt_ndata:       %d\n", tilt->tlt_ndata);
		fprintf(stderr, "dbg5       count    time (msec)    tilt (0.01 deg)\n");
		fprintf(stderr, "dbg5       -----    -----------    ------------------\n");
		for (int i = 0; i < tilt->tlt_ndata; i++)
			fprintf(stderr, "dbg5        %4d      %7d          %7d\n", i, tilt->tlt_time[i], tilt->tlt_tilt[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_TILT_HEADER_SIZE + EM3_TILT_SLICE_SIZE * tilt->tlt_ndata + 8), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_TILT;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)tilt->tlt_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)tilt->tlt_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)tilt->tlt_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)tilt->tlt_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)tilt->tlt_ndata, (void *)&line[12]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_TILT_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_TILT_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary tilt data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < tilt->tlt_ndata; i++) {
			mb_put_binary_short(swap, (unsigned short)tilt->tlt_time[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)tilt->tlt_tilt[i], (void *)&line[2]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_TILT_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_TILT_SLICE_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = 0;
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_extraparameters(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_extraparameters_struct *extraparameters;
	char line[EM3_EXTRAPARAMETERS_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	extraparameters = (struct mbsys_simrad3_extraparameters_struct *)store->extraparameters;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       xtr_date:        %d\n", extraparameters->xtr_date);
		fprintf(stderr, "dbg5       xtr_msec:        %d\n", extraparameters->xtr_msec);
		fprintf(stderr, "dbg5       xtr_count:       %d\n", extraparameters->xtr_count);
		fprintf(stderr, "dbg5       xtr_serial:      %d\n", extraparameters->xtr_serial);
		fprintf(stderr, "dbg5       xtr_id:          %d\n", extraparameters->xtr_id);
		fprintf(stderr, "dbg5       xtr_data_size:   %d\n", extraparameters->xtr_data_size);
		fprintf(stderr, "dbg5       xtr_nalloc:      %d\n", extraparameters->xtr_nalloc);
		if (extraparameters->xtr_id == 2) {
			fprintf(stderr, "dbg5       xtr_pqf_activepositioning:          %d\n", extraparameters->xtr_pqf_activepositioning);
			for (int i = 0; i < 3; i++) {
				fprintf(stderr, "dbg5       positioning system:%d qfsetting:%d nqf:%d\n", i,
				        extraparameters->xtr_pqf_qfsetting[i], extraparameters->xtr_pqf_nqualityfactors[i]);
				for (int j = 0; j < extraparameters->xtr_pqf_nqualityfactors[i]; j++)
					fprintf(stderr, "dbg5       quality factor:%d value:%d limit:%d\n", j,
					        extraparameters->xtr_pqf_qfvalues[i][j], extraparameters->xtr_pqf_qflimits[i][j]);
			}
		}
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_EXTRAPARAMETERS_HEADER_SIZE + extraparameters->xtr_data_size + 8), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_EXTRAPARAMETERS;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)extraparameters->xtr_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)extraparameters->xtr_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)extraparameters->xtr_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)extraparameters->xtr_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)extraparameters->xtr_id, (void *)&line[12]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_EXTRAPARAMETERS_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_EXTRAPARAMETERS_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary extraparameters data */
	if (status == MB_SUCCESS) {
		/* compute checksum */
		uchar_ptr = (mb_u_char *)extraparameters->xtr_data;
		for (int j = 0; j < extraparameters->xtr_data_size; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = extraparameters->xtr_data_size;
		status = mb_fileio_put(verbose, mbio_ptr, extraparameters->xtr_data, &write_len, error);
	}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = 0;
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_attitude(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_attitude_struct *attitude;
	char line[EM3_ATTITUDE_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	attitude = (struct mbsys_simrad3_attitude_struct *)store->attitude;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       att_date:        %d\n", attitude->att_date);
		fprintf(stderr, "dbg5       att_msec:        %d\n", attitude->att_msec);
		fprintf(stderr, "dbg5       att_count:       %d\n", attitude->att_count);
		fprintf(stderr, "dbg5       att_serial:      %d\n", attitude->att_serial);
		fprintf(stderr, "dbg5       att_ndata:       %d\n", attitude->att_ndata);
		fprintf(stderr, "dbg5       cnt   time   roll pitch heave heading\n");
		fprintf(stderr, "dbg5       -------------------------------------\n");
		for (int i = 0; i < attitude->att_ndata; i++)
			fprintf(stderr, "dbg5        %3d  %d  %d %d %d %d\n", i, attitude->att_time[i], attitude->att_roll[i],
			        attitude->att_pitch[i], attitude->att_heave[i], attitude->att_heading[i]);
		fprintf(stderr, "dbg5       att_sensordescriptor: %d\n", attitude->att_sensordescriptor);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_ATTITUDE_HEADER_SIZE + EM3_ATTITUDE_SLICE_SIZE * attitude->att_ndata + 8),
	                  (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_ATTITUDE;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)attitude->att_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)attitude->att_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)attitude->att_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)attitude->att_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)attitude->att_ndata, (void *)&line[12]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_ATTITUDE_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_ATTITUDE_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary heading data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < attitude->att_ndata; i++) {
			mb_put_binary_short(swap, (unsigned short)attitude->att_time[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)attitude->att_sensor_status[i], (void *)&line[2]);
			mb_put_binary_short(swap, (short)attitude->att_roll[i], (void *)&line[4]);
			mb_put_binary_short(swap, (short)attitude->att_pitch[i], (void *)&line[6]);
			mb_put_binary_short(swap, (short)attitude->att_heave[i], (void *)&line[8]);
			mb_put_binary_short(swap, (unsigned short)attitude->att_heading[i], (void *)&line[10]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_ATTITUDE_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_ATTITUDE_SLICE_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = (mb_u_char)attitude->att_sensordescriptor;
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_netattitude(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_netattitude_struct *netattitude;
	char line[EM3_NETATTITUDE_SLICE_SIZE + MBSYS_SIMRAD3_BUFFER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	int extrabyte;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	netattitude = (struct mbsys_simrad3_netattitude_struct *)store->netattitude;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:                 %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:                %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                 %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                 %d\n", store->msec);
		fprintf(stderr, "dbg5       nat_date:             %d\n", netattitude->nat_date);
		fprintf(stderr, "dbg5       nat_msec:             %d\n", netattitude->nat_msec);
		fprintf(stderr, "dbg5       nat_count:            %d\n", netattitude->nat_count);
		fprintf(stderr, "dbg5       nat_serial:           %d\n", netattitude->nat_serial);
		fprintf(stderr, "dbg5       nat_ndata:            %d\n", netattitude->nat_ndata);
		fprintf(stderr, "dbg5       nat_sensordescriptor: %d\n", netattitude->nat_sensordescriptor);
		fprintf(stderr, "dbg5       cnt   time   roll pitch heave heading\n");
		fprintf(stderr, "dbg5       -------------------------------------\n");
		for (int i = 0; i < netattitude->nat_ndata; i++) {
			fprintf(stderr, "dbg5        %3d  %d  %d %d %d %d %d\n", i, netattitude->nat_time[i], netattitude->nat_roll[i],
			        netattitude->nat_pitch[i], netattitude->nat_heave[i], netattitude->nat_heading[i],
			        netattitude->nat_nbyte_raw[i]);
			fprintf(stderr, "dbg5        nat_raw[%d]: ", netattitude->nat_nbyte_raw[i]);
			for (int j = 0; j < netattitude->nat_nbyte_raw[i]; j++)
				fprintf(stderr, "%x", netattitude->nat_raw[i * MBSYS_SIMRAD3_BUFFER_SIZE + j]);
			fprintf(stderr, "\n");
		}
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	write_size = EM3_NETATTITUDE_HEADER_SIZE + 8;
	for (int i = 0; i < netattitude->nat_ndata; i++) {
		write_size += EM3_NETATTITUDE_SLICE_SIZE + netattitude->nat_nbyte_raw[i];
	}
	extrabyte = 0;
	if (write_size % 2) {
		extrabyte++;
		write_size--;
	}
	mb_put_binary_int(swap, (int)write_size, (void *)line);
	write_len = 4;
	int status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_NETATTITUDE;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)netattitude->nat_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)netattitude->nat_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)netattitude->nat_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)netattitude->nat_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)netattitude->nat_ndata, (void *)&line[12]);
		line[14] = (mb_u_char)netattitude->nat_sensordescriptor;
		line[15] = (mb_u_char)0;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_NETATTITUDE_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_NETATTITUDE_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary attitude data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < netattitude->nat_ndata; i++) {
			mb_put_binary_short(swap, (unsigned short)netattitude->nat_time[i], (void *)&line[0]);
			mb_put_binary_short(swap, (short)netattitude->nat_roll[i], (void *)&line[2]);
			mb_put_binary_short(swap, (short)netattitude->nat_pitch[i], (void *)&line[4]);
			mb_put_binary_short(swap, (short)netattitude->nat_heave[i], (void *)&line[6]);
			mb_put_binary_short(swap, (unsigned short)netattitude->nat_heading[i], (void *)&line[8]);
			line[10] = (mb_u_char)netattitude->nat_nbyte_raw[i];
			for (int j = 0; j < netattitude->nat_nbyte_raw[i]; j++)
				line[j + 11] = netattitude->nat_raw[i * MBSYS_SIMRAD3_BUFFER_SIZE + j];

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_NETATTITUDE_SLICE_SIZE + netattitude->nat_nbyte_raw[i]; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_NETATTITUDE_SLICE_SIZE + netattitude->nat_nbyte_raw[i];
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = 0;
		line[1] = 0x03;

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		if (extrabyte) {
			write_len = 3;
			status = mb_fileio_put(verbose, mbio_ptr, &line[1], &write_len, error);
		}
		else {
			write_len = 4;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_pos(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[EM3_POS_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       pos_date:        %d\n", store->pos_date);
		fprintf(stderr, "dbg5       pos_msec:        %d\n", store->pos_msec);
		fprintf(stderr, "dbg5       pos_count:       %d\n", store->pos_count);
		fprintf(stderr, "dbg5       pos_serial:      %d\n", store->pos_serial);
		fprintf(stderr, "dbg5       pos_latitude:    %d\n", store->pos_latitude);
		fprintf(stderr, "dbg5       pos_longitude:   %d\n", store->pos_longitude);
		fprintf(stderr, "dbg5       pos_quality:     %d\n", store->pos_quality);
		fprintf(stderr, "dbg5       pos_speed:       %d\n", store->pos_speed);
		fprintf(stderr, "dbg5       pos_course:      %d\n", store->pos_course);
		fprintf(stderr, "dbg5       pos_heading:     %d\n", store->pos_heading);
		fprintf(stderr, "dbg5       pos_system:      %d\n", store->pos_system);
		fprintf(stderr, "dbg5       pos_input_size:  %d\n", store->pos_input_size);
		fprintf(stderr, "dbg5       pos_input:\ndbg5            %s\n", store->pos_input);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_POS_HEADER_SIZE + store->pos_input_size - (store->pos_input_size % 2) + 8),
	                  (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_POS;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)store->pos_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)store->pos_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)store->pos_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)store->pos_serial, (void *)&line[10]);
		mb_put_binary_int(swap, (int)store->pos_latitude, (void *)&line[12]);
		mb_put_binary_int(swap, (int)store->pos_longitude, (void *)&line[16]);
		mb_put_binary_short(swap, (unsigned short)store->pos_quality, (void *)&line[20]);
		mb_put_binary_short(swap, (unsigned short)store->pos_speed, (void *)&line[22]);
		mb_put_binary_short(swap, (unsigned short)store->pos_course, (void *)&line[24]);
		mb_put_binary_short(swap, (unsigned short)store->pos_heading, (void *)&line[26]);
		line[28] = (mb_u_char)store->pos_system;
		line[29] = (mb_u_char)store->pos_input_size;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_POS_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_POS_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output original ascii nav data */
	if (status == MB_SUCCESS) {
		write_size = store->pos_input_size - (store->pos_input_size % 2) + 1;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)store->pos_input;
		for (int j = 0; j < write_size; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = write_size;
		status = mb_fileio_put(verbose, mbio_ptr, store->pos_input, &write_len, error);
	}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[1] = 0x03;

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 3;
		status = mb_fileio_put(verbose, mbio_ptr, &line[1], &write_len, error);
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
int mbr_em710raw_wr_svp(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[EM3_SVP_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       svp_use_date:    %d\n", store->svp_use_date);
		fprintf(stderr, "dbg5       svp_use_msec:    %d\n", store->svp_use_msec);
		fprintf(stderr, "dbg5       svp_count:       %d\n", store->svp_count);
		fprintf(stderr, "dbg5       svp_serial:      %d\n", store->svp_serial);
		fprintf(stderr, "dbg5       svp_origin_date: %d\n", store->svp_origin_date);
		fprintf(stderr, "dbg5       svp_origin_msec: %d\n", store->svp_origin_msec);
		fprintf(stderr, "dbg5       svp_num:         %d\n", store->svp_num);
		fprintf(stderr, "dbg5       svp_depth_res:   %d\n", store->svp_depth_res);
		fprintf(stderr, "dbg5       count    depth    speed\n");
		fprintf(stderr, "dbg5       -----------------------\n");
		for (int i = 0; i < store->svp_num; i++)
			fprintf(stderr, "dbg5        %d   %d  %d\n", i, store->svp_depth[i], store->svp_vel[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_SVP_HEADER_SIZE + EM3_SVP_SLICE_SIZE * store->svp_num + 8), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_SVP;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)store->svp_use_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)store->svp_use_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)store->svp_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)store->svp_serial, (void *)&line[10]);
		mb_put_binary_int(swap, (int)store->svp_origin_date, (void *)&line[12]);
		mb_put_binary_int(swap, (int)store->svp_origin_msec, (void *)&line[16]);
		mb_put_binary_short(swap, (unsigned short)store->svp_num, (void *)&line[20]);
		mb_put_binary_short(swap, (unsigned short)store->svp_depth_res, (void *)&line[22]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_SVP_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_SVP_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary svp data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < store->svp_num; i++) {
			mb_put_binary_short(swap, (unsigned short)store->svp_depth[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)store->svp_vel[i], (void *)&line[4]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_SVP_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_SVP_SLICE_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = '\0';
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_svp2(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	char line[EM3_SVP2_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       svp_use_date:    %d\n", store->svp_use_date);
		fprintf(stderr, "dbg5       svp_use_msec:    %d\n", store->svp_use_msec);
		fprintf(stderr, "dbg5       svp_count:       %d\n", store->svp_count);
		fprintf(stderr, "dbg5       svp_serial:      %d\n", store->svp_serial);
		fprintf(stderr, "dbg5       svp_origin_date: %d\n", store->svp_origin_date);
		fprintf(stderr, "dbg5       svp_origin_msec: %d\n", store->svp_origin_msec);
		fprintf(stderr, "dbg5       svp_num:         %d\n", store->svp_num);
		fprintf(stderr, "dbg5       svp_depth_res:   %d\n", store->svp_depth_res);
		fprintf(stderr, "dbg5       count    depth    speed\n");
		fprintf(stderr, "dbg5       -----------------------\n");
		for (int i = 0; i < store->svp_num; i++)
			fprintf(stderr, "dbg5        %d   %d  %d\n", i, store->svp_depth[i], store->svp_vel[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_SVP2_HEADER_SIZE + EM3_SVP2_SLICE_SIZE * store->svp_num + 8), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_SVP2;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)store->svp_use_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)store->svp_use_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)store->svp_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)store->svp_serial, (void *)&line[10]);
		mb_put_binary_int(swap, (int)store->svp_origin_date, (void *)&line[12]);
		mb_put_binary_int(swap, (int)store->svp_origin_msec, (void *)&line[16]);
		mb_put_binary_short(swap, (unsigned short)store->svp_num, (void *)&line[20]);
		mb_put_binary_short(swap, (unsigned short)store->svp_depth_res, (void *)&line[22]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_SVP2_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_SVP2_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary svp data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < store->svp_num; i++) {
			mb_put_binary_int(swap, (int)store->svp_depth[i], (void *)&line[0]);
			mb_put_binary_int(swap, (int)store->svp_vel[i], (void *)&line[4]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_SVP2_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_SVP2_SLICE_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = '\0';
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_bath2(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_ping_struct *ping;
	char line[EM3_BATH2_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* set which storage structure to use */
	ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:                  %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:                 %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                  %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                  %d\n", store->msec);
		fprintf(stderr, "dbg5       png_date:              %d\n", ping->png_date);
		fprintf(stderr, "dbg5       png_msec:              %d\n", ping->png_msec);
		fprintf(stderr, "dbg5       png_count:             %d\n", ping->png_count);
		fprintf(stderr, "dbg5       png_serial:            %d\n", ping->png_serial);
		fprintf(stderr, "dbg5       png_heading:           %d\n", ping->png_heading);
		fprintf(stderr, "dbg5       png_ssv:               %d\n", ping->png_ssv);
		fprintf(stderr, "dbg5       png_xducer_depth:      %f\n", ping->png_xducer_depth);
		fprintf(stderr, "dbg5       png_nbeams:            %d\n", ping->png_nbeams);
		fprintf(stderr, "dbg5       png_nbeams_valid:      %d\n", ping->png_nbeams_valid);
		fprintf(stderr, "dbg5       png_sample_rate:       %f\n", ping->png_sample_rate);
		fprintf(stderr, "dbg5       png_spare:             %d\n", ping->png_spare);
		fprintf(stderr, "dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %7.2f %7.2f %7.2f %5d %5d %5d %4d %3d %3d\n", i, ping->png_depth[i],
			        ping->png_acrosstrack[i], ping->png_alongtrack[i], ping->png_window[i], ping->png_quality[i],
			        ping->png_iba[i], ping->png_detection[i], ping->png_clean[i], ping->png_amp[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM3_BATH2_HEADER_SIZE + EM3_BATH2_BEAM_SIZE * ping->png_nbeams + 8), (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_BATH2;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)ping->png_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)ping->png_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)ping->png_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)ping->png_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)ping->png_heading, (void *)&line[12]);
		mb_put_binary_short(swap, (unsigned short)ping->png_ssv, (void *)&line[14]);
		mb_put_binary_float(swap, (float)ping->png_xducer_depth, (void *)&line[16]);
		mb_put_binary_short(swap, (unsigned short)ping->png_nbeams, (void *)&line[20]);
		mb_put_binary_short(swap, (unsigned short)ping->png_nbeams_valid, (void *)&line[22]);
		mb_put_binary_float(swap, (float)ping->png_sample_rate, (void *)&line[24]);
		mb_put_binary_short(swap, (unsigned short)ping->png_spare, (void *)&line[28]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_BATH2_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_BATH2_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary beam data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_nbeams; i++) {
			mb_put_binary_float(swap, ping->png_depth[i], (void *)&line[0]);
			mb_put_binary_float(swap, ping->png_acrosstrack[i], (void *)&line[4]);
			mb_put_binary_float(swap, ping->png_alongtrack[i], (void *)&line[8]);
			mb_put_binary_short(swap, (unsigned short)ping->png_window[i], (void *)&line[12]);
			line[14] = (mb_u_char)ping->png_quality[i];
			line[15] = (mb_s_char)ping->png_iba[i];
			line[16] = (mb_u_char)ping->png_detection[i];
			line[17] = (mb_s_char)ping->png_clean[i];
			mb_put_binary_short(swap, (short)ping->png_amp[i], (void *)&line[18]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_BATH2_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_BATH2_BEAM_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = 0x00;
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_rawbeam4(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_ping_struct *ping;
	char line[EM3_RAWBEAM4_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* set which storage structure to use */
	ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       png_raw_read:               %d\n", ping->png_raw_read);
		fprintf(stderr, "dbg5       png_raw_date:                %d\n", ping->png_raw_date);
		fprintf(stderr, "dbg5       png_raw_msec:                %d\n", ping->png_raw_msec);
		fprintf(stderr, "dbg5       png_raw_count:               %d\n", ping->png_raw_count);
		fprintf(stderr, "dbg5       png_raw_serial:              %d\n", ping->png_raw_serial);
		fprintf(stderr, "dbg5       png_raw_ssv:                 %d\n", ping->png_raw_ssv);
		fprintf(stderr, "dbg5       png_raw_ntx:                 %d\n", ping->png_raw_ntx);
		fprintf(stderr, "dbg5       png_raw_nbeams:              %d\n", ping->png_raw_nbeams);
		fprintf(stderr, "dbg5       png_raw_detections:          %d\n", ping->png_raw_detections);
		fprintf(stderr, "dbg5       png_raw_sample_rate:         %f\n", ping->png_raw_sample_rate);
		fprintf(stderr, "dbg5       png_raw_spare:               %d\n", ping->png_raw_spare);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       transmit pulse values:\n");
		fprintf(stderr, "dbg5       tiltangle focus length offset center bandwidth waveform sector\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_ntx; i++)
			fprintf(stderr, "dbg5       %3d %5d %5d %f %f %f %4d %4d %4d %f\n", i, ping->png_raw_txtiltangle[i],
			        ping->png_raw_txfocus[i], ping->png_raw_txsignallength[i], ping->png_raw_txoffset[i],
			        ping->png_raw_txcenter[i], ping->png_raw_txabsorption[i], ping->png_raw_txwaveform[i],
			        ping->png_raw_txsector[i], ping->png_raw_txbandwidth[i]);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       beam values:\n");
		fprintf(stderr, "dbg5       angle range sector amp quality window beam\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %5d %3d %3d %4d %3d %5d %f %5d %5d %5d\n", i, ping->png_raw_rxpointangle[i],
			        ping->png_raw_rxsector[i], ping->png_raw_rxdetection[i], ping->png_raw_rxwindow[i],
			        ping->png_raw_rxquality[i], ping->png_raw_rxspare1[i], ping->png_raw_rxrange[i], ping->png_raw_rxamp[i],
			        ping->png_raw_rxcleaning[i], ping->png_raw_rxspare2[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap,
	                  (int)(EM3_RAWBEAM4_HEADER_SIZE + EM3_RAWBEAM4_TX_SIZE * ping->png_raw_ntx +
	                        EM3_RAWBEAM4_BEAM_SIZE * ping->png_raw_nbeams + 8),
	                  (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_RAWBEAM4;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)ping->png_raw_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)ping->png_raw_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_ssv, (void *)&line[12]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_ntx, (void *)&line[14]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_nbeams, (void *)&line[16]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_detections, (void *)&line[18]);
		mb_put_binary_float(swap, ping->png_raw_sample_rate, (void *)&line[20]);
		mb_put_binary_int(swap, (int)ping->png_raw_spare, (void *)&line[24]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_RAWBEAM4_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_RAWBEAM4_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary tx data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_raw_ntx; i++) {
			mb_put_binary_short(swap, (short)ping->png_raw_txtiltangle[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)ping->png_raw_txfocus[i], (void *)&line[2]);
			mb_put_binary_float(swap, ping->png_raw_txsignallength[i], (void *)&line[4]);
			mb_put_binary_float(swap, ping->png_raw_txoffset[i], (void *)&line[8]);
			mb_put_binary_float(swap, ping->png_raw_txcenter[i], (void *)&line[12]);
			mb_put_binary_short(swap, (unsigned short)ping->png_raw_txabsorption[i], (void *)&line[16]);
			line[18] = (mb_u_char)ping->png_raw_txwaveform[i];
			line[19] = (mb_u_char)ping->png_raw_txsector[i];
			mb_put_binary_float(swap, ping->png_raw_txbandwidth[i], (void *)&line[20]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_RAWBEAM4_TX_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_RAWBEAM4_TX_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output binary beam data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_raw_nbeams; i++) {
			mb_put_binary_short(swap, (short)ping->png_raw_rxpointangle[i], (void *)&line[0]);
			line[2] = (mb_u_char)ping->png_raw_rxsector[i];
			line[3] = (mb_u_char)ping->png_raw_rxdetection[i];
			mb_put_binary_short(swap, (short)ping->png_raw_rxwindow[i], (void *)&line[4]);
			line[6] = (mb_u_char)ping->png_raw_rxquality[i];
			line[7] = (mb_u_char)ping->png_raw_rxspare1[i];
			mb_put_binary_float(swap, ping->png_raw_rxrange[i], (void *)&line[8]);
			mb_put_binary_short(swap, (short)ping->png_raw_rxamp[i], (void *)&line[12]);
			line[14] = (mb_u_char)ping->png_raw_rxcleaning[i];
			line[15] = (mb_u_char)ping->png_raw_rxspare2[i];

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_RAWBEAM4_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_RAWBEAM4_BEAM_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = 0x00;
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_quality(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_ping_struct *ping;
	char line[EM3_QUALITY_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;
	int index;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* set which storage structure to use */
	ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:                  %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:                 %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:                  %d\n", store->date);
		fprintf(stderr, "dbg5       msec:                  %d\n", store->msec);
		fprintf(stderr, "dbg5       png_quality_date:              %d\n", ping->png_quality_date);
		fprintf(stderr, "dbg5       png_quality_msec:              %d\n", ping->png_quality_msec);
		fprintf(stderr, "dbg5       png_quality_count:             %d\n", ping->png_quality_count);
		fprintf(stderr, "dbg5       png_quality_serial:            %d\n", ping->png_quality_serial);
		fprintf(stderr, "dbg5       png_quality_nbeams:            %d\n", ping->png_quality_nbeams);
		fprintf(stderr, "dbg5       png_quality_nparameters:       %d\n", ping->png_quality_nparameters);
		fprintf(stderr, "dbg5       png_quality_spare:         v   %d\n", ping->png_quality_spare);
		fprintf(stderr, "dbg5       cnt  quality parameters\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_quality_nbeams; i++) {
			fprintf(stderr, "dbg5       %3d ", i);
			for (int j = 0; j < ping->png_quality_nparameters; j++)
				fprintf(stderr, "%f", ping->png_quality_parameters[i][j]);
			fprintf(stderr, "\n");
		}
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(
	    swap, (int)(EM3_QUALITY_HEADER_SIZE + ping->png_quality_nbeams * ping->png_quality_nparameters * sizeof(float) + 8),
	    (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);

	/* write the record label */
	labelchar = (char *)&label;
	labelchar[0] = EM3_START_BYTE;
	labelchar[1] = EM3_ID_QUALITY;
	write_len = 2;
	int status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

	/* compute checksum */
	uchar_ptr = (mb_u_char *)&label;
	checksum += uchar_ptr[1];

	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)ping->png_quality_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)ping->png_quality_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)ping->png_quality_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)ping->png_quality_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)ping->png_quality_nbeams, (void *)&line[12]);
		line[14] = (mb_u_char)ping->png_quality_nparameters;
		line[15] = (mb_u_char)ping->png_quality_spare;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_QUALITY_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_QUALITY_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary beam data */
	if (status == MB_SUCCESS) {
		write_len = (size_t)(ping->png_quality_nparameters * sizeof(float));
		for (int i = 0; i < ping->png_quality_nbeams; i++) {
			index = 0;
			for (int j = 0; j < ping->png_quality_nparameters; j++) {
				mb_put_binary_float(swap, ping->png_quality_parameters[i][j], (void *)&line[index]);
				index += 4;
			}

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (unsigned int j = 0; j < write_len; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}
	}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = 0x00;
		line[1] = 0x03;

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, &line[0], &write_len, error);
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
int mbr_em710raw_wr_ss2(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_ping_struct *ping;
	char line[EM3_SS2_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* set which storage structure to use */
	ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:               %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:              %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:               %d\n", store->date);
		fprintf(stderr, "dbg5       msec:               %d\n", store->msec);
		fprintf(stderr, "dbg5       png_date:           %d\n", ping->png_date);
		fprintf(stderr, "dbg5       png_msec:           %d\n", ping->png_msec);

		fprintf(stderr, "dbg5       png_date:              %d\n", ping->png_date);
		fprintf(stderr, "dbg5       png_msec:              %d\n", ping->png_msec);
		fprintf(stderr, "dbg5       png_count:             %d\n", ping->png_count);
		fprintf(stderr, "dbg5       png_serial:            %d\n", ping->png_serial);
		fprintf(stderr, "dbg5       png_heading:           %d\n", ping->png_heading);
		fprintf(stderr, "dbg5       png_ssv:               %d\n", ping->png_ssv);
		fprintf(stderr, "dbg5       png_xducer_depth:      %f\n", ping->png_xducer_depth);
		fprintf(stderr, "dbg5       png_nbeams:            %d\n", ping->png_nbeams);
		fprintf(stderr, "dbg5       png_nbeams_valid:      %d\n", ping->png_nbeams_valid);
		fprintf(stderr, "dbg5       png_sample_rate:       %f\n", ping->png_sample_rate);
		fprintf(stderr, "dbg5       png_spare:             %d\n", ping->png_spare);
		fprintf(stderr, "dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %7.2f %7.2f %7.2f %5d %5d %5d %4d %3d %3d\n", i, ping->png_depth[i],
			        ping->png_acrosstrack[i], ping->png_alongtrack[i], ping->png_window[i], ping->png_quality[i],
			        ping->png_iba[i], ping->png_detection[i], ping->png_clean[i], ping->png_amp[i]);

		fprintf(stderr, "dbg5       png_ss_date:        %d\n", ping->png_ss_date);
		fprintf(stderr, "dbg5       png_ss_msec:        %d\n", ping->png_ss_msec);
		fprintf(stderr, "dbg5       png_ss_count:       %d\n", ping->png_ss_count);
		fprintf(stderr, "dbg5       png_ss_serial:      %d\n", ping->png_ss_serial);
		fprintf(stderr, "dbg5       png_ss_sample_rate: %f\n", ping->png_ss_sample_rate);
		fprintf(stderr, "dbg5       png_r_zero:         %d\n", ping->png_r_zero);
		fprintf(stderr, "dbg5       png_bsn:            %d\n", ping->png_bsn);
		fprintf(stderr, "dbg5       png_bso:            %d\n", ping->png_bso);
		fprintf(stderr, "dbg5       png_tx:             %d\n", ping->png_tx);
		fprintf(stderr, "dbg5       png_tvg_crossover:  %d\n", ping->png_tvg_crossover);
		fprintf(stderr, "dbg5       png_nbeams_ss:      %d\n", ping->png_nbeams_ss);
		fprintf(stderr, "dbg5       png_npixels:        %d\n", ping->png_npixels);
		fprintf(stderr, "dbg5       cnt  index sort samples start center\n");
		fprintf(stderr, "dbg5       --------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams_ss; i++)
			fprintf(stderr, "dbg5        %4d %2d %4d %4d %4d %4d\n", i, ping->png_sort_direction[i], ping->png_ssdetection[i],
			        ping->png_beam_samples[i], ping->png_start_sample[i], ping->png_center_sample[i]);
		fprintf(stderr, "dbg5       cnt  ss\n");
		fprintf(stderr, "dbg5       --------------------------------------------------\n");
		for (int i = 0; i < ping->png_npixels; i++)
			fprintf(stderr, "dbg5        %d %d\n", i, ping->png_ssraw[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(
	    swap, (int)(EM3_SS2_HEADER_SIZE + EM3_SS2_BEAM_SIZE * ping->png_nbeams_ss + sizeof(short) * ping->png_npixels + 8),
	    (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);

	/* write the record label */
	labelchar = (char *)&label;
	labelchar[0] = EM3_START_BYTE;
	labelchar[1] = EM3_ID_SS2;
	write_len = 2;
	int status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

	/* compute checksum */
	uchar_ptr = (mb_u_char *)&label;
	checksum += uchar_ptr[1];

	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)ping->png_ss_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)ping->png_ss_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)ping->png_ss_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)ping->png_ss_serial, (void *)&line[10]);
		mb_put_binary_float(swap, ping->png_ss_sample_rate, (void *)&line[12]);
		mb_put_binary_short(swap, (unsigned short)ping->png_r_zero, (void *)&line[16]);
		mb_put_binary_short(swap, (short)ping->png_bsn, (void *)&line[18]);
		mb_put_binary_short(swap, (short)ping->png_bso, (void *)&line[20]);
		mb_put_binary_short(swap, (unsigned short)ping->png_tx, (void *)&line[22]);
		mb_put_binary_short(swap, (unsigned short)ping->png_tvg_crossover, (void *)&line[24]);
		mb_put_binary_short(swap, (unsigned short)ping->png_nbeams_ss, (void *)&line[26]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_SS2_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_SS2_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary beam data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_nbeams_ss; i++) {
			line[0] = (mb_s_char)ping->png_sort_direction[i];
			line[1] = (mb_u_char)ping->png_ssdetection[i];
			mb_put_binary_short(swap, (unsigned short)ping->png_beam_samples[i], (void *)&line[2]);
			mb_put_binary_short(swap, (unsigned short)ping->png_center_sample[i], (void *)&line[4]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_SS2_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_SS2_BEAM_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}

	/* output sidescan data */
	if (status == MB_SUCCESS) {
		write_size = 2 * ping->png_npixels;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)ping->png_ssraw;
		for (int j = 0; j < write_size; j++)
			checksum += uchar_ptr[j];

		write_len = write_size;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)ping->png_ssraw, &write_len, error);
	}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = 0;
		line[1] = 0x03;

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 4;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
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
int mbr_em710raw_wr_wc(int verbose, void *mbio_ptr, int swap, struct mbsys_simrad3_struct *store, int *error) {
	struct mbsys_simrad3_watercolumn_struct *wc;
	char line[EM3_WC_HEADER_SIZE];
	short label;
	char *labelchar;
	size_t write_len;
	int write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;
	int record_size;
	int pad;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	wc = (struct mbsys_simrad3_watercolumn_struct *)store->wc;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       wtc_date:        %d\n", wc->wtc_date);
		fprintf(stderr, "dbg5       wtc_msec:        %d\n", wc->wtc_msec);
		fprintf(stderr, "dbg5       wtc_count:       %d\n", wc->wtc_count);
		fprintf(stderr, "dbg5       wtc_serial:      %d\n", wc->wtc_serial);

		fprintf(stderr, "dbg5       wtc_ndatagrams:  %d\n", wc->wtc_ndatagrams);
		fprintf(stderr, "dbg5       wtc_datagram:    %d\n", wc->wtc_datagram);
		fprintf(stderr, "dbg5       wtc_ntx:         %d\n", wc->wtc_ntx);
		fprintf(stderr, "dbg5       wtc_nrx:         %d\n", wc->wtc_nrx);
		fprintf(stderr, "dbg5       wtc_nbeam:       %d\n", wc->wtc_nbeam);
		fprintf(stderr, "dbg5       wtc_ssv:         %d\n", wc->wtc_ssv);
		fprintf(stderr, "dbg5       wtc_sfreq:       %d\n", wc->wtc_sfreq);
		fprintf(stderr, "dbg5       wtc_heave:       %d\n", wc->wtc_heave);
		fprintf(stderr, "dbg5       wtc_spare1:      %d\n", wc->wtc_spare1);
		fprintf(stderr, "dbg5       wtc_spare2:      %d\n", wc->wtc_spare2);
		fprintf(stderr, "dbg5       wtc_spare3:      %d\n", wc->wtc_spare3);
		fprintf(stderr, "dbg5       ---------------------------\n");
		fprintf(stderr, "dbg5       cnt  tilt center sector\n");
		fprintf(stderr, "dbg5       ---------------------------\n");
		for (int i = 0; i < wc->wtc_ntx; i++)
			fprintf(stderr, "dbg5       %3d %6d %6d %6d\n", i, wc->wtc_txtiltangle[i], wc->wtc_txcenter[i], wc->wtc_txsector[i]);
		for (int i = 0; i < wc->wtc_nbeam; i++) {
			fprintf(stderr, "dbg5       --------------------------------------------------\n");
			fprintf(stderr, "dbg5       cnt  angle start samples unknown sector beam\n");
			fprintf(stderr, "dbg5       --------------------------------------------------\n");
			fprintf(stderr, "dbg5        %4d %3d %2d %4d %4d %4d %4d\n", i, wc->beam[i].wtc_rxpointangle,
			        wc->beam[i].wtc_start_sample, wc->beam[i].wtc_beam_samples, wc->beam[i].wtc_beam_spare,
			        wc->beam[i].wtc_sector, wc->beam[i].wtc_beam);
			/*			fprintf(stderr,"dbg5       --------------------------------------------------\n");
			            fprintf(stderr,"dbg5       beam[%d]: sample amplitude\n",i);
			            fprintf(stderr,"dbg5       --------------------------------------------------\n");
			            for (j=0;j<wc->beam[i].wtc_beam_samples;j++)
			                fprintf(stderr,"dbg5        %4d %4d\n",
			                    j, wc->beam[i].wtc_amp[j]);*/
		}
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	record_size = EM3_WC_HEADER_SIZE + EM3_WC_BEAM_SIZE * wc->wtc_nbeam + EM3_WC_TX_SIZE * wc->wtc_ntx + 8;
	for (int i = 0; i < wc->wtc_nbeam; i++) {
		record_size += wc->beam[i].wtc_beam_samples;
	}
	pad = (record_size % 2);
	record_size += pad;
	mb_put_binary_int(swap, record_size, (void *)&write_size);
	write_len = 4;
	mb_fileio_put(verbose, mbio_ptr, (char *)&write_size, &write_len, error);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		labelchar = (char *)&label;
		labelchar[0] = EM3_START_BYTE;
		labelchar[1] = EM3_ID_WATERCOLUMN;
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = 2;
		status = mb_fileio_put(verbose, mbio_ptr, (char *)&label, &write_len, error);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)wc->wtc_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)wc->wtc_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_ndatagrams, (void *)&line[12]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_datagram, (void *)&line[14]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_ntx, (void *)&line[16]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_nrx, (void *)&line[18]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_nbeam, (void *)&line[20]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_ssv, (void *)&line[22]);
		mb_put_binary_int(swap, (int)wc->wtc_sfreq, (void *)&line[24]);
		mb_put_binary_short(swap, (short)wc->wtc_heave, (void *)&line[28]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_spare1, (void *)&line[30]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_spare2, (void *)&line[32]);
		mb_put_binary_short(swap, (unsigned short)wc->wtc_spare3, (void *)&line[34]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM3_WC_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = EM3_WC_HEADER_SIZE;
		status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
	}

	/* output binary beam data */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < wc->wtc_ntx; i++) {
			mb_put_binary_short(swap, (short)wc->wtc_txtiltangle[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)wc->wtc_txcenter[i], (void *)&line[2]);
			line[4] = (mb_u_char)wc->wtc_txsector[i];
			line[5] = (mb_u_char)0;

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_WC_TX_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_WC_TX_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);
		}
		for (int i = 0; i < wc->wtc_nbeam; i++) {
			mb_put_binary_short(swap, (short)wc->beam[i].wtc_rxpointangle, (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)wc->beam[i].wtc_start_sample, (void *)&line[2]);
			mb_put_binary_short(swap, (unsigned short)wc->beam[i].wtc_beam_samples, (void *)&line[4]);
			mb_put_binary_short(swap, (unsigned short)wc->beam[i].wtc_beam_spare, (void *)&line[6]);
			line[8] = (mb_u_char)wc->beam[i].wtc_sector;
			line[9] = (mb_u_char)wc->beam[i].wtc_beam;

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM3_WC_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = EM3_WC_BEAM_SIZE;
			status = mb_fileio_put(verbose, mbio_ptr, line, &write_len, error);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)wc->beam[i].wtc_amp;
			for (int j = 0; j < wc->beam[i].wtc_beam_samples; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = wc->beam[i].wtc_beam_samples;
			status = mb_fileio_put(verbose, mbio_ptr, (char *)wc->beam[i].wtc_amp, &write_len, error);
		}
	}

	/* output end of record */
	if (status == MB_SUCCESS) {
		if (pad == 1) {
			line[0] = 0;
			checksum += line[0];
		}
		line[1] = 0x03;

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = 3 + pad;
		status = mb_fileio_put(verbose, mbio_ptr, &line[!pad], &write_len, error);
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
int mbr_em710raw_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to raw data structure */
	struct mbsys_simrad3_struct *store = (struct mbsys_simrad3_struct *)store_ptr;

#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "\nstart of mbr_em710raw_wr_data:\n");
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	fprintf(stderr, "kind:%d %d type:%x\n", store->kind, mb_io_ptr->new_kind, store->type);
#endif

	/* figure out which storage structure to use */
	struct mbsys_simrad3_ping_struct *ping = (struct mbsys_simrad3_ping_struct *)&(store->pings[store->ping_index]);

	/* set swap flag */
	const bool swap = true;

	int status = MB_SUCCESS;
	if (store->kind == MB_DATA_COMMENT || store->kind == MB_DATA_START || store->kind == MB_DATA_STOP) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_start kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_start\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_start(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_STATUS && store->type == EM3_PU_ID) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_puid kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_puid\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_puid(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_STATUS && store->type == EM3_PU_STATUS) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_status kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_status\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_status(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_RUN_PARAMETER) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_run_parameter kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_run_parameter\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_run_parameter(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_CLOCK) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_clock kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_clock\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_clock(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_TIDE) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_tide kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_tide\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_tide(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_HEIGHT) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_height kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_height\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_height(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_HEADING) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_heading kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_heading\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_heading(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_SSV) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_ssv kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_ssv\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_ssv(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_TILT) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_tilt kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_tilt\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_tilt(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_PARAMETER) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_extraparameters kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_extraparameters\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_extraparameters(verbose, mbio_ptr, swap, store, error);
	}
	else if ((store->kind == MB_DATA_ATTITUDE || store->kind == MB_DATA_ATTITUDE1) && store->type == EM3_ATTITUDE) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_attitude kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_attitude\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_attitude(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_ATTITUDE2 && store->type == EM3_NETATTITUDE) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_netattitude kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_netattitude\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_netattitude(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_NAV || store->kind == MB_DATA_NAV1 || store->kind == MB_DATA_NAV2 ||
	         store->kind == MB_DATA_NAV3) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_pos kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_pos\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_pos(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_VELOCITY_PROFILE) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_svp kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_svp\n\n", store->type, store->sonar);
#endif
#endif
		if (store->type == EM3_SVP)
			status = mbr_em710raw_wr_svp(verbose, mbio_ptr, swap, store, error);
		else
			status = mbr_em710raw_wr_svp2(verbose, mbio_ptr, swap, store, error);
	}
	else if (store->kind == MB_DATA_DATA) {
		if (ping->png_raw_read) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_wr_rawbeam4 kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_rawbeam4: sonar 1 ping:%d\n", store->type,
			        store->sonar, store->pings[store->ping_index].png_count);
#endif
#endif
			status = mbr_em710raw_wr_rawbeam4(verbose, mbio_ptr, swap, store, error);
		}
#ifdef MBR_EM710RAW_DEBUG
		else
			fprintf(stderr, "NOT call mbr_em710raw_wr_rawbeam4 kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		else
			fprintf(stderr, "NOT call mbr_em710raw_wr_rawbeam4\n");
#endif
#endif

		if (ping->png_quality_read) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_wr_quality kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_quality: sonar 1 ping:%d\n", store->type,
			        store->sonar, store->pings[store->ping_index].png_count);
#endif
#endif
			status = mbr_em710raw_wr_quality(verbose, mbio_ptr, swap, store, error);
		}
#ifdef MBR_EM710RAW_DEBUG
		else
			fprintf(stderr, "NOT call mbr_em710raw_wr_quality kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		else
			fprintf(stderr, "NOT call mbr_em710raw_wr_quality\n");
#endif
#endif

#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_bath2 kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_bath2: sonar 1 ping:%d\n", store->type,
		        store->sonar, store->pings[store->ping_index].png_count);
#endif
#endif
		status = mbr_em710raw_wr_bath2(verbose, mbio_ptr, swap, store, error);
		if (ping->png_ss_read) {
#ifdef MBR_EM710RAW_DEBUG
			fprintf(stderr, "call mbr_em710raw_wr_ss2 kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
			fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_ss2: sonar 1 ping:%d\n\n", store->type,
			        store->sonar, store->pings[store->ping_index].png_count);
#endif
#endif
			status = mbr_em710raw_wr_ss2(verbose, mbio_ptr, swap, store, error);
		}
#ifdef MBR_EM710RAW_DEBUG
		else
			fprintf(stderr, "NOT call mbr_em710raw_wr_ss2 kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		else
			fprintf(stderr, "NOT call mbr_em710raw_wr_ss2\n\n");
#endif
#endif
	}
	else if (store->kind == MB_DATA_WATER_COLUMN) {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call mbr_em710raw_wr_wc kind:%d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "type:%x sonar:%d                      mbr_em710raw_wr_wc\n\n", store->type, store->sonar);
#endif
#endif
		status = mbr_em710raw_wr_wc(verbose, mbio_ptr, swap, store, error);
	}
	else {
#ifdef MBR_EM710RAW_DEBUG
		fprintf(stderr, "call nothing bad kind: %d type %x\n", store->kind, store->type);
#else
#ifdef MBR_EM710RAW_DEBUG3
		fprintf(stderr, "call nothing bad kind: %d type %x\n\n", store->kind, store->type);
#endif
#endif
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
	}

#ifdef MBR_EM710RAW_DEBUG
	fprintf(stderr, "status:%d error:%d\n", status, *error);
	fprintf(stderr, "end of mbr_em710raw_wr_data:\n");
#endif

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
int mbr_wt_em710raw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* write next data to file */
	const int status = mbr_em710raw_wr_data(verbose, mbio_ptr, store_ptr, error);

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
int mbr_register_em710raw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_em710raw(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_em710raw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_em710raw;
	mb_io_ptr->mb_io_store_alloc = &mbsys_simrad3_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_simrad3_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_em710raw;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_em710raw;
	mb_io_ptr->mb_io_dimensions = &mbsys_simrad3_dimensions;
	mb_io_ptr->mb_io_pingnumber = &mbsys_simrad3_pingnumber;
	mb_io_ptr->mb_io_sonartype = &mbsys_simrad3_sonartype;
	mb_io_ptr->mb_io_sidescantype = &mbsys_simrad3_sidescantype;
	mb_io_ptr->mb_io_preprocess = &mbsys_simrad3_preprocess;
	mb_io_ptr->mb_io_extract_platform = &mbsys_simrad3_extract_platform;
	mb_io_ptr->mb_io_extract = &mbsys_simrad3_extract;
	mb_io_ptr->mb_io_insert = &mbsys_simrad3_insert;
	mb_io_ptr->mb_io_extract_nnav = &mbsys_simrad3_extract_nnav;
	mb_io_ptr->mb_io_extract_nav = &mbsys_simrad3_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_simrad3_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_simrad3_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_simrad3_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_simrad3_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_simrad3_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_simrad3_detects;
	mb_io_ptr->mb_io_pulses = &mbsys_simrad3_pulses;
	mb_io_ptr->mb_io_gains = &mbsys_simrad3_gains;
	mb_io_ptr->mb_io_copyrecord = &mbsys_simrad3_copy;
  mb_io_ptr->mb_io_makess = &mbsys_simrad3_makess;
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
		fprintf(stderr, "dbg2       pulses:             %p\n", (void *)mb_io_ptr->mb_io_pulses);
    fprintf(stderr, "dbg2       makess:             %p\n", (void *)mb_io_ptr->mb_io_makess);
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
