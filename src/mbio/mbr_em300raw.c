/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em300raw.c	10/16/98
 *
 *    Copyright (c) 1998-2025 by
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
 * mbr_em300raw.c contains the functions for reading and writing
 * multibeam data in the EM300RAW format.
 * These functions include:
 *   mbr_alm_em300raw	- allocate read/write memory
 *   mbr_dem_em300raw	- deallocate read/write memory
 *   mbr_rt_em300raw	- read and translate data
 *   mbr_wt_em300raw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 16,  1998
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_swap.h"
#include "mbsys_simrad2.h"

/* turn on debug statements here */
// #define MBR_EM300RAW_DEBUG 1

/*--------------------------------------------------------------------*/
int mbr_info_em300raw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_SIMRAD2;
	*beams_bath_max = 254;
	*beams_amp_max = 254;
	*pixels_ss_max = 1024;
	strncpy(format_name, "EM300RAW", MB_NAME_LENGTH);
	strncpy(system_name, "SIMRAD2", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_EM300RAW\nInformal Description: Simrad current multibeam vendor format\nAttributes:       "
	        "    Simrad EM120, EM300, EM1002, EM3000, \n                      bathymetry, amplitude, and sidescan,\n             "
	        "         up to 254 beams, variable pixels, ascii + binary, Simrad.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = true;
	*traveltime = true;
	*beam_flagging = false;
	*platform_source = MB_DATA_START;
	*nav_source = MB_DATA_NAV;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
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
int mbr_alm_em300raw(int verbose, void *mbio_ptr, int *error) {
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
	const int status = mbsys_simrad2_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* initialize saved values */
	int *databyteswapped = (int *)&mb_io_ptr->save10;
	double *pixel_size = &mb_io_ptr->saved1;
	double *swath_width = &mb_io_ptr->saved2;
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
int mbr_dem_em300raw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	const int status = mbsys_simrad2_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_em300raw_chk_label(int verbose, void *mbio_ptr, char *label, short *type, short *sonar) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       label:      %x%x%x%x\n", label[0], label[1], label[2], label[3]);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	short *sonar_save = (short *)(&mb_io_ptr->save4);
	int *databyteswapped = (int *)&mb_io_ptr->save10;

#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr, "Check label: %x|%x|%x|%x\n", label[0], label[1], label[2], label[3]);
#endif

	/* check for valid start byte and type */
	const mb_u_char startbyte = label[0];
	const mb_u_char typebyte = label[1];
	bool typegood = true;
	if (startbyte == EM2_START_BYTE &&
	    (typebyte == EM2_ID_STOP2 || typebyte == EM2_ID_OFF || typebyte == EM2_ID_ON || typebyte == EM2_ID_EXTRAPARAMETERS ||
	     typebyte == EM2_ID_ATTITUDE || typebyte == EM2_ID_CLOCK || typebyte == EM2_ID_BATH || typebyte == EM2_ID_SBDEPTH ||
	     typebyte == EM2_ID_RAWBEAM || typebyte == EM2_ID_SSV || typebyte == EM2_ID_HEADING || typebyte == EM2_ID_START ||
	     typebyte == EM2_ID_TILT || typebyte == EM2_ID_CBECHO || typebyte == EM2_ID_POS || typebyte == EM2_ID_RUN_PARAMETER ||
	     typebyte == EM2_ID_SS || typebyte == EM2_ID_TIDE || typebyte == EM2_ID_SVP2 || typebyte == EM2_ID_SVP ||
	     typebyte == EM2_ID_SSPINPUT || typebyte == EM2_ID_RAWBEAM2 || typebyte == EM2_ID_RAWBEAM3 || typebyte == EM2_ID_HEIGHT ||
	     typebyte == EM2_ID_STOP || typebyte == EM2_ID_WATERCOLUMN || typebyte == EM2_ID_REMOTE || typebyte == EM2_ID_SSP ||
	     typebyte == EM2_ID_BATH_MBA || typebyte == EM2_ID_SS_MBA)) {
		typegood = true;
	}
	else {
		typegood = false;
	}

	bool sonargood = true;

	/* check for data byte swapping if necessary */
	if (typegood && *databyteswapped == -1) {
		const short sonarunswap = *((short *)&label[2]);
		const short sonarswap = mb_swap_short(sonarunswap);

		/* check for valid sonarunswap */
                bool sonarunswapgood = true;
		if (sonarunswap == MBSYS_SIMRAD2_EM120 || sonarunswap == MBSYS_SIMRAD2_EM300 || sonarunswap == MBSYS_SIMRAD2_EM1002 ||
		    sonarunswap == MBSYS_SIMRAD2_EM2000 || sonarunswap == MBSYS_SIMRAD2_EM3000 ||
		    sonarunswap == MBSYS_SIMRAD2_EM3000D_1 || sonarunswap == MBSYS_SIMRAD2_EM3000D_2 ||
		    sonarunswap == MBSYS_SIMRAD2_EM3000D_3 || sonarunswap == MBSYS_SIMRAD2_EM3000D_4 ||
		    sonarunswap == MBSYS_SIMRAD2_EM3000D_5 || sonarunswap == MBSYS_SIMRAD2_EM3000D_6 ||
		    sonarunswap == MBSYS_SIMRAD2_EM3000D_7 || sonarunswap == MBSYS_SIMRAD2_EM3000D_8 ||
		    sonarunswap == MBSYS_SIMRAD2_EM3002 || sonarunswap == MBSYS_SIMRAD2_EM710) {
			sonarunswapgood = true;
		}
		else {
			sonarunswapgood = false;
		}

		/* check for valid sonarswap */
		bool sonarswapgood = true;
		if (sonarswap == MBSYS_SIMRAD2_EM120 || sonarswap == MBSYS_SIMRAD2_EM300 || sonarswap == MBSYS_SIMRAD2_EM1002 ||
		    sonarswap == MBSYS_SIMRAD2_EM2000 || sonarswap == MBSYS_SIMRAD2_EM3000 || sonarswap == MBSYS_SIMRAD2_EM3000D_1 ||
		    sonarswap == MBSYS_SIMRAD2_EM3000D_2 || sonarswap == MBSYS_SIMRAD2_EM3000D_3 ||
		    sonarswap == MBSYS_SIMRAD2_EM3000D_4 || sonarswap == MBSYS_SIMRAD2_EM3000D_5 ||
		    sonarswap == MBSYS_SIMRAD2_EM3000D_6 || sonarswap == MBSYS_SIMRAD2_EM3000D_7 ||
		    sonarswap == MBSYS_SIMRAD2_EM3000D_8 || sonarswap == MBSYS_SIMRAD2_EM3002 || sonarswap == MBSYS_SIMRAD2_EM710) {
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
	/* swap = *databyteswapped; */

	*type = *((short *)&label[0]);
	*sonar = *((short *)&label[2]);
	if (mb_io_ptr->byteswapped)
		*type = mb_swap_short(*type);
	if (*databyteswapped != mb_io_ptr->byteswapped) {
		*sonar = mb_swap_short(*sonar);
	}

#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr, "typegood:%d mb_io_ptr->byteswapped:%d sonarswapgood:%d *databyteswapped:%d *type:%d *sonar:%d\n", typegood,
	        mb_io_ptr->byteswapped, sonarswapgood, *databyteswapped, *type, *sonar);
#endif

	/* check for valid sonar */
	if (*sonar != MBSYS_SIMRAD2_EM120 && *sonar != MBSYS_SIMRAD2_EM300 && *sonar != MBSYS_SIMRAD2_EM1002 &&
	    *sonar != MBSYS_SIMRAD2_EM2000 && *sonar != MBSYS_SIMRAD2_EM3000 && *sonar != MBSYS_SIMRAD2_EM3000D_1 &&
	    *sonar != MBSYS_SIMRAD2_EM3000D_2 && *sonar != MBSYS_SIMRAD2_EM3000D_3 && *sonar != MBSYS_SIMRAD2_EM3000D_4 &&
	    *sonar != MBSYS_SIMRAD2_EM3000D_5 && *sonar != MBSYS_SIMRAD2_EM3000D_6 && *sonar != MBSYS_SIMRAD2_EM3000D_7 &&
	    *sonar != MBSYS_SIMRAD2_EM3000D_8 && *sonar != MBSYS_SIMRAD2_EM3002 && *sonar != MBSYS_SIMRAD2_EM710) {
		sonargood = false;
	}
	else {
		sonargood = true;
	}

	if (startbyte == EM2_START_BYTE && !typegood && sonargood) {
		mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_BAD_DATAGRAM);
		if (verbose >= 1)
			fprintf(stderr, "Bad datagram type: %4.4hX %4.4hX | %d %d\n", *type, *sonar, *type, *sonar);
	}
	int status = MB_SUCCESS;
	if (!typegood || !sonargood) {
		status = MB_FAILURE;
	}

	/* save sonar if successful */
	if (status == MB_SUCCESS)
		*sonar_save = *sonar;

	/* allow exception found in some EM3000 data */
	if (*type == EM2_SVP && *sonar == 0 && *sonar_save == MBSYS_SIMRAD2_EM3000) {
		status = MB_SUCCESS;
		*sonar = *sonar_save;
	}

	/* allow exception found in some data */
	if (*type == EM2_SSV && *sonar == 0 && *sonar_save != 0) {
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
int mbr_em300raw_rd_start(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short type, short sonar,
                          int *version, int *goodend, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       type:       %d\n", type);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* make sure comment is initialized */
	store->par_com[0] = '\0';

	/* set type value */
	store->type = type;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary values into char array */
	char line[MBSYS_SIMRAD2_BUFFER_SIZE];
	int read_len = fread(line, 1, EM2_START_HEADER_SIZE, mbfp);
	if (read_len == EM2_START_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->par_date);
		store->date = store->par_date;
		mb_get_binary_int(swap, &line[4], &store->par_msec);
		store->msec = store->par_msec;
		unsigned short ushort_val = 0;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		store->par_line_num = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		store->par_serial_1 = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		store->par_serial_2 = (int)(ushort_val);
	}

	/* check for dual head sonars */
	if (store->par_serial_1 != 0 && store->par_serial_2 != 0)
		store->numberheads = 2;

	/* now loop over reading individual characters to
	    handle ASCII parameter values */
	unsigned int len = 0;
	bool done = false;
	while (status == MB_SUCCESS && !done) {
		read_len = fread(&line[len], 1, 1, mbfp);
		if (read_len == 1) {
			status = MB_SUCCESS;
			len++;
		}
		else {
			done = true;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
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
			else if (strncmp("GO1=", line, 4) == 0)
				mb_get_double(&(store->par_go1), &line[4], len - 5);
			else if (strncmp("GO2=", line, 4) == 0)
				mb_get_double(&(store->par_go2), &line[4], len - 5);
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
				int i1;
				int i2;
				int i3;
				if (sscanf(store->par_psv, "%d.%d.%d", &i1, &i2, &i3) == 3)
					*version = i3 + 100 * i2 + 10000 * i1;
			}
			else if (strncmp("OSV=", line, 4) == 0)
				strncpy(store->par_osv, &line[4], MIN(len - 5, 15));
			else if (strncmp("DSD=", line, 4) == 0)
				mb_get_double(&(store->par_dsd), &line[4], len - 5);
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
			else if (strncmp("GCG=", line, 4) == 0)
				mb_get_double(&(store->par_gcg), &line[4], len - 5);
			else if (strncmp("CPR=", line, 4) == 0)
				strncpy(store->par_cpr, &line[4], MIN(len - 5, 3));
			else if (strncmp("ROP=", line, 4) == 0)
				strncpy(store->par_rop, &line[4], MIN(len - 5, MBSYS_SIMRAD2_COMMENT_LENGTH - 1));
			else if (strncmp("SID=", line, 4) == 0)
				strncpy(store->par_sid, &line[4], MIN(len - 5, MBSYS_SIMRAD2_COMMENT_LENGTH - 1));
			else if (strncmp("PLL=", line, 4) == 0)
				strncpy(store->par_pll, &line[4], MIN(len - 5, MBSYS_SIMRAD2_COMMENT_LENGTH - 1));
			else if (strncmp("COM=", line, 4) == 0) {
				strncpy(store->par_com, &line[4], MIN(len - 5, MBSYS_SIMRAD2_COMMENT_LENGTH - 1));
				store->par_com[MIN(len - 5, MBSYS_SIMRAD2_COMMENT_LENGTH - 1)] = 0;
				/* replace caret (^) values with commas (,) to circumvent
				   the format's inability to store commas in comments */
				char *comma_ptr;
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

	/* now set the data kind */
	if (status == MB_SUCCESS) {
		if (strlen(store->par_com) > 0 && store->par_date == 0)
			store->kind = MB_DATA_COMMENT;
		else if (store->type == EM2_START)
			store->kind = MB_DATA_START;
		else if (store->type == EM2_STOP)
			store->kind = MB_DATA_STOP;
		else if (store->type == EM2_STOP2)
			store->kind = MB_DATA_STOP;
		else if (store->type == EM2_OFF)
			store->kind = MB_DATA_STOP;
		else if (store->type == EM2_ON)
			store->kind = MB_DATA_START;
	}

	/* read end of record and last two check sum bytes */
	if (status == MB_SUCCESS) {
		/* if EM2_END not yet found then the
		next byte should be EM2_END */
		if (line[0] != EM2_END) {
			/* read_len = */ fread(&line[0], 1, 1, mbfp);
		}

		/* if EM2_END not yet found then the
		next byte should be EM2_END */
		if (line[0] != EM2_END) {
			/* read_len = */ fread(&line[0], 1, 1, mbfp);
		}

		/* if we got the end byte then get check sum bytes */
		if (line[0] == EM2_END) {
			if (line[0] == EM2_END)
				*goodend = true;
			/* read_len = */ fread(&line[1], 2, 1, mbfp);
/* don't check success of read
	- return success here even if read fails
	because all of the
important information in this record has
already been read - next attempt to read
file will return error */
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[0], line[0], line[1], line[1], line[2],
			        line[2]);
#endif
		}
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "\n");
#endif
	}

	if (verbose >= 2) {
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
		fprintf(stderr, "dbg5       par_go1:         %f\n", store->par_go1);
		fprintf(stderr, "dbg5       par_go2:         %f\n", store->par_go2);
		fprintf(stderr, "dbg5       par_tsv:         %s\n", store->par_tsv);
		fprintf(stderr, "dbg5       par_rsv:         %s\n", store->par_rsv);
		fprintf(stderr, "dbg5       par_bsv:         %s\n", store->par_bsv);
		fprintf(stderr, "dbg5       par_psv:         %s\n", store->par_psv);
		fprintf(stderr, "dbg5       par_osv:         %s\n", store->par_osv);
		fprintf(stderr, "dbg5       par_dsd:         %f\n", store->par_dsd);
		fprintf(stderr, "dbg5       par_dso:         %f\n", store->par_dso);
		fprintf(stderr, "dbg5       par_dsf:         %f\n", store->par_dsf);
		fprintf(stderr, "dbg5       par_dsh:         %c%c\n", store->par_dsh[0], store->par_dsh[1]);
		fprintf(stderr, "dbg5       par_aps:         %d\n", store->par_aps);
		fprintf(stderr, "dbg5       par_p1m:         %d\n", store->par_p1m);
		fprintf(stderr, "dbg5       par_p1t:         %d\n", store->par_p1t);
		fprintf(stderr, "dbg5       par_p1z:         %f\n", store->par_p1z);
		fprintf(stderr, "dbg5       par_p1x:         %f\n", store->par_p1x);
		fprintf(stderr, "dbg5       par_p1y:         %f\n", store->par_p1y);
		fprintf(stderr, "dbg5       par_p1d:         %f\n", store->par_p1d);
		fprintf(stderr, "dbg5       par_p1g:         %s\n", store->par_p1g);
		fprintf(stderr, "dbg5       par_p2m:         %d\n", store->par_p2m);
		fprintf(stderr, "dbg5       par_p2t:         %d\n", store->par_p2t);
		fprintf(stderr, "dbg5       par_p2z:         %f\n", store->par_p2z);
		fprintf(stderr, "dbg5       par_p2x:         %f\n", store->par_p2x);
		fprintf(stderr, "dbg5       par_p2y:         %f\n", store->par_p2y);
		fprintf(stderr, "dbg5       par_p2d:         %f\n", store->par_p2d);
		fprintf(stderr, "dbg5       par_p2g:         %s\n", store->par_p2g);
		fprintf(stderr, "dbg5       par_p3m:         %d\n", store->par_p3m);
		fprintf(stderr, "dbg5       par_p3t:         %d\n", store->par_p3t);
		fprintf(stderr, "dbg5       par_p3z:         %f\n", store->par_p3z);
		fprintf(stderr, "dbg5       par_p3x:         %f\n", store->par_p3x);
		fprintf(stderr, "dbg5       par_p3y:         %f\n", store->par_p3y);
		fprintf(stderr, "dbg5       par_p3d:         %f\n", store->par_p3d);
		fprintf(stderr, "dbg5       par_p3g:         %s\n", store->par_p3g);
		fprintf(stderr, "dbg5       par_msz:         %f\n", store->par_msz);
		fprintf(stderr, "dbg5       par_msx:         %f\n", store->par_msx);
		fprintf(stderr, "dbg5       par_msy:         %f\n", store->par_msy);
		fprintf(stderr, "dbg5       par_mrp:         %c%c\n", store->par_mrp[0], store->par_mrp[1]);
		fprintf(stderr, "dbg5       par_msd:         %f\n", store->par_msd);
		fprintf(stderr, "dbg5       par_msr:         %f\n", store->par_msr);
		fprintf(stderr, "dbg5       par_msp:         %f\n", store->par_msp);
		fprintf(stderr, "dbg5       par_msg:         %f\n", store->par_msg);
		fprintf(stderr, "dbg5       par_gcg:         %f\n", store->par_gcg);
		fprintf(stderr, "dbg5       par_cpr:         %s\n", store->par_cpr);
		fprintf(stderr, "dbg5       par_rop:         %s\n", store->par_rop);
		fprintf(stderr, "dbg5       par_sid:         %s\n", store->par_sid);
		fprintf(stderr, "dbg5       par_pll:         %s\n", store->par_pll);
		fprintf(stderr, "dbg5       par_com:         %s\n", store->par_com);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       version:    %d\n", *version);
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_run_parameter(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar,
                                  int *goodend, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_RUN_PARAMETER;
	store->type = EM2_RUN_PARAMETER;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary values into char array */
	char line[EM2_RUN_PARAMETER_SIZE];
	int read_len = fread(line, 1, EM2_RUN_PARAMETER_SIZE - 4, mbfp);
	if (read_len == EM2_RUN_PARAMETER_SIZE - 4)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->run_date);
		if (store->run_date != 0)
			store->date = store->run_date;
		mb_get_binary_int(swap, &line[4], &store->run_msec);
		if (store->run_date != 0)
			store->msec = store->run_msec;
		unsigned short ushort_val = 0;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		store->run_ping_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		store->run_serial = (int)(ushort_val);
		mb_get_binary_int(swap, &line[12], &store->run_status);
		store->run_mode = (mb_u_char)line[16];
		store->run_filter_id = (mb_u_char)line[17];
		mb_get_binary_short(swap, &line[18], &ushort_val);
		store->run_min_depth = (int)(ushort_val);
		mb_get_binary_short(swap, &line[20], &ushort_val);
		store->run_max_depth = (int)(ushort_val);
		mb_get_binary_short(swap, &line[22], &ushort_val);
		store->run_absorption = (int)(ushort_val);
		mb_get_binary_short(swap, &line[24], &ushort_val);
		store->run_tran_pulse = (int)(ushort_val);
		mb_get_binary_short(swap, &line[26], &ushort_val);
		store->run_tran_beam = (int)(ushort_val);
		store->run_tran_pow = (mb_u_char)line[28];
		store->run_rec_beam = (mb_u_char)line[29];
		store->run_rec_band = (mb_u_char)line[30];
		store->run_rec_gain = (mb_u_char)line[31];
		store->run_tvg_cross = (mb_u_char)line[32];
		store->run_ssv_source = (mb_u_char)line[33];
		mb_get_binary_short(swap, &line[34], &ushort_val);
		store->run_max_swath = (int)(ushort_val);
		store->run_beam_space = (mb_u_char)line[36];
		store->run_swath_angle = (mb_u_char)line[37];
		store->run_stab_mode = (mb_u_char)line[38];
		for (int i = 0; i < 6; i++)
			store->run_spare[i] = line[39 + i];
		if (line[EM2_RUN_PARAMETER_SIZE - 7] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM2_RUN_PARAMETER_SIZE - 7],
		        line[EM2_RUN_PARAMETER_SIZE - 7], line[EM2_RUN_PARAMETER_SIZE - 6], line[EM2_RUN_PARAMETER_SIZE - 6],
		        line[EM2_RUN_PARAMETER_SIZE - 5], line[EM2_RUN_PARAMETER_SIZE - 5]);
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
int mbr_em300raw_rd_clock(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                          int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_CLOCK;
	store->type = EM2_CLOCK;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary values into char array */
	char line[EM2_CLOCK_SIZE];
	int read_len = fread(line, 1, EM2_CLOCK_SIZE - 4, mbfp);
	if (read_len == EM2_CLOCK_SIZE - 4)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->clk_date);
		store->date = store->clk_date;
		mb_get_binary_int(swap, &line[4], &store->clk_msec);
		store->msec = store->clk_msec;
		unsigned short ushort_val = 0;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		store->clk_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		store->clk_serial = (int)(ushort_val);
		mb_get_binary_int(swap, &line[12], &store->clk_origin_date);
		mb_get_binary_int(swap, &line[16], &store->clk_origin_msec);
		store->clk_1_pps_use = (mb_u_char)line[20];
		if (line[EM2_CLOCK_SIZE - 7] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM2_CLOCK_SIZE - 7], line[EM2_CLOCK_SIZE - 7],
		        line[EM2_CLOCK_SIZE - 6], line[EM2_CLOCK_SIZE - 6], line[EM2_CLOCK_SIZE - 5], line[EM2_CLOCK_SIZE - 5]);
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
int mbr_em300raw_rd_tide(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                         int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_TIDE;
	store->type = EM2_TIDE;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary values into char array */
	char line[EM2_TIDE_SIZE];
	size_t read_len = fread(line, 1, EM2_TIDE_SIZE - 4, mbfp);
	if (read_len == EM2_TIDE_SIZE - 4)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->tid_date);
		store->date = store->tid_date;
		mb_get_binary_int(swap, &line[4], &store->tid_msec);
		store->msec = store->tid_msec;
		short short_val = 0;
		unsigned short ushort_val = 0;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		store->tid_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		store->tid_serial = (int)(ushort_val);
		mb_get_binary_int(swap, &line[12], &store->tid_origin_date);
		mb_get_binary_int(swap, &line[16], &store->tid_origin_msec);
		mb_get_binary_short(swap, &line[20], &short_val);
		store->tid_tide = (int)short_val;
		if (line[EM2_TIDE_SIZE - 7] == 0x03)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM2_TIDE_SIZE - 7], line[EM2_TIDE_SIZE - 7],
		        line[EM2_TIDE_SIZE - 6], line[EM2_TIDE_SIZE - 6], line[EM2_TIDE_SIZE - 5], line[EM2_TIDE_SIZE - 5]);
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
int mbr_em300raw_rd_height(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                           int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_HEIGHT;
	store->type = EM2_HEIGHT;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary values into char array */
	char line[EM2_HEIGHT_SIZE];
	int read_len = fread(line, 1, EM2_HEIGHT_SIZE - 4, mbfp);
	if (read_len == EM2_HEIGHT_SIZE - 4)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->hgt_date);
		store->date = store->hgt_date;
		mb_get_binary_int(swap, &line[4], &store->hgt_msec);
		store->msec = store->hgt_msec;
		unsigned short ushort_val = 0;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		store->hgt_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		store->hgt_serial = (int)(ushort_val);
		mb_get_binary_int(swap, &line[12], &store->hgt_height);
		store->hgt_type = (mb_u_char)line[16];
		if (line[EM2_HEIGHT_SIZE - 7] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[EM2_HEIGHT_SIZE - 7], line[EM2_HEIGHT_SIZE - 7],
		        line[EM2_HEIGHT_SIZE - 6], line[EM2_HEIGHT_SIZE - 6], line[EM2_HEIGHT_SIZE - 5], line[EM2_HEIGHT_SIZE - 5]);
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
int mbr_em300raw_rd_heading(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                            int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_heading_struct *heading = (struct mbsys_simrad2_heading_struct *)store->heading;

	/* set kind and type values */
	store->kind = MB_DATA_HEADING;
	store->type = EM2_HEADING;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	char line[EM2_HEADING_HEADER_SIZE];
	int read_len = fread(line, 1, EM2_HEADING_HEADER_SIZE, mbfp);
	if (read_len == EM2_HEADING_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &heading->hed_date);
		store->date = heading->hed_date;
		mb_get_binary_int(swap, &line[4], &heading->hed_msec);
		store->msec = heading->hed_msec;
		unsigned short ushort_val = 0;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		heading->hed_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		heading->hed_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		heading->hed_ndata = (int)(ushort_val);
	}

	/* read binary heading values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < heading->hed_ndata && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_HEADING_SLICE_SIZE, mbfp);
			if (read_len == EM2_HEADING_SLICE_SIZE && i < MBSYS_SIMRAD2_MAXHEADING) {
				status = MB_SUCCESS;
				unsigned short ushort_val = 0;
				mb_get_binary_short(swap, &line[0], &ushort_val);
				heading->hed_time[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[2], &ushort_val);
				heading->hed_heading[i] = (int)(ushort_val);
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
		heading->hed_ndata = MIN(heading->hed_ndata, MBSYS_SIMRAD2_MAXHEADING);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
			heading->hed_heading_status = (mb_u_char)line[0];
		}
		else {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
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
int mbr_em300raw_rd_ssv(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                        int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_ssv_struct *ssv = (struct mbsys_simrad2_ssv_struct *)store->ssv;

	/* set kind and type values */
	store->kind = MB_DATA_SSV;
	store->type = EM2_SSV;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	char line[EM2_SSV_HEADER_SIZE];
	int read_len = fread(line, 1, EM2_SSV_HEADER_SIZE, mbfp);
	if (read_len == EM2_SSV_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ssv->ssv_date);
		store->date = ssv->ssv_date;
		mb_get_binary_int(swap, &line[4], &ssv->ssv_msec);
		store->msec = ssv->ssv_msec;
		unsigned short ushort_val = 0;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		ssv->ssv_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		ssv->ssv_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		ssv->ssv_ndata = (int)(ushort_val);
	}

	/* read binary ssv values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ssv->ssv_ndata && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_SSV_SLICE_SIZE, mbfp);
			if (read_len == EM2_SSV_SLICE_SIZE && i < MBSYS_SIMRAD2_MAXSSV) {
				status = MB_SUCCESS;
				unsigned short ushort_val = 0;
				mb_get_binary_short(swap, &line[0], &ushort_val);
				ssv->ssv_time[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[2], &ushort_val);
				ssv->ssv_ssv[i] = (int)(ushort_val);
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
		ssv->ssv_ndata = MIN(ssv->ssv_ndata, MBSYS_SIMRAD2_MAXSSV);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
		}
		else {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
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
int mbr_em300raw_rd_tilt(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                         int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_tilt_struct *tilt = (struct mbsys_simrad2_tilt_struct *)store->tilt;

	/* set kind and type values */
	store->kind = MB_DATA_TILT;
	store->type = EM2_TILT;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	char line[EM2_TILT_HEADER_SIZE];
	int read_len = fread(line, 1, EM2_TILT_HEADER_SIZE, mbfp);
	if (read_len == EM2_TILT_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &tilt->tlt_date);
		store->date = tilt->tlt_date;
		mb_get_binary_int(swap, &line[4], &tilt->tlt_msec);
		store->msec = tilt->tlt_msec;
		unsigned short ushort_val = 0;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		tilt->tlt_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		tilt->tlt_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		tilt->tlt_ndata = (int)(ushort_val);
	}

	/* read binary tilt values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < tilt->tlt_ndata && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_TILT_SLICE_SIZE, mbfp);
			if (read_len == EM2_TILT_SLICE_SIZE && i < MBSYS_SIMRAD2_MAXTILT) {
				status = MB_SUCCESS;
				unsigned short ushort_val = 0;
				mb_get_binary_short(swap, &line[0], &ushort_val);
				tilt->tlt_time[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[2], &ushort_val);
				tilt->tlt_tilt[i] = (int)(ushort_val);
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
		tilt->tlt_ndata = MIN(tilt->tlt_ndata, MBSYS_SIMRAD2_MAXTILT);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
		}
		else {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
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
int mbr_em300raw_rd_extraparameters(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar,
                                    int *goodend, int *error) {
	char line[EM2_EXTRAPARAMETERS_HEADER_SIZE];
	unsigned short ushort_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_extraparameters_struct *extraparameters =
	    (struct mbsys_simrad2_extraparameters_struct *)store->extraparameters;

	/* set kind and type values */
	store->kind = MB_DATA_PARAMETER;
	store->type = EM2_EXTRAPARAMETERS;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_EXTRAPARAMETERS_HEADER_SIZE, mbfp);
	if (read_len == EM2_EXTRAPARAMETERS_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &extraparameters->xtr_date);
		store->date = extraparameters->xtr_date;
		mb_get_binary_int(swap, &line[4], &extraparameters->xtr_msec);
		store->msec = extraparameters->xtr_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		extraparameters->xtr_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		extraparameters->xtr_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		extraparameters->xtr_id = (int)(ushort_val);
	}

	/* read data */
	if (status == MB_SUCCESS) {
		read_len = fread((char *)extraparameters->xtr_data, 1, extraparameters->xtr_data_size, mbfp);
		if (read_len == (size_t) extraparameters->xtr_data_size)
			status = MB_SUCCESS;
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
	}

	/* parse data if possible */
