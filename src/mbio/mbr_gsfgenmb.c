/*--------------------------------------------------------------------
 *    The MB-system:	mbr_gsfgenmb.c	2/27/98
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
 * mbr_gsfgenmb.c contains the functions for reading and writing
 * multibeam data in the GSFGENMB format.
 * These functions include:
 *   mbr_alm_gsfgenmb	- allocate read/write memory
 *   mbr_dem_gsfgenmb	- deallocate read/write memory
 *   mbr_rt_gsfgenmb	- read and translate data
 *   mbr_wt_gsfgenmb	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 27, 1998
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbf_gsfgenmb.h"
#include "mbsys_gsf.h"

/* GSF error value */
extern int gsfError;

/*--------------------------------------------------------------------*/
int mbr_info_gsfgenmb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_GSF;
	*beams_bath_max = 254;
	*beams_amp_max = 254;
	*pixels_ss_max = 8192;
	strncpy(format_name, "GSFGENMB", MB_NAME_LENGTH);
	strncpy(system_name, "GSF", MB_NAME_LENGTH);
	sprintf(format_description,
	        "Format name:          MBF_GSFGENMB\nInformal Description: Leidos Generic Sensor Format (GSF) version %s\nAttributes:           "
	        "variable beams,  bathymetry and amplitude,\n                      binary, single files, Leidos (formerly SAIC).\n",
	        GSF_VERSION);
	*numfile = 1;
	*filetype = MB_FILETYPE_GSF;
	*variable_beams = true;
	*traveltime = true;
	*beam_flagging = true;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

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
		fprintf(stderr, "dbg2       attitude_source:    %d\n", *attitude_source);
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
int mbr_alm_gsfgenmb(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_gsfgenmb_struct);
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	memset(mb_io_ptr->raw_data, 0, mb_io_ptr->structure_size);
	status &= mbsys_gsf_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* set processing parameter output flag */
	mb_io_ptr->save1 = false;

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
int mbr_dem_gsfgenmb(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	// struct mbf_gsfgenmb_struct *data = (struct mbf_gsfgenmb_struct *)mb_io_ptr->raw_data;
	// gsfRecords *records = &(data->records);

	/* deallocate memory for data descriptor */
	/*gsfFree(records);*/
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->raw_data, error);
	status &= mbsys_gsf_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_rt_gsfgenmb(int verbose, void *mbio_ptr, void *store_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor and data structure */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_gsf_struct *store = (struct mbsys_gsf_struct *)store_ptr;

	/* get pointer to raw data structure */
	struct mbf_gsfgenmb_struct *data = (struct mbf_gsfgenmb_struct *)mb_io_ptr->raw_data;

	/* get pointers to GSF structures */
	gsfDataID *dataID = &(data->dataID);
	gsfRecords *records = &(data->records);
	gsfSwathBathyPing *mb_ping = &(records->mb_ping);

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record from file */
	int ret = gsfRead((int)mb_io_ptr->gsfid, GSF_NEXT_RECORD, dataID, records, NULL, 0);

	int status = MB_SUCCESS;

	/* deal with errors */
	if (ret < 0) {
#ifndef WIN32
		if (gsfError == GSF_READ_TO_END_OF_FILE || gsfError == GSF_PARTIAL_RECORD_AT_END_OF_FILE)
#else
		/* On Windows the external gsfError set in gsf.c is not visible here */
		if (ret < 0)
#endif
		{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* else deal with data */
	else {
/*
if (dataID->recordID == GSF_RECORD_HEADER)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_HEADER\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SWATH_BATHYMETRY_PING)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_SWATH_BATHYMETRY_PING\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SOUND_VELOCITY_PROFILE)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_SOUND_VELOCITY_PROFILE\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_PROCESSING_PARAMETERS)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_PROCESSING_PARAMETERS\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SENSOR_PARAMETERS)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_SENSOR_PARAMETERS\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_COMMENT)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_COMMENT\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_HISTORY)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_HISTORY\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_NAVIGATION_ERROR)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_NAVIGATION_ERROR\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SWATH_BATHY_SUMMARY)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_SWATH_BATHY_SUMMARY\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SINGLE_BEAM_PING)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_SINGLE_BEAM_PING\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_HV_NAVIGATION_ERROR)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_HV_NAVIGATION_ERROR\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_ATTITUDE)
fprintf(stderr, "dataID->recordID: %d  Read GSF_RECORD_ATTITUDE\n", dataID->recordID);
*/
		if (dataID->recordID == GSF_RECORD_HEADER) {
			data->kind = MB_DATA_HEADER;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else if (dataID->recordID == GSF_RECORD_SWATH_BATHYMETRY_PING) {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			data->kind = MB_DATA_DATA;

			/* get beam widths */
			ret = gsfGetSwathBathyBeamWidths(records, &(mb_io_ptr->beamwidth_ltrack), &(mb_io_ptr->beamwidth_xtrack));
			if (ret < 0) {
				mb_io_ptr->beamwidth_ltrack = 0.0;
				mb_io_ptr->beamwidth_xtrack = 0.0;
			}

			/* if needed create array for beam flags */
			if (mb_ping->number_beams > 0 && mb_ping->beam_flags == NULL) {
				mb_ping->beam_flags = (unsigned char *)malloc(mb_ping->number_beams * sizeof(unsigned char));
				for (int i = 0; i < mb_ping->number_beams; i++)
					mb_ping->beam_flags[i] = MB_FLAG_NONE;
			}

			/* if needed create array for along_track */
			if (mb_ping->number_beams > 0 && mb_ping->along_track == NULL) {
				mb_ping->along_track = (double *)malloc(mb_ping->number_beams * sizeof(double));
				for (int i = 0; i < mb_ping->number_beams; i++)
					mb_ping->along_track[i] = 0.0;
			}
		}

		else if (dataID->recordID == GSF_RECORD_SOUND_VELOCITY_PROFILE) {
			data->kind = MB_DATA_VELOCITY_PROFILE;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else if (dataID->recordID == GSF_RECORD_PROCESSING_PARAMETERS) {
			data->kind = MB_DATA_PROCESSING_PARAMETERS;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else if (dataID->recordID == GSF_RECORD_SENSOR_PARAMETERS) {
			data->kind = MB_DATA_PROCESSING_PARAMETERS;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else if (dataID->recordID == GSF_RECORD_COMMENT) {
			/* copy comment */
			data->kind = MB_DATA_COMMENT;
			if (records->comment.comment != NULL) {
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}

		else if (dataID->recordID == GSF_RECORD_HISTORY) {
			data->kind = MB_DATA_HISTORY;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else if (dataID->recordID == GSF_RECORD_NAVIGATION_ERROR) /* 10/19/98 This record is obsolete */
		{
			data->kind = MB_DATA_NAVIGATION_ERROR;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else if (dataID->recordID == GSF_RECORD_SWATH_BATHY_SUMMARY) {
			data->kind = MB_DATA_SUMMARY;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else if (dataID->recordID == GSF_RECORD_SINGLE_BEAM_PING) {
			data->kind = MB_DATA_SINGLE_BEAM_PING;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else if (dataID->recordID == GSF_RECORD_HV_NAVIGATION_ERROR) /* This record replaces GSF_RECORD_NAVIGATION_ERROR */
		{
			data->kind = MB_DATA_NAVIGATION_ERROR;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else if (dataID->recordID == GSF_RECORD_ATTITUDE) {
			data->kind = MB_DATA_ATTITUDE;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* output debug info */
	if (verbose >= 4) {
		fprintf(stderr, "\ndbg4  New record read by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg4  New record kind:\n");
		fprintf(stderr, "dbg4       error:      %d\n", mb_io_ptr->new_error);
		fprintf(stderr, "dbg4       kind:       %d\n", mb_io_ptr->new_kind);
		fprintf(stderr, "dbg4       dataID->recordID: %d\n", dataID->recordID);
	}
	if (verbose >= 4 && data->kind == MB_DATA_COMMENT) {
		fprintf(stderr, "\ndbg4  New comment read by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg4  New comment values:\n");
		fprintf(stderr, "dbg4       kind:              %d\n", data->kind);
		fprintf(stderr, "dbg4       comment time sec:  %ld\n", records->comment.comment_time.tv_sec);
		fprintf(stderr, "dbg4       comment time nsec: %ld\n", records->comment.comment_time.tv_nsec);
		fprintf(stderr, "dbg4       comment length:    %d\n", records->comment.comment_length);
		fprintf(stderr, "dbg4       comment:           %s\n", records->comment.comment);
	}
	if (verbose >= 4 && data->kind == MB_DATA_DATA) {
		fprintf(stderr, "\ndbg4  New ping read by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg4  New ping values:\n");
		fprintf(stderr, "dbg4       kind:       %d\n", data->kind);
		fprintf(stderr, "dbg4       time sec:   %ld\n", mb_ping->ping_time.tv_sec);
		fprintf(stderr, "dbg4       time nsec:  %ld\n", mb_ping->ping_time.tv_nsec);
		fprintf(stderr, "dbg4       longitude:  %f\n", mb_ping->longitude);
		fprintf(stderr, "dbg4       latitude:   %f\n", mb_ping->latitude);
		fprintf(stderr, "dbg4       speed:      %f\n", mb_ping->speed);
		fprintf(stderr, "dbg4       heading:    %f\n", mb_ping->heading);
		fprintf(stderr, "dbg4       beams:      %d\n", mb_ping->number_beams);
		for (int i = 0; i < mb_ping->number_beams; i++)
			fprintf(stderr, "dbg4       beam:%d  flag:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, mb_ping->beam_flags[i],
			        mb_ping->depth[i], mb_ping->across_track[i], mb_ping->along_track[i]);
		if (mb_ping->mc_amplitude != NULL)
			for (int i = 0; i < mb_ping->number_beams; i++)
				fprintf(stderr, "dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n", i, mb_ping->mc_amplitude[i],
				        mb_ping->across_track[i], mb_ping->along_track[i]);
		if (mb_ping->mr_amplitude != NULL)
			for (int i = 0; i < mb_ping->number_beams; i++)
				fprintf(stderr, "dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n", i, mb_ping->mr_amplitude[i],
				        mb_ping->across_track[i], mb_ping->along_track[i]);
	}

	/* translate values to data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		gsfFree(&(store->records));
		gsfCopyRecords(&(store->records), records);
		store->dataID = *dataID;
		store->kind = data->kind;
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
int mbr_wt_gsfgenmb(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor and data structure */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_gsf_struct *store = (struct mbsys_gsf_struct *)store_ptr;

	/* get pointer to raw data structure */
	struct mbf_gsfgenmb_struct *data = (struct mbf_gsfgenmb_struct *)mb_io_ptr->raw_data;

	/* get pointers to GSF structures */
	gsfDataID *dataID = &(data->dataID);
	gsfRecords *records = &(data->records);
	gsfSwathBathyPing *mb_ping = &(records->mb_ping);

	/* translate values to data storage structure */
	if (store != NULL) {
		gsfFree(records);
		gsfCopyRecords(records, &(store->records));
		*dataID = store->dataID;
		data->kind = store->kind;
	}

	/* output debug info */
	if (verbose >= 4) {
		fprintf(stderr, "\ndbg4  New record to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg4  New record kind:\n");
		fprintf(stderr, "dbg4       kind:              %d\n", data->kind);
	}
	if (verbose >= 4 && data->kind == MB_DATA_COMMENT) {
		fprintf(stderr, "\ndbg4  New comment to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg4  New comment values:\n");
		fprintf(stderr, "dbg4       kind:              %d\n", data->kind);
		fprintf(stderr, "dbg4       comment time sec:  %ld\n", records->comment.comment_time.tv_sec);
		fprintf(stderr, "dbg4       comment time nsec: %ld\n", records->comment.comment_time.tv_nsec);
		fprintf(stderr, "dbg4       comment length:    %d\n", records->comment.comment_length);
		fprintf(stderr, "dbg4       comment:           %s\n", records->comment.comment);
	}
	if (verbose >= 4 && data->kind == MB_DATA_DATA) {
		fprintf(stderr, "\ndbg4  New ping to be written by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg4  New ping values:\n");
		fprintf(stderr, "dbg4       kind:       %d\n", data->kind);
		fprintf(stderr, "dbg4       time sec:   %ld\n", mb_ping->ping_time.tv_sec);
		fprintf(stderr, "dbg4       time nsec:  %ld\n", mb_ping->ping_time.tv_nsec);
		fprintf(stderr, "dbg4       longitude:  %f\n", mb_ping->longitude);
		fprintf(stderr, "dbg4       latitude:   %f\n", mb_ping->latitude);
		fprintf(stderr, "dbg4       speed:      %f\n", mb_ping->speed);
		fprintf(stderr, "dbg4       heading:    %f\n", mb_ping->heading);
		fprintf(stderr, "dbg4       beams:      %d\n", mb_ping->number_beams);
		for (int i = 0; i < mb_ping->number_beams; i++)
			fprintf(stderr, "dbg4       beam:%d  flag:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, mb_ping->beam_flags[i],
			        mb_ping->depth[i], mb_ping->across_track[i], mb_ping->along_track[i]);
		if (mb_ping->mc_amplitude != NULL)
			for (int i = 0; i < mb_ping->number_beams; i++)
				fprintf(stderr, "dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n", i, mb_ping->mc_amplitude[i],
				        mb_ping->across_track[i], mb_ping->along_track[i]);
		if (mb_ping->mr_amplitude != NULL)
			for (int i = 0; i < mb_ping->number_beams; i++)
				fprintf(stderr, "dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n", i, mb_ping->mr_amplitude[i],
				        mb_ping->across_track[i], mb_ping->along_track[i]);
	}

	int status = MB_SUCCESS;

	/* write gsf data to file */
	if (status == MB_SUCCESS) {
		/* if first survey ping and no processing parameters output,
		    output the processing parameters */
		if (data->kind == MB_DATA_DATA && !mb_io_ptr->save1) {
			/* write a processing parameter record */
			dataID->recordID = GSF_RECORD_PROCESSING_PARAMETERS;
/* 
if (dataID->recordID == GSF_RECORD_HEADER)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_HEADER\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SWATH_BATHYMETRY_PING)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SWATH_BATHYMETRY_PING\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SOUND_VELOCITY_PROFILE)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SOUND_VELOCITY_PROFILE\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_PROCESSING_PARAMETERS)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_PROCESSING_PARAMETERS\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SENSOR_PARAMETERS)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SENSOR_PARAMETERS\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_COMMENT)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_COMMENT\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_HISTORY)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_HISTORY\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_NAVIGATION_ERROR)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_NAVIGATION_ERROR\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SWATH_BATHY_SUMMARY)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SWATH_BATHY_SUMMARY\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SINGLE_BEAM_PING)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SINGLE_BEAM_PING\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_HV_NAVIGATION_ERROR)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_HV_NAVIGATION_ERROR\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_ATTITUDE)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_ATTITUDE\n", dataID->recordID);
*/
			if (gsfWrite((int)mb_io_ptr->gsfid, dataID, records) < 0) {
				status = MB_FAILURE;
				*error = MB_ERROR_WRITE_FAIL;
			}
			dataID->recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
			mb_io_ptr->save1 = true;
		}

		/* if a processing parameter record is output, keep track of it */
		else if (data->kind == MB_DATA_PROCESSING_PARAMETERS)
			mb_io_ptr->save1 = true;

		/* write the record */
/*
if (dataID->recordID == GSF_RECORD_HEADER)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_HEADER\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SWATH_BATHYMETRY_PING)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SWATH_BATHYMETRY_PING\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SOUND_VELOCITY_PROFILE)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SOUND_VELOCITY_PROFILE\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_PROCESSING_PARAMETERS)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_PROCESSING_PARAMETERS\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SENSOR_PARAMETERS)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SENSOR_PARAMETERS\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_COMMENT)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_COMMENT\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_HISTORY)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_HISTORY\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_NAVIGATION_ERROR)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_NAVIGATION_ERROR\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SWATH_BATHY_SUMMARY)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SWATH_BATHY_SUMMARY\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_SINGLE_BEAM_PING)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_SINGLE_BEAM_PING\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_HV_NAVIGATION_ERROR)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_HV_NAVIGATION_ERROR\n", dataID->recordID);
if (dataID->recordID == GSF_RECORD_ATTITUDE)
fprintf(stderr, "dataID->recordID: %d  Write GSF_RECORD_ATTITUDE\n", dataID->recordID);
*/
		if (gsfWrite((int)mb_io_ptr->gsfid, dataID, records) < 0) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
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
int mbr_register_gsfgenmb(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_gsfgenmb(
                verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
                mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
                &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
                &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
                &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_gsfgenmb;
	mb_io_ptr->mb_io_format_free = &mbr_dem_gsfgenmb;
	mb_io_ptr->mb_io_store_alloc = &mbsys_gsf_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_gsf_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_gsfgenmb;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_gsfgenmb;
	mb_io_ptr->mb_io_sonartype = &mbsys_gsf_sonartype;
	mb_io_ptr->mb_io_sidescantype = &mbsys_gsf_sidescantype;
	mb_io_ptr->mb_io_dimensions = &mbsys_gsf_dimensions;
	mb_io_ptr->mb_io_sonartype = &mbsys_gsf_sonartype;
	mb_io_ptr->mb_io_sidescantype = &mbsys_gsf_sidescantype;
	mb_io_ptr->mb_io_extract = &mbsys_gsf_extract;
	mb_io_ptr->mb_io_insert = &mbsys_gsf_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_gsf_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_gsf_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_gsf_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = &mbsys_gsf_insert_altitude;
	mb_io_ptr->mb_io_extract_svp = &mbsys_gsf_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_gsf_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_gsf_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_gsf_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_gsf_copy;
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
		fprintf(stderr, "dbg2       sensordepth_source: %d\n", mb_io_ptr->sensordepth_source);
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
