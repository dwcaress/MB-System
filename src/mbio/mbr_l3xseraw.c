/*--------------------------------------------------------------------
 *    The MB-system:	mbr_l3xseraw.c	3/27/2000
 *
 *    Copyright (c) 2000-2023 by
 *    D. W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_sb2102xs.c contains the functions for reading and writing
 * multibeam data in the L3XSERAW format.
 * These functions include:
 *   mbr_alm_l3xseraw	- allocate read/write memory
 *   mbr_dem_l3xseraw	- deallocate read/write memory
 *   mbr_rt_l3xseraw	- read and translate data
 *   mbr_wt_l3xseraw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 27, 1999
 * Additional Authors:	P. A. Cohen and S. Dzurenko
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
#include "mbsys_xse.h"

/* #define MB_DEBUG 1 */
/* #define MB_DEBUG2 1 */

/* set up byte swapping scenario */
#ifdef DATAINPCBYTEORDER
#define SWAPFLAG true
#else
#define SWAPFLAG false
#endif

/*--------------------------------------------------------------------*/
int mbr_info_l3xseraw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_XSE;
	*beams_bath_max = 151;
	*beams_amp_max = 151;
	*pixels_ss_max = 2000;
	strncpy(format_name, "L3XSERAW", MB_NAME_LENGTH);
	strncpy(system_name, "XSE", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_L3XSERAW\nInformal Description: ELAC/SeaBeam XSE vendor format\nAttributes:           "
	        "Bottomchart MkII 50 kHz and 180 kHz multibeam, \n                      SeaBeam 2120 20 KHz multibeam,\n             "
	        "         bathymetry, amplitude and sidescan,\n                      variable beams and pixels, binary, \n           "
	        "           L3 Communications (Elac Nautik \n                      and SeaBeam Instruments).\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = true;
	*traveltime = true;
	*beam_flagging = true;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 2.0;
	*beamwidth_ltrack = 2.0;

	int status = MB_SUCCESS;

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
int mbr_alm_l3xseraw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, MBSYS_XSE_BUFFER_SIZE, (void **)&mb_io_ptr->hdr_comment, error);
	mbsys_xse_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  char *label = (char *) mb_io_ptr->save_label;
	int *frame_expect = (int *)&mb_io_ptr->save1;
	bool *frame_save = (bool *)&mb_io_ptr->saveb1;
	int *frame_id_save = (int *)&mb_io_ptr->save3;
	int *frame_source_save = (int *)&mb_io_ptr->save4;
	int *frame_sec_save = (int *)&mb_io_ptr->save5;
	int *frame_usec_save = (int *)&mb_io_ptr->save6;
	int *buffer_size_save = (int *)&mb_io_ptr->save7;
	int *buffer_size_max = (int *)&mb_io_ptr->save8;
	//char *buffer = mb_io_ptr->hdr_comment;
  memset((void *)label, 0, 12);
  *frame_expect = MBSYS_XSE_NONE_FRAME;
  *frame_save = false;
  *frame_id_save = MBSYS_XSE_NONE_FRAME;
  *frame_source_save = 0;
  *frame_sec_save = 0;
  *frame_usec_save = 0;
  *buffer_size_save = 0;
  *buffer_size_max = 0;

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
int mbr_dem_l3xseraw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->store_data, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->hdr_comment, error);

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
int mbr_l3xseraw_rd_svp(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	int byte_count = 0;
	int group_id = 0;
	int index = 0;
#ifdef MB_DEBUG
	int skip = 0;
#endif

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->svp_source);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->svp_sec);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->svp_usec);
	index += 4;

	const int status = MB_SUCCESS;

	/* loop over groups */
	bool done = false;
	while (index <= buffer_size && status == MB_SUCCESS && !done) {
		/* look for group start or frame end */
#ifdef MB_DEBUG
		skip = 0;
#endif
#ifdef DATAINPCBYTEORDER
		while (index < buffer_size && strncmp(&buffer[index], "GSH$", 4) && strncmp(&buffer[index], "FSH#", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "FSH#", 4))
			done = true;
		else
			index += 4;
#else
		while (index < buffer_size && strncmp(&buffer[index], "$HSG", 4) && strncmp(&buffer[index], "#HSF", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "#HSF", 4))
			done = true;
		else
			index += 4;
#endif

#ifdef MB_DEBUG
		if (skip > 4)
			fprintf(stderr, "%s:%d | skipped %d bytes in function <%s>\n", __FILE__, __LINE__, skip - 4, __func__);
#endif

		/* deal with group */
		if (!done) {
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&byte_count);
			index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&group_id);
			index += 4;

			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n", group_id, byte_count,
				        __func__);
			}
#ifdef MB_DEBUG
			fprintf(stderr, "%s:%d | Group %d of %d bytes to be parsed in MBIO function <%s>\n", __FILE__, __LINE__, group_id,
			        byte_count, __func__);
#endif

			/* handle general group */
			if (group_id == MBSYS_XSE_SVP_GROUP_GEN) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_GEN\n", __FILE__, __LINE__);
#endif
				/* currently unused by MB-System */
			}

			/* handle depth group */
			if (group_id == MBSYS_XSE_SVP_GROUP_DEPTH) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_DEPTH\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->svp_nsvp);
				index += 4;
				for (int i = 0; i < store->svp_nsvp; i++) {
					if (i < MBSYS_XSE_MAXSVP) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_depth[i]);
						index += 8;
					}
				}
			}

			/* handle velocity group */
			else if (group_id == MBSYS_XSE_SVP_GROUP_VELOCITY) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_VELOCITY\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->svp_nsvp);
				index += 4;
				for (int i = 0; i < store->svp_nsvp; i++) {
					if (i < MBSYS_XSE_MAXSVP) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_velocity[i]);
						index += 8;
					}
				}
			}

			/* handle conductivity group */
			else if (group_id == MBSYS_XSE_SVP_GROUP_CONDUCTIVITY) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_CONDUCTIVITY\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->svp_nctd);
				index += 4;
				for (int i = 0; i < store->svp_nctd; i++) {
					if (i < MBSYS_XSE_MAXSVP) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_conductivity[i]);
						index += 8;
					}
				}
			}

			/* handle salinity group */
			else if (group_id == MBSYS_XSE_SVP_GROUP_SALINITY) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_SALINITY\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->svp_nctd);
				index += 4;
				for (int i = 0; i < store->svp_nctd; i++) {
					if (i < MBSYS_XSE_MAXSVP) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_salinity[i]);
						index += 8;
					}
				}
			}

			/* handle temperature group */
			else if (group_id == MBSYS_XSE_SVP_GROUP_TEMP) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_TEMP\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->svp_nctd);
				index += 4;
				for (int i = 0; i < store->svp_nctd; i++) {
					if (i < MBSYS_XSE_MAXSVP) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_temperature[i]);
						index += 8;
					}
				}
			}

			/* handle pressure group */
			else if (group_id == MBSYS_XSE_SVP_GROUP_PRESSURE) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_PRESSURE\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->svp_nctd);
				index += 4;
				for (int i = 0; i < store->svp_nctd; i++) {
					if (i < MBSYS_XSE_MAXSVP) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_pressure[i]);
						index += 8;
					}
				}
			}

			/* handle ssv group */
			else if (group_id == MBSYS_XSE_SVP_GROUP_SSV) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_SSV\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_ssv);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_ssv_depth);
				index += 8;
				store->svp_ssv_depthflag = buffer[index];
				index++;
			}

			/* handle point group */
			else if (group_id == MBSYS_XSE_SVP_GROUP_POS) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_POS\n", __FILE__, __LINE__);
#endif
				/* currently unused by MB-System */
			}

			else {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP_GROUP_OTHER\n", __FILE__, __LINE__);
#endif
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       svp_source:          %d\n", store->svp_source);
		fprintf(stderr, "dbg5       svp_sec:             %u\n", store->svp_sec);
		fprintf(stderr, "dbg5       svp_usec:            %u\n", store->svp_usec);
		fprintf(stderr, "dbg5       svp_nsvp:            %d\n", store->svp_nsvp);
		fprintf(stderr, "dbg5       svp_nctd:            %d\n", store->svp_nctd);
		fprintf(stderr, "dbg5       svp_ssv:             %f\n", store->svp_ssv);
		for (int i = 0; i < store->svp_nsvp; i++)
			fprintf(stderr, "dbg5       svp[%d]:	        %f %f\n", i, store->svp_depth[i], store->svp_velocity[i]);
		for (int i = 0; i < store->svp_nctd; i++)
			fprintf(stderr, "dbg5       cstd[%d]:        %f %f %f %f\n", i, store->svp_conductivity[i], store->svp_salinity[i],
			        store->svp_temperature[i], store->svp_pressure[i]);
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
int mbr_l3xseraw_rd_tide(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* The tide frame is currently unused by MB-System */

	const int status = MB_SUCCESS;

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
int mbr_l3xseraw_rd_ship(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	int byte_count = 0;
	int group_id = 0;
	int index = 0;
#ifdef MB_DEBUG
	int skip = 0;
#endif
	int nchar = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->par_source);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->par_sec);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->par_usec);
	index += 4;

	const int status = MB_SUCCESS;

	/* loop over groups */
	bool done = false;
	while (index <= buffer_size && status == MB_SUCCESS && !done) {
		/* look for group start or frame end */
#ifdef MB_DEBUG
		skip = 0;
#endif
#ifdef DATAINPCBYTEORDER
		while (index < buffer_size && strncmp(&buffer[index], "GSH$", 4) && strncmp(&buffer[index], "FSH#", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "FSH#", 4))
			done = true;
		else
			index += 4;
#else
		while (index < buffer_size && strncmp(&buffer[index], "$HSG", 4) && strncmp(&buffer[index], "#HSF", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "#HSF", 4))
			done = true;
		else
			index += 4;
#endif

#ifdef MB_DEBUG
		if (skip > 4)
			fprintf(stderr, "%s:%d | skipped %d bytes in function <%s>\n", __FILE__, __LINE__, skip - 4, __func__);
#endif

		/* deal with group */
		if (!done) {
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&byte_count);
			index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&group_id);
			index += 4;

			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n", group_id, byte_count,
				        __func__);
			}
#ifdef MB_DEBUG
			fprintf(stderr, "%s:%d | Group %d of %d bytes to be parsed in MBIO function <%s>\n", __FILE__, __LINE__, group_id,
			        byte_count, __func__);
#endif

			/* handle general group */
			if (group_id == MBSYS_XSE_SHP_GROUP_GEN) {
				mb_get_binary_int(SWAPFLAG, &buffer[index], &nchar);
				index += 4;
				for (int i = 0; i < nchar; i++) {
					store->par_ship_name[i] = buffer[index];
					index++;
				}
				store->par_ship_name[nchar] = 0;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_length);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_beam);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_draft);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_height);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_displacement);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_weight);
				index += 8;
			}

			/* handle time group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_TIME) {
				/* currently unused by MB-System */
			}

			/* handle draft group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_DRAFT) {
				/* currently unused by MB-System */
			}

			/* handle sensors group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_SENSORS) {
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_ship_nsensor);
				index += 4;
				for (int i = 0; i < store->par_ship_nsensor; i++) {
					mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_ship_sensor_id[i]);
					index += 4;
				}
				for (int i = 0; i < store->par_ship_nsensor; i++) {
					mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_ship_sensor_type[i]);
					index += 4;
				}
				for (int i = 0; i < store->par_ship_nsensor; i++) {
					mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_ship_sensor_frequency[i]);
					index += 4;
				}
			}

			/* handle motion group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_MOTION) {
				/* currently unused by MB-System */
			}

			/* handle geometry group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_GEOMETRY) {
				/* currently unused by MB-System */
			}

			/* handle description group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_DESCRIPTION) {
				/* currently unused by MB-System */
			}

			/* handle parameter group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_PARAMETER) {
				store->par_parameter = true;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_roll_bias);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_pitch_bias);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_heading_bias);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_time_delay);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_x_port);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_y_port);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_z_port);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_x_stbd);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_y_stbd);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_z_stbd);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_err_port);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_err_stbd);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_nav_x);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_nav_y);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_nav_z);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_hrp_x);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_hrp_y);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_hrp_z);
				index += 4;
			}

			/* handle navigation and motion group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_NAVIGATIONANDMOTION) {
				store->par_navigationandmotion = true;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_roll_bias);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_pitch_bias);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_heave_bias);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_heading_bias);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_time_delay);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_nav_x);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_nav_y);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_nav_z);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_hrp_x);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_hrp_y);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_hrp_z);
				index += 8;
			}

			/* handle transducer group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_TRANSDUCER) {
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_xdr_num_transducer);
				index += 4;
				for (int i = 0; i < store->par_xdr_num_transducer; i++) {
					mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_xdr_sensorid[i]);
					index += 4;
					mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->par_xdr_frequency[i]);
					index += 4;
					store->par_xdr_transducer[i] = buffer[index];
					index++;
					store->par_xdr_side[i] = buffer[index];
					index++;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_mountingroll[i]);
					index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_mountingpitch[i]);
					index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_mountingazimuth[i]);
					index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_mountingdistance[i]);
					index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_x[i]);
					index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_y[i]);
					index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_z[i]);
					index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_roll[i]);
					index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_pitch[i]);
					index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_azimuth[i]);
					index += 8;
				}
			}

			/* handle transducer extended group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_TRANSDUCEREXTENDED) {
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_xdx_num_transducer);
				index += 4;
				for (int i = 0; i < store->par_xdx_num_transducer; i++) {
					store->par_xdx_roll[i] = buffer[index];
					index++;
					store->par_xdx_pitch[i] = buffer[index];
					index++;
					store->par_xdx_azimuth[i] = buffer[index];
					index++;
					index += 48;
				}
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       par_source:          %d\n", store->par_source);
		fprintf(stderr, "dbg5       par_sec:             %u\n", store->par_sec);
		fprintf(stderr, "dbg5       par_usec:            %u\n", store->par_usec);
		fprintf(stderr, "dbg5       par_ship_name:       %s\n", store->par_ship_name);
		fprintf(stderr, "dbg5       par_ship_length:     %f\n", store->par_ship_length);
		fprintf(stderr, "dbg5       par_ship_beam:       %f\n", store->par_ship_beam);
		fprintf(stderr, "dbg5       par_ship_draft:      %f\n", store->par_ship_draft);
		fprintf(stderr, "dbg5       par_ship_height:     %f\n", store->par_ship_height);
		fprintf(stderr, "dbg5       par_ship_displacement: %f\n", store->par_ship_displacement);
		fprintf(stderr, "dbg5       par_ship_weight:     %f\n", store->par_ship_weight);
		for (int i = 0; i < store->par_ship_nsensor; i++) {
			fprintf(stderr, "dbg5       par_ship_sensor_id[%d]:        %d\n", i, store->par_ship_sensor_id[i]);
			fprintf(stderr, "dbg5       par_ship_sensor_type[%d]:      %d\n", i, store->par_ship_sensor_type[i]);
			fprintf(stderr, "dbg5       par_ship_sensor_frequency[%d]: %d\n", i, store->par_ship_sensor_frequency[i]);
		}
		fprintf(stderr, "dbg5       par_parameter:       %d\n", store->par_parameter);
		fprintf(stderr, "dbg5       par_roll_bias:       %f\n", store->par_roll_bias);
		fprintf(stderr, "dbg5       par_pitch_bias:      %f\n", store->par_pitch_bias);
		fprintf(stderr, "dbg5       par_heading_bias:    %f\n", store->par_heading_bias);
		fprintf(stderr, "dbg5       par_time_delay:      %f\n", store->par_time_delay);
		fprintf(stderr, "dbg5       par_trans_x_port:    %f\n", store->par_trans_x_port);
		fprintf(stderr, "dbg5       par_trans_y_port:    %f\n", store->par_trans_y_port);
		fprintf(stderr, "dbg5       par_trans_z_port:    %f\n", store->par_trans_z_port);
		fprintf(stderr, "dbg5       par_trans_x_stbd:    %f\n", store->par_trans_x_stbd);
		fprintf(stderr, "dbg5       par_trans_y_stbd:    %f\n", store->par_trans_y_stbd);
		fprintf(stderr, "dbg5       par_trans_z_stbd:    %f\n", store->par_trans_z_stbd);
		fprintf(stderr, "dbg5       par_trans_err_port:  %f\n", store->par_trans_err_port);
		fprintf(stderr, "dbg5       par_trans_err_stbd:  %f\n", store->par_trans_err_stbd);
		fprintf(stderr, "dbg5       par_nav_x:           %f\n", store->par_nav_x);
		fprintf(stderr, "dbg5       par_nav_y:           %f\n", store->par_nav_y);
		fprintf(stderr, "dbg5       par_nav_z:           %f\n", store->par_nav_z);
		fprintf(stderr, "dbg5       par_hrp_x:           %f\n", store->par_hrp_x);
		fprintf(stderr, "dbg5       par_hrp_y:           %f\n", store->par_hrp_y);
		fprintf(stderr, "dbg5       par_hrp_z:           %f\n", store->par_hrp_z);
		fprintf(stderr, "dbg5       par_navigationandmotion: %d\n", store->par_navigationandmotion);
		fprintf(stderr, "dbg5       par_nam_roll_bias:       %f\n", store->par_nam_roll_bias);
		fprintf(stderr, "dbg5       par_nam_pitch_bias:      %f\n", store->par_nam_pitch_bias);
		fprintf(stderr, "dbg5       par_nam_heave_bias:      %f\n", store->par_nam_heave_bias);
		fprintf(stderr, "dbg5       par_nam_heading_bias:    %f\n", store->par_nam_heading_bias);
		fprintf(stderr, "dbg5       par_nam_time_delay:      %f\n", store->par_nam_time_delay);
		fprintf(stderr, "dbg5       par_nam_nav_x:           %f\n", store->par_nam_nav_x);
		fprintf(stderr, "dbg5       par_nam_nav_y:           %f\n", store->par_nam_nav_y);
		fprintf(stderr, "dbg5       par_nam_nav_z:           %f\n", store->par_nam_nav_z);
		fprintf(stderr, "dbg5       par_nam_hrp_x:           %f\n", store->par_nam_hrp_x);
		fprintf(stderr, "dbg5       par_nam_hrp_y:           %f\n", store->par_nam_hrp_y);
		fprintf(stderr, "dbg5       par_nam_hrp_z:           %f\n", store->par_nam_hrp_z);
		fprintf(stderr, "dbg5       par_xdr_num_transducer:  %d\n", store->par_xdr_num_transducer);
		fprintf(stderr, "dbg5       # sensor xducer freq side roll pitch azi dist\n");
		for (int i = 0; i < store->par_xdr_num_transducer; i++)
			fprintf(stderr, "dbg5       %d %d %d %d %d %f %f %f %f\n", i, store->par_xdr_sensorid[i],
			        store->par_xdr_transducer[i], store->par_xdr_frequency[i], store->par_xdr_side[i],
			        store->par_xdr_mountingroll[i], store->par_xdr_mountingpitch[i], store->par_xdr_mountingazimuth[i],
			        store->par_xdr_mountingdistance[i]);
		fprintf(stderr, "dbg5       # x y z roll pitch azimuth\n");
		for (int i = 0; i < store->par_xdr_num_transducer; i++)
			fprintf(stderr, "dbg5       %d %f %f %f %f %f %f\n", i, store->par_xdr_x[i], store->par_xdr_y[i], store->par_xdr_z[i],
			        store->par_xdr_roll[i], store->par_xdr_pitch[i], store->par_xdr_azimuth[i]);
		fprintf(stderr, "dbg5       par_xdx_num_transducer:  %d\n", store->par_xdx_num_transducer);
		fprintf(stderr, "dbg5       # roll pitch azimuth\n");
		for (int i = 0; i < store->par_xdx_num_transducer; i++)
			fprintf(stderr, "dbg5       %d %d %d %d\n", i, store->par_xdx_roll[i], store->par_xdx_pitch[i],
			        store->par_xdx_azimuth[i]);
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
int mbr_l3xseraw_rd_sidescan(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	int byte_count = 0;
	int group_id = 0;
	int index = 0;
	int ngoodss;
	double xmin, xmax, binsize;
#ifdef MB_DEBUG
	int skip = 0;
#endif

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_source);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_sec);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_usec);
	index += 4;

	const int status = MB_SUCCESS;

	/* loop over groups */
	bool done = false;
	while (index <= buffer_size && status == MB_SUCCESS && !done) {
		/* look for group start or frame end */
#ifdef MB_DEBUG
		skip = 0;
#endif
#ifdef DATAINPCBYTEORDER
		while (index < buffer_size && strncmp(&buffer[index], "GSH$", 4) && strncmp(&buffer[index], "FSH#", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "FSH#", 4))
			done = true;
		else
			index += 4;
#else
		while (index < buffer_size && strncmp(&buffer[index], "$HSG", 4) && strncmp(&buffer[index], "#HSF", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "#HSF", 4))
			done = true;
		else
			index += 4;
#endif

#ifdef MB_DEBUG
		if (skip > 4)
			fprintf(stderr, "%s:%d | skipped %d bytes in function <%s>\n", __FILE__, __LINE__, skip - 4, __func__);
#endif

		/* deal with group */
		if (!done) {
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&byte_count);
			index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&group_id);
			index += 4;

			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n", group_id, byte_count,
				        __func__);
			}