/*	if (status == MB_SUCCESS && extraparameters->xtr_id == 2) {
		index = 0;
		mb_get_binary_int(swap, &(extraparameters->xtr_data[index]), &extraparameters->xtr_pqf_activepositioning);
    index +=4;
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
	} */

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
		}
		else {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
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
/*		if (extraparameters->xtr_id == 2) {
			fprintf(stderr, "dbg5       xtr_pqf_activepositioning:          %d\n", extraparameters->xtr_pqf_activepositioning);
			for (int i = 0; i < 3; i++) {
				fprintf(stderr, "dbg5       positioning system:%d qfsetting:%d nqf:%d\n", i,
				        extraparameters->xtr_pqf_qfsetting[i], extraparameters->xtr_pqf_nqualityfactors[i]);
				for (int j = 0; j < extraparameters->xtr_pqf_nqualityfactors[i]; j++)
					fprintf(stderr, "dbg5       quality factor:%d value:%d limit:%d\n", j,
					        extraparameters->xtr_pqf_qfvalues[i][j], extraparameters->xtr_pqf_qflimits[i][j]);
			}
		}*/
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
int mbr_em300raw_rd_attitude(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                             int *error) {
	char line[EM2_ATTITUDE_HEADER_SIZE];
	short short_val = 0;
	unsigned short ushort_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_attitude_struct *attitude = (struct mbsys_simrad2_attitude_struct *)store->attitude;

	/* set kind and type values */
	store->kind = MB_DATA_ATTITUDE;
	store->type = EM2_ATTITUDE;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_ATTITUDE_HEADER_SIZE, mbfp);
	if (read_len == EM2_ATTITUDE_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &attitude->att_date);
		store->date = attitude->att_date;
		mb_get_binary_int(swap, &line[4], &attitude->att_msec);
		store->msec = attitude->att_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		attitude->att_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		attitude->att_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		attitude->att_ndata = (int)(ushort_val);
	}

	/* read binary attitude values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < attitude->att_ndata && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_ATTITUDE_SLICE_SIZE, mbfp);
			if (read_len == EM2_ATTITUDE_SLICE_SIZE && i < MBSYS_SIMRAD2_MAXATTITUDE) {
				status = MB_SUCCESS;
				mb_get_binary_short(swap, &line[0], &ushort_val);
				attitude->att_time[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[2], &ushort_val);
				attitude->att_sensor_status[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[4], &short_val);
				attitude->att_roll[i] = (int)short_val;
				mb_get_binary_short(swap, &line[6], &short_val);
				attitude->att_pitch[i] = (int)short_val;
				mb_get_binary_short(swap, &line[8], &short_val);
				attitude->att_heave[i] = (int)short_val;
				mb_get_binary_short(swap, &line[10], &ushort_val);
				attitude->att_heading[i] = (int)(ushort_val);
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
		attitude->att_ndata = MIN(attitude->att_ndata, MBSYS_SIMRAD2_MAXATTITUDE);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
			attitude->att_heading_status = (mb_u_char)line[0];
		}
		else {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
#endif
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
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
		fprintf(stderr, "dbg5       att_heading_status: %d\n", attitude->att_heading_status);
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
int mbr_em300raw_rd_pos(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                        int *error) {
	char line[MBSYS_SIMRAD2_COMMENT_LENGTH];
	unsigned short ushort_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_NAV;
	store->type = EM2_POS;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_POS_HEADER_SIZE, mbfp);
	if (read_len == EM2_POS_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->pos_date);
		store->date = store->pos_date;
		mb_get_binary_int(swap, &line[4], &store->pos_msec);
		store->msec = store->pos_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		store->pos_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		store->pos_serial = (int)(ushort_val);
		mb_get_binary_int(swap, &line[12], &store->pos_latitude);
		mb_get_binary_int(swap, &line[16], &store->pos_longitude);
		mb_get_binary_short(swap, &line[20], &ushort_val);
		store->pos_quality = (int)(ushort_val);
		mb_get_binary_short(swap, &line[22], &ushort_val);
		store->pos_speed = (int)(ushort_val);
		mb_get_binary_short(swap, &line[24], &ushort_val);
		store->pos_course = (int)(ushort_val);
		mb_get_binary_short(swap, &line[26], &ushort_val);
		store->pos_heading = (int)(ushort_val);
		store->pos_system = (mb_u_char)line[28];
		store->pos_input_size = (mb_u_char)line[29];
	}

	/* read input position string */
	if (status == MB_SUCCESS && store->pos_input_size < 256) {
		read_len = fread(store->pos_input, 1, store->pos_input_size, mbfp);
		if (read_len == (size_t) store->pos_input_size) {
			status = MB_SUCCESS;
			store->pos_input[store->pos_input_size] = '\0';
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
	}

	/* now loop over reading individual characters to
	    get last bytes of record */
	if (status == MB_SUCCESS) {
		bool done = false;
		while (!done) {
			read_len = fread(&line[0], 1, 1, mbfp);
			if (read_len == 1 && line[0] == EM2_END) {
				done = true;
				status = MB_SUCCESS;
				/* get last two check sum bytes */
				if (sonar != MBSYS_SIMRAD2_EM3000)
					read_len = fread(&line[1], 2, 1, mbfp);
				if (line[0] == EM2_END)
					*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
				if (sonar != MBSYS_SIMRAD2_EM3000)
					fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[0], line[0], line[1], line[1], line[2],
					        line[2]);
#endif
			}
			else if (read_len == 1) {
				status = MB_SUCCESS;
			}
			else {
				done = true;
				/* return success here because all of the
				    important information in this record has
				    already been read - next attempt to read
				    file will return error */
				status = MB_SUCCESS;
			}
		}
#ifdef MBR_EM300RAW_DEBUG
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
			const int navchannel = (store->pos_system & 0x03);
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
int mbr_em300raw_rd_svp(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                        int *error) {
	char line[EM2_SVP_HEADER_SIZE];
	unsigned short ushort_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_VELOCITY_PROFILE;
	store->type = EM2_SVP;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_SVP_HEADER_SIZE, mbfp);
	if (read_len == EM2_SVP_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->svp_use_date);
		store->date = store->svp_use_date;
		mb_get_binary_int(swap, &line[4], &store->svp_use_msec);
		store->msec = store->svp_use_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		store->svp_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		store->svp_serial = (int)(ushort_val);
		mb_get_binary_int(swap, &line[12], &store->svp_origin_date);
		mb_get_binary_int(swap, &line[16], &store->svp_origin_msec);
		mb_get_binary_short(swap, &line[20], &ushort_val);
		store->svp_num = (int)(ushort_val);
		mb_get_binary_short(swap, &line[22], &ushort_val);
		store->svp_depth_res = (int)(ushort_val);
	}

	/* read binary svp values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < store->svp_num && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_SVP_SLICE_SIZE, mbfp);
			if (read_len != EM2_SVP_SLICE_SIZE) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
			else if (i < MBSYS_SIMRAD2_MAXSVP) {
				status = MB_SUCCESS;
				mb_get_binary_short(swap, &line[0], &ushort_val);
				store->svp_depth[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[2], &ushort_val);
				store->svp_vel[i] = (int)(ushort_val);
			}
		}
		store->svp_num = MIN(store->svp_num, MBSYS_SIMRAD2_MAXSVP);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
		}
		else {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
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
int mbr_em300raw_rd_svp2(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                         int *error) {
	char line[EM2_SVP2_HEADER_SIZE];
	unsigned short ushort_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* set kind and type values */
	store->kind = MB_DATA_VELOCITY_PROFILE;
	store->type = EM2_SVP2;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_SVP_HEADER_SIZE, mbfp);
	if (read_len == EM2_SVP_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &store->svp_use_date);
		store->date = store->svp_use_date;
		mb_get_binary_int(swap, &line[4], &store->svp_use_msec);
		store->msec = store->svp_use_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		store->svp_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		store->svp_serial = (int)(ushort_val);
		mb_get_binary_int(swap, &line[12], &store->svp_origin_date);
		mb_get_binary_int(swap, &line[16], &store->svp_origin_msec);
		mb_get_binary_short(swap, &line[20], &ushort_val);
		store->svp_num = (int)(ushort_val);
		mb_get_binary_short(swap, &line[22], &ushort_val);
		store->svp_depth_res = (int)(ushort_val);
	}

	/* read binary svp values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < store->svp_num && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_SVP2_SLICE_SIZE, mbfp);
			if (read_len != EM2_SVP2_SLICE_SIZE) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
			else if (i < MBSYS_SIMRAD2_MAXSVP) {
				status = MB_SUCCESS;
				mb_get_binary_int(swap, &line[0], &store->svp_depth[i]);
				mb_get_binary_int(swap, &line[4], &store->svp_vel[i]);
			}
		}
		store->svp_num = MIN(store->svp_num, MBSYS_SIMRAD2_MAXSVP);
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
		}
		else {
			/* return success here because all of the
			    important information in this record has
			    already been read - next attempt to read
			    file will return error */
			status = MB_SUCCESS;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
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
int mbr_em300raw_rd_bath(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, bool *match, short sonar,
                         int version, int *goodend, int *error) {
	char line[EM2_BATH_HEADER_SIZE];
	short short_val = 0;
	unsigned short ushort_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
		fprintf(stderr, "dbg2       version:    %d\n", version);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	/* set kind and type values */
	store->kind = MB_DATA_DATA;
	store->type = EM2_BATH;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_BATH_HEADER_SIZE, mbfp);
	if (read_len == EM2_BATH_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* in case of dual head EM2002 check if the data are from the second head and switch ping structure if so */
	if (status == MB_SUCCESS && sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2) {
		mb_get_binary_short(swap, &line[8], &ushort_val);
		const int png_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		const int png_serial = (int)(ushort_val);

		if (png_count == ping->png_count && png_serial != ping->png_serial) {
			ping = (struct mbsys_simrad2_ping_struct *)store->ping2;
		}
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ping->png_date);
		store->date = ping->png_date;
		mb_get_binary_int(swap, &line[4], &ping->png_msec);
		store->msec = ping->png_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		ping->png_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		ping->png_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		ping->png_heading = (int)(ushort_val);
		mb_get_binary_short(swap, &line[14], &ushort_val);
		ping->png_ssv = (int)(ushort_val);
		mb_get_binary_short(swap, &line[16], &ushort_val);
		ping->png_xducer_depth = (int)(ushort_val);
		ping->png_nbeams_max = (mb_u_char)line[18];
		ping->png_nbeams = (mb_u_char)line[19];
		ping->png_depth_res = (mb_u_char)line[20];
		ping->png_distance_res = (mb_u_char)line[21];
		mb_get_binary_short(swap, &line[22], &ushort_val);
		ping->png_sample_rate = (int)(ushort_val);
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_nbeams > ping->png_nbeams_max || ping->png_nbeams < 0 || ping->png_nbeams_max < 0 ||
		    ping->png_nbeams > MBSYS_SIMRAD2_MAXBEAMS || ping->png_nbeams_max > MBSYS_SIMRAD2_MAXBEAMS) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_nbeams && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_BATH_BEAM_SIZE, mbfp);
			if (read_len == EM2_BATH_BEAM_SIZE && i < MBSYS_SIMRAD2_MAXBEAMS) {
				status = MB_SUCCESS;
				if (store->sonar == MBSYS_SIMRAD2_EM120 || store->sonar == MBSYS_SIMRAD2_EM300) {
					mb_get_binary_short(swap, &line[0], &ushort_val);
					ping->png_depth[i] = (int)(ushort_val);
				}
				else {
					mb_get_binary_short(swap, &line[0], &short_val);
					ping->png_depth[i] = (int)short_val;
				}

				mb_get_binary_short(swap, &line[2], &short_val);
				ping->png_acrosstrack[i] = (int)short_val;
				mb_get_binary_short(swap, &line[4], &short_val);
				ping->png_alongtrack[i] = (int)short_val;
				mb_get_binary_short(swap, &line[6], &short_val);
				ping->png_depression[i] = (int)short_val;
				mb_get_binary_short(swap, &line[8], &ushort_val);
				ping->png_azimuth[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[10], &ushort_val);
				ping->png_range[i] = (int)(ushort_val);
				ping->png_quality[i] = (mb_u_char)line[12];
				ping->png_window[i] = (mb_u_char)line[13];
				ping->png_amp[i] = (mb_s_char)line[14];
				ping->png_beam_num[i] = (mb_u_char)line[15];
				if (ping->png_depth[i] == 0)
					ping->png_beamflag[i] = MB_FLAG_NULL;
				else
					ping->png_beamflag[i] = MB_FLAG_NONE;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
			ping->png_offset_multiplier = (mb_s_char)line[0];
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
#endif
	}

	/* check sonar version and adjust data as necessary */
	if (status == MB_SUCCESS && sonar >= MBSYS_SIMRAD2_EM3000 && version != 0 && version < 20000) {
		ping->png_offset_multiplier = 0;
	}

	/* check for some other indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_nbeams > 0 && ping->png_beam_num[0] > ping->png_nbeams_max) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
		for (int i = 1; i < ping->png_nbeams; i++) {
			if (ping->png_beam_num[i] < ping->png_beam_num[i - 1] || ping->png_beam_num[i] > ping->png_nbeams_max) {
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}
	}

	/* check if bath and sidescan time tags agree
	   - we cannot pair bath
	   and sidescan records from different pings */
	if (status == MB_SUCCESS) {
		if (ping->png_date == ping->png_ss_date && ping->png_msec == ping->png_ss_msec)
			*match = true;
		else
			*match = false;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       png_date:        %d\n", ping->png_date);
		fprintf(stderr, "dbg5       png_msec:        %d\n", ping->png_msec);
		fprintf(stderr, "dbg5       png_count:       %d\n", ping->png_count);
		fprintf(stderr, "dbg5       png_serial:      %d\n", ping->png_serial);
		fprintf(stderr, "dbg5       png_heading:     %d\n", ping->png_heading);
		fprintf(stderr, "dbg5       png_ssv:         %d\n", ping->png_ssv);
		fprintf(stderr, "dbg5       png_xducer_depth:      %d\n", ping->png_xducer_depth);
		fprintf(stderr, "dbg5       png_offset_multiplier: %d\n", ping->png_offset_multiplier);
		fprintf(stderr, "dbg5       png_nbeams_max:        %d\n", ping->png_nbeams_max);
		fprintf(stderr, "dbg5       png_nbeams:            %d\n", ping->png_nbeams);
		fprintf(stderr, "dbg5       png_depth_res:         %d\n", ping->png_depth_res);
		fprintf(stderr, "dbg5       png_distance_res:      %d\n", ping->png_distance_res);
		fprintf(stderr, "dbg5       png_sample_rate:       %d\n", ping->png_sample_rate);
		fprintf(stderr, "dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %6d %6d %6d %5d %5d %5d %4d %3d %3d %3d\n", i, ping->png_depth[i],
			        ping->png_acrosstrack[i], ping->png_alongtrack[i], ping->png_depression[i], ping->png_azimuth[i],
			        ping->png_range[i], ping->png_quality[i], ping->png_window[i], ping->png_amp[i], ping->png_beam_num[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       match:      %d\n", *match);
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_rawbeam(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                            int *error) {
	char line[EM2_RAWBEAM_HEADER_SIZE];
	short short_val = 0;
	unsigned short ushort_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_RAWBEAM_HEADER_SIZE, mbfp);
	if (read_len == EM2_RAWBEAM_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ping->png_raw_date);
		store->date = ping->png_raw_date;
		mb_get_binary_int(swap, &line[4], &ping->png_raw_msec);
		store->msec = ping->png_raw_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		ping->png_raw_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		ping->png_raw_serial = (int)(ushort_val);
		ping->png_raw_nbeams_max = (mb_u_char)line[12];
		ping->png_raw_nbeams = (mb_u_char)line[13];
		mb_get_binary_short(swap, &line[14], &ushort_val);
		ping->png_raw_ssv = (int)(ushort_val);
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_raw_nbeams > ping->png_nbeams_max || ping->png_raw_nbeams < 0 || ping->png_raw_nbeams_max < 0 ||
		    ping->png_raw_nbeams > MBSYS_SIMRAD2_MAXBEAMS || ping->png_raw_nbeams_max > MBSYS_SIMRAD2_MAXBEAMS) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_raw_nbeams && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_RAWBEAM_BEAM_SIZE, mbfp);
			if (read_len == EM2_RAWBEAM_BEAM_SIZE && i < MBSYS_SIMRAD2_MAXBEAMS) {
				status = MB_SUCCESS;
				mb_get_binary_short(swap, &line[0], &short_val);
				ping->png_raw_rxpointangle[i] = (int)short_val;
				mb_get_binary_short(swap, &line[2], &short_val);
				ping->png_raw_rxtiltangle[i] = (int)short_val;
				mb_get_binary_short(swap, &line[4], &ushort_val);
				ping->png_raw_rxrange[i] = (int)(ushort_val);
				ping->png_raw_rxamp[i] = (mb_s_char)line[6];
				ping->png_raw_rxbeam_num[i] = (mb_u_char)line[7];
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
#endif
	}

	/* check for some other indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_raw_nbeams > 0 && ping->png_raw_rxbeam_num[0] > ping->png_raw_nbeams_max) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
		for (int i = 1; i < ping->png_raw_nbeams; i++) {
			if (ping->png_raw_rxbeam_num[i] < ping->png_raw_rxbeam_num[i - 1] ||
			    ping->png_raw_rxbeam_num[i] > ping->png_raw_nbeams_max) {
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
		fprintf(stderr, "dbg5       png_raw_date:        %d\n", ping->png_raw_date);
		fprintf(stderr, "dbg5       png_raw_msec:        %d\n", ping->png_raw_msec);
		fprintf(stderr, "dbg5       png_raw_count:       %d\n", ping->png_raw_count);
		fprintf(stderr, "dbg5       png_raw_serial:      %d\n", ping->png_raw_serial);
		fprintf(stderr, "dbg5       png_raw_nbeams_max:  %d\n", ping->png_raw_nbeams_max);
		fprintf(stderr, "dbg5       png_raw_nbeams:      %d\n", ping->png_raw_nbeams);
		fprintf(stderr, "dbg5       png_raw_ssv:         %d\n", ping->png_raw_ssv);
		fprintf(stderr, "dbg5       cnt  point   tilt   rng  amp num\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %5d %5d %5d %3d %3d\n", i, ping->png_raw_rxpointangle[i],
			        ping->png_raw_rxtiltangle[i], ping->png_raw_rxrange[i], ping->png_raw_rxamp[i], ping->png_raw_rxbeam_num[i]);
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
int mbr_em300raw_rd_rawbeam2(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                             int *error) {
	char line[EM2_RAWBEAM2_HEADER_SIZE];
	short short_val = 0;
	unsigned short ushort_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_RAWBEAM2_HEADER_SIZE, mbfp);
	if (read_len == EM2_RAWBEAM2_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ping->png_raw_date);
		store->date = ping->png_raw_date;
		mb_get_binary_int(swap, &line[4], &ping->png_raw_msec);
		store->msec = ping->png_raw_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		ping->png_raw_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		ping->png_raw_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		ping->png_raw_heading = (int)(ushort_val);
		mb_get_binary_short(swap, &line[14], &ushort_val);
		ping->png_raw_ssv = (int)(ushort_val);
		mb_get_binary_short(swap, &line[16], &ushort_val);
		ping->png_raw_xducer_depth = (int)(ushort_val);
		ping->png_raw_nbeams_max = (mb_u_char)line[18];
		ping->png_raw_nbeams = (mb_u_char)line[19];
		ping->png_raw_depth_res = (mb_u_char)line[20];
		ping->png_raw_distance_res = (mb_u_char)line[21];
		mb_get_binary_short(swap, &line[22], &ushort_val);
		ping->png_raw_sample_rate = (int)(ushort_val);
		mb_get_binary_int(swap, &line[24], &ping->png_raw_status);
		mb_get_binary_short(swap, &line[28], &ushort_val);
		ping->png_raw_rangenormal = (int)(ushort_val);
		ping->png_raw_normalbackscatter = (mb_s_char)line[30];
		ping->png_raw_obliquebackscatter = (mb_s_char)line[31];
		ping->png_raw_fixedgain = (mb_u_char)line[32];
		ping->png_raw_txpower = (mb_s_char)line[33];
		ping->png_raw_mode = (mb_u_char)line[34];
		ping->png_raw_coverage = (mb_u_char)line[35];
		mb_get_binary_short(swap, &line[36], &ushort_val);
		ping->png_raw_yawstabheading = (int)(ushort_val);
		mb_get_binary_short(swap, &line[38], &ushort_val);
		ping->png_raw_ntx = (int)(ushort_val);
		mb_get_binary_short(swap, &line[40], &ushort_val);
		/* spare = (int)(ushort_val); */
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_raw_nbeams > ping->png_raw_nbeams_max || ping->png_raw_nbeams < 0 || ping->png_raw_nbeams_max < 0 ||
		    ping->png_raw_nbeams > MBSYS_SIMRAD2_MAXBEAMS || ping->png_raw_nbeams_max > MBSYS_SIMRAD2_MAXBEAMS ||
		    ping->png_raw_ntx > MBSYS_SIMRAD2_MAXTX) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary tx values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_raw_ntx && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_RAWBEAM2_TX_SIZE, mbfp);
			if (read_len == EM2_RAWBEAM2_TX_SIZE && i < MBSYS_SIMRAD2_MAXTX) {
				status = MB_SUCCESS;
				mb_get_binary_short(swap, &line[0], &ushort_val);
				ping->png_raw_txlastbeam[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				ping->png_raw_txtiltangle[i] = (int)short_val;
				mb_get_binary_short(swap, &line[4], &ushort_val);
				ping->png_raw_txheading[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[6], &short_val);
				ping->png_raw_txroll[i] = (int)short_val;
				mb_get_binary_short(swap, &line[8], &short_val);
				ping->png_raw_txpitch[i] = (int)short_val;
				mb_get_binary_short(swap, &line[10], &short_val);
				ping->png_raw_txheave[i] = (int)short_val;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_raw_nbeams && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_RAWBEAM2_BEAM_SIZE, mbfp);
			if (read_len == EM2_RAWBEAM2_BEAM_SIZE && i < MBSYS_SIMRAD2_MAXBEAMS) {
				status = MB_SUCCESS;
				mb_get_binary_short(swap, &line[0], &ushort_val);
				ping->png_raw_rxrange[i] = (int)(ushort_val);
				ping->png_raw_rxquality[i] = (mb_u_char)line[2];
				ping->png_raw_rxwindow[i] = (mb_u_char)line[3];
				ping->png_raw_rxamp[i] = (mb_s_char)line[4];
				ping->png_raw_rxbeam_num[i] = (mb_u_char)line[5];
				mb_get_binary_short(swap, &line[6], &short_val);
				ping->png_raw_rxpointangle[i] = (int)short_val;
				mb_get_binary_short(swap, &line[8], &ushort_val);
				ping->png_raw_rxheading[i] = (int)(ushort_val);
				mb_get_binary_short(swap, &line[10], &short_val);
				ping->png_raw_rxroll[i] = (int)short_val;
				mb_get_binary_short(swap, &line[12], &short_val);
				ping->png_raw_rxpitch[i] = (int)short_val;
				mb_get_binary_short(swap, &line[14], &short_val);
				ping->png_raw_rxheave[i] = (int)short_val;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
#endif
	}

	/* check for some other indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_raw_nbeams > 0 && ping->png_raw_rxbeam_num[0] > ping->png_raw_nbeams_max) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
		for (int i = 1; i < ping->png_raw_nbeams; i++) {
			if (ping->png_raw_rxbeam_num[i] < ping->png_raw_rxbeam_num[i - 1] ||
			    ping->png_raw_rxbeam_num[i] > ping->png_raw_nbeams_max) {
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
		fprintf(stderr, "dbg5       png_raw_date:                %d\n", ping->png_raw_date);
		fprintf(stderr, "dbg5       png_raw_msec:                %d\n", ping->png_raw_msec);
		fprintf(stderr, "dbg5       png_raw_count:               %d\n", ping->png_raw_count);
		fprintf(stderr, "dbg5       png_raw_serial:              %d\n", ping->png_raw_serial);
		fprintf(stderr, "dbg5       png_raw_heading:             %d\n", ping->png_raw_heading);
		fprintf(stderr, "dbg5       png_raw_ssv:                 %d\n", ping->png_raw_ssv);
		fprintf(stderr, "dbg5       png_raw_xducer_depth:        %d\n", ping->png_raw_xducer_depth);
		fprintf(stderr, "dbg5       png_raw_nbeams_max:          %d\n", ping->png_raw_nbeams_max);
		fprintf(stderr, "dbg5       png_raw_nbeams:              %d\n", ping->png_raw_nbeams);
		fprintf(stderr, "dbg5       png_raw_depth_res:           %d\n", ping->png_raw_depth_res);
		fprintf(stderr, "dbg5       png_raw_distance_res:        %d\n", ping->png_raw_distance_res);
		fprintf(stderr, "dbg5       png_raw_sample_rate:         %d\n", ping->png_raw_sample_rate);
		fprintf(stderr, "dbg5       png_raw_status:              %d\n", ping->png_raw_status);
		fprintf(stderr, "dbg5       png_raw_rangenormal:         %d\n", ping->png_raw_rangenormal);
		fprintf(stderr, "dbg5       png_raw_normalbackscatter:   %d\n", ping->png_raw_normalbackscatter);
		fprintf(stderr, "dbg5       png_raw_obliquebackscatter:  %d\n", ping->png_raw_obliquebackscatter);
		fprintf(stderr, "dbg5       png_raw_fixedgain:           %d\n", ping->png_raw_fixedgain);
		fprintf(stderr, "dbg5       png_raw_txpower:             %d\n", ping->png_raw_txpower);
		fprintf(stderr, "dbg5       png_raw_mode:                %d\n", ping->png_raw_mode);
		fprintf(stderr, "dbg5       png_raw_coverage:            %d\n", ping->png_raw_coverage);
		fprintf(stderr, "dbg5       png_raw_yawstabheading:      %d\n", ping->png_raw_yawstabheading);
		fprintf(stderr, "dbg5       png_raw_ntx:                 %d\n", ping->png_raw_ntx);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       transmit pulse values:\n");
		fprintf(stderr, "dbg5       cnt lastbeam tiltangle heading roll pitch heave\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_ntx; i++)
			fprintf(stderr, "dbg5       %3d %3d %4d %5d %4d %4d %4d\n", i, ping->png_raw_txlastbeam[i],
			        ping->png_raw_txtiltangle[i], ping->png_raw_txheading[i], ping->png_raw_txroll[i], ping->png_raw_txpitch[i],
			        ping->png_raw_txheave[i]);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       beam values:\n");
		fprintf(stderr, "dbg5       cnt range quality window amp beam angle heading roll pitch heave\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %5d %3d %3d %4d %3d %5d %5d %4d %4d %4d\n", i, ping->png_raw_rxrange[i],
			        ping->png_raw_rxquality[i], ping->png_raw_rxwindow[i], ping->png_raw_rxamp[i], ping->png_raw_rxbeam_num[i],
			        ping->png_raw_rxpointangle[i], ping->png_raw_rxheading[i], ping->png_raw_rxroll[i], ping->png_raw_rxpitch[i],
			        ping->png_raw_rxheave[i]);
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
int mbr_em300raw_rd_rawbeam3(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                             int *error) {
	char line[EM2_RAWBEAM3_HEADER_SIZE];
	short short_val = 0;
	unsigned short ushort_val = 0;
	int int_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_RAWBEAM3_HEADER_SIZE, mbfp);
	if (read_len == EM2_RAWBEAM3_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* in case of dual head EM2002 check if the data are from the second head and if so switch ping structure */
	if (status == MB_SUCCESS && sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2) {
		mb_get_binary_short(swap, &line[8], &ushort_val);
		const int png_raw3_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		const int png_raw3_serial = (int)(ushort_val);

		if (png_raw3_count == ping->png_raw3_count && png_raw3_serial != ping->png_raw3_serial) {
			ping = (struct mbsys_simrad2_ping_struct *)store->ping2;
		}
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ping->png_raw3_date);
		store->date = ping->png_raw3_date;
		mb_get_binary_int(swap, &line[4], &ping->png_raw3_msec);
		store->msec = ping->png_raw3_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		ping->png_raw3_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		ping->png_raw3_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		ping->png_raw3_ntx = (int)(ushort_val);
		mb_get_binary_short(swap, &line[14], &ushort_val);
		ping->png_raw3_nbeams = (int)(ushort_val);
		mb_get_binary_int(swap, &line[16], &int_val);
		ping->png_raw3_sample_rate = (int)(int_val);
		mb_get_binary_int(swap, &line[20], &int_val);
		ping->png_raw3_xducer_depth = (int)(int_val);
		mb_get_binary_short(swap, &line[24], &ushort_val);
		ping->png_raw3_ssv = (int)(ushort_val);
		mb_get_binary_short(swap, &line[26], &ushort_val);
		ping->png_raw3_nbeams_max = (int)(ushort_val);
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_raw3_nbeams > ping->png_raw3_nbeams_max || ping->png_raw3_nbeams < 0 || ping->png_raw3_nbeams_max < 0 ||
		    ping->png_raw3_nbeams > MBSYS_SIMRAD2_MAXBEAMS || ping->png_raw3_nbeams_max > MBSYS_SIMRAD2_MAXBEAMS ||
		    ping->png_raw3_ntx > MBSYS_SIMRAD2_MAXTX) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary tx values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_raw3_ntx && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_RAWBEAM3_TX_SIZE, mbfp);
			if (read_len == EM2_RAWBEAM3_TX_SIZE && i < MBSYS_SIMRAD2_MAXTX) {
				status = MB_SUCCESS;
				mb_get_binary_short(swap, &line[0], &short_val);
				ping->png_raw3_txtiltangle[i] = (int)short_val;
				mb_get_binary_short(swap, &line[2], &short_val);
				ping->png_raw3_txfocus[i] = (int)short_val;
				mb_get_binary_int(swap, &line[4], &int_val);
				ping->png_raw3_txsignallength[i] = int_val;
				mb_get_binary_int(swap, &line[8], &int_val);
				ping->png_raw3_txoffset[i] = int_val;
				mb_get_binary_int(swap, &line[12], &int_val);
				ping->png_raw3_txcenter[i] = int_val;
				mb_get_binary_short(swap, &line[16], &short_val);
				ping->png_raw3_txbandwidth[i] = (int)short_val;
				ping->png_raw3_txwaveform[i] = (int)line[18];
				ping->png_raw3_txsector[i] = (int)line[19];
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < ping->png_raw3_nbeams && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_RAWBEAM3_BEAM_SIZE, mbfp);
			if (read_len == EM2_RAWBEAM3_BEAM_SIZE && i < MBSYS_SIMRAD2_MAXBEAMS) {
				status = MB_SUCCESS;
				mb_get_binary_short(swap, &line[0], &short_val);
				ping->png_raw3_rxpointangle[i] = (int)short_val;
				mb_get_binary_short(swap, &line[2], &ushort_val);
				ping->png_raw3_rxrange[i] = (int)(ushort_val);
				ping->png_raw3_rxsector[i] = (mb_u_char)line[4];
				ping->png_raw3_rxamp[i] = (mb_s_char)line[5];
				ping->png_raw3_rxquality[i] = (mb_u_char)line[6];
				ping->png_raw3_rxwindow[i] = (mb_u_char)line[7];
				mb_get_binary_short(swap, &line[8], &short_val);
				ping->png_raw3_rxbeam_num[i] = (int)(short_val);
				mb_get_binary_short(swap, &line[10], &ushort_val);
				ping->png_raw3_rxspare[i] = (int)(ushort_val);
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
	}

	/* now get last bytes of record */
	if (status == MB_SUCCESS) {
		read_len = fread(&line[0], 1, 4, mbfp);
		if (read_len == 4) {
			status = MB_SUCCESS;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		if (line[1] == EM2_END)
			*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[1], line[1], line[2], line[2], line[3], line[3]);
#endif
	}

	/* check for some other indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_raw3_nbeams > 0 && ping->png_raw3_rxbeam_num[0] > ping->png_raw3_nbeams_max) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
		for (int i = 1; i < ping->png_raw3_nbeams; i++) {
			if (ping->png_raw3_rxbeam_num[i] < ping->png_raw3_rxbeam_num[i - 1] ||
			    ping->png_raw3_rxbeam_num[i] > ping->png_raw3_nbeams_max) {
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
		fprintf(stderr, "dbg5       png_raw3_date:                %d\n", ping->png_raw3_date);
		fprintf(stderr, "dbg5       png_raw3_msec:                %d\n", ping->png_raw3_msec);
		fprintf(stderr, "dbg5       png_raw3_count:               %d\n", ping->png_raw3_count);
		fprintf(stderr, "dbg5       png_raw3_serial:              %d\n", ping->png_raw3_serial);
		fprintf(stderr, "dbg5       png_raw3_ntx:                 %d\n", ping->png_raw3_ntx);
		fprintf(stderr, "dbg5       png_raw3_nbeams:              %d\n", ping->png_raw3_nbeams);
		fprintf(stderr, "dbg5       png_raw3_sample_rate:         %d\n", ping->png_raw3_sample_rate);
		fprintf(stderr, "dbg5       png_raw3_xducer_depth:        %d\n", ping->png_raw3_xducer_depth);
		fprintf(stderr, "dbg5       png_raw3_ssv:                 %d\n", ping->png_raw3_ssv);
		fprintf(stderr, "dbg5       png_raw3_nbeams_max:          %d\n", ping->png_raw3_nbeams_max);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       transmit pulse values:\n");
		fprintf(stderr, "dbg5       tiltangle focus length offset center bandwidth waveform sector\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw3_ntx; i++)
			fprintf(stderr, "dbg5       %3d %5d %5d %6d %4d %4d %4d %4d %4d\n", i, ping->png_raw3_txtiltangle[i],
			        ping->png_raw3_txfocus[i], ping->png_raw3_txsignallength[i], ping->png_raw3_txoffset[i],
			        ping->png_raw3_txcenter[i], ping->png_raw3_txbandwidth[i], ping->png_raw3_txwaveform[i],
			        ping->png_raw3_txsector[i]);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       beam values:\n");
		fprintf(stderr, "dbg5       angle range sector amp quality window beam\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw3_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %5d %3d %3d %4d %3d %5d %5d %5d\n", i, ping->png_raw3_rxpointangle[i],
			        ping->png_raw3_rxrange[i], ping->png_raw3_rxsector[i], ping->png_raw3_rxamp[i], ping->png_raw3_rxquality[i],
			        ping->png_raw3_rxwindow[i], ping->png_raw3_rxbeam_num[i], ping->png_raw3_rxspare[i]);
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
int mbr_em300raw_rd_ss(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int length, bool *match,
                       int *goodend, int *error) {
	char line[EM2_SS_HEADER_SIZE];
	unsigned short ushort_val = 0;
	size_t read_len;
	int junk_bytes;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
		fprintf(stderr, "dbg2       length:     %d\n", length);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	/* set kind and type values */
	store->kind = MB_DATA_DATA;
	store->type = EM2_SS;
	store->sonar = sonar;

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_SS_HEADER_SIZE, mbfp);
	if (read_len == EM2_SS_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* in case of dual head EM2002 check if the data are from the second head and if so switch ping structure */
	if (status == MB_SUCCESS && sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2) {
		mb_get_binary_short(swap, &line[8], &ushort_val);
		const int png_ss_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		const int png_ss_serial = (int)(ushort_val);

		if ((png_ss_count == ping->png_ss_count && png_ss_serial != ping->png_ss_serial) ||
		    (png_ss_count == store->ping2->png_count && png_ss_serial == store->ping2->png_serial)) {
			ping = (struct mbsys_simrad2_ping_struct *)store->ping2;
		}
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &ping->png_ss_date);
		store->date = ping->png_ss_date;
		mb_get_binary_int(swap, &line[4], &ping->png_ss_msec);
		store->msec = ping->png_ss_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		ping->png_ss_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		ping->png_ss_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		ping->png_max_range = (int)(ushort_val);
		mb_get_binary_short(swap, &line[14], &ushort_val);
		ping->png_r_zero = (int)(ushort_val);
		mb_get_binary_short(swap, &line[16], &ushort_val);
		ping->png_r_zero_corr = (int)(ushort_val);
		mb_get_binary_short(swap, &line[18], &ushort_val);
		ping->png_tvg_start = (int)(ushort_val);
		mb_get_binary_short(swap, &line[20], &ushort_val);
		ping->png_tvg_stop = (int)(ushort_val);
		ping->png_bsn = (mb_s_char)line[22];
		ping->png_bso = (mb_s_char)line[23];
		mb_get_binary_short(swap, &line[24], &ushort_val);
		ping->png_tx = (int)(ushort_val);
		ping->png_tvg_crossover = (mb_u_char)line[26];
		ping->png_nbeams_ss = (mb_u_char)line[27];
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_nbeams_ss < 0 || ping->png_nbeams_ss > MBSYS_SIMRAD2_MAXBEAMS) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		ping->png_npixels = 0;
		for (int i = 0; i < ping->png_nbeams_ss && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_SS_BEAM_SIZE, mbfp);
			if (read_len == EM2_SS_BEAM_SIZE && i < MBSYS_SIMRAD2_MAXBEAMS) {
				status = MB_SUCCESS;
				ping->png_beam_index[i] = (mb_u_char)line[0];
				ping->png_sort_direction[i] = (mb_s_char)line[1];
				mb_get_binary_short(swap, &line[2], &ushort_val);
				ping->png_beam_samples[i] = (int)(ushort_val);
				ping->png_start_sample[i] = ping->png_npixels;
				mb_get_binary_short(swap, &line[4], &ushort_val);
				ping->png_center_sample[i] = (int)(ushort_val);
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
			ping->png_npixels += ping->png_beam_samples[i];
			if (ping->png_npixels > MBSYS_SIMRAD2_MAXRAWPIXELS) {
				ping->png_beam_samples[i] -= (ping->png_npixels - MBSYS_SIMRAD2_MAXRAWPIXELS);
				if (ping->png_beam_samples[i] < 0)
					ping->png_beam_samples[i] = 0;
			}
		}

		/* check for no pixel data - frequently occurs with EM1002 */
		if (length == EM2_SS_HEADER_SIZE + ping->png_nbeams_ss * EM2_SS_BEAM_SIZE + 8) {
			if (verbose > 0)
				fprintf(stderr, "WARNING: No Simrad multibeam sidescan pixels in data record!\n");
			junk_bytes = 0;
			ping->png_npixels = 0;
		}

		/* check for too much pixel data */
		if (ping->png_npixels > MBSYS_SIMRAD2_MAXRAWPIXELS) {
			if (verbose > 0)
				fprintf(stderr, "WARNING: Simrad multibeam sidescan pixels %d exceed maximum %d!\n", ping->png_npixels,
				        MBSYS_SIMRAD2_MAXRAWPIXELS);
			junk_bytes = ping->png_npixels - MBSYS_SIMRAD2_MAXRAWPIXELS;
			ping->png_npixels = MBSYS_SIMRAD2_MAXRAWPIXELS;
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
		else
			junk_bytes = 0;
	}

	/* check for some other indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (ping->png_nbeams_ss > 0 && ping->png_beam_index[0] > MBSYS_SIMRAD2_MAXBEAMS) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
		for (int i = 1; i < ping->png_nbeams_ss; i++) {
			if (ping->png_beam_index[i] < ping->png_beam_index[i - 1] || ping->png_beam_index[0] > MBSYS_SIMRAD2_MAXBEAMS) {
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}
	}

	/* read binary sidescan values */
	if (status == MB_SUCCESS) {
		read_len = fread(ping->png_ssraw, 1, ping->png_npixels, mbfp);
		if (read_len == (size_t) ping->png_npixels) {
			status = MB_SUCCESS;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
	}

	/* read any leftover binary sidescan values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < junk_bytes; i++)
			read_len = fread(&line[0], 1, 1, mbfp);
	}

	/* now loop over reading individual characters to
	    get last bytes of record */
	if (status == MB_SUCCESS) {
		bool done = false;
		while (!done) {
			read_len = fread(&line[0], 1, 1, mbfp);
			if (read_len == 1 && line[0] == EM2_END) {
				done = true;
				status = MB_SUCCESS;
				/* get last two check sum bytes */
				read_len = fread(&line[1], 2, 1, mbfp);
				*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
				fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[0], line[0], line[1], line[1], line[2],
				        line[2]);
#endif
			}
			else if (read_len == 1) {
				status = MB_SUCCESS;
			}
			else {
				done = true;
				/* return success here because all of the
				    important information in this record has
				    already been read - next attempt to read
				    file will return error */
				status = MB_SUCCESS;
			}
		}
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "\n");
#endif
	}

	/* check if bath and sidescan time tags agree
	   - we cannot pair bath
	   and sidescan records from different pings */
	if (status == MB_SUCCESS) {
		if (ping->png_date == ping->png_ss_date && ping->png_msec == ping->png_ss_msec)
			*match = true;
		else
			*match = false;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       png_date:        %d\n", ping->png_date);
		fprintf(stderr, "dbg5       png_msec:        %d\n", ping->png_msec);
		fprintf(stderr, "dbg5       png_ss_date:     %d\n", ping->png_ss_date);
		fprintf(stderr, "dbg5       png_ss_msec:     %d\n", ping->png_ss_msec);
		fprintf(stderr, "dbg5       png_ss_count:    %d\n", ping->png_ss_count);
		fprintf(stderr, "dbg5       png_ss_serial:   %d\n", ping->png_ss_serial);

		fprintf(stderr, "dbg5       png_heading:     %d\n", ping->png_heading);
		fprintf(stderr, "dbg5       png_ssv:         %d\n", ping->png_ssv);
		fprintf(stderr, "dbg5       png_xducer_depth:      %d\n", ping->png_xducer_depth);
		fprintf(stderr, "dbg5       png_offset_multiplier: %d\n", ping->png_offset_multiplier);
		fprintf(stderr, "dbg5       png_nbeams_max:        %d\n", ping->png_nbeams_max);
		fprintf(stderr, "dbg5       png_nbeams:            %d\n", ping->png_nbeams);
		fprintf(stderr, "dbg5       png_depth_res:         %d\n", ping->png_depth_res);
		fprintf(stderr, "dbg5       png_distance_res:      %d\n", ping->png_distance_res);
		fprintf(stderr, "dbg5       png_sample_rate:       %d\n", ping->png_sample_rate);
		fprintf(stderr, "dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr, "dbg5       ----------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %6d %6d %6d %5d %5d %5d %4d %3d %3d %3d\n", i, ping->png_depth[i],
			        ping->png_acrosstrack[i], ping->png_alongtrack[i], ping->png_depression[i], ping->png_azimuth[i],
			        ping->png_range[i], ping->png_quality[i], ping->png_window[i], ping->png_amp[i], ping->png_beam_num[i]);
		fprintf(stderr, "dbg5       png_max_range:   %d\n", ping->png_max_range);
		fprintf(stderr, "dbg5       png_r_zero:      %d\n", ping->png_r_zero);
		fprintf(stderr, "dbg5       png_r_zero_corr: %d\n", ping->png_r_zero_corr);
		fprintf(stderr, "dbg5       png_tvg_start:   %d\n", ping->png_tvg_start);
		fprintf(stderr, "dbg5       png_tvg_stop:    %d\n", ping->png_tvg_stop);
		fprintf(stderr, "dbg5       png_bsn:         %d\n", ping->png_bsn);
		fprintf(stderr, "dbg5       png_bso:         %d\n", ping->png_bso);
		fprintf(stderr, "dbg5       png_tx:          %d\n", ping->png_tx);
		fprintf(stderr, "dbg5       png_tvg_crossover: %d\n", ping->png_tvg_crossover);
		fprintf(stderr, "dbg5       png_nbeams_ss:     %d\n", ping->png_nbeams_ss);
		fprintf(stderr, "dbg5       png_npixels:       %d\n", ping->png_npixels);
		fprintf(stderr, "dbg5       cnt  index sort samples start center\n");
		fprintf(stderr, "dbg5       --------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams_ss; i++)
			fprintf(stderr, "dbg5        %4d %3d %2d %4d %4d %4d\n", i, ping->png_beam_index[i], ping->png_sort_direction[i],
			        ping->png_beam_samples[i], ping->png_start_sample[i], ping->png_center_sample[i]);
		fprintf(stderr, "dbg5       cnt  ss\n");
		fprintf(stderr, "dbg5       --------------------------------------------------\n");
		for (int i = 0; i < ping->png_npixels; i++)
			fprintf(stderr, "dbg5        %d %d\n", i, ping->png_ssraw[i]);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       match:      %d\n", *match);
		fprintf(stderr, "dbg2       goodend:    %d\n", *goodend);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_em300raw_rd_wc(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, short sonar, int *goodend,
                       int *error) {
	char line[EM2_WC_HEADER_SIZE];
	short short_val = 0;
	unsigned short ushort_val = 0;
	size_t read_len;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       sonar:      %d\n", sonar);
	}

	/* set goodend false until a good end is found */
	*goodend = false;

	/* get  storage structure */
	struct mbsys_simrad2_watercolumn_struct *wc = (struct mbsys_simrad2_watercolumn_struct *)store->wc;

	/* set kind and type values */
	store->kind = MB_DATA_WATER_COLUMN;
	store->type = EM2_WATERCOLUMN;
	store->sonar = sonar;
	/* file_bytes = ftell(mbfp); */

	int status = MB_SUCCESS;

	/* read binary header values into char array */
	read_len = fread(line, 1, EM2_WC_HEADER_SIZE, mbfp);
	if (read_len == EM2_WC_HEADER_SIZE)
		status = MB_SUCCESS;
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* get binary header data */
	if (status == MB_SUCCESS) {
		mb_get_binary_int(swap, &line[0], &wc->wtc_date);
		store->date = wc->wtc_date;
		mb_get_binary_int(swap, &line[4], &wc->wtc_msec);
		store->msec = wc->wtc_msec;
		mb_get_binary_short(swap, &line[8], &ushort_val);
		wc->wtc_count = (int)(ushort_val);
		mb_get_binary_short(swap, &line[10], &ushort_val);
		wc->wtc_serial = (int)(ushort_val);
		mb_get_binary_short(swap, &line[12], &ushort_val);
		wc->wtc_ndatagrams = (int)(ushort_val);
		mb_get_binary_short(swap, &line[14], &ushort_val);
		wc->wtc_datagram = (int)(ushort_val);
		mb_get_binary_short(swap, &line[16], &ushort_val);
		wc->wtc_ntx = (int)(ushort_val);
		mb_get_binary_short(swap, &line[18], &ushort_val);
		wc->wtc_nrx = (int)(ushort_val);
		mb_get_binary_short(swap, &line[20], &ushort_val);
		wc->wtc_nbeam = (int)(ushort_val);
		mb_get_binary_short(swap, &line[22], &ushort_val);
		wc->wtc_ssv = (int)(ushort_val);
		mb_get_binary_int(swap, &line[24], &(wc->wtc_sfreq));
		mb_get_binary_short(swap, &line[28], &short_val);
		wc->wtc_heave = (int)(short_val);
		mb_get_binary_short(swap, &line[30], &ushort_val);
		wc->wtc_spare1 = (int)(ushort_val);
		mb_get_binary_short(swap, &line[32], &ushort_val);
		wc->wtc_spare2 = (int)(ushort_val);
		mb_get_binary_short(swap, &line[34], &ushort_val);
		wc->wtc_spare3 = (int)(ushort_val);
	}

	/* check for some indicators of a broken record
	    - these do happen!!!! */
	if (status == MB_SUCCESS) {
		if (wc->wtc_nbeam < 0 || wc->wtc_nbeam > MBSYS_SIMRAD2_MAXBEAMS || wc->wtc_ntx < 0 || wc->wtc_ntx > MBSYS_SIMRAD2_MAXTX) {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* read binary beam values */
	if (status == MB_SUCCESS) {
		for (int i = 0; i < wc->wtc_ntx && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_WC_TX_SIZE, mbfp);
			if (read_len == EM2_WC_TX_SIZE && i < MBSYS_SIMRAD2_MAXTX) {
				mb_get_binary_short(swap, &line[0], &short_val);
				wc->wtc_txtiltangle[i] = (int)(short_val);
				mb_get_binary_short(swap, &line[2], &short_val);
				wc->wtc_txcenter[i] = (int)(short_val);
				wc->wtc_txsector[i] = (int)((mb_u_char)line[4]);
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
		for (int i = 0; i < wc->wtc_nbeam && status == MB_SUCCESS; i++) {
			read_len = fread(line, 1, EM2_WC_BEAM_SIZE, mbfp);
			if (read_len == EM2_WC_BEAM_SIZE && i < MBSYS_SIMRAD2_MAXBEAMS) {
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
			read_len = fread(wc->beam[i].wtc_amp, 1, wc->beam[i].wtc_beam_samples, mbfp);
		}
	}

	/* now loop over reading individual characters to
	    get last bytes of record */
	if (status == MB_SUCCESS) {
		bool done = false;
		while (!done) {
			read_len = fread(&line[0], 1, 1, mbfp);
			if (read_len == 1 && line[0] == EM2_END) {
				done = true;
				status = MB_SUCCESS;
				/* get last two check sum bytes */
				read_len = fread(&line[1], 2, 1, mbfp);
				*goodend = true;
#ifdef MBR_EM300RAW_DEBUG
				fprintf(stderr, "End Bytes: %2.2hhX %d | %2.2hhX %d | %2.2hhX %d\n", line[0], line[0], line[1], line[1], line[2],
				        line[2]);
#endif
			}
			else if (read_len == 1) {
				status = MB_SUCCESS;
			}
			else {
				done = true;
				/* return success here because all of the
				    important information in this record has
				    already been read - next attempt to read
				    file will return error */
				status = MB_SUCCESS;
			}
		}
#ifdef MBR_EM300RAW_DEBUG
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
int mbr_em300raw_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int *databyteswapped;
	int record_size;
	int *record_size_save;
	char *label;
	int *label_save_flag;
	char *record_size_char;
	short expect;
	short type;
	short sonar;
	int *version;
	short first_type;
	short *expect_save;
	int *expect_save_flag;
	short *first_type_save;
	short *typelast;
	short *sonarlast;
	int *nbadrec;
	int *length;
	int good_end_bytes;  // TODO(schwehr): Can this be a bool?
	bool match;
	int skip = 0;
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
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;
	struct mbsys_simrad2_ping_struct *ping2 = (struct mbsys_simrad2_ping_struct *)store->ping2;
	FILE *mbfp = mb_io_ptr->mbfp;

	/* get saved values */
	databyteswapped = (int *)&mb_io_ptr->save10;
	record_size_save = (int *)&mb_io_ptr->save5;
	label = (char *)mb_io_ptr->save_label;
	version = (int *)(&mb_io_ptr->save3);
	label_save_flag = (int *)&mb_io_ptr->save_label_flag;
	expect_save_flag = (int *)&mb_io_ptr->save_flag;
	expect_save = (short *)&mb_io_ptr->save1;
	first_type_save = (short *)&mb_io_ptr->save2;
	typelast = (short *)&mb_io_ptr->save6;
	sonarlast = (short *)&mb_io_ptr->save9;
	nbadrec = (int *)&mb_io_ptr->save7;
	length = (int *)&mb_io_ptr->save8;
	record_size_char = (char *)&record_size;
	if (*expect_save_flag) {
		expect = *expect_save;
		first_type = *first_type_save;
		*expect_save_flag = false;
	}
	else {
		expect = EM2_NONE;
		first_type = EM2_NONE;
		if (ping != NULL) {
			ping->png_raw1_read = false;
			ping->png_raw2_read = false;
			ping->png_ss_read = false;
			ping->png_raw_nbeams = 0;
			ping->png_nbeams_ss = 0;
		}
		if (ping2 != NULL) {
			ping2->png_raw1_read = false;
			ping2->png_raw2_read = false;
			ping2->png_ss_read = false;
			ping2->png_raw_nbeams = 0;
			ping2->png_nbeams_ss = 0;
		}
	}

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* set flag to swap bytes if necessary */
	int swap = *databyteswapped;

	int status = MB_SUCCESS;

	/* loop over reading data until a record is ready for return */
	bool done = false;
	*error = MB_ERROR_NO_ERROR;
	while (!done) {
		/* if no label saved get next record label */
		if (!*label_save_flag) {
			/* read four byte record size */
			size_t read_len;
			if ((read_len = fread(&record_size, 1, 4, mb_io_ptr->mbfp)) != 4) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}

			/* read label */
			if ((read_len = fread(label, 1, 4, mb_io_ptr->mbfp)) != 4) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}

			/* check label - if not a good label read a byte
			    at a time until a good label is found */
			skip = 0;
			while (status == MB_SUCCESS && mbr_em300raw_chk_label(verbose, mbio_ptr, label, &type, &sonar) != MB_SUCCESS) {
				/* get next byte */
				for (int i = 0; i < 3; i++)
					record_size_char[i] = record_size_char[i + 1];
				record_size_char[3] = label[0];
				for (int i = 0; i < 3; i++)
					label[i] = label[i + 1];
				size_t read_len;
				if ((read_len = fread(&label[3], 1, 1, mb_io_ptr->mbfp)) != 1) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
				skip++;
			}

			/* report problem */
			if (skip > 0 && verbose > 0) {
				if (*nbadrec == 0)
					fprintf(stderr, "\nThe MBF_EM300RAW module skipped data between identified\n\
data records. Something is broken, most probably the data...\n\
However, the data may include a data record type that we\n\
haven't seen yet, or there could be an error in the code.\n\
If skipped data are reported multiple times, \n\
we recommend you send a data sample and problem \n\
description to the MB-System team \n\
(caress@mbari.org and dale@ldeo.columbia.edu)\n\
Have a nice day...\n");
				fprintf(stderr, "MBF_EM300RAW skipped %d bytes between records %4.4hX:%d and %4.4hX:%d\n", skip, *typelast,
				        *typelast, type, type);
				(*nbadrec)++;
			}
			*typelast = type;
			*sonarlast = sonar;

			/* set flag to swap MBR_EM300RAW_DEBUGbytes if necessary */
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
		}

#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "\nstart of mbr_em300raw_rd_data loop:\n");
		fprintf(stderr, "skip:%d expect:%x type:%x first_type:%x sonar:%d recsize:%u done:%d\n", skip, expect, type, first_type,
		        sonar, *record_size_save, done);
#endif

		/* allocate secondary data structure for
		    extraparameters data if needed */
		if (status == MB_SUCCESS && (type == EM2_EXTRAPARAMETERS)) {
			if (store->extraparameters == NULL) {
				status = mbsys_simrad2_extraparameters_alloc(verbose, mbio_ptr, store_ptr, error);
			}
			if (status == MB_SUCCESS && store->extraparameters != NULL) {
				struct mbsys_simrad2_extraparameters_struct *extraparameters =
				    (struct mbsys_simrad2_extraparameters_struct *)store->extraparameters;
				extraparameters->xtr_data_size = *record_size_save - EM2_EXTRAPARAMETERS_HEADER_SIZE - 8;
				if (extraparameters->xtr_data_size > extraparameters->xtr_nalloc) {
					status = mb_reallocd(verbose, __FILE__, __LINE__, extraparameters->xtr_data_size,
					                     (void **)&extraparameters->xtr_data, error);
					if (status == MB_SUCCESS)
						extraparameters->xtr_nalloc = extraparameters->xtr_data_size;
					else
						extraparameters->xtr_nalloc = 0;
				}
			}
		}

		/* allocate secondary data structure for
		    heading data if needed */
		if (status == MB_SUCCESS && (type == EM2_HEADING) && store->heading == NULL) {
			status = mbsys_simrad2_heading_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    attitude data if needed */
		if (status == MB_SUCCESS && (type == EM2_ATTITUDE) && store->attitude == NULL) {
			status = mbsys_simrad2_attitude_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    ssv data if needed */
		if (status == MB_SUCCESS && (type == EM2_SSV) && store->ssv == NULL) {
			status = mbsys_simrad2_ssv_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    tilt data if needed */
		if (status == MB_SUCCESS && (type == EM2_TILT) && store->tilt == NULL) {
			status = mbsys_simrad2_tilt_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* allocate secondary data structure for
		    survey data if needed */
		if (status == MB_SUCCESS &&
		    (type == EM2_BATH || type == EM2_RAWBEAM || type == EM2_RAWBEAM2 || type == EM2_RAWBEAM3 || type == EM2_SS)) {
			if (store->ping == NULL)
				status = mbsys_simrad2_survey_alloc(verbose, mbio_ptr, store_ptr, error);
			ping = (struct mbsys_simrad2_ping_struct *)store->ping;
			ping2 = (struct mbsys_simrad2_ping_struct *)store->ping2;
		}

		/* allocate secondary data structure for
		    water column data if needed */
		if (status == MB_SUCCESS && (type == EM2_WATERCOLUMN)) {
			if (store->wc == NULL)
				status = mbsys_simrad2_wc_alloc(verbose, mbio_ptr, store_ptr, error);
		}

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == EM2_NONE) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call nothing, read failure, no expect\n");
#endif
			done = true;
			record_size = 0;
			*record_size_save = record_size;
		}
		else if (status == MB_FAILURE && expect != EM2_NONE) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call nothing, read failure, expect %x\n", expect);
#endif
			done = true;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else if (type != EM2_STOP2 && type != EM2_OFF && type != EM2_ON && type != EM2_EXTRAPARAMETERS && type != EM2_ATTITUDE &&
		         type != EM2_CLOCK && type != EM2_BATH && type != EM2_SBDEPTH && type != EM2_RAWBEAM && type != EM2_SSV &&
		         type != EM2_HEADING && type != EM2_START && type != EM2_TILT && type != EM2_CBECHO && type != EM2_POS &&
		         type != EM2_RUN_PARAMETER && type != EM2_SS && type != EM2_TIDE && type != EM2_SVP2 && type != EM2_SVP &&
		         type != EM2_SSPINPUT && type != EM2_RAWBEAM2 && type != EM2_RAWBEAM3 && type != EM2_HEIGHT && type != EM2_STOP &&
		         type != EM2_WATERCOLUMN && type != EM2_REMOTE && type != EM2_SSP && type != EM2_BATH_MBA && type != EM2_SS_MBA) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call nothing, try again\n");
#endif
			done = false;
		}
		else if ((type == EM2_START || type == EM2_STOP) && expect != EM2_NONE) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call nothing, expect %x but got type %x\n", expect, type);
#endif
			done = true;
			expect = EM2_NONE;
			type = first_type;
			*label_save_flag = true;
			store->kind = MB_DATA_DATA;
		}
		else if (type == EM2_START || type == EM2_STOP) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_start type %x\n", type);
#endif
			status = mbr_em300raw_rd_start(verbose, mbfp, swap, store, type, sonar, version, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_RUN_PARAMETER) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_run_parameter type %x\n", type);
#endif
			status = mbr_em300raw_rd_run_parameter(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_CLOCK) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_clock type %x\n", type);
#endif
			status = mbr_em300raw_rd_clock(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_TIDE) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_tide type %x\n", type);
#endif
			status = mbr_em300raw_rd_tide(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_HEIGHT) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_height type %x\n", type);
#endif
			status = mbr_em300raw_rd_height(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_HEADING) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_heading type %x\n", type);
#endif
			status = mbr_em300raw_rd_heading(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_SSV) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_ssv type %x\n", type);
#endif
			status = mbr_em300raw_rd_ssv(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_TILT) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_tilt type %x\n", type);
#endif
			status = mbr_em300raw_rd_tilt(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_EXTRAPARAMETERS) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_extraparameters type %x\n", type);
#endif
			status = mbr_em300raw_rd_extraparameters(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_ATTITUDE) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_attitude type %x\n", type);
#endif
			status = mbr_em300raw_rd_attitude(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_POS) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_pos type %x\n", type);
#endif
			status = mbr_em300raw_rd_pos(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_SVP) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_svp type %x\n", type);
#endif
			status = mbr_em300raw_rd_svp(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_SVP2) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_svp2 type %x\n", type);
#endif
			status = mbr_em300raw_rd_svp2(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else if (type == EM2_BATH && sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2) {
			if (expect == EM2_SS && store->ping->png_count == store->ping2->png_count &&
			    store->ping->png_serial != store->ping2->png_serial) {
#ifdef MBR_EM300RAW_DEBUG
				fprintf(stderr, "call nothing, expect %x but got type %x\n", expect, type);
#endif
				done = true;
				expect = EM2_NONE;
				type = first_type;
				*label_save_flag = true;
				store->kind = MB_DATA_DATA;
			}
			else {
#ifdef MBR_EM300RAW_DEBUG
				fprintf(stderr, "call mbr_em300raw_rd_bath type %x\n", type);
#endif
				status = mbr_em300raw_rd_bath(verbose, mbfp, swap, store, &match, sonar, *version, &good_end_bytes, error);
				if (status == MB_SUCCESS) {
					if (first_type == EM2_NONE || !match || store->ping->png_count != store->ping2->png_count ||
					    store->ping->png_serial != store->ping2->png_serial) {
						done = false;
						first_type = EM2_BATH;
						expect = EM2_SS;
					}
					else {
						done = true;
						expect = EM2_NONE;
					}
				}
			}
		}
		else if (type == EM2_BATH && expect == EM2_SS) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call nothing, expect %x but got type %x\n", expect, type);
#endif
			done = true;
			expect = EM2_NONE;
			type = first_type;
			*label_save_flag = true;
			store->kind = MB_DATA_DATA;
		}
		else if (type == EM2_BATH) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_bath type %x\n", type);
#endif
			status = mbr_em300raw_rd_bath(verbose, mbfp, swap, store, &match, sonar, *version, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				if (first_type == EM2_NONE || !match) {
					done = false;
					first_type = EM2_BATH;
					expect = EM2_SS;
				}
				else {
					done = true;
					expect = EM2_NONE;
				}
			}
		}
		else if (type == EM2_RAWBEAM) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_rawbeam type %x\n", type);
#endif
			status = mbr_em300raw_rd_rawbeam(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS)
				ping->png_raw1_read = true;
			if (expect == EM2_SS && ping->png_nbeams == 0) {
				done = true;
				expect = EM2_NONE;
			}
		}
		else if (type == EM2_RAWBEAM2) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_rawbeam2 type %x\n", type);
#endif
			status = mbr_em300raw_rd_rawbeam2(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS)
				ping->png_raw2_read = true;
			if (expect == EM2_SS && ping->png_nbeams == 0) {
				done = true;
				expect = EM2_NONE;
			}
		}
		else if (type == EM2_RAWBEAM3 && sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_rawbeam3 type %x\n", type);
#endif
			status = mbr_em300raw_rd_rawbeam3(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS)
				ping->png_raw3_read = true;
		}
		else if (type == EM2_RAWBEAM3) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_rawbeam3 type %x\n", type);
#endif
			status = mbr_em300raw_rd_rawbeam3(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS)
				ping->png_raw3_read = true;
			if (expect == EM2_SS && ping->png_nbeams == 0) {
				done = true;
				expect = EM2_NONE;
			}
		}
		else if (type == EM2_SS && sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_ss a type %x\n", type);
#endif
			status = mbr_em300raw_rd_ss(verbose, mbfp, swap, store, sonar, *length, &match, &good_end_bytes, error);
			if (status == MB_SUCCESS)
				ping->png_ss_read = true;
			if (status == MB_SUCCESS && ping->png_count == store->ping2->png_count && ping->png_count == ping->png_raw3_count &&
			    ping->png_count == ping->png_ss_count && store->ping2->png_count == store->ping2->png_raw3_count &&
			    store->ping2->png_count == store->ping2->png_ss_count) {
				done = true;
				expect = EM2_NONE;
			}
		}
		else if (type == EM2_SS && expect != EM2_NONE && expect != EM2_SS) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call nothing, expect %x but got type %x\n", expect, type);
#endif
			done = true;
			expect = EM2_NONE;
			type = first_type;
			*label_save_flag = true;
			store->kind = MB_DATA_DATA;
		}
		else if (type == EM2_SS) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_ss b type %x\n", type);
#endif
			status = mbr_em300raw_rd_ss(verbose, mbfp, swap, store, sonar, *length, &match, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				ping->png_ss_read = true;
				if (first_type == EM2_NONE || !match) {
					done = false;
					first_type = EM2_SS;
					expect = EM2_BATH;
				}
				else {
					done = true;
					expect = EM2_NONE;
				}
			}

			/* salvage bath even if sidescan is corrupt */
			else {
				if (first_type == EM2_BATH && match) {
					status = MB_SUCCESS;
					done = true;
					expect = EM2_NONE;
				}
			}
		}
		else if (type == EM2_WATERCOLUMN) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_rd_wc type %x\n", type);