#ifdef MB_DEBUG
			fprintf(stderr, "%s:%d | Group %d of %d bytes to be parsed in MBIO function <%s>\n", __FILE__, __LINE__, group_id,
			        byte_count, __func__);
#endif

			/* handle general group */
			if (group_id == MBSYS_XSE_SSN_GROUP_GEN) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SSN_GROUP_GEN\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_ping);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_frequency);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_pulse);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_power);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_bandwidth);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_sample);
				index += 4;
#ifdef MB_DEBUG2
				fprintf(stderr, "ping=%u\n", store->sid_ping);
				fprintf(stderr, "frequency=%g\n", store->sid_frequency);
				fprintf(stderr, "pulse=%g\n", store->sid_pulse);
				fprintf(stderr, "power=%g\n", store->sid_power);
				fprintf(stderr, "bandwidth=%g\n", store->sid_bandwidth);
				fprintf(stderr, "sample=%g\n", store->sid_sample);
#endif
			}

			/* handle amplitude vs traveltime group */
			else if (group_id == MBSYS_XSE_SSN_GROUP_AMPVSTT) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SSN_GROUP_AMPVSTT\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_avt_sampleus);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->sid_avt_offset);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->sid_avt_num_samples);
				index += 4;
				for (int i = 0; i < store->sid_avt_num_samples; i++)
					if (i < MBSYS_XSE_MAXPIXELS) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_avt_amp[i]);
						index += 2;
					}
				store->sid_group_avt = true;
#ifdef MB_DEBUG2
				fprintf(stderr, "sid_avt_sampleus=%d\n", store->sid_avt_sampleus);
				fprintf(stderr, "sid_avt_offset=%d\n", store->sid_avt_offset);
				fprintf(stderr, "sid_avt_num_samples=%d\n", store->sid_avt_num_samples);
				for (int i = 0; i < store->sid_avt_num_samples; i++)
					fprintf(stderr, "sid_avt_amp[%d]:%d\n", i, store->sid_avt_amp[i]);
#endif
			}

			/* handle phase vs traveltime group */
			else if (group_id == MBSYS_XSE_SSN_GROUP_PHASEVSTT) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SSN_GROUP_PHASEVSTT\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_pvt_sampleus);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->sid_pvt_offset);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->sid_pvt_num_samples);
				index += 4;
				for (int i = 0; i < store->sid_pvt_num_samples; i++)
					if (i < MBSYS_XSE_MAXPIXELS) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_pvt_phase[i]);
						index += 2;
					}
				store->sid_group_pvt = true;
#ifdef MB_DEBUG2
				fprintf(stderr, "sid_pvt_sampleus=%d\n", store->sid_pvt_sampleus);
				fprintf(stderr, "sid_pvt_offset=%d\n", store->sid_pvt_offset);
				fprintf(stderr, "sid_pvt_num_samples=%d\n", store->sid_pvt_num_samples);
				for (int i = 0; i < store->sid_pvt_num_samples; i++)
					fprintf(stderr, "sid_pvt_phase[%d]:%d\n", i, store->sid_pvt_phase[i]);
#endif
			}

			/* handle amplitude vs lateral group */
			else if (group_id == MBSYS_XSE_SSN_GROUP_AMPVSLAT) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SSN_GROUP_AMPVSLAT\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_avl_binsize);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_avl_offset);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_avl_num_samples);
				index += 4;
				for (int i = 0; i < store->sid_avl_num_samples; i++)
					if (i < MBSYS_XSE_MAXPIXELS) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_avl_amp[i]);
						index += 2;
					}
				store->sid_group_avl = true;
#ifdef MB_DEBUG2
				fprintf(stderr, "sid_avl_binsize=%d\n", store->sid_avl_binsize);
				fprintf(stderr, "sid_avl_offset=%d\n", store->sid_avl_offset);
				fprintf(stderr, "sid_avl_num_samples=%d\n", store->sid_avl_num_samples);
				for (int i = 0; i < store->sid_avl_num_samples; i++)
					fprintf(stderr, "sid_avl_amp[%d]:%d\n", i, store->sid_avl_amp[i]);
#endif
			}

			else if (group_id == MBSYS_XSE_SSN_GROUP_PHASEVSLAT) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SSN_GROUP_PHASEVSLAT\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_pvl_binsize);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_pvl_offset);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_pvl_num_samples);
				index += 4;
				for (int i = 0; i < store->sid_pvl_num_samples; i++)
					if (i < MBSYS_XSE_MAXPIXELS) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_pvl_phase[i]);
						index += 2;
					}
				store->sid_group_pvl = true;
#ifdef MB_DEBUG2
				fprintf(stderr, "sid_pvl_binsize=%d\n", store->sid_pvl_binsize);
				fprintf(stderr, "sid_pvl_offset=%d\n", store->sid_pvl_offset);
				fprintf(stderr, "sid_pvl_num_samples=%d\n", store->sid_pvl_num_samples);
				for (int i = 0; i < store->sid_pvl_num_samples; i++)
					fprintf(stderr, "sid_pvl_phase[%d]:%d\n", i, store->sid_pvl_phase[i]);
#endif
			}

			else if (group_id == MBSYS_XSE_SSN_GROUP_SIGNAL) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBSYS_XSE_SSN_GROUP_SIGNAL\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_sig_ping);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_sig_channel);
				index += 4;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->sid_sig_offset);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->sid_sig_sample);
				index += 8;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_sig_num_samples);
				index += 4;
				for (int i = 0; i < store->sid_sig_num_samples; i++)
					if (i < MBSYS_XSE_MAXPIXELS) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_sig_phase[i]);
						index += 2;
					}
				store->sid_group_signal = true;
#ifdef MB_DEBUG2
				fprintf(stderr, "sid_sig_ping=%d\n", store->sid_sig_ping);
				fprintf(stderr, "sid_sig_channel=%d\n", store->sid_sig_channel);
				fprintf(stderr, "sid_sig_offset=%g\n", store->sid_sig_offset);
				fprintf(stderr, "sid_sig_sample=%g\n", store->sid_sig_sample);
				fprintf(stderr, "sid_sig_num_samples=%d\n", store->sid_sig_num_samples);
				for (int i = 0; i < store->sid_sig_num_samples; i++)
					fprintf(stderr, "sid_sig_phase[%d]:%d\n", i, store->sid_sig_phase[i]);
#endif
			}

			else if (group_id == MBSYS_XSE_SSN_GROUP_PINGTYPE) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBSYS_XSE_SSN_GROUP_PINGTYPE\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_png_pulse);
				index += 4;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->sid_png_startfrequency);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->sid_png_endfrequency);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->sid_png_duration);
				index += 8;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_png_mancode);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_png_pulseid);
				index += 4;
				for (int i = 0; i < byte_count - 40; i++) {
					store->sid_png_pulsename[i] = buffer[index];
					index++;
				}
				store->sid_group_ping = true;
#ifdef MB_DEBUG2
				fprintf(stderr, "sid_png_pulse=%d\n", store->sid_png_pulse);
				fprintf(stderr, "sid_png_startfrequency=%f\n", store->sid_png_startfrequency);
				fprintf(stderr, "sid_png_endfrequency=%f\n", store->sid_png_endfrequency);
				fprintf(stderr, "sid_png_duration=%f\n", store->sid_png_duration);
				fprintf(stderr, "sid_png_mancode=%d\n", store->sid_png_mancode);
				fprintf(stderr, "sid_png_pulseid=%d\n", store->sid_png_pulseid);
				fprintf(stderr, "sid_png_pulsename=%s\n", store->sid_png_pulsename);
#endif
			}

			else if (group_id == MBSYS_XSE_SSN_GROUP_COMPLEXSIGNAL) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBSYS_XSE_SSN_GROUP_COMPLEXSIGNAL\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_cmp_ping);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_cmp_channel);
				index += 4;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->sid_cmp_offset);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->sid_cmp_sample);
				index += 8;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_cmp_num_samples);
				index += 4;
				for (int i = 0; i < store->sid_cmp_num_samples; i++)
					if (i < MBSYS_XSE_MAXPIXELS) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_cmp_real[i]);
						index += 2;
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_cmp_imaginary[i]);
						index += 2;
					}
				store->sid_group_complex = true;
#ifdef MB_DEBUG2
				fprintf(stderr, "sid_cmp_ping=%d\n", store->sid_cmp_ping);
				fprintf(stderr, "sid_cmp_channel=%d\n", store->sid_cmp_channel);
				fprintf(stderr, "sid_cmp_offset=%f\n", store->sid_cmp_offset);
				fprintf(stderr, "sid_cmp_sample=%f\n", store->sid_cmp_sample);
				fprintf(stderr, "sid_cmp_num_samples=%d\n", store->sid_cmp_num_samples);
				for (int i = 0; i < store->sid_cmp_num_samples; i++)
					fprintf(stderr, "sid_cmp_real[%d]:%d sid_cmp_imaginary[%d]:%d\n", i, store->sid_cmp_real[i], i,
					        store->sid_cmp_imaginary[i]);
#endif
			}

			else if (group_id == MBSYS_XSE_SSN_GROUP_WEIGHTING) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBSYS_XSE_SSN_GROUP_WEIGHTING\n", __FILE__, __LINE__);
#endif
				mb_get_binary_short(SWAPFLAG, &buffer[index], (short *)&store->sid_wgt_factorleft);
				index += 2;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_wgt_samplesleft);
				index += 4;
				mb_get_binary_short(SWAPFLAG, &buffer[index], (short *)&store->sid_wgt_factorright);
				index += 2;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sid_wgt_samplesright);
				index += 4;
				store->sid_group_weighting = true;
#ifdef MB_DEBUG2
				fprintf(stderr, "sid_wgt_factorleft=%d\n", store->sid_wgt_factorleft);
				fprintf(stderr, "sid_wgt_samplesleft=%d\n", store->sid_wgt_samplesleft);
				fprintf(stderr, "sid_wgt_factorright=%d\n", store->sid_wgt_factorright);
				fprintf(stderr, "sid_wgt_samplesright=%d\n", store->sid_wgt_samplesright);
#endif
			}

			else {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SSN_GROUP_OTHER\n", __FILE__, __LINE__);
#endif
			}
		}
	}

	/* now if multibeam already read but bin size lacking then
	   calculate bin size from bathymetry */
	if (store->mul_frame && store->mul_num_beams > 1 && store->sid_avl_num_samples > 1 && store->sid_avl_binsize <= 0) {
		/* get width of bathymetry swath size */
		xmin = 9999999.9;
		xmax = -9999999.9;
		for (int i = 0; i < store->mul_num_beams; i++) {
			xmin = MIN(xmin, store->beams[i].lateral);
			xmax = MAX(xmax, store->beams[i].lateral);
		}

		/* get number of nonzero pixels */
		ngoodss = 0;
		for (int i = 0; i < store->sid_avl_num_samples; i++)
			if (store->sid_avl_amp[i] != 0)
				ngoodss++;

		/* get bin size */
		if (xmax > xmin && ngoodss > 1) {
			binsize = (xmax - xmin) / (ngoodss - 1);
			store->sid_avl_binsize = 1000 * binsize;
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sid_frame:            %d\n", store->sid_frame);
		fprintf(stderr, "dbg5       sid_group_avt:        %d\n", store->sid_group_avt);
		fprintf(stderr, "dbg5       sid_group_pvt:        %d\n", store->sid_group_pvt);
		fprintf(stderr, "dbg5       sid_group_avl:        %d\n", store->sid_group_avl);
		fprintf(stderr, "dbg5       sid_group_pvl:        %d\n", store->sid_group_pvl);
		fprintf(stderr, "dbg5       sid_group_signal:     %d\n", store->sid_group_signal);
		fprintf(stderr, "dbg5       sid_group_ping:       %d\n", store->sid_group_ping);
		fprintf(stderr, "dbg5       sid_group_complex:    %d\n", store->sid_group_complex);
		fprintf(stderr, "dbg5       sid_group_weighting:  %d\n", store->sid_group_weighting);
		fprintf(stderr, "dbg5       sid_source:           %d\n", store->sid_source);
		fprintf(stderr, "dbg5       sid_sec:              %d\n", store->sid_sec);
		fprintf(stderr, "dbg5       sid_usec:             %u\n", store->sid_usec);
		fprintf(stderr, "dbg5       sid_ping:             %u\n", store->sid_ping);
		fprintf(stderr, "dbg5       sid_frequency:        %f\n", store->sid_frequency);
		fprintf(stderr, "dbg5       sid_pulse:            %f\n", store->sid_pulse);
		fprintf(stderr, "dbg5       sid_power:            %f\n", store->sid_power);
		fprintf(stderr, "dbg5       sid_bandwidth:        %f\n", store->sid_bandwidth);
		fprintf(stderr, "dbg5       sid_sample:           %f\n", store->sid_sample);
		fprintf(stderr, "dbg5       sid_avt_sampleus:     %d\n", store->sid_avt_sampleus);
		fprintf(stderr, "dbg5       sid_avt_offset:       %d\n", store->sid_avt_offset);
		fprintf(stderr, "dbg5       sid_avt_num_samples:  %d\n", store->sid_avt_num_samples);
		for (int i = 0; i < store->sid_avt_num_samples; i++)
			fprintf(stderr, "dbg5       sid_avt_amp[%d]:%d\n", i, store->sid_avt_amp[i]);
		fprintf(stderr, "dbg5       sid_pvt_sampleus:  %d\n", store->sid_pvt_sampleus);
		fprintf(stderr, "dbg5       sid_pvt_offset:  %d\n", store->sid_pvt_offset);
		fprintf(stderr, "dbg5       sid_pvt_num_samples:  %d\n", store->sid_pvt_num_samples);
		for (int i = 0; i < store->sid_pvt_num_samples; i++)
			fprintf(stderr, "dbg5       sid_pvt_phase[%d]:%d\n", i, store->sid_pvt_phase[i]);
		fprintf(stderr, "dbg5       sid_avl_binsize:  %d\n", store->sid_avl_binsize);
		fprintf(stderr, "dbg5       sid_avl_offset:  %d\n", store->sid_avl_offset);
		fprintf(stderr, "dbg5       sid_avl_num_samples:  %d\n", store->sid_avl_num_samples);
		for (int i = 0; i < store->sid_avl_num_samples; i++)
			fprintf(stderr, "dbg5       sid_avl_amp[%d]:%d\n", i, store->sid_avl_amp[i]);
		fprintf(stderr, "dbg5       sid_pvl_binsize:  %d\n", store->sid_pvl_binsize);
		fprintf(stderr, "dbg5       sid_pvl_offset:  %d\n", store->sid_pvl_offset);
		fprintf(stderr, "dbg5       sid_pvl_num_samples:  %d\n", store->sid_pvl_num_samples);
		for (int i = 0; i < store->sid_pvl_num_samples; i++)
			fprintf(stderr, "dbg5       sid_pvl_phase[%d]:%d\n", i, store->sid_pvl_phase[i]);
		fprintf(stderr, "dbg5       sid_sig_ping:  %d\n", store->sid_sig_ping);
		fprintf(stderr, "dbg5       sid_sig_channel:  %d\n", store->sid_sig_channel);
		fprintf(stderr, "dbg5       sid_sig_offset:  %f\n", store->sid_sig_offset);
		fprintf(stderr, "dbg5       sid_sig_sample:  %f\n", store->sid_sig_sample);
		fprintf(stderr, "dbg5       sid_sig_num_samples:  %d\n", store->sid_sig_num_samples);
		for (int i = 0; i < store->sid_sig_num_samples; i++)
			fprintf(stderr, "dbg5       sid_sig_phase[%d]:%d\n", i, store->sid_sig_phase[i]);
		fprintf(stderr, "dbg5       sid_png_pulse:  %u\n", store->sid_png_pulse);
		fprintf(stderr, "dbg5       sid_png_startfrequency:  %f\n", store->sid_png_startfrequency);
		fprintf(stderr, "dbg5       sid_png_endfrequency:  %f\n", store->sid_png_endfrequency);
		fprintf(stderr, "dbg5       sid_png_duration:  %f\n", store->sid_png_duration);
		fprintf(stderr, "dbg5       sid_png_mancode:  %d\n", store->sid_png_mancode);
		fprintf(stderr, "dbg5       sid_png_pulseid:  %d\n", store->sid_png_pulseid);
		fprintf(stderr, "dbg5       sid_png_pulsename:  %s\n", store->sid_png_pulsename);
		fprintf(stderr, "dbg5       sid_cmp_ping:  %d\n", store->sid_cmp_ping);
		fprintf(stderr, "dbg5       sid_cmp_channel:  %d\n", store->sid_cmp_channel);
		fprintf(stderr, "dbg5       sid_cmp_offset:  %f\n", store->sid_cmp_offset);
		fprintf(stderr, "dbg5       sid_cmp_sample:  %f\n", store->sid_cmp_sample);
		fprintf(stderr, "dbg5       sid_cmp_num_samples:  %d\n", store->sid_cmp_num_samples);
		for (int i = 0; i < store->sid_sig_num_samples; i++)
			fprintf(stderr, "dbg5       sid_cmp_real[%d]:%d sid_cmp_imaginary[%d]:%d\n", i, store->sid_cmp_real[i], i,
			        store->sid_cmp_imaginary[i]);
		fprintf(stderr, "dbg5       sid_wgt_factorleft:  %d\n", store->sid_wgt_factorleft);
		fprintf(stderr, "dbg5       sid_wgt_samplesleft:  %d\n", store->sid_wgt_samplesleft);
		fprintf(stderr, "dbg5       sid_wgt_factorright:  %d\n", store->sid_wgt_factorright);
		fprintf(stderr, "dbg5       sid_wgt_samplesright:  %d\n", store->sid_wgt_samplesright);
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
int mbr_l3xseraw_rd_multibeam(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	int byte_count = 0;
	int group_id = 0;
	int index = 0;
	double alpha, beta, theta, phi;
	double rr, xx, zz;
	double xmin, xmax, binsize;
	int ngoodss;
#ifdef MB_DEBUG
	int skip = 0;
#endif

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* set group flags off */
	store->mul_group_beam = false;         /* boolean flag - beam group read */
	store->mul_group_tt = false;           /* boolean flag - tt group read */
	store->mul_group_quality = false;      /* boolean flag - quality group read */
	store->mul_group_amp = false;          /* boolean flag - amp group read */
	store->mul_group_delay = false;        /* boolean flag - delay group read */
	store->mul_group_lateral = false;      /* boolean flag - lateral group read */
	store->mul_group_along = false;        /* boolean flag - along group read */
	store->mul_group_depth = false;        /* boolean flag - depth group read */
	store->mul_group_angle = false;        /* boolean flag - angle group read */
	store->mul_group_heave = false;        /* boolean flag - heave group read */
	store->mul_group_roll = false;         /* boolean flag - roll group read */
	store->mul_group_pitch = false;        /* boolean flag - pitch group read */
	store->mul_group_gates = false;        /* boolean flag - gates group read */
	store->mul_group_noise = false;        /* boolean flag - noise group read */
	store->mul_group_length = false;       /* boolean flag - length group read */
	store->mul_group_hits = false;         /* boolean flag - hits group read */
	store->mul_group_heavereceive = false; /* boolean flag - heavereceive group read */
	store->mul_group_azimuth = false;      /* boolean flag - azimuth group read */
	store->mul_group_properties = false;   /* boolean flag - properties group read */
	store->mul_group_normamp = false;      /* boolean flag - normalized amplitude group read */
	store->mul_group_mbsystemnav = false;  /* boolean flag - mbsystemnav group read */

	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_source);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_sec);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_usec);
	index += 4;

	int status = MB_SUCCESS;

	/* loop over groups */
	bool done = false;
	while (index <= buffer_size && status == MB_SUCCESS && !done) {
		/* look for group start or frame end */
#ifdef MB_DEBUG
		skip = 0;
#endif
#ifdef DATAINPCBYTEORDER
		while (index < buffer_size && strncmp(&buffer[index], "GSH$", 4) && strncmp(&buffer[index], "FSH#", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "FSH#", 4))
			done = true;
		else
			index += 4;
#else
		while (index < buffer_size && strncmp(&buffer[index], "$HSG", 4) && strncmp(&buffer[index], "#HSF", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "#HSF", 4))
			done = true;
		else
			index += 4;
#endif

#ifdef MB_DEBUG
		if (skip > 4)
			fprintf(stderr, "%s:%d | skipped %d bytes in function <%s>\n", __FILE__, __LINE__, skip - 4, __func__);
#endif

		/* deal with group */
		if (!done) {
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&byte_count);
			index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&group_id);
			index += 4;

			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n", group_id, byte_count,
				        __func__);
			}
#ifdef MB_DEBUG
			fprintf(stderr, "%s:%d | Group %d of %d bytes to be parsed in MBIO function <%s>\n", __FILE__, __LINE__, group_id,
			        byte_count, __func__);
#endif

			/* handle general group */
			if (group_id == MBSYS_XSE_MBM_GROUP_GEN) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_GEN\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_ping);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_frequency);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_pulse);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_power);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_bandwidth);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_sample);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_swath);
				index += 4;
#ifdef MB_DEBUG2
				fprintf(stderr, "ping=%u\n", store->mul_ping);
				fprintf(stderr, "frequency=%f\n", store->mul_frequency);
				fprintf(stderr, "pulse=%f\n", store->mul_pulse);
				fprintf(stderr, "power=%f\n", store->mul_power);
				fprintf(stderr, "bandwidth=%f\n", store->mul_bandwidth);
				fprintf(stderr, "sample=%f\n", store->mul_sample);
				fprintf(stderr, "swath=%f\n", store->mul_swath);
#endif
			}

			/* handle beam group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_BEAM) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_BEAM\n", __FILE__, __LINE__);
#endif
				store->mul_group_beam = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->beams[i].beam);
						index += 2;
					}
				}
			}

			/* handle traveltime group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_TT) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_TT\n", __FILE__, __LINE__);
#endif
				store->mul_group_tt = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].tt);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "tt[%d]=%f\n", i, store->beams[i].tt);
#endif
			}

			/* handle quality group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_QUALITY) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_QUALITY\n", __FILE__, __LINE__);
#endif
				store->mul_group_quality = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS)
						store->beams[i].quality = buffer[index++];
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "quality[%d]=%u\n", i, store->beams[i].quality);
#endif
			}

			/* handle amplitude group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_AMP) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_AMP\n", __FILE__, __LINE__);
#endif
				store->mul_group_amp = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->beams[i].amplitude);
						index += 2;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "amp[%d]=%d\n", i, store->beams[i].amplitude);
#endif
			}

			/* handle delay group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_DELAY) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_DELAY\n", __FILE__, __LINE__);
#endif
				store->mul_group_delay = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].delay);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "delay[%d]=%lf\n", i, store->beams[i].delay);
#endif
			}

			/* handle lateral group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_LATERAL) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_LATERAL\n", __FILE__, __LINE__);
#endif
				store->mul_group_lateral = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].lateral);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "lateral[%d]=%lf\n", i, store->beams[i].lateral);
#endif
			}

			/* handle along group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_ALONG) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_ALONG\n", __FILE__, __LINE__);
#endif
				store->mul_group_along = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].along);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "along[%d]=%lf\n", i, store->beams[i].along);
#endif
			}

			/* handle depth group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_DEPTH) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_DEPTH\n", __FILE__, __LINE__);
#endif
				store->mul_group_depth = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].depth);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "depth[%d]=%lf\n", i, store->beams[i].depth);
#endif
			}

			/* handle angle group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_ANGLE) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_ANGLE\n", __FILE__, __LINE__);
#endif
				store->mul_group_angle = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].angle);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "angle[%d]=%lf\n", i, store->beams[i].angle);
#endif
			}

			/* handle heave group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_HEAVE) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_HEAVE\n", __FILE__, __LINE__);
#endif
				store->mul_group_heave = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].heave);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "heave[%d]=%lf\n", i, store->beams[i].heave);
#endif
			}

			/* handle roll group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_ROLL) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_ROLL\n", __FILE__, __LINE__);
#endif
				store->mul_group_roll = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].roll);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "roll[%d]=%lf\n", i, store->beams[i].roll);
#endif
			}

			/* handle pitch group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_PITCH) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_PITCH\n", __FILE__, __LINE__);
#endif
				store->mul_group_pitch = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].pitch);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "pitch[%d]=%lf\n", i, store->beams[i].pitch);
#endif
			}

			/* handle gates group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_GATES) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_GATES\n", __FILE__, __LINE__);
#endif
				store->mul_group_gates = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].gate_angle);
						index += 8;
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].gate_start);
						index += 8;
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].gate_stop);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "gate_angle[%d]=%lf gate_start[%d]=%lf gate_stop[%d]=%lf\n", i, store->beams[i].gate_angle, i,
					        store->beams[i].gate_start, i, store->beams[i].gate_stop);
#endif
			}

			/* handle noise group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_NOISE) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_NOISE\n", __FILE__, __LINE__);
#endif
				store->mul_group_noise = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_float(SWAPFLAG, &buffer[index], &store->beams[i].noise);
						index += 4;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "noise[%d]=%f\n", i, store->beams[i].noise);
#endif
			}

			/* handle length group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_LENGTH) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_LENGTH\n", __FILE__, __LINE__);
#endif
				store->mul_group_length = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_float(SWAPFLAG, &buffer[index], &store->beams[i].length);
						index += 4;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "length[%d]=%f\n", i, store->beams[i].length);
#endif
			}

			/* handle hits group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_HITS) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_HITS\n", __FILE__, __LINE__);
#endif
				store->mul_group_hits = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->beams[i].hits);
						index += 4;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "hits[%d]=%d\n", i, store->beams[i].hits);
#endif
			}

			/* handle heavereceive group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_HEAVERECEIVE) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_HEAVERECEIVE\n", __FILE__, __LINE__);
#endif
				store->mul_group_heavereceive = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->beams[i].heavereceive);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "heavereceive[%d]=%f\n", i, store->beams[i].heavereceive);
#endif
			}

			/* handle azimuth group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_AZIMUTH) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_AZIMUTH\n", __FILE__, __LINE__);
#endif
				store->mul_group_azimuth = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_beams);
				index += 4;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->beams[i].azimuth);
						index += 8;
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "azimuth[%d]=%f\n", i, store->beams[i].azimuth);
#endif
			}

			/* handle properties group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_PROPERTIES) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_PROPERTIES\n", __FILE__, __LINE__);
#endif
				store->mul_group_properties = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_num_properties);
				index += 4;
				for (int i = 0; i < store->mul_num_properties; i++) {
					if (i < MBSYS_XSE_MAXPROPERTIES) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], (double *)&store->mul_properties_type[i]);
						index += 2;
						mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->mul_properties_value);
						index += 8;
					}
				}
				for (int i = 0; i < 40; i++) {
					store->mul_properties_reserved[i] = buffer[index];
					index++;
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_properties);
				for (int i = 0; i < store->mul_num_properties; i++)
					fprintf(stderr, "dbg5       mul_properties[%d]: %d %f\n", i, store->mul_properties_type[i],
					        store->mul_properties_value[i]);
				for (int i = 0; i < 40; i++)
					fprintf(stderr, "dbg5       mul_properties_reserved[%d]: %d\n", i, store->mul_properties_reserved[i]);
#endif
			}

			/* handle normalized amplitude group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_NORMAMP) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_NORMAMP\n", __FILE__, __LINE__);
#endif
				store->mul_group_normamp = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_normamp_num_beams);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->mul_normamp_flags);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->mul_normamp_along_beamwidth);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->mul_normamp_across_beamwidth);
				index += 4;
				for (int i = 0; i < store->mul_normamp_num_beams; i++) {
					if (i < MBSYS_XSE_MAXBEAMS) {
						mb_get_binary_short(SWAPFLAG, &buffer[index], (double *)&store->beams[i].normamp);
						index += 2;
					}
				}
				if (store->mul_normamp_flags == 0) {
					mb_get_binary_float(SWAPFLAG, &buffer[index], (double *)&store->beams[0].frequency);
					index += 4;
					for (int i = 1; i < store->mul_normamp_num_beams; i++) {
						store->beams[i].frequency = store->beams[0].frequency;
					}
				}
				else {
					for (int i = 0; i < store->mul_normamp_num_beams; i++) {
						if (i < MBSYS_XSE_MAXBEAMS) {
							mb_get_binary_float(SWAPFLAG, &buffer[index], (double *)&store->beams[i].frequency);
							index += 4;
						}
					}
				}
#ifdef MB_DEBUG2
				fprintf(stderr, "N=%u\n", store->mul_num_beams);
				for (int i = 0; i < store->mul_num_beams; i++)
					fprintf(stderr, "beam[%d]: normamp=%d frequency=%f\n", i, store->beams[i].normamp, store->beams[i].frequency);
#endif
			}

			/* handle mbsystemnav group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_MBSYSTEMNAV) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBSYS_XSE_MBM_GROUP_MBSYSTEMNAV\n", __FILE__, __LINE__);
#endif
				store->mul_group_mbsystemnav = true;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->mul_lon);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->mul_lat);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->mul_heading);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *)&store->mul_speed);
				index += 8;
#ifdef MB_DEBUG2
				fprintf(stderr, "lon=%f\n", store->mul_lon);
				fprintf(stderr, "lat=%f\n", store->mul_lat);
				fprintf(stderr, "heading=%f\n", store->mul_heading);
				fprintf(stderr, "speed=%f\n", store->mul_speed);
#endif
			}

			else {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MBM_GROUP_OTHER  group_id:%d\n", __FILE__, __LINE__, group_id);
#endif
			}
		}
	}

	/* now if tt and angles read but bathymetry not read
	   calculate bathymetry assuming 1500 m/s velocity */
	if (status == MB_SUCCESS && store->mul_group_tt && store->mul_group_angle &&
	    store->mul_group_heave && store->mul_group_roll && store->mul_group_pitch &&
	    !store->mul_group_depth) {
		store->mul_group_lateral = true;
		store->mul_group_along = true;
		store->mul_group_depth = true;
		for (int i = 0; i < store->mul_num_beams; i++) {
			beta = 90.0 - RTD * store->beams[i].angle;
			alpha = RTD * store->beams[i].pitch;
			mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
			/* divide range by 2 because of round trip travel time */
			rr = 1500.0 * store->beams[i].tt / 2.0;
			xx = rr * sin(DTR * theta);
			zz = rr * cos(DTR * theta);
			store->beams[i].lateral = xx * cos(DTR * phi);
			store->beams[i].along = xx * sin(DTR * phi) + 0.5 * store->nav_speed_ground * store->beams[i].delay;
			store->beams[i].depth = zz;
		}
	}

	/* check for sensible bathymetry */
	if (status == MB_SUCCESS && store->mul_group_depth) {
		for (int i = 0; i < store->mul_num_beams; i++) {
			if (fabs(store->beams[i].depth) > 11000.0) {
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;

				if (fabs(store->beams[i].heave) > 100.0)
					store->beams[i].heave = 0.0;
			}
		}
	}

	/* now if sidescan already read but bin size lacking then
	   calculate bin size from bathymetry */
	if (store->mul_num_beams > 1 && store->sid_frame && store->sid_avl_num_samples > 1 && store->sid_avl_binsize <= 0) {
		/* get width of bathymetry swath size */
		xmin = 9999999.9;
		xmax = -9999999.9;
		for (int i = 0; i < store->mul_num_beams; i++) {
			xmin = MIN(xmin, store->beams[i].lateral);
			xmax = MAX(xmax, store->beams[i].lateral);
		}

		/* get number of nonzero pixels */
		ngoodss = 0;
		for (int i = 0; i < store->sid_avl_num_samples; i++)
			if (store->sid_avl_amp[i] != 0)
				ngoodss++;

		/* get bin size */
		if (xmax > xmin && ngoodss > 1) {
			binsize = (xmax - xmin) / (ngoodss - 1);
			store->sid_avl_binsize = 1000 * binsize;
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       mul_group_beam:      %d\n", store->mul_group_beam);
		fprintf(stderr, "dbg5       mul_group_tt:        %d\n", store->mul_group_tt);
		fprintf(stderr, "dbg5       mul_group_quality:   %d\n", store->mul_group_quality);
		fprintf(stderr, "dbg5       mul_group_amp:       %d\n", store->mul_group_amp);
		fprintf(stderr, "dbg5       mul_group_delay:     %d\n", store->mul_group_delay);
		fprintf(stderr, "dbg5       mul_group_lateral:   %d\n", store->mul_group_lateral);
		fprintf(stderr, "dbg5       mul_group_along:     %d\n", store->mul_group_along);
		fprintf(stderr, "dbg5       mul_group_depth:     %d\n", store->mul_group_depth);
		fprintf(stderr, "dbg5       mul_group_angle:     %d\n", store->mul_group_angle);
		fprintf(stderr, "dbg5       mul_group_heave:     %d\n", store->mul_group_heave);
		fprintf(stderr, "dbg5       mul_group_roll:      %d\n", store->mul_group_roll);
		fprintf(stderr, "dbg5       mul_group_pitch:     %d\n", store->mul_group_pitch);
		fprintf(stderr, "dbg5       mul_group_gates:     %d\n", store->mul_group_gates);
		fprintf(stderr, "dbg5       mul_group_noise:     %d\n", store->mul_group_noise);
		fprintf(stderr, "dbg5       mul_group_length:    %d\n", store->mul_group_length);
		fprintf(stderr, "dbg5       mul_group_hits:      %d\n", store->mul_group_hits);
		fprintf(stderr, "dbg5       mul_group_heavereceive: %d\n", store->mul_group_heavereceive);
		fprintf(stderr, "dbg5       mul_group_azimuth:    %d\n", store->mul_group_azimuth);
		fprintf(stderr, "dbg5       mul_group_properties: %d\n", store->mul_group_properties);
		fprintf(stderr, "dbg5       mul_group_normamp:    %d\n", store->mul_group_normamp);
		fprintf(stderr, "dbg5       mul_group_mbsystemnav:  %d\n", store->mul_group_mbsystemnav);
		fprintf(stderr, "dbg5       mul_source:          %d\n", store->mul_source);
		fprintf(stderr, "dbg5       mul_sec:             %u\n", store->mul_sec);
		fprintf(stderr, "dbg5       mul_usec:            %u\n", store->mul_usec);
		fprintf(stderr, "dbg5       mul_ping:            %d\n", store->mul_ping);
		fprintf(stderr, "dbg5       mul_frequency:       %f\n", store->mul_frequency);
		fprintf(stderr, "dbg5       mul_pulse:           %f\n", store->mul_pulse);
		fprintf(stderr, "dbg5       mul_power:           %f\n", store->mul_power);
		fprintf(stderr, "dbg5       mul_bandwidth:       %f\n", store->mul_bandwidth);
		fprintf(stderr, "dbg5       mul_sample:          %f\n", store->mul_sample);
		fprintf(stderr, "dbg5       mul_swath:           %f\n", store->mul_swath);
		fprintf(stderr, "dbg5       mul_num_beams:       %d\n", store->mul_num_beams);
		fprintf(stderr, "dbg5       mul_lon:             %f\n", store->mul_lon);
		fprintf(stderr, "dbg5       mul_lat:             %f\n", store->mul_lat);
		fprintf(stderr, "dbg5       mul_heading:         %f\n", store->mul_heading);
		fprintf(stderr, "dbg5       mul_speed:           %f\n", store->mul_speed);
		for (int i = 0; i < store->mul_num_beams; i++)
			fprintf(stderr,
			        "dbg5       beam[%d]: %3d %7.2f %7.2f %7.2f %3d %3d %6.3f %6.2f %5.3f %5.2f %6.2f %6.2f  %f %f %f %f %f %d "
			        "%f %f %d %f\n",
			        i, store->beams[i].beam, store->beams[i].lateral, store->beams[i].along, store->beams[i].depth,
			        store->beams[i].amplitude, store->beams[i].quality, store->beams[i].tt, store->beams[i].angle,
			        store->beams[i].delay, store->beams[i].heave, store->beams[i].roll, store->beams[i].pitch,
			        store->beams[i].gate_angle, store->beams[i].gate_start, store->beams[i].gate_stop, store->beams[i].noise,
			        store->beams[i].length, store->beams[i].hits, store->beams[i].heavereceive, store->beams[i].azimuth,
			        store->beams[i].normamp, store->beams[i].frequency);
		fprintf(stderr, "dbg5       mul_num_properties: %d\n", store->mul_num_properties);
		for (int i = 0; i < store->mul_num_properties; i++)
			fprintf(stderr, "dbg5       mun_property[%d]: %d %f\n", i, store->mul_properties_type[i],
			        store->mul_properties_value[i]);
		for (int i = 0; i < 40; i++)
			fprintf(stderr, "dbg5       mul_properties_reserved[%d]: %d\n", i, store->mul_properties_reserved[i]);
		fprintf(stderr, "dbg5       mul_normamp_num_beams:        %d\n", store->mul_normamp_num_beams);
		fprintf(stderr, "dbg5       mul_normamp_flags:            %d\n", store->mul_normamp_flags);
		fprintf(stderr, "dbg5       mul_normamp_along_beamwidth:  %f\n", store->mul_normamp_along_beamwidth);
		fprintf(stderr, "dbg5       mul_normamp_across_beamwidth: %f\n", store->mul_normamp_across_beamwidth);
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
int mbr_l3xseraw_rd_singlebeam(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* The singlebeam frame is currently unused by MB-System */

	const int status = MB_SUCCESS;

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
int mbr_l3xseraw_rd_message(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* The message frame is currently unused by MB-System */

	const int status = MB_SUCCESS;

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
int mbr_l3xseraw_rd_seabeam(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
  int byte_count = 0;
	int group_id = 0;
	int index = 0;
#ifdef MB_DEBUG
	int skip = 0;
#endif

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_source);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_sec);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_usec);
	index += 4;

	const int status = MB_SUCCESS;

	/* loop over groups */
	bool done = false;
	while (index <= buffer_size && status == MB_SUCCESS && !done) {
		/* look for group start or frame end */
#ifdef MB_DEBUG
		skip = 0;
#endif
#ifdef DATAINPCBYTEORDER
		while (index < buffer_size && strncmp(&buffer[index], "GSH$", 4) && strncmp(&buffer[index], "FSH#", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "FSH#", 4))
			done = true;
		else
			index += 4;
#else
		while (index < buffer_size && strncmp(&buffer[index], "$HSG", 4) && strncmp(&buffer[index], "#HSF", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "#HSF", 4))
			done = true;
		else
			index += 4;
#endif

#ifdef MB_DEBUG
		if (skip > 4)
			fprintf(stderr, "%s:%d | skipped %d bytes in function <%s>\n", __FILE__, __LINE__, skip - 4, __func__);
#endif

		/* deal with group */
		if (!done) {
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&byte_count);
			index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&group_id);
			index += 4;

			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n", group_id, byte_count,
				        __func__);
			}