#endif
			status = mbr_em300raw_rd_wc(verbose, mbfp, swap, store, sonar, &good_end_bytes, error);
			if (status == MB_SUCCESS) {
				done = true;
				if (expect != EM2_NONE) {
					*expect_save = expect;
					*expect_save_flag = true;
					*first_type_save = first_type;
				}
				else
					*expect_save_flag = false;
			}
		}
		else {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "skip over %d bytes of unsupported datagram type %x\n", *record_size_save, type);
#endif
			for (int i = 0; i < *record_size_save - 4; i++) {
				size_t read_len;
				if ((read_len = fread(&junk, 1, 1, mb_io_ptr->mbfp)) != 1) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					expect = EM2_NONE;
				}
			}
			done = false;
		}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done = true;

		/* if necessary read over unread but expected bytes */
		int bytes_read = ftell(mbfp) - mb_io_ptr->file_bytes - 4;
		if (!*label_save_flag && !good_end_bytes && bytes_read < record_size) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "skip over %d unread bytes of supported datagram type %x\n", record_size - bytes_read, type);
#endif
			for (int i = 0; i < record_size - bytes_read; i++) {
				size_t read_len;
				if ((read_len = fread(&junk, 1, 1, mb_io_ptr->mbfp)) != 1) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					expect = EM2_NONE;
				}
			}
		}