#ifdef MB_DEBUG
			fprintf(stderr, "%s:%d | Group %d of %d bytes to be parsed in MBIO function <%s>\n", __FILE__, __LINE__, group_id,
			        byte_count, __func__);
#endif

			/* handle properties group */
			if (group_id == MBSYS_XSE_SBM_GROUP_PROPERTIES) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SBM_GROUP_PROPERTIES\n", __FILE__, __LINE__);
#endif
				store->sbm_properties = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_ping);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_ping_gain);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_pulse_width);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_transmit_power);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_pixel_width);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_swath_width);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_time_slice);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_depth_mode);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_beam_mode);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_ssv);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_frequency);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_bandwidth);
				index += 4;
			}

			/* handle hrp group */
			if (group_id == MBSYS_XSE_SBM_GROUP_HRP) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SBM_GROUP_HRP\n", __FILE__, __LINE__);
#endif
				store->sbm_hrp = true;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->sbm_heave);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->sbm_roll);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->sbm_pitch);
				index += 8;
			}

			/* handle center group */
			if (group_id == MBSYS_XSE_SBM_GROUP_SIGNAL) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SBM_GROUP_SIGNAL\n", __FILE__, __LINE__);
#endif
				store->sbm_signal = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_signal_beam);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_signal_count);
				index += 4;
				store->sbm_signal_count = MIN(MBSYS_XSE_MAXSAMPLES, store->sbm_signal_count);
				for (int i = 0; i < store->sbm_signal_count; i++) {
					mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_signal_amp[i]);
					index += 4;
				}
			}

			/* handle message group */
			if (group_id == MBSYS_XSE_SBM_GROUP_MESSAGE) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SBM_GROUP_MESSAGE\n", __FILE__, __LINE__);
#endif
				store->sbm_message = true;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_message_id);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_message_len);
				index += 4;
				if (store->sbm_message_len > buffer_size)
					fprintf(stderr, "Read message: %d %d %d\n", buffer_size, store->sbm_message_len, store->sbm_message_id);
				for (int i = 0; i < store->sbm_message_len; i++) {
					store->sbm_message_txt[i] = buffer[index];
					index++;
				}
				store->sbm_message_txt[store->sbm_message_len] = 0;
			}

			/* handle sweep segments group */
			if (group_id == MBSYS_XSE_SBM_GROUP_SWEEPSEGMENTS) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SBM_GROUP_SWEEPSEGMENTS\n", __FILE__, __LINE__);
#endif
				store->sbm_sweepsegments = true;
				store->sbm_sweep_direction = buffer[index];
				index++;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_azimuth);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_segments);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_seconds);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_micro);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_extrapolateazimuth);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_interpolatedazimuth);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_extrapolatepitch);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_interpolatedpitch);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_extrapolateroll);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_interpolatedroll);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_sweep_stabilizedangle);
				index += 4;
			}

			/* handle spacing mode group */
			if (group_id == MBSYS_XSE_SBM_GROUP_SPACINGMODE) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SBM_GROUP_SPACINGMODE\n", __FILE__, __LINE__);
#endif
				store->sbm_spacingmode = true;
				store->sbm_spacing_mode = buffer[index];
				index++;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_spacing_equidistance);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_spacing_equidistance_min);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (int *)&store->sbm_spacing_equidistance_max);
				index += 4;
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sbm_source:          %d\n", store->sbm_source);
		fprintf(stderr, "dbg5       sbm_sec:             %u\n", store->sbm_sec);
		fprintf(stderr, "dbg5       sbm_usec:            %u\n", store->sbm_usec);
	}
	if (verbose >= 5 && store->sbm_properties) {
		fprintf(stderr, "dbg5       sbm_ping:            %d\n", store->sbm_ping);
		fprintf(stderr, "dbg5       sbm_ping_gain:       %f\n", store->sbm_ping_gain);
		fprintf(stderr, "dbg5       sbm_pulse_width:     %f\n", store->sbm_pulse_width);
		fprintf(stderr, "dbg5       sbm_transmit_power:  %f\n", store->sbm_transmit_power);
		fprintf(stderr, "dbg5       sbm_pixel_width:     %f\n", store->sbm_pixel_width);
		fprintf(stderr, "dbg5       sbm_swath_width:     %f\n", store->sbm_swath_width);
		fprintf(stderr, "dbg5       sbm_time_slice:      %f\n", store->sbm_time_slice);
		fprintf(stderr, "dbg5       sbm_depth_mode:      %d\n", store->sbm_depth_mode);
		fprintf(stderr, "dbg5       sbm_beam_mode:       %d\n", store->sbm_beam_mode);
		fprintf(stderr, "dbg5       sbm_ssv:             %f\n", store->sbm_ssv);
		fprintf(stderr, "dbg5       sbm_frequency:       %f\n", store->sbm_frequency);
		fprintf(stderr, "dbg5       sbm_bandwidth:       %f\n", store->sbm_bandwidth);
	}
	if (verbose >= 5 && store->sbm_hrp) {
		fprintf(stderr, "dbg5       sbm_heave:           %f\n", store->sbm_heave);
		fprintf(stderr, "dbg5       sbm_roll:            %f\n", store->sbm_roll);
		fprintf(stderr, "dbg5       sbm_pitch:           %f\n", store->sbm_pitch);
	}
	if (verbose >= 5 && store->sbm_signal) {
		fprintf(stderr, "dbg5       sbm_signal_beam:     %d\n", store->sbm_signal_beam);
		fprintf(stderr, "dbg5       sbm_signal_count:    %d\n", store->sbm_signal_count);
		for (int i = 0; i < store->sbm_signal_count; i++)
			fprintf(stderr, "dbg5       sample[%d]: %f\n", i, store->sbm_signal_amp[i]);
	}
	if (verbose >= 5 && store->sbm_message) {
		fprintf(stderr, "dbg5       sbm_message_id:      %d\n", store->sbm_message_id);
		fprintf(stderr, "dbg5       sbm_message_len:     %d\n", store->sbm_message_len);
		fprintf(stderr, "dbg5       sbm_message_txt:     %s\n", store->sbm_message_txt);
	}
	if (verbose >= 5 && store->sbm_sweepsegments) {
		fprintf(stderr, "dbg5       sbm_sweep_direction: %d\n", store->sbm_sweep_direction);
		fprintf(stderr, "dbg5       sbm_sweep_azimuth:   %f\n", store->sbm_sweep_azimuth);
		fprintf(stderr, "dbg5       sbm_sweep_segments:  %d\n", store->sbm_sweep_segments);
		fprintf(stderr, "dbg5       sbm_sweep_seconds:   %d\n", store->sbm_sweep_seconds);
		fprintf(stderr, "dbg5       sbm_sweep_micro:     %d\n", store->sbm_sweep_micro);
		fprintf(stderr, "dbg5       sbm_sweep_extrapolateazimuth:  %f\n", store->sbm_sweep_extrapolateazimuth);
		fprintf(stderr, "dbg5       sbm_sweep_interpolatedazimuth: %f\n", store->sbm_sweep_interpolatedazimuth);
		fprintf(stderr, "dbg5       sbm_sweep_extrapolatepitch:    %f\n", store->sbm_sweep_extrapolatepitch);
		fprintf(stderr, "dbg5       sbm_sweep_interpolatedpitch:   %f\n", store->sbm_sweep_interpolatedpitch);
		fprintf(stderr, "dbg5       sbm_sweep_extrapolateroll:     %f\n", store->sbm_sweep_extrapolateroll);
		fprintf(stderr, "dbg5       sbm_sweep_interpolatedroll:    %f\n", store->sbm_sweep_interpolatedroll);
		fprintf(stderr, "dbg5       sbm_sweep_stabilizedangle:     %f\n", store->sbm_sweep_stabilizedangle);
	}
	if (verbose >= 5 && store->sbm_spacingmode) {
		fprintf(stderr, "dbg5       sbm_spacing_mode:      	      %d\n", store->sbm_spacing_mode);
		fprintf(stderr, "dbg5       sbm_spacing_equidistance:      %f\n", store->sbm_spacing_equidistance);
		fprintf(stderr, "dbg5       sbm_spacing_equidistance_min:  %f\n", store->sbm_spacing_equidistance_min);
		fprintf(stderr, "dbg5       sbm_spacing_equidistance_max:  %f\n", store->sbm_spacing_equidistance_max);
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
int mbr_l3xseraw_rd_geodetic(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* The geodetic frame is currently unused by MB-System */

	const int status = MB_SUCCESS;

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
int mbr_l3xseraw_rd_native(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* The native frame is currently unused by MB-System */

	const int status = MB_SUCCESS;

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
int mbr_l3xseraw_rd_product(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* The product frame is currently unused by MB-System */

	const int status = MB_SUCCESS;

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
int mbr_l3xseraw_rd_bathymetry(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* The bathymetry frame is currently unused by MB-System */

	const int status = MB_SUCCESS;

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
int mbr_l3xseraw_rd_control(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* The control frame is currently unused by MB-System */

	const int status = MB_SUCCESS;

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
int mbr_l3xseraw_rd_comment(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	int byte_count = 0;
	int group_id = 0;
	int index = 0;
#ifdef MB_DEBUG
	int skip = 0;
#endif
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->com_source);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->com_sec);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->com_usec);
	index += 4;

	const int status = MB_SUCCESS;

	/* loop over groups */
	bool done = false;
	while (index <= buffer_size && status == MB_SUCCESS && !done) {
		/* look for group start or frame end */
#ifdef MB_DEBUG
		skip = 0;
#endif
#ifdef DATAINPCBYTEORDER
		while (index < buffer_size && strncmp(&buffer[index], "GSH$", 4) && strncmp(&buffer[index], "FSH#", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "FSH#", 4))
			done = true;
		else
			index += 4;
#else
		while (index < buffer_size && strncmp(&buffer[index], "$HSG", 4) && strncmp(&buffer[index], "#HSF", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "#HSF", 4))
			done = true;
		else
			index += 4;
#endif

#ifdef MB_DEBUG
		if (skip > 4)
			fprintf(stderr, "%s:%d | skipped %d bytes in function <%s>\n", __FILE__, __LINE__, skip - 4, __func__);
#endif

		/* deal with group */
		if (!done) {
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&byte_count);
			index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&group_id);
			index += 4;

			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n", group_id, byte_count,
				        __func__);
			}
#ifdef MB_DEBUG
			fprintf(stderr, "%s:%d | Group %d of %d bytes to be parsed in MBIO function <%s>\n", __FILE__, __LINE__, group_id,
			        byte_count, __func__);
#endif

			/* handle general group */
			if (group_id == MBSYS_XSE_COM_GROUP_GEN) {
				for (int i = 0; i < byte_count; i++) {
					if (i < MBSYS_XSE_COMMENT_LENGTH - 1)
						store->comment[i] = buffer[index++];
				}
				store->comment[MIN(byte_count - 4, MBSYS_XSE_COMMENT_LENGTH - 1)] = '\0';
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       comment:             %s\n", store->comment);
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
int mbr_l3xseraw_rd_nav(int verbose, int buffer_size, char *buffer, void *store_ptr, int *error) {
	int byte_count = 0;
	int group_id = 0;
	int index = 0;
#ifdef MB_DEBUG
	int skip = 0;
#endif

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer_size:%d\n", buffer_size);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->nav_source);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->nav_sec);
	index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->nav_usec);
	index += 4;

	/* reset group read flags */
	store->nav_group_general = false;  /* boolean flag */
	store->nav_group_position = false; /* boolean flag */
	store->nav_group_accuracy = false; /* boolean flag */
	store->nav_group_motiongt = false; /* boolean flag */
	store->nav_group_motiontw = false; /* boolean flag */
	store->nav_group_track = false;    /* boolean flag */
	store->nav_group_hrp = false;      /* boolean flag */
	store->nav_group_heave = false;    /* boolean flag */
	store->nav_group_roll = false;     /* boolean flag */
	store->nav_group_pitch = false;    /* boolean flag */
	store->nav_group_heading = false;  /* boolean flag */
	store->nav_group_log = false;      /* boolean flag */
	store->nav_group_gps = false;      /* boolean flag */

	const int status = MB_SUCCESS;

	/* loop over groups */
	bool done = false;
	while (index <= buffer_size && status == MB_SUCCESS && !done) {
		/* look for group start or frame end */
#ifdef MB_DEBUG
		skip = 0;
#endif
#ifdef DATAINPCBYTEORDER
		while (index < buffer_size && strncmp(&buffer[index], "GSH$", 4) && strncmp(&buffer[index], "FSH#", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "FSH#", 4))
			done = true;
		else
			index += 4;
#else
		while (index < buffer_size && strncmp(&buffer[index], "$HSG", 4) && strncmp(&buffer[index], "#HSF", 4)) {
			index++;
#ifdef MB_DEBUG
			skip++;
#endif
		}
		if (index >= buffer_size || !strncmp(&buffer[index], "#HSF", 4))
			done = true;
		else
			index += 4;
#endif

#ifdef MB_DEBUG
		if (skip > 4)
			fprintf(stderr, "%s:%d | skipped %d bytes in function <%s>\n", __FILE__, __LINE__, skip - 4, __func__);
#endif

		/* deal with group */
		if (!done) {
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&byte_count);
			index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&group_id);
			index += 4;

			if (verbose >= 5) {
				fprintf(stderr, "\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n", group_id, byte_count,
				        __func__);
			}
#ifdef MB_DEBUG
			fprintf(stderr, "%s:%d | Group %d of %d bytes to be parsed in MBIO function <%s>\n", __FILE__, __LINE__, group_id,
			        byte_count, __func__);
#endif

			/* handle general group */
			if (group_id == MBSYS_XSE_NAV_GROUP_GEN) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_GEN\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->nav_quality);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->nav_status);
				index += 4;
				store->nav_group_general = true;
			}

			/* handle point group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_POS) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_POS\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->nav_description_len);
				index += 4;
				for (int i = 0; i < store->nav_description_len; i++) {
					store->nav_description[i] = buffer[index];
					index++;
				}
				store->nav_description[store->nav_description_len] = '\0';
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_x);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_y);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_z);
				index += 8;
				store->nav_group_position = true;
			}

			/* handle accuracy group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_ACCURACY) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_ACCURACY\n", __FILE__, __LINE__);
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->nav_acc_quality);
				index += 4;
				store->nav_acc_numsatellites = buffer[index];
				index++;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (float *)&store->nav_acc_horizdilution);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (float *)&store->nav_acc_diffage);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&store->nav_acc_diffref);
				index += 4;
				store->nav_group_accuracy = true;
			}

			/* handle motion ground truth group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_MOTIONGT) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_MOTIONGT\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_speed_ground);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_course_ground);
				index += 8;
				store->nav_group_motiongt = true;
			}

			/* handle motion through water group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_MOTIONTW) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_MOTIONTW\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_speed_water);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_course_water);
				index += 8;
				store->nav_group_motiontw = true;
			}

			/* handle current track steering properties group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_TRACK) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_TRACK\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_offset_track);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_offset_sol);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_offset_eol);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_distance_sol);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_azimuth_sol);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_distance_eol);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_azimuth_eol);
				index += 8;
				store->nav_group_track = true;
			}

			/* handle the heaverollpitch group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_HRP) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_HRP\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hrp_heave);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hrp_roll);
				index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hrp_pitch);
				index += 8;
				store->nav_group_hrp = true;
				/* heave, roll, and pitch are best obtained from the multibeam frame */
			}

			/* handle the heave group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_HEAVE) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_HEAVE\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hea_heave);
				index += 8;
				store->nav_group_heave = true;
				/* heave is obtained from the multibeam frame */
			}

			/* handle the roll group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_ROLL) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_ROLL\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_rol_roll);
				index += 8;
				store->nav_group_roll = true;
				/* roll is obtained from the multibeam frame */
			}

			/* handle the pitch group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_PITCH) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_PITCH\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_pit_pitch);
				index += 8;
				store->nav_group_pitch = true;
				/* pitch is obtained from the multibeam frame */
			}

			/* handle the heading group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_HEADING) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_HEADING\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hdg_heading);
				index += 8;
				store->nav_group_heading = true;
				/* Heading Group value overrides the MTW Group course value */
			}

			/* handle the log group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_LOG) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_LOG\n", __FILE__, __LINE__);
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_log_speed);
				index += 8;
				store->nav_group_log = true;
				/* speed is obtained from the motion ground truth */
				/* and motion through water groups */
			}

			/* handle the gps group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_GPS) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_LOG\n", __FILE__, __LINE__);
#endif
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->nav_gps_altitude);
				index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->nav_gps_geoidalseparation);
				index += 4;
				store->nav_group_gps = true;
			}

			else {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV_GROUP_OTHER\n", __FILE__, __LINE__);
#endif
			}
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       nav_source:          %d\n", store->nav_source);
		fprintf(stderr, "dbg5       nav_sec:             %u\n", store->nav_sec);
		fprintf(stderr, "dbg5       nav_usec:            %u\n", store->nav_usec);
		fprintf(stderr, "dbg5       nav_quality:         %d\n", store->nav_quality);
		fprintf(stderr, "dbg5       nav_status:          %d\n", store->nav_status);
		fprintf(stderr, "dbg5       nav_description_len: %d\n", store->nav_description_len);
		fprintf(stderr, "dbg5       nav_description:     %s\n", store->nav_description);
		fprintf(stderr, "dbg5       nav_x:               %f\n", store->nav_x);
		fprintf(stderr, "dbg5       nav_y:               %f\n", store->nav_y);
		fprintf(stderr, "dbg5       nav_z:               %f\n", store->nav_z);
		fprintf(stderr, "dbg5       nav_speed_ground:    %f\n", store->nav_speed_ground);
		fprintf(stderr, "dbg5       nav_course_ground:   %f\n", store->nav_course_ground);
		fprintf(stderr, "dbg5       nav_speed_water:     %f\n", store->nav_speed_water);
		fprintf(stderr, "dbg5       nav_course_water:    %f\n", store->nav_course_water);
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
int mbr_l3xseraw_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;
	FILE *mbfp = mb_io_ptr->mbfp;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

  /* set variables to be saved in mb_io structure */
	*error = MB_ERROR_NO_ERROR;
  char *label = (char *) mb_io_ptr->save_label;
	int *frame_expect = (int *)&mb_io_ptr->save1;
	bool *frame_save = (bool *)&mb_io_ptr->saveb1;
	int *frame_id_save = (int *)&mb_io_ptr->save3;
	int *frame_source_save = (int *)&mb_io_ptr->save4;
	int *frame_sec_save = (int *)&mb_io_ptr->save5;
	int *frame_usec_save = (int *)&mb_io_ptr->save6;
	int *buffer_size_save = (int *)&mb_io_ptr->save7;
	int *buffer_size_max = (int *)&mb_io_ptr->save8;
	char *buffer = mb_io_ptr->hdr_comment;

	store->sbm_properties = false;
	store->sbm_hrp = false;
	store->sbm_signal = false;
	store->sbm_message = false;
	if (*frame_save) {
		store->mul_frame = false;
		store->sid_frame = false;
	}

	int frame_id;
	int frame_source;
	int frame_sec;
	int frame_usec;
	int frame_transaction;
	int frame_address;
	int buffer_size;
	int frame_size;
	int index = 0;
	int read_len;
#ifdef MB_DEBUG
	int skip = 0;
#endif
	int status = MB_SUCCESS;

	/* read until done */
	bool done = false;
	while (!done) {
		/* use saved frame if available */
		if (*frame_save) {
			frame_id = *frame_id_save;
			frame_source = *frame_source_save;
			frame_sec = *frame_sec_save;
			frame_usec = *frame_usec_save;
			buffer_size = *buffer_size_save;
			*frame_save = false;
		}

		/* else read from file */
		else {

			/* look for the next frame start */
#ifdef MB_DEBUG
			skip = 0;
#endif
			if ((read_len = fread(&label[0], 1, 4, mb_io_ptr->mbfp)) != 4) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}

#ifdef MB_DEBUG
			fprintf(stderr, "Byte: %d %c %o %x\n", label[0], label[0], label[0], label[0]);
			fprintf(stderr, "Byte: %d %c %o %x\n", label[1], label[1], label[1], label[1]);
			fprintf(stderr, "Byte: %d %c %o %x\n", label[2], label[2], label[2], label[2]);
			fprintf(stderr, "Byte: %d %c %o %x\n", label[3], label[3], label[3], label[3]);
#endif

#ifdef DATAINPCBYTEORDER
			while (status == MB_SUCCESS && strncmp(label, "FSH$", 4))
#else
			while (status == MB_SUCCESS && strncmp(label, "$HSF", 4))
#endif
			{
				/* get next byte */
				for (int i = 0; i < 3; i++)
					label[i] = label[i + 1];
				if ((read_len = fread(&label[3], 1, 1, mb_io_ptr->mbfp)) != 1) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
				else {
#ifdef MB_DEBUG
					skip++;
					fprintf(stderr, "Byte: %d %c %o %x\n", label[3], label[3], label[3], label[3]);
#endif
				}
			}

			/* Read entire data record into buffer. The XSE frame byte count value */
			/* is notorious for being incorrect.  So we read the data record by */
			/* reading up to the next frame end mark. */

			/* copy the frame start label to the buffer */
			if (status == MB_SUCCESS) {
				strncpy(buffer, label, 4);
				index = 4;
				buffer_size = 4;
			}

			/* Read next four bytes from the file into buffer to get us started. */
			if (status == MB_SUCCESS) {
				if ((read_len = fread(&buffer[index], 1, 4, mb_io_ptr->mbfp)) != 4) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					frame_size = 0;
				}
				else {
					buffer_size += 4;
					mb_get_binary_int(SWAPFLAG, &buffer[4], (int *)&frame_size);
				}
			}

/* now read a byte at a time, continuing until we find the end mark */
#ifdef DATAINPCBYTEORDER
			while (status == MB_SUCCESS && strncmp(&buffer[index], "FSH#", 4))
#else
			while (status == MB_SUCCESS && strncmp(&buffer[index], "#HSF", 4))
#endif
			{
				/* read next byte */
				if ((read_len = fread(&buffer[buffer_size], 1, 1, mb_io_ptr->mbfp)) != 1) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
				else {
					buffer_size++;
					index++;
				}

				/* don't let buffer overflow - error if record exceeds max size */
				if (buffer_size >= MBSYS_XSE_BUFFER_SIZE) {
					status = MB_FAILURE;
					*error = MB_ERROR_UNINTELLIGIBLE;
				}
			}
			*buffer_size_max = MAX(buffer_size, *buffer_size_max);

#ifdef MB_DEBUG
			if (*error != MB_ERROR_EOF) {
				fprintf(stderr, "%s:%d | \n", __FILE__, __LINE__);
				if (skip > 0)
					fprintf(stderr, "\n%s:%d | BYTES SKIPPED BETWEEN FRAMES: %d\n", __FILE__, __LINE__, skip);
				fprintf(stderr, "\n%s:%d | BUFFER SIZE: %u  MAX FOUND: %u  MAX: %u\n", __FILE__, __LINE__, buffer_size,
				        *buffer_size_max, MBSYS_XSE_BUFFER_SIZE);
			}