#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "record_size:%d bytes read:%ld file_pos old:%ld new:%ld\n", record_size,
		        ftell(mbfp) - mb_io_ptr->file_bytes, mb_io_ptr->file_bytes, ftell(mbfp));
		fprintf(stderr, "done:%d expect:%x status:%d error:%d\n", done, expect, status, *error);
		fprintf(stderr, "end of mbr_em300raw_rd_data loop:\n\n");
#endif

		/* get file position */
		if (*label_save_flag)
			mb_io_ptr->file_bytes = ftell(mbfp) - 2;
		else
			mb_io_ptr->file_bytes = ftell(mbfp);
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
int mbr_rt_em300raw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int time_i[7];
	double ntime_d, ptime_d, atime_d;
	double bath_time_d, ss_time_d;
	double plon, plat, pspeed, roll, pitch, heave;

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
	int status = mbr_em300raw_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* get pointers to data structures */
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;
	struct mbsys_simrad2_attitude_struct *attitude = (struct mbsys_simrad2_attitude_struct *)store->attitude;
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	/* save fix if nav data */
	if (status == MB_SUCCESS && store->kind == MB_DATA_NAV) {
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
		if (store->pos_longitude != EM2_INVALID_INT && store->pos_latitude != EM2_INVALID_INT)
			mb_navint_add(verbose, mbio_ptr, ntime_d, (double)(0.0000001 * store->pos_longitude),
			              (double)(0.00000005 * store->pos_latitude), error);
	}

	/* save attitude if attitude data */
	if (status == MB_SUCCESS && store->kind == MB_DATA_ATTITUDE) {
		/* get attitude time */
		time_i[0] = attitude->att_date / 10000;
		time_i[1] = (attitude->att_date % 10000) / 100;
		time_i[2] = attitude->att_date % 100;
		time_i[3] = attitude->att_msec / 3600000;
		time_i[4] = (attitude->att_msec % 3600000) / 60000;
		time_i[5] = (attitude->att_msec % 60000) / 1000;
		time_i[6] = (attitude->att_msec % 1000) * 1000;
		mb_get_time(verbose, time_i, &atime_d);

		double att_time_d[MBSYS_SIMRAD2_MAXATTITUDE];
		double att_roll[MBSYS_SIMRAD2_MAXATTITUDE];
		double att_pitch[MBSYS_SIMRAD2_MAXATTITUDE];
		double att_heave[MBSYS_SIMRAD2_MAXATTITUDE];
		/* add latest attitude samples */
		for (int i = 0; i < MIN(attitude->att_ndata, MBSYS_SIMRAD2_MAXATTITUDE); i++) {
			att_time_d[i] = (double)(atime_d + 0.001 * attitude->att_time[i]);
			att_heave[i] = (double)(0.01 * attitude->att_heave[i]);
			att_roll[i] = (double)(0.01 * attitude->att_roll[i]);
			att_pitch[i] = (double)(0.01 * attitude->att_pitch[i]);
		}
		mb_attint_nadd(verbose, mbio_ptr, attitude->att_ndata, att_time_d, att_heave, att_roll, att_pitch, error);
	}

	/* if no sidescan read then zero sidescan data */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && !ping->png_ss_read) {
		status = mbsys_simrad2_zero_ss(verbose, store_ptr, error);
	}

	/* else check that bath and sidescan data record time stamps
	   match for survey data - we can have bath without
	   sidescan but not sidescan without bath */
	else if (status == MB_SUCCESS && store->kind == MB_DATA_DATA) {
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

		/* check for time match - if bath newer than
		   sidescan then zero sidescan,  if sidescan
		   newer than bath then set error,  if ok then
		   check that beam ids are the same */
		if (ping->png_ss_date == 0 || ping->png_nbeams_ss == 0 || bath_time_d > ss_time_d) {
			status = mbsys_simrad2_zero_ss(verbose, store_ptr, error);
		}
		else if (bath_time_d > ss_time_d) {
			if (verbose > 0)
				fprintf(stderr, "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan zeroed, bathtime:%f >  sstime:%f\n",
				        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], bath_time_d,
				        ss_time_d);
			status = mbsys_simrad2_zero_ss(verbose, store_ptr, error);
		}
		else if (bath_time_d < ss_time_d) {
			if (verbose > 0)
				fprintf(stderr, "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Ping unintelligible bathtime:%f < sstime%f\n",
				        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], bath_time_d,
				        ss_time_d);
			*error = MB_ERROR_UNINTELLIGIBLE;
			status = MB_FAILURE;
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
			else if (ping->png_nbeams == ping->png_nbeams_ss) {
				for (int i = 0; i < ping->png_nbeams; i++) {
					if (ping->png_beam_num[i] != ping->png_beam_index[i] + 1 &&
					    ping->png_beam_num[i] != ping->png_beam_index[i] - 1) {
						if (verbose > 1)
							fprintf(stderr,
							        "%s: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d Sidescan ignored: bath and ss beam indexes "
							        "don't match: : %d %d %d\n",
							        __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], i,
							        ping->png_beam_num[i], ping->png_beam_index[i]);
					}
				}
			}
		}
	}

	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA) {
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
		double rawspeed;
		if (store->pos_speed == 0 || store->pos_speed == EM2_INVALID_SHORT)
			rawspeed = 0.0;
		else
			rawspeed = 0.036 * store->pos_speed;
		const double pheading = 0.01 * ping->png_heading;
		mb_navint_interp(verbose, mbio_ptr, ptime_d, pheading, rawspeed, &plon, &plat, &pspeed, error);
		if (plon == 0.0 && plat == 0.0) {
			ping->png_longitude = (int)EM2_INVALID_INT;
			ping->png_latitude = (int)EM2_INVALID_INT;
		}
		else {
			ping->png_longitude = (int)rint(10000000 * plon);
			ping->png_latitude = (int)rint(20000000 * plat);
		}
		ping->png_speed = (int)rint(pspeed / 0.036);

		/* interpolate from saved attitude */
		mb_attint_interp(verbose, mbio_ptr, ptime_d, &heave, &roll, &pitch, error);
		ping->png_roll = (int)rint(roll / 0.01);
		ping->png_pitch = (int)rint(pitch / 0.01);
		ping->png_heave = (int)rint(heave / 0.01);

		/* generate processed sidescan */
		ping->png_pixel_size = 0;
		ping->png_pixels_ss = 0;
	  double *pixel_size = (double *)&mb_io_ptr->saved1;
	  double *swath_width = (double *)&mb_io_ptr->saved2;
		status = mbsys_simrad2_makess(verbose, mbio_ptr, store_ptr, false, pixel_size, false, swath_width, 0, error);
	}

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
		mb_attint_interp(verbose, mbio_ptr, ntime_d, &heave, &roll, &pitch, error);
		store->pos_roll = (int)rint(roll / 0.01);
		store->pos_pitch = (int)rint(pitch / 0.01);
		store->pos_heave = (int)rint(heave / 0.01);
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
int mbr_em300raw_wr_start(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[MBSYS_SIMRAD2_BUFFER_SIZE], *buff;
	size_t buff_len;
  size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
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
		fprintf(stderr, "dbg5       par_go1:         %f\n", store->par_go1);
		fprintf(stderr, "dbg5       par_go2:         %f\n", store->par_go2);
		fprintf(stderr, "dbg5       par_tsv:         %s\n", store->par_tsv);
		fprintf(stderr, "dbg5       par_rsv:         %s\n", store->par_rsv);
		fprintf(stderr, "dbg5       par_bsv:         %s\n", store->par_bsv);
		fprintf(stderr, "dbg5       par_psv:         %s\n", store->par_psv);
		fprintf(stderr, "dbg5       par_osv:         %s\n", store->par_osv);
		fprintf(stderr, "dbg5       par_dsd:         %f\n", store->par_dsd);
		fprintf(stderr, "dbg5       par_dso:         %f\n", store->par_dso);
		fprintf(stderr, "dbg5       par_dsf:         %f\n", store->par_dsf);
		fprintf(stderr, "dbg5       par_dsh:         %c%c\n", store->par_dsh[0], store->par_dsh[1]);
		fprintf(stderr, "dbg5       par_aps:         %d\n", store->par_aps);
		fprintf(stderr, "dbg5       par_p1m:         %d\n", store->par_p1m);
		fprintf(stderr, "dbg5       par_p1t:         %d\n", store->par_p1t);
		fprintf(stderr, "dbg5       par_p1z:         %f\n", store->par_p1z);
		fprintf(stderr, "dbg5       par_p1x:         %f\n", store->par_p1x);
		fprintf(stderr, "dbg5       par_p1y:         %f\n", store->par_p1y);
		fprintf(stderr, "dbg5       par_p1d:         %f\n", store->par_p1d);
		fprintf(stderr, "dbg5       par_p1g:         %s\n", store->par_p1g);
		fprintf(stderr, "dbg5       par_p2m:         %d\n", store->par_p2m);
		fprintf(stderr, "dbg5       par_p2t:         %d\n", store->par_p2t);
		fprintf(stderr, "dbg5       par_p2z:         %f\n", store->par_p2z);
		fprintf(stderr, "dbg5       par_p2x:         %f\n", store->par_p2x);
		fprintf(stderr, "dbg5       par_p2y:         %f\n", store->par_p2y);
		fprintf(stderr, "dbg5       par_p2d:         %f\n", store->par_p2d);
		fprintf(stderr, "dbg5       par_p2g:         %s\n", store->par_p2g);
		fprintf(stderr, "dbg5       par_p3m:         %d\n", store->par_p3m);
		fprintf(stderr, "dbg5       par_p3t:         %d\n", store->par_p3t);
		fprintf(stderr, "dbg5       par_p3z:         %f\n", store->par_p3z);
		fprintf(stderr, "dbg5       par_p3x:         %f\n", store->par_p3x);
		fprintf(stderr, "dbg5       par_p3y:         %f\n", store->par_p3y);
		fprintf(stderr, "dbg5       par_p3d:         %f\n", store->par_p3d);
		fprintf(stderr, "dbg5       par_p3g:         %s\n", store->par_p3g);
		fprintf(stderr, "dbg5       par_msz:         %f\n", store->par_msz);
		fprintf(stderr, "dbg5       par_msx:         %f\n", store->par_msx);
		fprintf(stderr, "dbg5       par_msy:         %f\n", store->par_msy);
		fprintf(stderr, "dbg5       par_mrp:         %c%c\n", store->par_mrp[0], store->par_mrp[1]);
		fprintf(stderr, "dbg5       par_msd:         %f\n", store->par_msd);
		fprintf(stderr, "dbg5       par_msr:         %f\n", store->par_msr);
		fprintf(stderr, "dbg5       par_msp:         %f\n", store->par_msp);
		fprintf(stderr, "dbg5       par_msg:         %f\n", store->par_msg);
		fprintf(stderr, "dbg5       par_gcg:         %f\n", store->par_gcg);
		fprintf(stderr, "dbg5       par_cpr:         %s\n", store->par_cpr);
		fprintf(stderr, "dbg5       par_rop:         %s\n", store->par_rop);
		fprintf(stderr, "dbg5       par_sid:         %s\n", store->par_sid);
		fprintf(stderr, "dbg5       par_pll:         %s\n", store->par_pll);
		fprintf(stderr, "dbg5       par_com:         %s\n", store->par_com);
	}

	/* zero checksum */
	checksum = 0;

	/* if data type not set - use start */
	if (store->type == EM2_NONE)
		store->type = EM2_START;

	/* if sonar not set use EM200 */
	if (store->sonar == 0)
		store->sonar = MBSYS_SIMRAD2_EM300;

	/* set up start of output buffer - we handle this
	   record differently because of the ascii data */
	memset(line, 0, MBSYS_SIMRAD2_BUFFER_SIZE);

	/* put binary header data into buffer */
	mb_put_binary_short(swap, (short)store->type, (void *)&line[4]);
	mb_put_binary_short(swap, (unsigned short)store->sonar, (void *)&line[6]);
	mb_put_binary_int(swap, (int)store->par_date, (void *)&line[8]);
	mb_put_binary_int(swap, (int)store->par_msec, (void *)&line[12]);
	mb_put_binary_short(swap, (unsigned short)store->par_line_num, (void *)&line[16]);
	mb_put_binary_short(swap, (unsigned short)store->par_serial_1, (void *)&line[18]);
	mb_put_binary_short(swap, (unsigned short)store->par_serial_2, (void *)&line[20]);

	/* construct ASCII parameter buffer */
	buff = &line[22];
	sprintf(&buff[0], "WLZ=%.2f,", store->par_wlz);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "SMH=%d,", store->par_smh);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1Z=%.2f,", store->par_s1z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1X=%.2f,", store->par_s1x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1Y=%.2f,", store->par_s1y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1H=%.2f,", store->par_s1h);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1R=%.2f,", store->par_s1r);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S1P=%.2f,", store->par_s1p);
	buff_len = strlen(buff);
	if (store->par_s1n > 0) {
		sprintf(&buff[buff_len], "S1N=%d,", store->par_s1n);
		buff_len = strlen(buff);
	}
	sprintf(&buff[buff_len], "S2Z=%.2f,", store->par_s2z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2X=%.2f,", store->par_s2x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2Y=%.2f,", store->par_s2y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2H=%.2f,", store->par_s2h);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2R=%.2f,", store->par_s2r);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "S2P=%.2f,", store->par_s2p);
	buff_len = strlen(buff);
	if (store->par_s2n > 0) {
		sprintf(&buff[buff_len], "S2N=%d,", store->par_s2n);
		buff_len = strlen(buff);
	}
	if (store->par_go1 != 0.0) {
		sprintf(&buff[buff_len], "GO1=%.2f,", store->par_go1);
		buff_len = strlen(buff);
	}
	if (store->par_go2 != 0.0) {
		sprintf(&buff[buff_len], "GO2=%.2f,", store->par_go2);
		buff_len = strlen(buff);
	}
	sprintf(&buff[buff_len], "TSV=%s,", store->par_tsv);
	buff_len = strlen(buff);
	if (strlen(store->par_rsv) > 0) {
		sprintf(&buff[buff_len], "RSV=%s,", store->par_rsv);
		buff_len = strlen(buff);
	}
	sprintf(&buff[buff_len], "BSV=%s,", store->par_bsv);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "PSV=%s,", store->par_tsv);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "OSV=%s,", store->par_osv);
	buff_len = strlen(buff);
	if (store->par_dsd != 0.0) {
		sprintf(&buff[buff_len], "DSD=%.1f,", store->par_dsd);
		buff_len = strlen(buff);
	}
	else {
		sprintf(&buff[buff_len], "DSD=,");
		buff_len = strlen(buff);
	}
	sprintf(&buff[buff_len], "DSO=%.6f,", store->par_dso);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSF=%.6f,", store->par_dsf);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "DSH=%c%c,", store->par_dsh[0], store->par_dsh[1]);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "APS=%d,", store->par_aps);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1M=%d,", store->par_p1m);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1T=%d,", store->par_p1t);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1Z=%.2f,", store->par_p1z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1X=%.2f,", store->par_p1x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1Y=%.2f,", store->par_p1y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1D=%.1f,", store->par_p1d);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P1G=%s,", store->par_p1g);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2M=%d,", store->par_p2m);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2T=%d,", store->par_p2t);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2Z=%.2f,", store->par_p2z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2X=%.2f,", store->par_p2x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2Y=%.2f,", store->par_p2y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2D=%.1f,", store->par_p2d);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P2G=%s,", store->par_p2g);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3M=%d,", store->par_p3m);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3T=%d,", store->par_p3t);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3Z=%.2f,", store->par_p3z);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3X=%.2f,", store->par_p3x);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3Y=%.2f,", store->par_p3y);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3D=%.1f,", store->par_p3d);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "P3G=%s,", store->par_p3g);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSZ=%.2f,", store->par_msz);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSX=%.2f,", store->par_msx);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSY=%.2f,", store->par_msy);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MRP=%c%c,", store->par_mrp[0], store->par_mrp[1]);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSD=%.2f,", store->par_msd);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSR=%.2f,", store->par_msr);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSP=%.2f,", store->par_msp);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "MSG=%.2f,", store->par_msg);
	buff_len = strlen(buff);
	sprintf(&buff[buff_len], "GCG=%.2f,", store->par_gcg);
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
	if (strlen(store->par_pll) > 0) {
		sprintf(&buff[buff_len], "PLL=%s,", store->par_pll);
		buff_len = strlen(buff);
	}
	if (strlen(store->par_com) > 0) {
		/* replace commas (,) with caret (^) values to circumvent
		   the format's inability to store commas in comments */
		char *comma_ptr;
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
	line[buff_len + 22] = EM2_END;

	/* get size of record */
	write_size = 25 + buff_len;
	mb_put_binary_int(swap, (int)(write_size - 4), (void *)&line[0]);

	/* compute checksum */
	uchar_ptr = (mb_u_char *)line;
	for (unsigned int j = 5; j < write_size - 3; j++)
		checksum += uchar_ptr[j];

	/* set checksum */
	mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[buff_len + 23]);

	int status = MB_SUCCESS;

	/* finally write out the data */
	write_len = fwrite(&line, 1, write_size, mbfp);
	if (write_len != write_size) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

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
int mbr_em300raw_wr_run_parameter(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_RUN_PARAMETER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
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

	int status = MB_SUCCESS;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_RUN_PARAMETER_SIZE), (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_RUN_PARAMETER), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		line[EM2_RUN_PARAMETER_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM2_RUN_PARAMETER_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM2_RUN_PARAMETER_SIZE - 6]);

		/* write out data */
		write_len = fwrite(line, 1, EM2_RUN_PARAMETER_SIZE - 4, mbfp);
		if (write_len != EM2_RUN_PARAMETER_SIZE - 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_clock(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_CLOCK_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
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

	int status = MB_SUCCESS;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_CLOCK_SIZE), (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_CLOCK), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		line[EM2_CLOCK_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM2_CLOCK_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM2_CLOCK_SIZE - 6]);

		/* write out data */
		write_len = fwrite(line, 1, EM2_CLOCK_SIZE - 4, mbfp);
		if (write_len != EM2_CLOCK_SIZE - 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_tide(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_TIDE_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
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

	int status = MB_SUCCESS;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_TIDE_SIZE), (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_TIDE), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		line[EM2_TIDE_SIZE - 8] = '\0';
		line[EM2_TIDE_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM2_TIDE_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM2_TIDE_SIZE - 6]);

		/* write out data */
		write_len = fwrite(line, 1, EM2_TIDE_SIZE - 4, mbfp);
		if (write_len != EM2_TIDE_SIZE - 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_height(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_HEIGHT_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
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

	int status = MB_SUCCESS;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_HEIGHT_SIZE), (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_HEIGHT), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		line[EM2_HEIGHT_SIZE - 7] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM2_HEIGHT_SIZE - 7; j++)
			checksum += uchar_ptr[j];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[EM2_HEIGHT_SIZE - 6]);

		/* write out data */
		write_len = fwrite(line, 1, EM2_HEIGHT_SIZE - 4, mbfp);
		if (write_len != EM2_HEIGHT_SIZE - 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_heading(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_HEADING_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	struct mbsys_simrad2_heading_struct *heading = (struct mbsys_simrad2_heading_struct *)store->heading;

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

	int status = MB_SUCCESS;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_HEADING_HEADER_SIZE + EM2_HEADING_SLICE_SIZE * heading->hed_ndata + 8),
	                  (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_HEADING), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		for (int j = 0; j < EM2_HEADING_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_HEADING_HEADER_SIZE, mbfp);
		if (write_len != EM2_HEADING_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary heading data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < heading->hed_ndata; i++) {
			mb_put_binary_short(swap, (unsigned short)heading->hed_time[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)heading->hed_heading[i], (void *)&line[2]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_HEADING_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_HEADING_SLICE_SIZE, mbfp);
			if (write_len != EM2_HEADING_SLICE_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_ssv(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_SSV_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	struct mbsys_simrad2_ssv_struct *ssv = (struct mbsys_simrad2_ssv_struct *)store->ssv;

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

	int status = MB_SUCCESS;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_SSV_HEADER_SIZE + EM2_SSV_SLICE_SIZE * ssv->ssv_ndata + 8), (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_SSV), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		for (int j = 0; j < EM2_SSV_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_SSV_HEADER_SIZE, mbfp);
		if (write_len != EM2_SSV_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary ssv data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ssv->ssv_ndata; i++) {
			mb_put_binary_short(swap, (unsigned short)ssv->ssv_time[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)ssv->ssv_ssv[i], (void *)&line[2]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_SSV_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_SSV_SLICE_SIZE, mbfp);
			if (write_len != EM2_SSV_SLICE_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_tilt(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_TILT_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	struct mbsys_simrad2_tilt_struct *tilt = (struct mbsys_simrad2_tilt_struct *)store->tilt;

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

	int status = MB_SUCCESS;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_TILT_HEADER_SIZE + EM2_TILT_SLICE_SIZE * tilt->tlt_ndata + 8), (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_TILT), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		for (int j = 0; j < EM2_TILT_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_TILT_HEADER_SIZE, mbfp);
		if (write_len != EM2_TILT_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary tilt data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < tilt->tlt_ndata; i++) {
			mb_put_binary_short(swap, (unsigned short)tilt->tlt_time[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)tilt->tlt_tilt[i], (void *)&line[2]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_TILT_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_TILT_SLICE_SIZE, mbfp);
			if (write_len != EM2_TILT_SLICE_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_extraparameters(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_EXTRAPARAMETERS_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	struct mbsys_simrad2_extraparameters_struct *extraparameters = (struct mbsys_simrad2_extraparameters_struct *)store->extraparameters;

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
/*		if (extraparameters->xtr_id == 2) {
			fprintf(stderr, "dbg5       xtr_pqf_activepositioning:          %d\n", extraparameters->xtr_pqf_activepositioning);
			for (int i = 0; i < 3; i++) {
				fprintf(stderr, "dbg5       positioning system:%d qfsetting:%d nqf:%d\n", i,
				        extraparameters->xtr_pqf_qfsetting[i], extraparameters->xtr_pqf_nqualityfactors[i]);
				for (int j = 0; j < extraparameters->xtr_pqf_nqualityfactors[i]; j++)
					fprintf(stderr, "dbg5       quality factor:%d value:%d limit:%d\n", j,
					        extraparameters->xtr_pqf_qfvalues[i][j], extraparameters->xtr_pqf_qflimits[i][j]);
			}
		}*/
	}

	/* zero checksum */
	checksum = 0;

	int status = MB_SUCCESS;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_EXTRAPARAMETERS_HEADER_SIZE + extraparameters->xtr_data_size + 8), (void *)&write_size);
	write_len = fwrite((char *)&write_size, 1, 4, mbfp);
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_EXTRAPARAMETERS), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		for (int j = 0; j < EM2_EXTRAPARAMETERS_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_EXTRAPARAMETERS_HEADER_SIZE, mbfp);
		if (write_len != EM2_EXTRAPARAMETERS_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary extraparameters data */
	if (status == MB_SUCCESS) {
		/* compute checksum */
		uchar_ptr = (mb_u_char *)extraparameters->xtr_data;
		for (int j = 0; j < extraparameters->xtr_data_size; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(extraparameters->xtr_data, 1, extraparameters->xtr_data_size, mbfp);
		if (write_len != (size_t) extraparameters->xtr_data_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_attitude(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_ATTITUDE_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	struct mbsys_simrad2_attitude_struct *attitude = (struct mbsys_simrad2_attitude_struct *)store->attitude;

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
		fprintf(stderr, "dbg5       att_heading_status: %d\n", attitude->att_heading_status);
	}

	/* zero checksum */
	checksum = 0;

	int status = MB_SUCCESS;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_ATTITUDE_HEADER_SIZE + EM2_ATTITUDE_SLICE_SIZE * attitude->att_ndata + 8),
	                  (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_ATTITUDE), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		for (int j = 0; j < EM2_ATTITUDE_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_ATTITUDE_HEADER_SIZE, mbfp);
		if (write_len != EM2_ATTITUDE_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
			for (int j = 0; j < EM2_ATTITUDE_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_ATTITUDE_SLICE_SIZE, mbfp);
			if (write_len != EM2_ATTITUDE_SLICE_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = (mb_u_char)attitude->att_heading_status;
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_pos(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_POS_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
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
	mb_put_binary_int(swap, (int)(EM2_POS_HEADER_SIZE + store->pos_input_size - (store->pos_input_size % 2) + 8),
	                  (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_POS), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		for (int j = 0; j < EM2_POS_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_POS_HEADER_SIZE, mbfp);
		if (write_len != EM2_POS_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output original ascii nav data */
	if (status == MB_SUCCESS) {
		write_size = store->pos_input_size - (store->pos_input_size % 2) + 1;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)store->pos_input;
		for (unsigned int j = 0; j < write_size; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(store->pos_input, 1, write_size, mbfp);
		if (write_len != write_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[1] = 0x03;

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = fwrite(&line[1], 1, 3, mbfp);
		if (write_len != 3) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_svp(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_SVP_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
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
	mb_put_binary_int(swap, (int)(EM2_SVP_HEADER_SIZE + EM2_SVP_SLICE_SIZE * store->svp_num + 8), (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_SVP), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		for (int j = 0; j < EM2_SVP_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_SVP_HEADER_SIZE, mbfp);
		if (write_len != EM2_SVP_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary svp data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < store->svp_num; i++) {
			mb_put_binary_short(swap, (unsigned short)store->svp_depth[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)store->svp_vel[i], (void *)&line[4]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_SVP_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_SVP_SLICE_SIZE, mbfp);
			if (write_len != EM2_SVP_SLICE_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_svp2(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_SVP2_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
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
	mb_put_binary_int(swap, (int)(EM2_SVP2_HEADER_SIZE + EM2_SVP2_SLICE_SIZE * store->svp_num + 8), (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_SVP2), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		for (int j = 0; j < EM2_SVP2_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_SVP2_HEADER_SIZE, mbfp);
		if (write_len != EM2_SVP2_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary svp data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < store->svp_num; i++) {
			mb_put_binary_int(swap, (int)store->svp_depth[i], (void *)&line[0]);
			mb_put_binary_int(swap, (int)store->svp_vel[i], (void *)&line[4]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_SVP2_SLICE_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_SVP2_SLICE_SIZE, mbfp);
			if (write_len != EM2_SVP2_SLICE_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_bath(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int head, int *error) {
	char line[EM2_BATH_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       head:       %d\n", head);
	}

	/* get storage structure */
	struct mbsys_simrad2_ping_struct *ping;
	if (store->sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2 && head == 1)
		ping = (struct mbsys_simrad2_ping_struct *)store->ping2;
	else
		ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       png_date:        %d\n", ping->png_date);
		fprintf(stderr, "dbg5       png_msec:        %d\n", ping->png_msec);
		fprintf(stderr, "dbg5       png_count:       %d\n", ping->png_count);
		fprintf(stderr, "dbg5       png_serial:      %d\n", ping->png_serial);
		fprintf(stderr, "dbg5       png_heading:     %d\n", ping->png_heading);
		fprintf(stderr, "dbg5       png_ssv:         %d\n", ping->png_ssv);
		fprintf(stderr, "dbg5       png_xducer_depth:      %d\n", ping->png_xducer_depth);
		fprintf(stderr, "dbg5       png_offset_multiplier: %d\n", ping->png_offset_multiplier);
		fprintf(stderr, "dbg5       png_nbeams_max:        %d\n", ping->png_nbeams_max);
		fprintf(stderr, "dbg5       png_nbeams:            %d\n", ping->png_nbeams);
		fprintf(stderr, "dbg5       png_depth_res:         %d\n", ping->png_depth_res);
		fprintf(stderr, "dbg5       png_distance_res:      %d\n", ping->png_distance_res);
		fprintf(stderr, "dbg5       png_sample_rate:       %d\n", ping->png_sample_rate);
		fprintf(stderr, "dbg5       cnt  depth xtrack ltrack dprsn   azi   rng  qual wnd amp num\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %6d %6d %6d %5d %5d %5d %4d %3d %3d %3d\n", i, ping->png_depth[i],
			        ping->png_acrosstrack[i], ping->png_alongtrack[i], ping->png_depression[i], ping->png_azimuth[i],
			        ping->png_range[i], ping->png_quality[i], ping->png_window[i], ping->png_amp[i], ping->png_beam_num[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_BATH_HEADER_SIZE + EM2_BATH_BEAM_SIZE * ping->png_nbeams + 8), (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_BATH), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		mb_put_binary_short(swap, (unsigned short)ping->png_xducer_depth, (void *)&line[16]);
		line[18] = (mb_u_char)ping->png_nbeams_max;
		line[19] = (mb_u_char)ping->png_nbeams;
		line[20] = (mb_u_char)ping->png_depth_res;
		line[21] = (mb_u_char)ping->png_distance_res;
		mb_put_binary_short(swap, (short)ping->png_sample_rate, (void *)&line[22]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM2_BATH_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_BATH_HEADER_SIZE, mbfp);
		if (write_len != EM2_BATH_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary beam data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_nbeams; i++) {
			if (!mb_beam_ok(ping->png_beamflag[i]))
				ping->png_depth[i] = 0;
			if (store->sonar == MBSYS_SIMRAD2_EM120 || store->sonar == MBSYS_SIMRAD2_EM300)
				mb_put_binary_short(swap, (unsigned short)ping->png_depth[i], (void *)&line[0]);
			else
				mb_put_binary_short(swap, (short)ping->png_depth[i], (void *)&line[0]);
			mb_put_binary_short(swap, (short)ping->png_acrosstrack[i], (void *)&line[2]);
			mb_put_binary_short(swap, (short)ping->png_alongtrack[i], (void *)&line[4]);
			mb_put_binary_short(swap, (short)ping->png_depression[i], (void *)&line[6]);
			mb_put_binary_short(swap, (unsigned short)ping->png_azimuth[i], (void *)&line[8]);
			mb_put_binary_short(swap, (unsigned short)ping->png_range[i], (void *)&line[10]);
			line[12] = (mb_u_char)ping->png_quality[i];
			line[13] = (mb_u_char)ping->png_window[i];
			line[14] = (mb_s_char)ping->png_amp[i];
			line[15] = (mb_u_char)ping->png_beam_num[i];

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_BATH_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_BATH_BEAM_SIZE, mbfp);
			if (write_len != EM2_BATH_BEAM_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
		}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[0] = (mb_s_char)ping->png_offset_multiplier;
		line[1] = 0x03;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		checksum += uchar_ptr[0];

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_rawbeam(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_RAWBEAM_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       png_raw_date:        %d\n", ping->png_raw_date);
		fprintf(stderr, "dbg5       png_raw_msec:        %d\n", ping->png_raw_msec);
		fprintf(stderr, "dbg5       png_raw_count:       %d\n", ping->png_raw_count);
		fprintf(stderr, "dbg5       png_raw_serial:      %d\n", ping->png_raw_serial);
		fprintf(stderr, "dbg5       png_raw_nbeams_max:  %d\n", ping->png_raw_nbeams_max);
		fprintf(stderr, "dbg5       png_raw_nbeams:      %d\n", ping->png_raw_nbeams);
		fprintf(stderr, "dbg5       png_raw_ssv:         %d\n", ping->png_raw_ssv);
		fprintf(stderr, "dbg5       cnt  point   tilt   rng  amp num\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %5d %5d %5d %3d %3d\n", i, ping->png_raw_rxpointangle[i],
			        ping->png_raw_rxtiltangle[i], ping->png_raw_rxrange[i], ping->png_raw_rxamp[i], ping->png_raw_rxbeam_num[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap, (int)(EM2_RAWBEAM_HEADER_SIZE + EM2_RAWBEAM_BEAM_SIZE * ping->png_raw_nbeams + 8),
	                  (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_RAWBEAM), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		line[12] = (mb_u_char)ping->png_raw_nbeams_max;
		line[13] = (mb_u_char)ping->png_raw_nbeams;
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_ssv, (void *)&line[14]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM2_RAWBEAM_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_RAWBEAM_HEADER_SIZE, mbfp);
		if (write_len != EM2_RAWBEAM_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary beam data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_raw_nbeams; i++) {
			mb_put_binary_short(swap, (short)ping->png_raw_rxpointangle[i], (void *)&line[0]);
			mb_put_binary_short(swap, (short)ping->png_raw_rxtiltangle[i], (void *)&line[2]);
			mb_put_binary_short(swap, (unsigned short)ping->png_raw_rxrange[i], (void *)&line[4]);
			line[6] = (mb_s_char)ping->png_raw_rxamp[i];
			line[7] = (mb_u_char)ping->png_raw_rxbeam_num[i];

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_RAWBEAM_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_RAWBEAM_BEAM_SIZE, mbfp);
			if (write_len != EM2_RAWBEAM_BEAM_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_rawbeam2(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_RAWBEAM2_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       png_raw_date:                %d\n", ping->png_raw_date);
		fprintf(stderr, "dbg5       png_raw_msec:                %d\n", ping->png_raw_msec);
		fprintf(stderr, "dbg5       png_raw_count:               %d\n", ping->png_raw_count);
		fprintf(stderr, "dbg5       png_raw_serial:              %d\n", ping->png_raw_serial);
		fprintf(stderr, "dbg5       png_raw_heading:             %d\n", ping->png_raw_heading);
		fprintf(stderr, "dbg5       png_raw_ssv:                 %d\n", ping->png_raw_ssv);
		fprintf(stderr, "dbg5       png_raw_xducer_depth:        %d\n", ping->png_raw_xducer_depth);
		fprintf(stderr, "dbg5       png_raw_nbeams_max:          %d\n", ping->png_raw_nbeams_max);
		fprintf(stderr, "dbg5       png_raw_nbeams:              %d\n", ping->png_raw_nbeams);
		fprintf(stderr, "dbg5       png_raw_depth_res:           %d\n", ping->png_raw_depth_res);
		fprintf(stderr, "dbg5       png_raw_distance_res:        %d\n", ping->png_raw_distance_res);
		fprintf(stderr, "dbg5       png_raw_sample_rate:         %d\n", ping->png_raw_sample_rate);
		fprintf(stderr, "dbg5       png_raw_status:              %d\n", ping->png_raw_status);
		fprintf(stderr, "dbg5       png_raw_rangenormal:         %d\n", ping->png_raw_rangenormal);
		fprintf(stderr, "dbg5       png_raw_normalbackscatter:   %d\n", ping->png_raw_normalbackscatter);
		fprintf(stderr, "dbg5       png_raw_obliquebackscatter:  %d\n", ping->png_raw_obliquebackscatter);
		fprintf(stderr, "dbg5       png_raw_fixedgain:           %d\n", ping->png_raw_fixedgain);
		fprintf(stderr, "dbg5       png_raw_txpower:             %d\n", ping->png_raw_txpower);
		fprintf(stderr, "dbg5       png_raw_mode:                %d\n", ping->png_raw_mode);
		fprintf(stderr, "dbg5       png_raw_coverage:            %d\n", ping->png_raw_coverage);
		fprintf(stderr, "dbg5       png_raw_yawstabheading:      %d\n", ping->png_raw_yawstabheading);
		fprintf(stderr, "dbg5       png_raw_ntx:                 %d\n", ping->png_raw_ntx);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       transmit pulse values:\n");
		fprintf(stderr, "dbg5       cnt lastbeam tiltangle heading roll pitch heave\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_ntx; i++)
			fprintf(stderr, "dbg5       %3d %3d %4d %5d %4d %4d %4d\n", i, ping->png_raw_txlastbeam[i],
			        ping->png_raw_txtiltangle[i], ping->png_raw_txheading[i], ping->png_raw_txroll[i], ping->png_raw_txpitch[i],
			        ping->png_raw_txheave[i]);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       beam values:\n");
		fprintf(stderr, "dbg5       cnt range quality window amp beam angle heading roll pitch heave\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %5d %3d %3d %4d %3d %5d %5d %4d %4d %4d\n", i, ping->png_raw_rxrange[i],
			        ping->png_raw_rxquality[i], ping->png_raw_rxwindow[i], ping->png_raw_rxamp[i], ping->png_raw_rxbeam_num[i],
			        ping->png_raw_rxpointangle[i], ping->png_raw_rxheading[i], ping->png_raw_rxroll[i], ping->png_raw_rxpitch[i],
			        ping->png_raw_rxheave[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap,
	                  (int)(EM2_RAWBEAM2_HEADER_SIZE + EM2_RAWBEAM2_TX_SIZE * ping->png_raw_ntx +
	                        EM2_RAWBEAM2_BEAM_SIZE * ping->png_raw_nbeams + 8),
	                  (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_RAWBEAM2), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_heading, (void *)&line[12]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_ssv, (void *)&line[14]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_xducer_depth, (void *)&line[16]);
		line[18] = (mb_u_char)ping->png_raw_nbeams_max;
		line[19] = (mb_u_char)ping->png_raw_nbeams;
		line[20] = (mb_u_char)ping->png_raw_depth_res;
		line[21] = (mb_u_char)ping->png_raw_distance_res;
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_sample_rate, (void *)&line[22]);
		mb_put_binary_int(swap, (int)ping->png_raw_status, (void *)&line[24]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_rangenormal, (void *)&line[28]);
		line[30] = (mb_s_char)ping->png_raw_normalbackscatter;
		line[31] = (mb_s_char)ping->png_raw_obliquebackscatter;
		line[32] = (mb_u_char)ping->png_raw_fixedgain;
		line[33] = (mb_s_char)ping->png_raw_txpower;
		line[34] = (mb_u_char)ping->png_raw_mode;
		line[35] = (mb_u_char)ping->png_raw_coverage;
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_yawstabheading, (void *)&line[36]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw_ntx, (void *)&line[38]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM2_RAWBEAM2_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_RAWBEAM2_HEADER_SIZE, mbfp);
		if (write_len != EM2_RAWBEAM2_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary tx data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_raw_ntx; i++) {
			mb_put_binary_short(swap, (unsigned short)ping->png_raw_txlastbeam[i], (void *)&line[0]);
			mb_put_binary_short(swap, (short)ping->png_raw_txtiltangle[i], (void *)&line[2]);
			mb_put_binary_short(swap, (unsigned short)ping->png_raw_txheading[i], (void *)&line[4]);
			mb_put_binary_short(swap, (short)ping->png_raw_txroll[i], (void *)&line[6]);
			mb_put_binary_short(swap, (short)ping->png_raw_txpitch[i], (void *)&line[8]);
			mb_put_binary_short(swap, (short)ping->png_raw_txheave[i], (void *)&line[10]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_RAWBEAM2_TX_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_RAWBEAM2_TX_SIZE, mbfp);
			if (write_len != EM2_RAWBEAM2_TX_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
		}

	/* output binary beam data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_raw_nbeams; i++) {
			mb_put_binary_short(swap, (unsigned short)ping->png_raw_rxrange[i], (void *)&line[0]);
			line[2] = (mb_u_char)ping->png_raw_rxquality[i];
			line[3] = (mb_u_char)ping->png_raw_rxwindow[i];
			line[4] = (mb_s_char)ping->png_raw_rxamp[i];
			line[5] = (mb_u_char)ping->png_raw_rxbeam_num[i];
			mb_put_binary_short(swap, (short)ping->png_raw_rxpointangle[i], (void *)&line[6]);
			mb_put_binary_short(swap, (unsigned short)ping->png_raw_rxheading[i], (void *)&line[8]);
			mb_put_binary_short(swap, (short)ping->png_raw_rxroll[i], (void *)&line[10]);
			mb_put_binary_short(swap, (short)ping->png_raw_rxpitch[i], (void *)&line[12]);
			mb_put_binary_short(swap, (short)ping->png_raw_rxheave[i], (void *)&line[14]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_RAWBEAM2_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_RAWBEAM2_BEAM_SIZE, mbfp);
			if (write_len != EM2_RAWBEAM2_BEAM_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_rawbeam3(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int head, int *error) {
	char line[EM2_RAWBEAM3_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       head:       %d\n", head);
	}

	/* get storage structure */
	struct mbsys_simrad2_ping_struct *ping;
	if (store->sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2 && head == 1)
		ping = (struct mbsys_simrad2_ping_struct *)store->ping2;
	else
		ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       png_raw3_date:                %d\n", ping->png_raw3_date);
		fprintf(stderr, "dbg5       png_raw3_msec:                %d\n", ping->png_raw3_msec);
		fprintf(stderr, "dbg5       png_raw3_count:               %d\n", ping->png_raw3_count);
		fprintf(stderr, "dbg5       png_raw3_serial:              %d\n", ping->png_raw3_serial);
		fprintf(stderr, "dbg5       png_raw3_ntx:                 %d\n", ping->png_raw3_ntx);
		fprintf(stderr, "dbg5       png_raw3_nbeams:              %d\n", ping->png_raw3_nbeams);
		fprintf(stderr, "dbg5       png_raw3_sample_rate:         %d\n", ping->png_raw3_sample_rate);
		fprintf(stderr, "dbg5       png_raw3_xducer_depth:        %d\n", ping->png_raw3_xducer_depth);
		fprintf(stderr, "dbg5       png_raw3_ssv:                 %d\n", ping->png_raw3_ssv);
		fprintf(stderr, "dbg5       png_raw3_nbeams_max:          %d\n", ping->png_raw3_nbeams_max);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       transmit pulse values:\n");
		fprintf(stderr, "dbg5       tiltangle focus length offset center bandwidth waveform sector\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw3_ntx; i++)
			fprintf(stderr, "dbg5       %3d %5d %5d %6d %4d %4d %4d %4d %4d\n", i, ping->png_raw3_txtiltangle[i],
			        ping->png_raw3_txfocus[i], ping->png_raw3_txsignallength[i], ping->png_raw3_txoffset[i],
			        ping->png_raw3_txcenter[i], ping->png_raw3_txbandwidth[i], ping->png_raw3_txwaveform[i],
			        ping->png_raw3_txsector[i]);
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		fprintf(stderr, "dbg5       beam values:\n");
		fprintf(stderr, "dbg5       angle range sector amp quality window beam\n");
		fprintf(stderr, "dbg5       ------------------------------------------------------------\n");
		for (int i = 0; i < ping->png_raw3_nbeams; i++)
			fprintf(stderr, "dbg5       %3d %5d %3d %3d %4d %3d %5d %5d %5d\n", i, ping->png_raw3_rxpointangle[i],
			        ping->png_raw3_rxrange[i], ping->png_raw3_rxsector[i], ping->png_raw3_rxamp[i], ping->png_raw3_rxquality[i],
			        ping->png_raw3_rxwindow[i], ping->png_raw3_rxbeam_num[i], ping->png_raw3_rxspare[i]);
	}

	/* zero checksum */
	checksum = 0;

	/* write the record size */
	mb_put_binary_int(swap,
	                  (int)(EM2_RAWBEAM3_HEADER_SIZE + EM2_RAWBEAM3_TX_SIZE * ping->png_raw3_ntx +
	                        EM2_RAWBEAM3_BEAM_SIZE * ping->png_raw3_nbeams + 8),
	                  (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_RAWBEAM3), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	/* write the sonar id */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[0];
		checksum += uchar_ptr[1];
	}

	/* output binary header data */
	if (status == MB_SUCCESS) {
		mb_put_binary_int(swap, (int)ping->png_raw3_date, (void *)&line[0]);
		mb_put_binary_int(swap, (int)ping->png_raw3_msec, (void *)&line[4]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw3_count, (void *)&line[8]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw3_serial, (void *)&line[10]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw3_ntx, (void *)&line[12]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw3_nbeams, (void *)&line[14]);
		mb_put_binary_int(swap, (unsigned int)ping->png_raw3_sample_rate, (void *)&line[16]);
		mb_put_binary_int(swap, (int)ping->png_raw3_xducer_depth, (void *)&line[20]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw3_ssv, (void *)&line[24]);
		mb_put_binary_short(swap, (unsigned short)ping->png_raw3_nbeams_max, (void *)&line[26]);
		mb_put_binary_short(swap, (unsigned short)0, (void *)&line[28]);
		mb_put_binary_short(swap, (unsigned short)0, (void *)&line[30]);

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM2_RAWBEAM3_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_RAWBEAM3_HEADER_SIZE, mbfp);
		if (write_len != EM2_RAWBEAM3_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary tx data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_raw3_ntx; i++) {
			mb_put_binary_short(swap, (short)ping->png_raw3_txtiltangle[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)ping->png_raw3_txfocus[i], (void *)&line[2]);
			mb_put_binary_int(swap, (int)ping->png_raw3_txsignallength[i], (void *)&line[4]);
			mb_put_binary_int(swap, (int)ping->png_raw3_txoffset[i], (void *)&line[8]);
			mb_put_binary_int(swap, (int)ping->png_raw3_txcenter[i], (void *)&line[12]);
			mb_put_binary_short(swap, (short)ping->png_raw3_txbandwidth[i], (void *)&line[16]);
			line[18] = (mb_u_char)ping->png_raw3_txwaveform[i];
			line[19] = (mb_u_char)ping->png_raw3_txsector[i];

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_RAWBEAM3_TX_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_RAWBEAM3_TX_SIZE, mbfp);
			if (write_len != EM2_RAWBEAM3_TX_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
		}

	/* output binary beam data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_raw3_nbeams; i++) {
			mb_put_binary_short(swap, (short)ping->png_raw3_rxpointangle[i], (void *)&line[0]);
			mb_put_binary_short(swap, (unsigned short)ping->png_raw3_rxrange[i], (void *)&line[2]);
			line[4] = (mb_u_char)ping->png_raw3_rxsector[i];
			line[5] = (mb_s_char)ping->png_raw3_rxamp[i];
			line[6] = (mb_u_char)ping->png_raw3_rxquality[i];
			line[7] = (mb_u_char)ping->png_raw3_rxwindow[i];
			mb_put_binary_short(swap, (short)ping->png_raw3_rxbeam_num[i], (void *)&line[8]);
			mb_put_binary_short(swap, (unsigned short)ping->png_raw3_rxspare[i], (void *)&line[10]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_RAWBEAM3_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_RAWBEAM3_BEAM_SIZE, mbfp);
			if (write_len != EM2_RAWBEAM3_BEAM_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
		write_len = fwrite(line, 1, 4, mbfp);
		if (write_len != 4) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_ss(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int head, int *error) {
	char line[EM2_SS_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		fprintf(stderr, "dbg2       head:       %d\n", head);
	}

	/* get storage structure */
	struct mbsys_simrad2_ping_struct *ping;
	if (store->sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2 && head == 1)
		ping = (struct mbsys_simrad2_ping_struct *)store->ping2;
	else
		ping = (struct mbsys_simrad2_ping_struct *)store->ping;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       type:            %d\n", store->type);
		fprintf(stderr, "dbg5       sonar:           %d\n", store->sonar);
		fprintf(stderr, "dbg5       date:            %d\n", store->date);
		fprintf(stderr, "dbg5       msec:            %d\n", store->msec);
		fprintf(stderr, "dbg5       png_ss_date:     %d\n", ping->png_ss_date);
		fprintf(stderr, "dbg5       png_ss_msec:     %d\n", ping->png_ss_msec);
		fprintf(stderr, "dbg5       png_ss_count:    %d\n", ping->png_ss_count);
		fprintf(stderr, "dbg5       png_ss_serial    %d\n", ping->png_ss_serial);
		fprintf(stderr, "dbg5       png_max_range:   %d\n", ping->png_max_range);
		fprintf(stderr, "dbg5       png_r_zero:      %d\n", ping->png_r_zero);
		fprintf(stderr, "dbg5       png_r_zero_corr: %d\n", ping->png_r_zero_corr);
		fprintf(stderr, "dbg5       png_tvg_start:   %d\n", ping->png_tvg_start);
		fprintf(stderr, "dbg5       png_tvg_stop:    %d\n", ping->png_tvg_stop);
		fprintf(stderr, "dbg5       png_bsn:         %d\n", ping->png_bsn);
		fprintf(stderr, "dbg5       png_bso:         %d\n", ping->png_bso);
		fprintf(stderr, "dbg5       png_tx:          %d\n", ping->png_tx);
		fprintf(stderr, "dbg5       png_tvg_crossover: %d\n", ping->png_tvg_crossover);
		fprintf(stderr, "dbg5       png_nbeams_ss:     %d\n", ping->png_nbeams_ss);
		fprintf(stderr, "dbg5       png_npixels:       %d\n", ping->png_npixels);
		fprintf(stderr, "dbg5       cnt  index sort samples start center\n");
		fprintf(stderr, "dbg5       --------------------------------------------------\n");
		for (int i = 0; i < ping->png_nbeams_ss; i++)
			fprintf(stderr, "dbg5        %4d %3d %2d %4d %4d %4d\n", i, ping->png_beam_index[i], ping->png_sort_direction[i],
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
	    swap,
	    (int)(EM2_SS_HEADER_SIZE + EM2_SS_BEAM_SIZE * ping->png_nbeams_ss + ping->png_npixels - (ping->png_npixels % 2) + 8),
	    (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_SS), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		mb_put_binary_short(swap, (unsigned short)ping->png_max_range, (void *)&line[12]);
		mb_put_binary_short(swap, (unsigned short)ping->png_r_zero, (void *)&line[14]);
		mb_put_binary_short(swap, (unsigned short)ping->png_r_zero_corr, (void *)&line[16]);
		mb_put_binary_short(swap, (unsigned short)ping->png_tvg_start, (void *)&line[18]);
		mb_put_binary_short(swap, (unsigned short)ping->png_tvg_stop, (void *)&line[20]);
		line[22] = (mb_s_char)ping->png_bsn;
		line[23] = (mb_s_char)ping->png_bso;
		mb_put_binary_short(swap, (unsigned short)ping->png_tx, (void *)&line[24]);
		line[26] = (mb_u_char)ping->png_tvg_crossover;
		line[27] = (mb_u_char)ping->png_nbeams_ss;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)line;
		for (int j = 0; j < EM2_SS_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_SS_HEADER_SIZE, mbfp);
		if (write_len != EM2_SS_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output binary beam data */
	if (status == MB_SUCCESS)
		for (int i = 0; i < ping->png_nbeams_ss; i++) {
			line[0] = (mb_u_char)ping->png_beam_index[i];
			line[1] = (mb_s_char)ping->png_sort_direction[i];
			mb_put_binary_short(swap, (unsigned short)ping->png_beam_samples[i], (void *)&line[2]);
			mb_put_binary_short(swap, (unsigned short)ping->png_center_sample[i], (void *)&line[4]);

			/* compute checksum */
			uchar_ptr = (mb_u_char *)line;
			for (int j = 0; j < EM2_SS_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_SS_BEAM_SIZE, mbfp);
			if (write_len != EM2_SS_BEAM_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
		}

	/* output sidescan data */
	if (status == MB_SUCCESS) {
		write_size = ping->png_npixels + 1 - (ping->png_npixels % 2);
		if (ping->png_npixels % 2 == 0)
			ping->png_ssraw[ping->png_npixels] = 0;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)ping->png_ssraw;
		for (unsigned int j = 0; j < write_size; j++)
			checksum += uchar_ptr[j];

		write_len = fwrite(ping->png_ssraw, 1, write_size, mbfp);
		if (write_len != write_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
	}

	/* output end of record */
	if (status == MB_SUCCESS) {
		line[1] = 0x03;

		/* set checksum */
		mb_put_binary_short(swap, (unsigned short)checksum, (void *)&line[2]);

		/* write out data */
		write_len = fwrite(&line[1], 1, 3, mbfp);
		if (write_len != 3) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_wc(int verbose, FILE *mbfp, int swap, struct mbsys_simrad2_struct *store, int *error) {
	char line[EM2_WC_HEADER_SIZE];
	short label;
	size_t write_len;
	size_t write_size;
	unsigned short checksum;
	mb_u_char *uchar_ptr;
	size_t record_size;
	size_t pad;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbfp:       %p\n", (void *)mbfp);
		fprintf(stderr, "dbg2       swap:       %d\n", swap);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* get storage structure */
	struct mbsys_simrad2_watercolumn_struct *wc = (struct mbsys_simrad2_watercolumn_struct *)store->wc;

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
	record_size = EM2_WC_HEADER_SIZE + EM2_WC_BEAM_SIZE * wc->wtc_nbeam + EM2_WC_TX_SIZE * wc->wtc_ntx + 8;
	for (int i = 0; i < wc->wtc_nbeam; i++) {
		record_size += wc->beam[i].wtc_beam_samples;
	}
	pad = (record_size % 2);
	record_size += pad;
	mb_put_binary_int(swap, record_size, (void *)&write_size);
	write_len = fwrite(&write_size, 1, 4, mbfp);
	int status = MB_SUCCESS;
	if (write_len != 4) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(EM2_WATERCOLUMN), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

		/* compute checksum */
		uchar_ptr = (mb_u_char *)&label;
		checksum += uchar_ptr[1];
	}

	if (status == MB_SUCCESS) {
		mb_put_binary_short(swap, (short)(store->sonar), (void *)&label);
		write_len = fwrite(&label, 1, 2, mbfp);
		if (write_len != 2) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		else
			status = MB_SUCCESS;

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
		for (int j = 0; j < EM2_WC_HEADER_SIZE; j++)
			checksum += uchar_ptr[j];

		/* write out data */
		write_len = fwrite(line, 1, EM2_WC_HEADER_SIZE, mbfp);
		if (write_len != EM2_WC_HEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
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
			for (int j = 0; j < EM2_WC_TX_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_WC_TX_SIZE, mbfp);
			if (write_len != EM2_WC_TX_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
			for (int j = 0; j < EM2_WC_BEAM_SIZE; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(line, 1, EM2_WC_BEAM_SIZE, mbfp);
			if (write_len != EM2_WC_BEAM_SIZE) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}

			/* compute checksum */
			uchar_ptr = (mb_u_char *)wc->beam[i].wtc_amp;
			for (int j = 0; j < wc->beam[i].wtc_beam_samples; j++)
				checksum += uchar_ptr[j];

			/* write out data */
			write_len = fwrite(wc->beam[i].wtc_amp, 1, wc->beam[i].wtc_beam_samples, mbfp);
			if (write_len != (size_t) wc->beam[i].wtc_beam_samples) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
			else {
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}
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
		write_len = fwrite(&line[!pad], 1, 3 + pad, mbfp);
		if (write_len != 3 + pad) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
		else {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
int mbr_em300raw_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;
	struct mbsys_simrad2_ping_struct *ping = (struct mbsys_simrad2_ping_struct *)store->ping;
	FILE *mbfp = mb_io_ptr->mbfp;

#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr, "\nstart of mbr_em300raw_wr_data:\n");
	fprintf(stderr, "kind:%d %d type:%x\n", store->kind, mb_io_ptr->new_kind, store->type);
#endif

	/* set swap flag */
	const bool swap = false;

	int status = MB_SUCCESS;

	if (store->kind == MB_DATA_COMMENT || store->kind == MB_DATA_START || store->kind == MB_DATA_STOP) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_start kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_start(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_RUN_PARAMETER) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_run_parameter kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_run_parameter(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_CLOCK) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_clock kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_clock(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_TIDE) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_tide kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_tide(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_HEIGHT) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_height kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_height(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_HEADING) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_heading kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_heading(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_SSV) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_ssv kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_ssv(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_TILT) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_tilt kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_tilt(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_PARAMETER) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_extraparameters kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_extraparameters(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_ATTITUDE) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_attitude kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_attitude(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_NAV || store->kind == MB_DATA_NAV1 || store->kind == MB_DATA_NAV2 ||
	         store->kind == MB_DATA_NAV3) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_pos kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_pos(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_VELOCITY_PROFILE) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_svp kind:%d type %x\n", store->kind, store->type);
#endif
		if (store->type == EM2_SVP)
			status = mbr_em300raw_wr_svp(verbose, mbfp, swap, store, error);
		else
			status = mbr_em300raw_wr_svp2(verbose, mbfp, swap, store, error);
	}
	else if (store->kind == MB_DATA_DATA) {
/* write out data from first head for all sonars */
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_bath kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_bath(verbose, mbfp, swap, store, 0, error);
		if (ping->png_raw1_read) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_wr_rawbeam kind:%d type %x\n", store->kind, store->type);
#endif
			status = mbr_em300raw_wr_rawbeam(verbose, mbfp, swap, store, error);
		}
		if (ping->png_raw2_read) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_wr_rawbeam2 kind:%d type %x\n", store->kind, store->type);
#endif
			status = mbr_em300raw_wr_rawbeam2(verbose, mbfp, swap, store, error);
		}
		if (ping->png_raw3_read) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_wr_rawbeam3 kind:%d type %x\n", store->kind, store->type);
#endif
			status = mbr_em300raw_wr_rawbeam3(verbose, mbfp, swap, store, 0, error);
		}
#ifdef MBR_EM300RAW_DEBUG
		if (!ping->png_raw1_read && !ping->png_raw2_read && !ping->png_raw3_read)
			fprintf(stderr, "NOT call mbr_em300raw_wr_rawbeam kind:%d type %x\n", store->kind, store->type);
#endif
		if (ping->png_ss_read) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_wr_ss kind:%d type %x\n", store->kind, store->type);
#endif
			status = mbr_em300raw_wr_ss(verbose, mbfp, swap, store, 0, error);
		}
#ifdef MBR_EM300RAW_DEBUG
		else
			fprintf(stderr, "NOT call mbr_em300raw_wr_ss kind:%d type %x\n", store->kind, store->type);
#endif

		/* write out data from second head for EM2002 */
		if (store->sonar == MBSYS_SIMRAD2_EM3002 && store->numberheads == 2 && store->ping2 != NULL &&
		    store->ping2->png_count == store->ping->png_count) {
#ifdef MBR_EM300RAW_DEBUG
			fprintf(stderr, "call mbr_em300raw_wr_bath kind:%d type %x\n", store->kind, store->type);
#endif
			status = mbr_em300raw_wr_bath(verbose, mbfp, swap, store, 1, error);
			if (ping->png_raw3_read) {
#ifdef MBR_EM300RAW_DEBUG
				fprintf(stderr, "call mbr_em300raw_wr_rawbeam3 kind:%d type %x\n", store->kind, store->type);
#endif
				status = mbr_em300raw_wr_rawbeam3(verbose, mbfp, swap, store, 1, error);
			}
#ifdef MBR_EM300RAW_DEBUG
			if (!ping->png_raw3_read)
				fprintf(stderr, "NOT call mbr_em300raw_wr_rawbeam kind:%d type %x\n", store->kind, store->type);
#endif
			if (ping->png_ss_read) {
#ifdef MBR_EM300RAW_DEBUG
				fprintf(stderr, "call mbr_em300raw_wr_ss kind:%d type %x\n", store->kind, store->type);
#endif
				status = mbr_em300raw_wr_ss(verbose, mbfp, swap, store, 1, error);
			}
#ifdef MBR_EM300RAW_DEBUG
			else
				fprintf(stderr, "NOT call mbr_em300raw_wr_ss kind:%d type %x\n", store->kind, store->type);
#endif
		}
	}
	else if (store->kind == MB_DATA_WATER_COLUMN) {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call mbr_em300raw_wr_wc kind:%d type %x\n", store->kind, store->type);
#endif
		status = mbr_em300raw_wr_wc(verbose, mbfp, swap, store, error);
	}
	else {
#ifdef MBR_EM300RAW_DEBUG
		fprintf(stderr, "call nothing bad kind: %d type %x\n", store->kind, store->type);
#endif
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
	}

#ifdef MBR_EM300RAW_DEBUG
	fprintf(stderr, "status:%d error:%d\n", status, *error);
	fprintf(stderr, "end of mbr_em300raw_wr_data:\n");
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
int mbr_wt_em300raw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_simrad2_struct *store = (struct mbsys_simrad2_struct *)store_ptr;

	/* write next data to file */
	const int status = mbr_em300raw_wr_data(verbose, mb_io_ptr, store, error);

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
int mbr_register_em300raw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_em300raw(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_em300raw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_em300raw;
	mb_io_ptr->mb_io_store_alloc = &mbsys_simrad2_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_simrad2_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_em300raw;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_em300raw;
	mb_io_ptr->mb_io_dimensions = &mbsys_simrad2_dimensions;
	mb_io_ptr->mb_io_pingnumber = &mbsys_simrad2_pingnumber;
	mb_io_ptr->mb_io_sonartype = &mbsys_simrad2_sonartype;
	mb_io_ptr->mb_io_sidescantype = &mbsys_simrad2_sidescantype;
	mb_io_ptr->mb_io_preprocess = &mbsys_simrad2_preprocess;
	mb_io_ptr->mb_io_extract_platform = &mbsys_simrad2_extract_platform;
	mb_io_ptr->mb_io_extract = &mbsys_simrad2_extract;
	mb_io_ptr->mb_io_insert = &mbsys_simrad2_insert;
	mb_io_ptr->mb_io_extract_nnav = &mbsys_simrad2_extract_nnav;
	mb_io_ptr->mb_io_extract_nav = &mbsys_simrad2_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_simrad2_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_simrad2_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_simrad2_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_simrad2_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_simrad2_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_simrad2_detects;
	mb_io_ptr->mb_io_pulses = &mbsys_simrad2_pulses;
	mb_io_ptr->mb_io_gains = &mbsys_simrad2_gains;
	mb_io_ptr->mb_io_copyrecord = &mbsys_simrad2_copy;
  mb_io_ptr->mb_io_makess = &mbsys_simrad2_makess;
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