#endif

			/* parse header values */
			if (status == MB_SUCCESS) {
				/* get frame id, source, and time */
				index = 8;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&frame_id);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&frame_source);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&frame_sec);
				index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&frame_usec);
				index += 4;

				/* if it's a control frame, get the transaction and address values */
				if (frame_id == MBSYS_XSE_CNT_FRAME) {
					mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&frame_transaction);
					index += 4;
					mb_get_binary_int(SWAPFLAG, &buffer[index], (int *)&frame_address);
					index += 4;
				}
			}
		}

		/* parse data if possible */
		if (status == MB_SUCCESS) {
#ifdef MB_DEBUG
			fprintf(stderr, "%s:%d | FRAME ID: %u  BUFFER SIZE:%d  FRAME SIZE:%d\n", __FILE__, __LINE__, frame_id, buffer_size,
			        frame_size);
#endif
			if (frame_id == MBSYS_XSE_NAV_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NAV\n", __FILE__, __LINE__);
#endif
				status = mbr_l3xseraw_rd_nav(verbose, buffer_size, buffer, store_ptr, error);
				if (store->nav_source > 0) {
					store->kind = MB_DATA_NAV;
#ifdef MB_DEBUG
					fprintf(stderr, "%s:%d | nav_source:%d  time:%u.%6.6u\n", __FILE__, __LINE__, store->nav_source,
					        store->nav_sec, store->nav_usec);
#endif
				}
				else
					store->kind = MB_DATA_RAW_LINE;
				done = true;
			}
			else if (frame_id == MBSYS_XSE_SVP_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SVP\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_VELOCITY_PROFILE;
				status = mbr_l3xseraw_rd_svp(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_TID_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ TIDE\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_tide(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_SHP_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ PARAMETER\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_PARAMETER;
				status = mbr_l3xseraw_rd_ship(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_SSN_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SIDESCAN\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_DATA;
				status = mbr_l3xseraw_rd_sidescan(verbose, buffer_size, buffer, store_ptr, error);
				store->sid_frame = true;
				if (frame_id == *frame_expect && store->sid_ping == store->mul_ping && store->sid_group_avl) {
					*frame_expect = MBSYS_XSE_NONE_FRAME;
					done = true;
				}
				else if (frame_id == *frame_expect && store->sid_ping == store->mul_ping && !store->sid_group_avl) {
					done = false;
				}
				else if (*frame_expect == MBSYS_XSE_NONE_FRAME) {
					*frame_expect = MBSYS_XSE_MBM_FRAME;
					done = false;
				}
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | \tframe_id:%d frame_expect:%d ping:%d %d sid_group_avl:%d\n", __FILE__, __LINE__,
				        frame_id, *frame_expect, store->sid_ping, store->mul_ping, store->sid_group_avl);
				fprintf(stderr, "%s:%d | \tDONE:%d BEAMS:%d PIXELS:%d\n", __FILE__, __LINE__, done, store->mul_num_beams,
				        store->sid_avl_num_samples);
#endif
			}
			else if (frame_id == MBSYS_XSE_MBM_FRAME && *frame_expect == MBSYS_XSE_SSN_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NOTHING - SAVE HEADER\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_DATA;
				*frame_save = true;
				*frame_id_save = frame_id;
				*frame_source_save = frame_source;
				*frame_sec_save = frame_sec;
				*frame_usec_save = frame_usec;
				*buffer_size_save = buffer_size;
				*frame_expect = MBSYS_XSE_NONE_FRAME;
				done = true;
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | \tDONE:%d BEAMS:%d PIXELS:%d\n", __FILE__, __LINE__, done, store->mul_num_beams,
				        store->sid_avl_num_samples);
#endif
			}
			else if (frame_id == MBSYS_XSE_MBM_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MULTIBEAM\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_DATA;
				status = mbr_l3xseraw_rd_multibeam(verbose, buffer_size, buffer, store_ptr, error);
				store->mul_frame = true;
				if (frame_id == *frame_expect && store->sid_ping == store->mul_ping) {
					*frame_expect = MBSYS_XSE_NONE_FRAME;
					done = true;
				}
				else if (frame_id == *frame_expect) {
					*frame_expect = MBSYS_XSE_SSN_FRAME;
					done = false;
				}
				else if (*frame_expect == MBSYS_XSE_NONE_FRAME) {
					*frame_expect = MBSYS_XSE_SSN_FRAME;
					done = false;
				}
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | \tDONE:%d BEAMS:%d PIXELS:%d\n", __FILE__, __LINE__, done, store->mul_num_beams,
				        store->sid_avl_num_samples);
#endif
			}
			else if (frame_id == MBSYS_XSE_SNG_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SINGLEBEAM\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_singlebeam(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_CNT_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ CONTROL\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_control(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_BTH_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ BATHYMETRY\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_bathymetry(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_PRD_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ PRODUCT\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_product(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_NTV_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ NATIVE\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_native(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_GEO_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ GEODETIC\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_geodetic(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_SBM_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ SEABEAM\n", __FILE__, __LINE__);
#endif
				status = mbr_l3xseraw_rd_seabeam(verbose, buffer_size, buffer, store_ptr, error);
				if (store->sbm_properties)
					store->kind = MB_DATA_RUN_PARAMETER;
				else
					store->kind = MB_DATA_RAW_LINE;
				done = true;
			}
			else if (frame_id == MBSYS_XSE_MSG_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ MESSAGE\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_message(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else if (frame_id == MBSYS_XSE_COM_FRAME) {
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ COMMENT\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_COMMENT;
				status = mbr_l3xseraw_rd_comment(verbose, buffer_size, buffer, store_ptr, error);
				done = true;
			}
			else /* handle an unrecognized frame */
			{
#ifdef MB_DEBUG
				fprintf(stderr, "%s:%d | READ OTHER\n", __FILE__, __LINE__);
#endif
				store->kind = MB_DATA_RAW_LINE;
			}

			if (store->kind == MB_DATA_RAW_LINE) {
				store->rawsize = buffer_size;
				for (int i = 0; i < buffer_size; i++)
					store->raw[i] = buffer[i];
				done = true;
			}
		}
		else if (*frame_expect != MBSYS_XSE_NONE_FRAME && frame_id != *frame_expect) {
#ifdef MB_DEBUG
			fprintf(stderr, "%s:%d | READ NOTHING - SAVE HEADER\n", __FILE__, __LINE__);
#endif
			store->kind = MB_DATA_DATA;
			*frame_save = true;
			*frame_id_save = frame_id;
			*frame_source_save = frame_source;
			*frame_sec_save = frame_sec;
			*frame_usec_save = frame_usec;
			*buffer_size_save = buffer_size;
			*frame_expect = MBSYS_XSE_NONE_FRAME;
			done = true;
		}

		/* check for status */
		if (status == MB_FAILURE) {
			done = true;
			*frame_save = false;
		}
	}

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mbfp);

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
int mbr_rt_l3xseraw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	double time_d;
	double lon, lat;
	double heading;
	double speed;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* read next data from file */
	const int status = mbr_l3xseraw_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

	/* save fix if nav data */
	if (status == MB_SUCCESS && store->kind == MB_DATA_NAV) {
		/* get time */
		time_d = store->nav_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * store->nav_usec;

		/* add nav to navlist */
		if (store->nav_group_position)
			mb_navint_add(verbose, mbio_ptr, time_d, RTD * store->nav_x, RTD * store->nav_y, error);

		/* add heading to navlist */
		if (store->nav_group_heading)
			mb_hedint_add(verbose, mbio_ptr, time_d, RTD * store->nav_hdg_heading, error);
		else if (store->nav_group_motiongt)
			mb_hedint_add(verbose, mbio_ptr, time_d, RTD * store->nav_course_ground, error);
		else if (store->nav_group_motiontw)
			mb_hedint_add(verbose, mbio_ptr, time_d, RTD * store->nav_course_water, error);
	}

	/* interpolate navigation for survey pings if needed */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && !store->mul_group_mbsystemnav) {
		/* get timestamp */
		time_d = store->mul_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * store->mul_usec;

		/* interpolate heading */
		mb_hedint_interp(verbose, mbio_ptr, time_d, &heading, error);

		/* get speed if possible */
		if (store->nav_group_log)
			speed = 3.6 * store->nav_log_speed;
		else if (store->nav_group_motiongt)
			speed = 3.6 * store->nav_speed_ground;
		else if (store->nav_group_motiontw)
			speed = 3.6 * store->nav_speed_water;
		else
			speed = 0.0;

		/* interpolate position */
		mb_navint_interp(verbose, mbio_ptr, time_d, heading, speed, &lon, &lat, &speed, error);

		/* set values */
		store->mul_lon = DTR * lon;
		store->mul_lat = DTR * lat;
		store->mul_heading = DTR * heading;
		store->mul_speed = speed / 3.6;
		store->mul_group_mbsystemnav = true;
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
int mbr_l3xseraw_wr_nav(int verbose, int *buffer_size, char *buffer, void *store_ptr, int *error) {
	int index = 0;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       nav_source:          %d\n", store->nav_source);
		fprintf(stderr, "dbg5       nav_sec:             %u\n", store->nav_sec);
		fprintf(stderr, "dbg5       nav_usec:            %u\n", store->nav_usec);
		fprintf(stderr, "dbg5       nav_quality:         %d\n", store->nav_quality);
		fprintf(stderr, "dbg5       nav_status:          %d\n", store->nav_status);
		fprintf(stderr, "dbg5       nav_description_len: %d\n", store->nav_description_len);
		fprintf(stderr, "dbg5       nav_description:     %s\n", store->nav_description);
		fprintf(stderr, "dbg5       nav_x:               %f\n", store->nav_x);
		fprintf(stderr, "dbg5       nav_y:               %f\n", store->nav_y);
		fprintf(stderr, "dbg5       nav_z:               %f\n", store->nav_z);
		fprintf(stderr, "dbg5       nav_speed_ground:    %f\n", store->nav_speed_ground);
		fprintf(stderr, "dbg5       nav_course_ground:   %f\n", store->nav_course_ground);
		fprintf(stderr, "dbg5       nav_speed_water:     %f\n", store->nav_speed_water);
		fprintf(stderr, "dbg5       nav_course_water:    %f\n", store->nav_course_water);
	}

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;

	/* get frame time */
	frame_id = MBSYS_XSE_NAV_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->nav_source, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->nav_sec, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->nav_usec, &buffer[index]);
	index += 4;
	frame_count += 16;

	/*****************************************/

	/* write general group */
	if (store->nav_group_general) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get pos group */
		group_id = MBSYS_XSE_NAV_GROUP_GEN;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->nav_quality, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->nav_status, &buffer[index]);
		index += 4;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += store->nav_description_len + 32;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write position group */
	if (store->nav_group_position) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get pos group */
		group_id = MBSYS_XSE_NAV_GROUP_POS;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->nav_description_len, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->nav_description_len; i++) {
			buffer[index] = store->nav_description[i];
			index++;
		}
		mb_put_binary_double(SWAPFLAG, store->nav_x, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_y, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_z, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += store->nav_description_len + 32;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write accuracy group */
	if (store->nav_group_accuracy) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get pos group */
		group_id = MBSYS_XSE_NAV_GROUP_ACCURACY;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_short(SWAPFLAG, store->nav_acc_quality, &buffer[index]);
		index += 2;
		buffer[index] = store->nav_acc_numsatellites;
		index++;
		mb_put_binary_float(SWAPFLAG, store->nav_acc_horizdilution, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->nav_acc_diffage, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->nav_acc_diffref, &buffer[index]);
		index += 4;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 19;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write motion ground truth group */
	if (store->nav_group_motiongt) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion ground truth group */
		group_id = MBSYS_XSE_NAV_GROUP_MOTIONGT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_speed_ground, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_course_ground, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write motion through water group */
	if (store->nav_group_motiontw) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_MOTIONTW;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_speed_water, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_course_water, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write track steering group */
	if (store->nav_group_track) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_TRACK;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_offset_track, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_offset_sol, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_offset_eol, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_distance_sol, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_azimuth_sol, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_distance_eol, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_azimuth_eol, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write heave roll pitch group */
	if (store->nav_group_hrp) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_HRP;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_hrp_heave, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_hrp_roll, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_hrp_pitch, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write heave group */
	if (store->nav_group_hrp) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_HEAVE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_hea_heave, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write roll group */
	if (store->nav_group_roll) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_ROLL;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_rol_roll, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write pitch group */
	if (store->nav_group_pitch) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_PITCH;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_pit_pitch, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write heading group */
	if (store->nav_group_heading) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_HEADING;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_hdg_heading, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 12;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write speed log group */
	if (store->nav_group_log) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_LOG;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_log_speed, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 12;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* write gps altitude group */
	if (store->nav_group_gps) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_GPS;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->nav_gps_altitude, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->nav_gps_geoidalseparation, &buffer[index]);
		index += 4;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 12;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

/*****************************************/

/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);

	/* set buffer size */
	*buffer_size = index;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       buffer_size:%d\n", *buffer_size);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_svp(int verbose, int *buffer_size, char *buffer, void *store_ptr, int *error) {
	int index = 0;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       svp_source:          %d\n", store->svp_source);
		fprintf(stderr, "dbg5       svp_sec:             %u\n", store->svp_sec);
		fprintf(stderr, "dbg5       svp_usec:            %u\n", store->svp_usec);
		fprintf(stderr, "dbg5       svp_nsvp:            %d\n", store->svp_nsvp);
		fprintf(stderr, "dbg5       svp_nctd:            %d\n", store->svp_nctd);
		fprintf(stderr, "dbg5       svp_ssv:             %f\n", store->svp_ssv);
		for (int i = 0; i < store->svp_nsvp; i++)
			fprintf(stderr, "dbg5       svp[%d]:	        %f %f\n", i, store->svp_depth[i], store->svp_velocity[i]);
		for (int i = 0; i < store->svp_nctd; i++)
			fprintf(stderr, "dbg5       cstd[%d]:        %f %f %f %f\n", i, store->svp_conductivity[i], store->svp_salinity[i],
			        store->svp_temperature[i], store->svp_pressure[i]);
	}

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;

	/* get frame time */
	frame_id = MBSYS_XSE_SVP_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->svp_source, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->svp_sec, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->svp_usec, &buffer[index]);
	index += 4;
	frame_count += 16;

	/* get depth and velocity groups */
	if (store->svp_nsvp > 0) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get depth array */
		group_id = MBSYS_XSE_SVP_GROUP_DEPTH;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->svp_nsvp, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->svp_nsvp; i++) {
			mb_put_binary_double(SWAPFLAG, store->svp_depth[i], &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->svp_nsvp * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;

/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get depth array */
		group_id = MBSYS_XSE_SVP_GROUP_VELOCITY;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->svp_nsvp, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->svp_nsvp; i++) {
			mb_put_binary_double(SWAPFLAG, store->svp_velocity[i], &buffer[index]);
			index += 8;
		}
		mb_put_binary_double(SWAPFLAG, store->svp_ssv_depth, &buffer[index]);
		index += 8;
		buffer[index] = store->svp_ssv_depthflag;
		index++;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 17 + store->svp_nsvp * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	bool write_conductivity = false;
	bool write_salinity = false;
	bool write_temperature = false;
	bool write_pressure = false;

	/* figure out which ctd groups are nonzero */
	if (store->svp_nctd > 0) {
		for (int i = 0; i < store->svp_nctd; i++) {
			if (store->svp_conductivity[i] != 0.0)
				write_conductivity = true;
			if (store->svp_salinity[i] != 0.0)
				write_salinity = true;
			if (store->svp_temperature[i] != 0.0)
				write_temperature = true;
			if (store->svp_pressure[i] != 0.0)
				write_pressure = true;
		}
	}

	/* get conductivity group */
	if (store->svp_nctd > 0 && write_conductivity) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get conductivity array */
		group_id = MBSYS_XSE_SVP_GROUP_CONDUCTIVITY;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->svp_nctd, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->svp_nctd; i++) {
			mb_put_binary_double(SWAPFLAG, store->svp_conductivity[i], &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->svp_nctd * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get salinity group */
	if (store->svp_nctd > 0 && write_salinity) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get salinity array */
		group_id = MBSYS_XSE_SVP_GROUP_SALINITY;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->svp_nctd, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->svp_nctd; i++) {
			mb_put_binary_double(SWAPFLAG, store->svp_salinity[i], &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->svp_nctd * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get temperature group */
	if (store->svp_nctd > 0 && write_temperature) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get temperature array */
		group_id = MBSYS_XSE_SVP_GROUP_TEMP;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->svp_nctd, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->svp_nctd; i++) {
			mb_put_binary_double(SWAPFLAG, store->svp_temperature[i], &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->svp_nctd * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get pressure group */
	if (store->svp_nctd > 0 && write_pressure) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get pressure array */
		group_id = MBSYS_XSE_SVP_GROUP_PRESSURE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->svp_nctd, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->svp_nctd; i++) {
			mb_put_binary_double(SWAPFLAG, store->svp_pressure[i], &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->svp_nctd * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get ssv group */
	if (store->svp_ssv > 0.0) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get ssv */
		group_id = MBSYS_XSE_SVP_GROUP_SSV;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->svp_ssv, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->svp_ssv_depth, &buffer[index]);
		index += 8;
		buffer[index] = store->svp_ssv_depthflag;
		index++;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 21;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);

	/* set buffer size */
	*buffer_size = index;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       buffer_size:%d\n", *buffer_size);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_ship(int verbose, int *buffer_size, char *buffer, void *store_ptr, int *error) {
	int index = 0;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id = 0;
	int nchar;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       par_source:          %d\n", store->par_source);
		fprintf(stderr, "dbg5       par_sec:             %u\n", store->par_sec);
		fprintf(stderr, "dbg5       par_usec:            %u\n", store->par_usec);
		fprintf(stderr, "dbg5       par_ship_name:       %s\n", store->par_ship_name);
		fprintf(stderr, "dbg5       par_ship_length:     %f\n", store->par_ship_length);
		fprintf(stderr, "dbg5       par_ship_beam:       %f\n", store->par_ship_beam);
		fprintf(stderr, "dbg5       par_ship_draft:      %f\n", store->par_ship_draft);
		fprintf(stderr, "dbg5       par_ship_height:     %f\n", store->par_ship_height);
		fprintf(stderr, "dbg5       par_ship_displacement: %f\n", store->par_ship_displacement);
		fprintf(stderr, "dbg5       par_ship_weight:     %f\n", store->par_ship_weight);
		for (int i = 0; i < store->par_ship_nsensor; i++) {
			fprintf(stderr, "dbg5       par_ship_sensor_id[%d]:        %d\n", i, store->par_ship_sensor_id[i]);
			fprintf(stderr, "dbg5       par_ship_sensor_type[%d]:      %d\n", i, store->par_ship_sensor_type[i]);
			fprintf(stderr, "dbg5       par_ship_sensor_frequency[%d]: %d\n", i, store->par_ship_sensor_frequency[i]);
		}
		fprintf(stderr, "dbg5       par_parameter:       %d\n", store->par_parameter);
		fprintf(stderr, "dbg5       par_roll_bias:       %f\n", store->par_roll_bias);
		fprintf(stderr, "dbg5       par_pitch_bias:      %f\n", store->par_pitch_bias);
		fprintf(stderr, "dbg5       par_heading_bias:    %f\n", store->par_heading_bias);
		fprintf(stderr, "dbg5       par_time_delay:      %f\n", store->par_time_delay);
		fprintf(stderr, "dbg5       par_trans_x_port:    %f\n", store->par_trans_x_port);
		fprintf(stderr, "dbg5       par_trans_y_port:    %f\n", store->par_trans_y_port);
		fprintf(stderr, "dbg5       par_trans_z_port:    %f\n", store->par_trans_z_port);
		fprintf(stderr, "dbg5       par_trans_x_stbd:    %f\n", store->par_trans_x_stbd);
		fprintf(stderr, "dbg5       par_trans_y_stbd:    %f\n", store->par_trans_y_stbd);
		fprintf(stderr, "dbg5       par_trans_z_stbd:    %f\n", store->par_trans_z_stbd);
		fprintf(stderr, "dbg5       par_trans_err_port:  %f\n", store->par_trans_err_port);
		fprintf(stderr, "dbg5       par_trans_err_stbd:  %f\n", store->par_trans_err_stbd);
		fprintf(stderr, "dbg5       par_nav_x:           %f\n", store->par_nav_x);
		fprintf(stderr, "dbg5       par_nav_y:           %f\n", store->par_nav_y);
		fprintf(stderr, "dbg5       par_nav_z:           %f\n", store->par_nav_z);
		fprintf(stderr, "dbg5       par_hrp_x:           %f\n", store->par_hrp_x);
		fprintf(stderr, "dbg5       par_hrp_y:           %f\n", store->par_hrp_y);
		fprintf(stderr, "dbg5       par_hrp_z:           %f\n", store->par_hrp_z);
		fprintf(stderr, "dbg5       par_navigationandmotion: %d\n", store->par_navigationandmotion);
		fprintf(stderr, "dbg5       par_nam_roll_bias:       %f\n", store->par_nam_roll_bias);
		fprintf(stderr, "dbg5       par_nam_pitch_bias:      %f\n", store->par_nam_pitch_bias);
		fprintf(stderr, "dbg5       par_nam_heave_bias:      %f\n", store->par_nam_heave_bias);
		fprintf(stderr, "dbg5       par_nam_heading_bias:    %f\n", store->par_nam_heading_bias);
		fprintf(stderr, "dbg5       par_nam_time_delay:      %f\n", store->par_nam_time_delay);
		fprintf(stderr, "dbg5       par_nam_nav_x:           %f\n", store->par_nam_nav_x);
		fprintf(stderr, "dbg5       par_nam_nav_y:           %f\n", store->par_nam_nav_y);
		fprintf(stderr, "dbg5       par_nam_nav_z:           %f\n", store->par_nam_nav_z);
		fprintf(stderr, "dbg5       par_nam_hrp_x:           %f\n", store->par_nam_hrp_x);
		fprintf(stderr, "dbg5       par_nam_hrp_y:           %f\n", store->par_nam_hrp_y);
		fprintf(stderr, "dbg5       par_nam_hrp_z:           %f\n", store->par_nam_hrp_z);
		fprintf(stderr, "dbg5       par_xdr_num_transducer:  %d\n", store->par_xdr_num_transducer);
		fprintf(stderr, "dbg5       # sensor xducer freq side roll pitch azi dist\n");
		for (int i = 0; i < store->par_xdr_num_transducer; i++)
			fprintf(stderr, "dbg5       %d %d %d %d %d %f %f %f %f\n", i, store->par_xdr_sensorid[i],
			        store->par_xdr_transducer[i], store->par_xdr_frequency[i], store->par_xdr_side[i],
			        store->par_xdr_mountingroll[i], store->par_xdr_mountingpitch[i], store->par_xdr_mountingazimuth[i],
			        store->par_xdr_mountingdistance[i]);
		fprintf(stderr, "dbg5       # x y z roll pitch azimuth\n");
		for (int i = 0; i < store->par_xdr_num_transducer; i++)
			fprintf(stderr, "dbg5       %d %f %f %f %f %f %f\n", i, store->par_xdr_x[i], store->par_xdr_y[i], store->par_xdr_z[i],
			        store->par_xdr_roll[i], store->par_xdr_pitch[i], store->par_xdr_azimuth[i]);
		fprintf(stderr, "dbg5       par_xdx_num_transducer:  %d\n", store->par_xdx_num_transducer);
		fprintf(stderr, "dbg5       # roll pitch azimuth\n");
		for (int i = 0; i < store->par_xdx_num_transducer; i++)
			fprintf(stderr, "dbg5       %d %d %d %d\n", i, store->par_xdx_roll[i], store->par_xdx_pitch[i],
			        store->par_xdx_azimuth[i]);
	}

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;

	/* get frame time */
	frame_id = MBSYS_XSE_SHP_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->par_source, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->par_sec, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->par_usec, &buffer[index]);
	index += 4;
	frame_count += 16;

/*****************************************/

/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* start the group byte count, but don't write it to buffer yet */
	/* mark the byte count spot in the buffer and increment index so we skip it */
	group_count = 0;
	group_cnt_index = index;
	index += 4;

	/* get general group */
	group_id = MBSYS_XSE_SHP_GROUP_GEN;
	mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
	index += 4;
	nchar = strlen(store->par_ship_name);
	mb_put_binary_int(SWAPFLAG, nchar, &buffer[index]);
	index += 4;
	for (int i = 0; i < nchar; i++) {
		buffer[index] = store->par_ship_name[i];
		index++;
	}
	mb_put_binary_double(SWAPFLAG, store->par_ship_length, &buffer[index]);
	index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_beam, &buffer[index]);
	index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_draft, &buffer[index]);
	index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_height, &buffer[index]);
	index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_displacement, &buffer[index]);
	index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_weight, &buffer[index]);
	index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;
	group_count += nchar + 56;

	/* go back and fill in group byte count */
	mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	/* add group count to the frame count */
	frame_count += group_count + 12;

	/*****************************************/

	if (store->par_ship_nsensor > 0) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get sensors group */
		group_id = MBSYS_XSE_SHP_GROUP_SENSORS;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->par_ship_nsensor, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->par_ship_nsensor; i++) {
			mb_put_binary_int(SWAPFLAG, store->par_ship_sensor_id[i], &buffer[index]);
			index += 4;
		}
		for (int i = 0; i < store->par_ship_nsensor; i++) {
			mb_put_binary_int(SWAPFLAG, store->par_ship_sensor_type[i], &buffer[index]);
			index += 4;
		}
		for (int i = 0; i < store->par_ship_nsensor; i++) {
			mb_put_binary_int(SWAPFLAG, store->par_ship_sensor_frequency[i], &buffer[index]);
			index += 4;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 12 * store->par_ship_nsensor + 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	if (store->par_parameter) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get parameter group */
		group_id = MBSYS_XSE_SHP_GROUP_PARAMETER;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_roll_bias, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_pitch_bias, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_heading_bias, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_time_delay, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_x_port, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_y_port, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_z_port, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_x_stbd, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_y_stbd, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_z_stbd, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_err_port, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_err_stbd, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_nav_x, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_nav_y, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_nav_z, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_hrp_x, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_hrp_y, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_hrp_z, &buffer[index]);
		index += 4;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 76;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	if (store->par_navigationandmotion) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get parameter group */
		group_id = MBSYS_XSE_SHP_GROUP_NAVIGATIONANDMOTION;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->par_nam_roll_bias, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_pitch_bias, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_heave_bias, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_heading_bias, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_time_delay, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_nav_x, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_nav_y, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_nav_z, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_hrp_x, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_hrp_y, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_hrp_z, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 92;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	if (store->par_xdr_num_transducer > 0) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get parameter group */
		group_id = MBSYS_XSE_SHP_GROUP_TRANSDUCER;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->par_xdr_num_transducer, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->par_xdr_num_transducer; i++) {
			mb_put_binary_int(SWAPFLAG, store->par_xdr_sensorid[i], &buffer[index]);
			index += 4;
			mb_put_binary_int(SWAPFLAG, store->par_xdr_frequency[i], &buffer[index]);
			index += 4;
			buffer[index] = store->par_xdr_transducer[i];
			index++;
			buffer[index] = store->par_xdr_side[i];
			index++;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_mountingroll[i], &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_mountingpitch[i], &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_mountingazimuth[i], &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_mountingdistance[i], &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_x[i], &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_y[i], &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_z[i], &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_roll[i], &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_pitch[i], &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_azimuth[i], &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->par_xdr_num_transducer * 90;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	if (store->par_xdx_num_transducer > 0) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get parameter group */
		group_id = MBSYS_XSE_SHP_GROUP_TRANSDUCEREXTENDED;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->par_xdx_num_transducer, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->par_xdx_num_transducer; i++) {
			buffer[index] = store->par_xdx_roll[i];
			index++;
			buffer[index] = store->par_xdx_pitch[i];
			index++;
			buffer[index] = store->par_xdx_azimuth[i];
			index++;
			for (int j = 0; j < 48; j++) {
				buffer[index] = 0;
				index++;
			}
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->par_xdx_num_transducer * 51;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

/*****************************************/

/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);

	/* set buffer size */
	*buffer_size = index;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       buffer_size:%d\n", *buffer_size);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_multibeam(int verbose, int *buffer_size, char *buffer, void *store_ptr, int *error) {
	int index = 0;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       mul_group_beam:      %d\n", store->mul_group_beam);
		fprintf(stderr, "dbg5       mul_group_tt:        %d\n", store->mul_group_tt);
		fprintf(stderr, "dbg5       mul_group_quality:   %d\n", store->mul_group_quality);
		fprintf(stderr, "dbg5       mul_group_amp:       %d\n", store->mul_group_amp);
		fprintf(stderr, "dbg5       mul_group_delay:     %d\n", store->mul_group_delay);
		fprintf(stderr, "dbg5       mul_group_lateral:   %d\n", store->mul_group_lateral);
		fprintf(stderr, "dbg5       mul_group_along:     %d\n", store->mul_group_along);
		fprintf(stderr, "dbg5       mul_group_depth:     %d\n", store->mul_group_depth);
		fprintf(stderr, "dbg5       mul_group_angle:     %d\n", store->mul_group_angle);
		fprintf(stderr, "dbg5       mul_group_heave:     %d\n", store->mul_group_heave);
		fprintf(stderr, "dbg5       mul_group_roll:      %d\n", store->mul_group_roll);
		fprintf(stderr, "dbg5       mul_group_pitch:     %d\n", store->mul_group_pitch);
		fprintf(stderr, "dbg5       mul_group_gates:     %d\n", store->mul_group_gates);
		fprintf(stderr, "dbg5       mul_group_noise:     %d\n", store->mul_group_noise);
		fprintf(stderr, "dbg5       mul_group_length:    %d\n", store->mul_group_length);
		fprintf(stderr, "dbg5       mul_group_hits:      %d\n", store->mul_group_hits);
		fprintf(stderr, "dbg5       mul_group_heavereceive: %d\n", store->mul_group_heavereceive);
		fprintf(stderr, "dbg5       mul_group_azimuth:    %d\n", store->mul_group_azimuth);
		fprintf(stderr, "dbg5       mul_group_properties: %d\n", store->mul_group_properties);
		fprintf(stderr, "dbg5       mul_group_normamp:    %d\n", store->mul_group_normamp);
		fprintf(stderr, "dbg5       mul_group_mbsystemnav:  %d\n", store->mul_group_mbsystemnav);
		fprintf(stderr, "dbg5       mul_source:          %d\n", store->mul_source);
		fprintf(stderr, "dbg5       mul_sec:             %u\n", store->mul_sec);
		fprintf(stderr, "dbg5       mul_usec:            %u\n", store->mul_usec);
		fprintf(stderr, "dbg5       mul_ping:            %d\n", store->mul_ping);
		fprintf(stderr, "dbg5       mul_frequency:       %f\n", store->mul_frequency);
		fprintf(stderr, "dbg5       mul_pulse:           %f\n", store->mul_pulse);
		fprintf(stderr, "dbg5       mul_power:           %f\n", store->mul_power);
		fprintf(stderr, "dbg5       mul_bandwidth:       %f\n", store->mul_bandwidth);
		fprintf(stderr, "dbg5       mul_sample:          %f\n", store->mul_sample);
		fprintf(stderr, "dbg5       mul_swath:           %f\n", store->mul_swath);
		fprintf(stderr, "dbg5       mul_num_beams:       %d\n", store->mul_num_beams);
		fprintf(stderr, "dbg5       mul_lon:             %f\n", store->mul_lon);
		fprintf(stderr, "dbg5       mul_lat:             %f\n", store->mul_lat);
		fprintf(stderr, "dbg5       mul_heading:         %f\n", store->mul_heading);
		fprintf(stderr, "dbg5       mul_speed:           %f\n", store->mul_speed);
		for (int i = 0; i < store->mul_num_beams; i++)
			fprintf(stderr,
			        "dbg5       beam[%d]: %3d %7.2f %7.2f %7.2f %3d %3d %6.3f %6.2f %5.3f %5.2f %6.2f %6.2f  %f %f %f %f %f %d "
			        "%f %f %d %f\n",
			        i, store->beams[i].beam, store->beams[i].lateral, store->beams[i].along, store->beams[i].depth,
			        store->beams[i].amplitude, store->beams[i].quality, store->beams[i].tt, store->beams[i].angle,
			        store->beams[i].delay, store->beams[i].heave, store->beams[i].roll, store->beams[i].pitch,
			        store->beams[i].gate_angle, store->beams[i].gate_start, store->beams[i].gate_stop, store->beams[i].noise,
			        store->beams[i].length, store->beams[i].hits, store->beams[i].heavereceive, store->beams[i].azimuth,
			        store->beams[i].normamp, store->beams[i].frequency);
		fprintf(stderr, "dbg5       mul_num_properties: %d\n", store->mul_num_properties);
		for (int i = 0; i < store->mul_num_properties; i++)
			fprintf(stderr, "dbg5       mun_property[%d]: %d %f\n", i, store->mul_properties_type[i],
			        store->mul_properties_value[i]);
		for (int i = 0; i < 40; i++)
			fprintf(stderr, "dbg5       mul_properties_reserved[%d]: %d\n", i, store->mul_properties_reserved[i]);
		fprintf(stderr, "dbg5       mul_normamp_num_beams:        %d\n", store->mul_normamp_num_beams);
		fprintf(stderr, "dbg5       mul_normamp_flags:            %d\n", store->mul_normamp_flags);
		fprintf(stderr, "dbg5       mul_normamp_along_beamwidth:  %f\n", store->mul_normamp_along_beamwidth);
		fprintf(stderr, "dbg5       mul_normamp_across_beamwidth: %f\n", store->mul_normamp_across_beamwidth);
	}

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;

	/* get frame time */
	frame_id = MBSYS_XSE_MBM_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->mul_source, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->mul_sec, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->mul_usec, &buffer[index]);
	index += 4;
	frame_count += 16;

/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* start the group byte count, but don't write it to buffer yet */
	/* mark the byte count spot in the buffer and increment index so we skip it */
	group_count = 0;
	group_cnt_index = index;
	index += 4;

	/* get general group */
	group_id = MBSYS_XSE_MBM_GROUP_GEN;
	mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->mul_ping, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_frequency, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_pulse, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_power, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_bandwidth, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_sample, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_swath, &buffer[index]);
	index += 4;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;
	group_count += 32;

	/* go back and fill in group byte count */
	mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	/* add group count to the frame count */
	frame_count += group_count + 12;

	/* get beam groups */
	if (store->mul_group_beam) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get beam array */
		group_id = MBSYS_XSE_MBM_GROUP_BEAM;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_short(SWAPFLAG, store->beams[i].beam, &buffer[index]);
			index += 2;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 2;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get tt groups */
	if (store->mul_group_tt) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get tt array */
		group_id = MBSYS_XSE_MBM_GROUP_TT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].tt, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get quality groups */
	if (store->mul_group_quality) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get quality array */
		group_id = MBSYS_XSE_MBM_GROUP_QUALITY;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			buffer[index] = store->beams[i].quality;
			index++;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += sizeof(int);
		group_count += 8 + store->mul_num_beams;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get amplitude groups */
	if (store->mul_group_amp) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_MBM_GROUP_AMP;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_short(SWAPFLAG, store->beams[i].amplitude, &buffer[index]);
			index += 2;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 2;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get delay groups */
	if (store->mul_group_delay) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get delay array */
		group_id = MBSYS_XSE_MBM_GROUP_DELAY;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].delay, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get lateral groups */
	if (store->mul_group_lateral) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get lateral array */
		group_id = MBSYS_XSE_MBM_GROUP_LATERAL;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].lateral, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get along groups */
	if (store->mul_group_along) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get along array */
		group_id = MBSYS_XSE_MBM_GROUP_ALONG;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].along, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += sizeof(int);
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get depth groups */
	if (store->mul_group_depth) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get depth array */
		group_id = MBSYS_XSE_MBM_GROUP_DEPTH;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].depth, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += sizeof(int);
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get angle groups */
	if (store->mul_group_angle) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get angle array */
		group_id = MBSYS_XSE_MBM_GROUP_ANGLE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].angle, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get heave groups */
	if (store->mul_group_heave) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get heave array */
		group_id = MBSYS_XSE_MBM_GROUP_HEAVE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].heave, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get roll groups */
	if (store->mul_group_roll) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get roll array */
		group_id = MBSYS_XSE_MBM_GROUP_ROLL;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].roll, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get pitch groups */
	if (store->mul_group_pitch) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get pitch array */
		group_id = MBSYS_XSE_MBM_GROUP_PITCH;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].pitch, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get gates groups */
	if (store->mul_group_gates) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get gates array */
		group_id = MBSYS_XSE_MBM_GROUP_GATES;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].gate_angle, &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->beams[i].gate_start, &buffer[index]);
			index += 8;
			mb_put_binary_double(SWAPFLAG, store->beams[i].gate_stop, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 24;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get noise groups */
	if (store->mul_group_noise) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get noise array */
		group_id = MBSYS_XSE_MBM_GROUP_NOISE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_float(SWAPFLAG, store->beams[i].noise, &buffer[index]);
			index += 4;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get length groups */
	if (store->mul_group_length) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get length array */
		group_id = MBSYS_XSE_MBM_GROUP_LENGTH;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_float(SWAPFLAG, store->beams[i].length, &buffer[index]);
			index += 4;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get hits groups */
	if (store->mul_group_hits) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get hits array */
		group_id = MBSYS_XSE_MBM_GROUP_HITS;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_int(SWAPFLAG, store->beams[i].hits, &buffer[index]);
			index += 4;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get heavereceive groups */
	if (store->mul_group_heavereceive) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get heavereceive array */
		group_id = MBSYS_XSE_MBM_GROUP_HEAVERECEIVE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].heavereceive, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get azimuth groups */
	if (store->mul_group_azimuth) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get azimuth array */
		group_id = MBSYS_XSE_MBM_GROUP_AZIMUTH;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_beams; i++) {
			mb_put_binary_double(SWAPFLAG, store->beams[i].azimuth, &buffer[index]);
			index += 8;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_beams * 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get properties groups */
	if (store->mul_group_properties) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get properties array */
		group_id = MBSYS_XSE_MBM_GROUP_PROPERTIES;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_num_properties, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_num_properties; i++) {
			mb_put_binary_short(SWAPFLAG, store->mul_properties_type[i], &buffer[index]);
			index += 2;
			mb_put_binary_double(SWAPFLAG, store->mul_properties_value[i], &buffer[index]);
			index += 8;
		}
		for (int i = 0; i < 40; i++) {
			buffer[index] = store->mul_properties_reserved[i];
			index++;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->mul_num_properties * 10 + 40;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get normalized amplitude groups */
	if (store->mul_group_normamp) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get properties array */
		group_id = MBSYS_XSE_MBM_GROUP_NORMAMP;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_normamp_num_beams, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->mul_normamp_flags, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->mul_normamp_along_beamwidth, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->mul_normamp_across_beamwidth, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->mul_normamp_num_beams; i++) {
			mb_put_binary_short(SWAPFLAG, store->beams[i].normamp, &buffer[index]);
			index += 2;
		}
		if (store->mul_normamp_flags == 0) {
			mb_put_binary_float(SWAPFLAG, store->beams[0].frequency, &buffer[index]);
			index += 4;
		}
		else
			for (int i = 0; i < store->mul_normamp_num_beams; i++) {
				mb_put_binary_float(SWAPFLAG, store->beams[i].frequency, &buffer[index]);
				index += 4;
			}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		if (store->mul_normamp_flags == 0)
			group_count += 24 + store->mul_normamp_num_beams * 2;
		else
			group_count += 20 + store->mul_normamp_num_beams * 2 + store->mul_normamp_num_beams * 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* get mbsystemnav groups */
	if (store->mul_group_mbsystemnav) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get azimuth array */
		group_id = MBSYS_XSE_MBM_GROUP_MBSYSTEMNAV;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->mul_lon, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->mul_lat, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->mul_heading, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->mul_speed, &buffer[index]);
		index += 8;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 36;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);

	/* set buffer size */
	*buffer_size = index;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       buffer_size:%d\n", *buffer_size);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_sidescan(int verbose, int *buffer_size, char *buffer, void *store_ptr, int *error) {
	int index = 0;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sid_frame:            %d\n", store->sid_frame);
		fprintf(stderr, "dbg5       sid_group_avt:        %d\n", store->sid_group_avt);
		fprintf(stderr, "dbg5       sid_group_pvt:        %d\n", store->sid_group_pvt);
		fprintf(stderr, "dbg5       sid_group_avl:        %d\n", store->sid_group_avl);
		fprintf(stderr, "dbg5       sid_group_pvl:        %d\n", store->sid_group_pvl);
		fprintf(stderr, "dbg5       sid_group_signal:     %d\n", store->sid_group_signal);
		fprintf(stderr, "dbg5       sid_group_ping:       %d\n", store->sid_group_ping);
		fprintf(stderr, "dbg5       sid_group_complex:    %d\n", store->sid_group_complex);
		fprintf(stderr, "dbg5       sid_group_weighting:  %d\n", store->sid_group_weighting);
		fprintf(stderr, "dbg5       sid_source:           %d\n", store->sid_source);
		fprintf(stderr, "dbg5       sid_sec:              %d\n", store->sid_sec);
		fprintf(stderr, "dbg5       sid_usec:             %u\n", store->sid_usec);
		fprintf(stderr, "dbg5       sid_ping:             %u\n", store->sid_ping);
		fprintf(stderr, "dbg5       sid_frequency:        %f\n", store->sid_frequency);
		fprintf(stderr, "dbg5       sid_pulse:            %f\n", store->sid_pulse);
		fprintf(stderr, "dbg5       sid_power:            %f\n", store->sid_power);
		fprintf(stderr, "dbg5       sid_bandwidth:        %f\n", store->sid_bandwidth);
		fprintf(stderr, "dbg5       sid_sample:           %f\n", store->sid_sample);
		fprintf(stderr, "dbg5       sid_avt_sampleus:     %d\n", store->sid_avt_sampleus);
		fprintf(stderr, "dbg5       sid_avt_offset:       %d\n", store->sid_avt_offset);
		fprintf(stderr, "dbg5       sid_avt_num_samples:  %d\n", store->sid_avt_num_samples);
		for (int i = 0; i < store->sid_avt_num_samples; i++)
			fprintf(stderr, "dbg5       sid_avt_amp[%d]:%d\n", i, store->sid_avt_amp[i]);
		fprintf(stderr, "dbg5       sid_pvt_sampleus:  %d\n", store->sid_pvt_sampleus);
		fprintf(stderr, "dbg5       sid_pvt_offset:  %d\n", store->sid_pvt_offset);
		fprintf(stderr, "dbg5       sid_pvt_num_samples:  %d\n", store->sid_pvt_num_samples);
		for (int i = 0; i < store->sid_pvt_num_samples; i++)
			fprintf(stderr, "dbg5       sid_pvt_phase[%d]:%d\n", i, store->sid_pvt_phase[i]);
		fprintf(stderr, "dbg5       sid_avl_binsize:  %d\n", store->sid_avl_binsize);
		fprintf(stderr, "dbg5       sid_avl_offset:  %d\n", store->sid_avl_offset);
		fprintf(stderr, "dbg5       sid_avl_num_samples:  %d\n", store->sid_avl_num_samples);
		for (int i = 0; i < store->sid_avl_num_samples; i++)
			fprintf(stderr, "dbg5       sid_avl_amp[%d]:%d\n", i, store->sid_avl_amp[i]);
		fprintf(stderr, "dbg5       sid_pvl_binsize:  %d\n", store->sid_pvl_binsize);
		fprintf(stderr, "dbg5       sid_pvl_offset:  %d\n", store->sid_pvl_offset);
		fprintf(stderr, "dbg5       sid_pvl_num_samples:  %d\n", store->sid_pvl_num_samples);
		for (int i = 0; i < store->sid_pvl_num_samples; i++)
			fprintf(stderr, "dbg5       sid_pvl_phase[%d]:%d\n", i, store->sid_pvl_phase[i]);
		fprintf(stderr, "dbg5       sid_sig_ping:  %d\n", store->sid_sig_ping);
		fprintf(stderr, "dbg5       sid_sig_channel:  %d\n", store->sid_sig_channel);
		fprintf(stderr, "dbg5       sid_sig_offset:  %f\n", store->sid_sig_offset);
		fprintf(stderr, "dbg5       sid_sig_sample:  %f\n", store->sid_sig_sample);
		fprintf(stderr, "dbg5       sid_sig_num_samples:  %d\n", store->sid_sig_num_samples);
		for (int i = 0; i < store->sid_sig_num_samples; i++)
			fprintf(stderr, "dbg5       sid_sig_phase[%d]:%d\n", i, store->sid_sig_phase[i]);
		fprintf(stderr, "dbg5       sid_png_pulse:  %u\n", store->sid_png_pulse);
		fprintf(stderr, "dbg5       sid_png_startfrequency:  %f\n", store->sid_png_startfrequency);
		fprintf(stderr, "dbg5       sid_png_endfrequency:  %f\n", store->sid_png_endfrequency);
		fprintf(stderr, "dbg5       sid_png_duration:  %f\n", store->sid_png_duration);
		fprintf(stderr, "dbg5       sid_png_mancode:  %d\n", store->sid_png_mancode);
		fprintf(stderr, "dbg5       sid_png_pulseid:  %d\n", store->sid_png_pulseid);
		fprintf(stderr, "dbg5       sid_png_pulsename:  %s\n", store->sid_png_pulsename);
		fprintf(stderr, "dbg5       sid_cmp_ping:  %d\n", store->sid_cmp_ping);
		fprintf(stderr, "dbg5       sid_cmp_channel:  %d\n", store->sid_cmp_channel);
		fprintf(stderr, "dbg5       sid_cmp_offset:  %f\n", store->sid_cmp_offset);
		fprintf(stderr, "dbg5       sid_cmp_sample:  %f\n", store->sid_cmp_sample);
		fprintf(stderr, "dbg5       sid_cmp_num_samples:  %d\n", store->sid_cmp_num_samples);
		for (int i = 0; i < store->sid_sig_num_samples; i++)
			fprintf(stderr, "dbg5       sid_cmp_real[%d]:%d sid_cmp_imaginary[%d]:%d\n", i, store->sid_cmp_real[i], i,
			        store->sid_cmp_imaginary[i]);
		fprintf(stderr, "dbg5       sid_wgt_factorleft:  %d\n", store->sid_wgt_factorleft);
		fprintf(stderr, "dbg5       sid_wgt_samplesleft:  %d\n", store->sid_wgt_samplesleft);
		fprintf(stderr, "dbg5       sid_wgt_factorright:  %d\n", store->sid_wgt_factorright);
		fprintf(stderr, "dbg5       sid_wgt_samplesright:  %d\n", store->sid_wgt_samplesright);
	}

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;

	/* get frame time */
	frame_id = MBSYS_XSE_SSN_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->sid_source, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->sid_sec, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->sid_usec, &buffer[index]);
	index += 4;
	frame_count += 16;

/*****************************************/

/* get general group */
/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* start the group byte count, but don't write it to buffer yet */
	/* mark the byte count spot in the buffer and increment index so we skip it */
	group_count = 0;
	group_cnt_index = index;
	index += 4;

	/* get general group */
	group_id = MBSYS_XSE_SSN_GROUP_GEN;
	mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->sid_ping, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_frequency, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_pulse, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_power, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_bandwidth, &buffer[index]);
	index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_sample, &buffer[index]);
	index += 4;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;
	group_count += 28;

	/* go back and fill in group byte count */
	mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	/* add group count to the frame count */
	frame_count += group_count + 12;

	/*****************************************/

	/* get amplitude vs traveltime group */
	if (store->sid_group_avt) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_AMPVSTT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avt_sampleus, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avt_offset, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avt_num_samples, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->sid_avt_num_samples; i++) {
			mb_put_binary_short(SWAPFLAG, store->sid_avt_amp[i], &buffer[index]);
			index += 2;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16 + 2 * store->sid_avt_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* get phase vs traveltime group */
	if (store->sid_group_avt) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_PHASEVSTT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvt_sampleus, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvt_offset, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvt_num_samples, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->sid_pvt_num_samples; i++) {
			mb_put_binary_short(SWAPFLAG, store->sid_pvt_phase[i], &buffer[index]);
			index += 2;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16 + 2 * store->sid_pvt_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* get amplitude vs lateral group */
	if (store->sid_group_avl) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_AMPVSLAT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avl_binsize, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avl_offset, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avl_num_samples, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->sid_avl_num_samples; i++) {
			mb_put_binary_short(SWAPFLAG, store->sid_avl_amp[i], &buffer[index]);
			index += 2;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16 + 2 * store->sid_avl_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* get phase vs lateral group */
	if (store->sid_group_pvl) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_PHASEVSLAT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvl_binsize, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvl_offset, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvl_num_samples, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->sid_pvl_num_samples; i++) {
			mb_put_binary_short(SWAPFLAG, store->sid_pvl_phase[i], &buffer[index]);
			index += 2;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16 + 2 * store->sid_pvl_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* get signal group */
	if (store->sid_group_signal) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_SIGNAL;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_sig_ping, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_sig_channel, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->sid_sig_offset, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->sid_sig_sample, &buffer[index]);
		index += 8;
		mb_put_binary_int(SWAPFLAG, store->sid_sig_num_samples, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->sid_sig_num_samples; i++) {
			mb_put_binary_short(SWAPFLAG, store->sid_sig_phase[i], &buffer[index]);
			index += 2;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 32 + 2 * store->sid_sig_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* get ping group */
	if (store->sid_group_ping) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_PINGTYPE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->sid_png_startfrequency, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->sid_png_endfrequency, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->sid_png_duration, &buffer[index]);
		index += 8;
		mb_put_binary_int(SWAPFLAG, store->sid_png_mancode, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_png_pulseid, &buffer[index]);
		index += 4;
		strcpy(&buffer[index], store->sid_png_pulsename);
		index += strlen(store->sid_png_pulsename);
		buffer[index] = 0;
		index++;
		if (strlen(store->sid_png_pulsename) % 2 > 0) {
			buffer[index] = 0;
			index++;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 36 + strlen(store->sid_png_pulsename) + 1 + (strlen(store->sid_png_pulsename) % 2);

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* get complex signal group */
	if (store->sid_group_ping) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_COMPLEXSIGNAL;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_cmp_ping, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_cmp_channel, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->sid_cmp_offset, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->sid_cmp_sample, &buffer[index]);
		index += 8;
		mb_put_binary_int(SWAPFLAG, store->sid_cmp_num_samples, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->sid_cmp_num_samples; i++) {
			mb_put_binary_short(SWAPFLAG, store->sid_cmp_real[i], &buffer[index]);
			index += 2;
			mb_put_binary_short(SWAPFLAG, store->sid_cmp_imaginary[i], &buffer[index]);
			index += 2;
		}

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 32 + 4 * store->sid_cmp_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/*****************************************/

	/* get weighting group */
	if (store->sid_group_weighting) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_WEIGHTING;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_short(SWAPFLAG, store->sid_wgt_factorleft, &buffer[index]);
		index += 2;
		mb_put_binary_int(SWAPFLAG, store->sid_wgt_samplesleft, &buffer[index]);
		index += 4;
		mb_put_binary_short(SWAPFLAG, store->sid_wgt_factorright, &buffer[index]);
		index += 2;
		mb_put_binary_int(SWAPFLAG, store->sid_wgt_samplesright, &buffer[index]);
		index += 4;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

/*****************************************/

/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);

	/* set buffer size */
	*buffer_size = index;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       buffer_size:%d\n", *buffer_size);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_seabeam(int verbose, int *buffer_size, char *buffer, void *store_ptr, int *error) {
	int index = 0;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sbm_source:          %d\n", store->sbm_source);
		fprintf(stderr, "dbg5       sbm_sec:             %u\n", store->sbm_sec);
		fprintf(stderr, "dbg5       sbm_usec:            %u\n", store->sbm_usec);
	}
	if (verbose >= 5 && store->sbm_properties) {
		fprintf(stderr, "dbg5       sbm_ping:            %d\n", store->sbm_ping);
		fprintf(stderr, "dbg5       sbm_ping_gain:       %f\n", store->sbm_ping_gain);
		fprintf(stderr, "dbg5       sbm_pulse_width:     %f\n", store->sbm_pulse_width);
		fprintf(stderr, "dbg5       sbm_transmit_power:  %f\n", store->sbm_transmit_power);
		fprintf(stderr, "dbg5       sbm_pixel_width:     %f\n", store->sbm_pixel_width);
		fprintf(stderr, "dbg5       sbm_swath_width:     %f\n", store->sbm_swath_width);
		fprintf(stderr, "dbg5       sbm_time_slice:      %f\n", store->sbm_time_slice);
		fprintf(stderr, "dbg5       sbm_depth_mode:      %d\n", store->sbm_depth_mode);
		fprintf(stderr, "dbg5       sbm_beam_mode:       %d\n", store->sbm_beam_mode);
		fprintf(stderr, "dbg5       sbm_ssv:             %f\n", store->sbm_ssv);
		fprintf(stderr, "dbg5       sbm_frequency:       %f\n", store->sbm_frequency);
		fprintf(stderr, "dbg5       sbm_bandwidth:       %f\n", store->sbm_bandwidth);
	}
	if (verbose >= 5 && store->sbm_hrp) {
		fprintf(stderr, "dbg5       sbm_heave:           %f\n", store->sbm_heave);
		fprintf(stderr, "dbg5       sbm_roll:            %f\n", store->sbm_roll);
		fprintf(stderr, "dbg5       sbm_pitch:           %f\n", store->sbm_pitch);
	}
	if (verbose >= 5 && store->sbm_signal) {
		fprintf(stderr, "dbg5       sbm_signal_beam:     %d\n", store->sbm_signal_beam);
		fprintf(stderr, "dbg5       sbm_signal_count:    %d\n", store->sbm_signal_count);
		for (int i = 0; i < store->sbm_signal_count; i++)
			fprintf(stderr, "dbg5       sample[%d]: %f\n", i, store->sbm_signal_amp[i]);
	}
	if (verbose >= 5 && store->sbm_message) {
		fprintf(stderr, "dbg5       sbm_message_id:      %d\n", store->sbm_message_id);
		fprintf(stderr, "dbg5       sbm_message_len:     %d\n", store->sbm_message_len);
		fprintf(stderr, "dbg5       sbm_message_txt:     %s\n", store->sbm_message_txt);
	}
	if (verbose >= 5 && store->sbm_sweepsegments) {
		fprintf(stderr, "dbg5       sbm_sweep_direction: %d\n", store->sbm_sweep_direction);
		fprintf(stderr, "dbg5       sbm_sweep_azimuth:   %f\n", store->sbm_sweep_azimuth);
		fprintf(stderr, "dbg5       sbm_sweep_segments:  %d\n", store->sbm_sweep_segments);
		fprintf(stderr, "dbg5       sbm_sweep_seconds:   %d\n", store->sbm_sweep_seconds);
		fprintf(stderr, "dbg5       sbm_sweep_micro:     %d\n", store->sbm_sweep_micro);
		fprintf(stderr, "dbg5       sbm_sweep_extrapolateazimuth:  %f\n", store->sbm_sweep_extrapolateazimuth);
		fprintf(stderr, "dbg5       sbm_sweep_interpolatedazimuth: %f\n", store->sbm_sweep_interpolatedazimuth);
		fprintf(stderr, "dbg5       sbm_sweep_extrapolatepitch:    %f\n", store->sbm_sweep_extrapolatepitch);
		fprintf(stderr, "dbg5       sbm_sweep_interpolatedpitch:   %f\n", store->sbm_sweep_interpolatedpitch);
		fprintf(stderr, "dbg5       sbm_sweep_extrapolateroll:     %f\n", store->sbm_sweep_extrapolateroll);
		fprintf(stderr, "dbg5       sbm_sweep_interpolatedroll:    %f\n", store->sbm_sweep_interpolatedroll);
		fprintf(stderr, "dbg5       sbm_sweep_stabilizedangle:     %f\n", store->sbm_sweep_stabilizedangle);
	}
	if (verbose >= 5 && store->sbm_spacingmode) {
		fprintf(stderr, "dbg5       sbm_spacing_mode:      	      %d\n", store->sbm_spacing_mode);
		fprintf(stderr, "dbg5       sbm_spacing_equidistance:      %f\n", store->sbm_spacing_equidistance);
		fprintf(stderr, "dbg5       sbm_spacing_equidistance_min:  %f\n", store->sbm_spacing_equidistance_min);
		fprintf(stderr, "dbg5       sbm_spacing_equidistance_max:  %f\n", store->sbm_spacing_equidistance_max);
	}

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;

	/* get frame time */
	frame_id = MBSYS_XSE_SBM_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->sbm_source, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->sbm_sec, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->sbm_usec, &buffer[index]);
	index += 4;
	frame_count += 16;

	/* deal with properties group */
	if (store->sbm_properties) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get properties group */
		group_id = MBSYS_XSE_SBM_GROUP_PROPERTIES;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_ping, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_ping_gain, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_pulse_width, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_transmit_power, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_pixel_width, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_swath_width, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_time_slice, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_depth_mode, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_beam_mode, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_ssv, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_frequency, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_bandwidth, &buffer[index]);
		index += 4;
		group_count += 52;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* deal with hrp group */
	if (store->sbm_hrp) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get hrp group */
		group_id = MBSYS_XSE_SBM_GROUP_HRP;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_double(SWAPFLAG, store->sbm_heave, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->sbm_roll, &buffer[index]);
		index += 8;
		mb_put_binary_double(SWAPFLAG, store->sbm_pitch, &buffer[index]);
		index += 8;
		group_count += 28;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* deal with signal group */
	if (store->sbm_signal) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get signal group */
		group_id = MBSYS_XSE_SBM_GROUP_SIGNAL;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_signal_beam, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_signal_count, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->sbm_signal_count; i++) {
			mb_put_binary_float(SWAPFLAG, store->sbm_signal_amp[i], &buffer[index]);
			index += 4;
		}
		group_count += 12 + 4 * store->sbm_signal_count;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* deal with message group */
	if (store->sbm_message) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get message group */
		group_id = MBSYS_XSE_SBM_GROUP_MESSAGE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_message_id, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_message_len, &buffer[index]);
		index += 4;
		for (int i = 0; i < store->sbm_message_len; i++) {
			buffer[index] = store->sbm_message_txt[i];
			index++;
		}
		group_count += 12 + store->sbm_message_len;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* deal with sweep segments group */
	if (store->sbm_sweepsegments) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get sweep segments group */
		group_id = MBSYS_XSE_SBM_GROUP_SWEEPSEGMENTS;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		buffer[index] = store->sbm_sweep_direction;
		index++;
		mb_put_binary_float(SWAPFLAG, store->sbm_sweep_azimuth, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_sweep_segments, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_sweep_seconds, &buffer[index]);
		index += 4;
		mb_put_binary_int(SWAPFLAG, store->sbm_sweep_micro, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_sweep_extrapolateazimuth, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_sweep_interpolatedazimuth, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_sweep_extrapolatepitch, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_sweep_interpolatedpitch, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_sweep_extrapolateroll, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_sweep_interpolatedroll, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_sweep_stabilizedangle, &buffer[index]);
		index += 4;
		group_count += 49;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

	/* deal with spacing mode group */
	if (store->sbm_message) {
/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get spacing mode group */
		group_id = MBSYS_XSE_SBM_GROUP_MESSAGE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
		index += 4;
		buffer[index] = store->sbm_spacing_mode;
		index++;
		mb_put_binary_float(SWAPFLAG, store->sbm_spacing_equidistance, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_spacing_equidistance_min, &buffer[index]);
		index += 4;
		mb_put_binary_float(SWAPFLAG, store->sbm_spacing_equidistance_max, &buffer[index]);
		index += 4;
		group_count += 17 + store->sbm_message_len;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
	}

/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);

	/* set buffer size */
	*buffer_size = index;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       buffer_size:%d\n", *buffer_size);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_comment(int verbose, int *buffer_size, char *buffer, void *store_ptr, int *error) {
	int index = 0;
	int size;
	int len;
	int frame_id;
	int group_id = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       comment:             %s\n", store->comment);
	}

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* get frame size */
	len = strlen(store->comment) + 4;
	if (len % 4 > 0)
		len += 4 - (len % 4);
	size = len + 32;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]);
	index += 4;

	/* get frame time */
	frame_id = MBSYS_XSE_COM_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->com_source, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->com_sec, &buffer[index]);
	index += 4;
	mb_put_binary_int(SWAPFLAG, store->com_usec, &buffer[index]);
	index += 4;

/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size and id */
	mb_put_binary_int(SWAPFLAG, (len + 4), &buffer[index]);
	index += 4;
	group_id = MBSYS_XSE_COM_GROUP_GEN;
	mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]);
	index += 4;
	strncpy(&buffer[index], store->comment, len);
	index += len;

/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;

/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;

	*buffer_size = index;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       buffer_size:%d\n", *buffer_size);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int buffer_size;
	int write_size;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to store data structure */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;
	FILE *mbfp = mb_io_ptr->mbfp;
	char *buffer = mb_io_ptr->hdr_comment;

#ifdef MB_DEBUG
	fprintf(stderr, "%s:%d | WRITE KIND: %d\n", __FILE__, __LINE__, store->kind);
#endif

	int status = MB_SUCCESS;

	if (store->kind == MB_DATA_COMMENT) {
#ifdef MB_DEBUG
		fprintf(stderr, "%s:%d | WRITE COMMMENT\n", __FILE__, __LINE__);
#endif
		status = mbr_l3xseraw_wr_comment(verbose, &buffer_size, buffer, store_ptr, error);
		if ((write_size = fwrite(buffer, 1, buffer_size, mbfp)) != buffer_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}
	else if (store->kind == MB_DATA_NAV) {
#ifdef MB_DEBUG
		fprintf(stderr, "%s:%d | WRITE NAV\n", __FILE__, __LINE__);
#endif
		status = mbr_l3xseraw_wr_nav(verbose, &buffer_size, buffer, store_ptr, error);
		if ((write_size = fwrite(buffer, 1, buffer_size, mbfp)) != buffer_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}
	else if (store->kind == MB_DATA_VELOCITY_PROFILE) {
#ifdef MB_DEBUG
		fprintf(stderr, "%s:%d | WRITE SVP\n", __FILE__, __LINE__);
#endif
		status = mbr_l3xseraw_wr_svp(verbose, &buffer_size, buffer, store_ptr, error);
		if ((write_size = fwrite(buffer, 1, buffer_size, mbfp)) != buffer_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}
	else if (store->kind == MB_DATA_PARAMETER) {
#ifdef MB_DEBUG
		fprintf(stderr, "%s:%d | WRITE SHIP\n", __FILE__, __LINE__);
#endif
		status = mbr_l3xseraw_wr_ship(verbose, &buffer_size, buffer, store_ptr, error);
		if ((write_size = fwrite(buffer, 1, buffer_size, mbfp)) != buffer_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}
	else if (store->kind == MB_DATA_DATA) {
#ifdef MB_DEBUG
		fprintf(stderr, "%s:%d | WRITE MULTIBEAM\n", __FILE__, __LINE__);
#endif
		if (store->mul_frame) {
			status = mbr_l3xseraw_wr_multibeam(verbose, &buffer_size, buffer, store_ptr, error);
			if ((write_size = fwrite(buffer, 1, buffer_size, mbfp)) != buffer_size) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
		}
#ifdef MB_DEBUG
		fprintf(stderr, "%s:%d | WRITE SIDESCAN\n", __FILE__, __LINE__);
#endif
		if (store->sid_frame) {
			status = mbr_l3xseraw_wr_sidescan(verbose, &buffer_size, buffer, store_ptr, error);
			if ((write_size = fwrite(buffer, 1, buffer_size, mbfp)) != buffer_size) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
		}
	}
	else if (store->kind == MB_DATA_RUN_PARAMETER) {
#ifdef MB_DEBUG
		fprintf(stderr, "%s:%d | WRITE RUN PARAMETER\n", __FILE__, __LINE__);
#endif
		status = mbr_l3xseraw_wr_seabeam(verbose, &buffer_size, buffer, store_ptr, error);
		if ((write_size = fwrite(buffer, 1, buffer_size, mbfp)) != buffer_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}
	else if (store->kind == MB_DATA_RAW_LINE) {
#ifdef MB_DEBUG
		fprintf(stderr, "%s:%d | WRITE RAW LINE\n", __FILE__, __LINE__);
#endif
		if (store->rawsize > 0) {
			if ((write_size = fwrite(store->raw, 1, store->rawsize, mbfp)) != store->rawsize) {
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
			}
		}
	}
	else {
#ifdef MB_DEBUG
		fprintf(stderr, "%s:%d | WRITE FAILURE BAD KIND\n", __FILE__, __LINE__);
#endif
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
int mbr_wt_l3xseraw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* write next data to file */
	const int status = mbr_l3xseraw_wr_data(verbose, mbio_ptr, store_ptr, error);

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
int mbr_register_l3xseraw(int verbose, void *mbio_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_l3xseraw(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_l3xseraw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_l3xseraw;
	mb_io_ptr->mb_io_store_alloc = &mbsys_xse_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_xse_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_l3xseraw;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_l3xseraw;
	mb_io_ptr->mb_io_dimensions = &mbsys_xse_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_xse_extract;
	mb_io_ptr->mb_io_insert = &mbsys_xse_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_xse_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_xse_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_xse_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_xse_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_xse_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_xse_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_xse_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_xse_copy;
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
