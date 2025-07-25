/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsmdaraw.c	2/11/93
 *	$Header: /system/link/server/cvs/root/mbsystem/src/mbio/mbr_hsmdaraw.c,v 5.8 2005/11/05 00:48:04 caress Exp $
 *
 *    Copyright (c) 1993-2025 by
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
 * mbr_hsmdaraw.c contains the functions for reading and writing
 * multibeam data in the HSMDARAW format.
 * These functions include:
 *   mbr_alm_hsmdaraw	- allocate read/write memory
 *   mbr_dem_hsmdaraw	- deallocate read/write memory
 *   mbr_rt_hsmdaraw	- read and translate data
 *   mbr_wt_hsmdaraw	- translate and write data
 *
 * Author:	Dale Chayes
 * Date:	August 11, 1995
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
#include "mbf_hsmdaraw.h"
#include "mbsys_hsmd.h"

/*--------------------------------------------------------------------*/
int mbr_info_hsmdaraw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_HSMD;
	*beams_bath_max = 79;
	*beams_amp_max = 0;
	*pixels_ss_max = 319;
	strncpy(format_name, "HSMDARAW", MB_NAME_LENGTH);
	strncpy(system_name, "HSMD", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_HSMDARAW\nInformal Description: Atlas HSMD medium depth multibeam raw format\nAttributes: "
	        "          40 beam bathymetry, 160 pixel sidescan,\n                      XDR (binary), STN Atlas Elektronik.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_XDR;
	*variable_beams = false;
	*traveltime = true;
	*beam_flagging = true;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_NAV;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 1.7;
	*beamwidth_ltrack = 1.7;

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
int mbr_zero_hsmdaraw(int verbose, char *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to data descriptor */
	struct mbf_hsmdaraw_struct *data = (struct mbf_hsmdaraw_struct *)data_ptr;

	/* initialize everything to zeros */
	if (data != NULL) {
		for (int i = 0; i < 4; i++) {
			data->scsid[i] = 0;
			data->scsart[i] = 0;
		}
		data->scslng = 0;
		data->scsext = 0;
		data->scsblcnt = 0;
		data->scsres1 = 0.0;
		data->transid = 0;    /* indicates what kind of data */
		data->reftime = -1.0; /* unitialized */

		data->datuhr = -1.0;

		for (int i = 0; i < 8; i++)
			data->mksysint[i] = 0;

		for (int i = 0; i < 84; i++)
			data->mktext[i] = 0;

		data->navid = 0;
		data->year = 0;
		data->month = 0;
		data->day = 0;
		data->hour = 0;
		data->minute = 0;
		data->second = 0;
		data->millisecond = 0.0;

		data->lon = 0.0;
		data->lat = 0.0;

		data->ckeel = 0.0;
		data->cmean = 0.0;
		data->Port = 0;
		data->noho = 0;
		data->skals = 0;

		for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++) {
			data->spfb[i] = 0;
			data->angle[i] = mbf_hsmdaraw_beamangle[i];
			data->depth[i] = 0.0;
			data->distance[i] = 0.0;
		}

		data->ss_range = 0.0;
		for (int i = 0; i < MBF_HSMDARAW_PIXELS_PING; i++)
			data->ss[i] = 0;

		data->heading_tx = 0.0;
		for (int i = 0; i < 5; i++)
			data->heading_rx[i] = 0.0;

		data->roll_tx = 0.0;
		for (int i = 0; i < 5; i++)
			data->roll_rx[i] = 0.0;

		data->pitch_tx = 0.0;
		for (int i = 0; i < 5; i++)
			data->pitch_rx[i] = 0.0;

		data->num_vel = 0;
		for (int i = 0; i < MBF_HSMDARAW_MAXVEL; i++) {
			data->vdepth[i] = 0.0;
			data->velocity[i] = 0.0;
		}
	}

	/* assume success */
	const int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

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
int mbr_alm_hsmdaraw(int verbose, void *mbio_ptr, int *error) {
	double *FirstReftime; /* time from the first header */
	int *Header_count;    /* number of header records encounterd */
	int *Rev_count;       /* Raw Event counter */
	int *Nav_count;       /* number of Nav records */
	int *Angle_count;     /* etc....... */
	int *Svp_count;
	int *Raw_count;
	int *MDevent_count;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_hsmdaraw_struct);
	mb_io_ptr->data_structure_size = 0;
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status &= mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_hsmd_struct), &mb_io_ptr->store_data, error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbf_hsmdaraw_struct *data = (struct mbf_hsmdaraw_struct *)mb_io_ptr->raw_data;
	char *data_ptr = (char *)data;

	/* initialize saved values */
	FirstReftime = &mb_io_ptr->saved1; /* time from the first header */
	Header_count = &mb_io_ptr->save1;  /* number of header records encounterd */
	Rev_count = &mb_io_ptr->save2;     /* Raw Event counter */
	Nav_count = &mb_io_ptr->save3;     /* number of Nav records */
	Angle_count = &mb_io_ptr->save4;   /* etc....... */
	Svp_count = &mb_io_ptr->save5;
	Raw_count = &mb_io_ptr->save6;
	MDevent_count = &mb_io_ptr->save7;
	*FirstReftime = 0.0; /* time from the first header */
	*Header_count = 0;   /* number of header records encounterd */
	*Rev_count = 0;      /* Raw Event counter */
	*Nav_count = 0;      /* number of Nav records */
	*Angle_count = 0;    /* etc....... */
	*Svp_count = 0;
	*Raw_count = 0;
	*MDevent_count = 0;

	/* initialize everything to zeros */
	mbr_zero_hsmdaraw(verbose, data_ptr, error);

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
int mbr_dem_hsmdaraw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data descriptor */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->raw_data, error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->store_data, error);

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
int mbr_hsmdaraw_rd_data(int verbose, void *mbio_ptr, int *error) {
	XDR *xdrs; /* xdr i/o pointer */
	int time_i[7];
	double scale;
	double PingTime; /* Synthesised time of this ping
	                 PingTime = Base_time
	                 + (current.datuhr
	                 - FirstReftime) */

	double *FirstReftime; /* time from the first header */
	int *Header_count;    /* number of header records encounterd */
	int *Rev_count;       /* Raw Event counter */
	int *Nav_count;       /* number of Nav records */
	int *Angle_count;     /* etc....... */
	int *Svp_count;
	int *Raw_count;
	int *MDevent_count;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_hsmdaraw_struct *data = (struct mbf_hsmdaraw_struct *)mb_io_ptr->raw_data;
	xdrs = mb_io_ptr->xdrs;
	FirstReftime = &mb_io_ptr->saved1; /* time from the first header */
	Header_count = &mb_io_ptr->save1;  /* number of header records encounterd */
	Rev_count = &mb_io_ptr->save2;     /* Raw Event counter */
	Nav_count = &mb_io_ptr->save3;     /* number of Nav records */
	Angle_count = &mb_io_ptr->save4;   /* etc....... */
	Svp_count = &mb_io_ptr->save5;
	Raw_count = &mb_io_ptr->save6;
	MDevent_count = &mb_io_ptr->save7;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	int status = MB_SUCCESS;

	/* Start reading an HSMD Header structure */
	/* read the first four bytes */
	for (int i = 0; i < 4; i++)
		status = xdr_char(xdrs, &data->scsid[i]);

	/* loop until the beginning of a record is found */
	while (status == MB_SUCCESS && strncmp(data->scsid, "DXT", 3) != 0) {
		if (data->scsid[1] == 'D' || data->scsid[2] == 'D' || data->scsid[3] == 'D') {
			for (int i = 0; i < 3; i++)
				data->scsid[i] = data->scsid[i + 1];
			status = xdr_char(xdrs, &data->scsid[3]);
		}
		else {
			while (status == MB_SUCCESS && data->scsid[0] != 'D') {
				if ((status = fread(&data->scsid[0], 1, 1, mb_io_ptr->mbfp)) != 1) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
			}
			if (status == MB_SUCCESS) {
				for (int i = 1; i < 4; i++)
					status = xdr_char(xdrs, &data->scsid[i]);
			}
		}
	}

	/* now read the rest of the record */
	if (status == MB_SUCCESS)
		for (int i = 0; i < 4; i++)
			status = xdr_char(xdrs, &data->scsart[i]);
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->scslng);
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->scsext);
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->scsblcnt);
	if (status == MB_SUCCESS)
		status = xdr_double(xdrs, &data->scsres1);
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->transid);

	/* get first time and initialize the time base */
	if (status == MB_SUCCESS)
		status = xdr_double(xdrs, &data->reftime);
	if (status == MB_SUCCESS && data->transid != MBF_HSMDARAW_COM) {
		(*Header_count)++;
		if (*Header_count == 1)
			*FirstReftime = data->reftime;
	}

	/* check status */
	if (status == MB_SUCCESS)
		*error = MB_ERROR_NO_ERROR;
	else
		*error = MB_ERROR_EOF;

	/* print out some debug messages */
	if (verbose >= 2 && status == MB_SUCCESS) {
		fprintf(stderr, "\ndbg2: ========================== \n");
		fprintf(stderr, "dbg2: HED (0) # %d\t%.3lf\t%.3lf \n", *Header_count, data->reftime, data->reftime - *FirstReftime);
	}
	if (verbose >= 5 && status == MB_SUCCESS) {
		fprintf(stderr, "dbg5: data  From Header:\n");
		fprintf(stderr, "dbg5: \t->scsid : \t%s\n", data->scsid);
		fprintf(stderr, "dbg5: \t->scsart: \t%s\n", data->scsart);
		fprintf(stderr, "dbg5: \t->scslng: \t%d\t0x%0X\n", data->scslng, data->scslng);
		fprintf(stderr, "dbg5: \t->scsext:  \t%d\n", data->scsext);
		fprintf(stderr, "dbg5: \t->scsblcnt:\t%d\n", data->scsblcnt);
		fprintf(stderr, "dbg5: \t->scsres1: \t%lf\n", data->scsres1);
		fprintf(stderr, "dgb5: \t->transid: \t%d\n", data->transid);
		fprintf(stderr, "dgb5: \t->reftime: \t%lf\n", data->reftime);
	}

	/* done reading the header part of this data record
	- now read the rest*/

	/* read the appropriate data records */
	if (status == MB_SUCCESS) {
		switch (data->transid) {
		case (MBF_HSMDARAW_RAW): /* 1, Raw data record */
		{
			data->kind = MB_DATA_DATA;

			(*Raw_count)++; /* the number of this kind of record */

			/* get water velocity and travel time data */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->ckeel);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->cmean);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->Port);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->noho);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->skals);
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++)
					status = xdr_int(xdrs, &data->spfb[i]);

			/* Check for bad beams - broken records produce
			    bogus data - it happens with HSMD systems!!! */
			if (data->skals)
				scale = 0.00015;
			else
				scale = 0.000015;
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++) {
					if (data->spfb[i] < -65535 || data->spfb[i] > 65535) {
						data->spfb[i] = 0;
					}
				}

			/* Calculate bathymetry.
			    The travel times are scaled to
			    seconds, then adjusted for the mean sound
			    speed, then do the simple geometry to
			    calculate depth and cross-track. */
			if (data->skals)
				scale = 0.00015;
			else
				scale = 0.000015;
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++) {
					data->depth[i] = (fabs(scale * data->spfb[i]) * 0.5 * data->cmean) * cos(data->angle[i] * DTR);
					data->distance[i] = data->depth[i] * tan(data->angle[i] * DTR);
					if (data->spfb[i] < 0)
						data->depth[i] = -data->depth[i];
					if (data->Port == -1)
						data->distance[i] = -data->distance[i];
				}

			/* get sidescan data */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->ss_range);
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_PIXELS_PING; i++)
					status = xdr_char(xdrs, (char *)&data->ss[i]);

			/* get attitude data */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->heading_tx);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 5; i++)
					status = xdr_double(xdrs, &data->heading_rx[i]);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->roll_tx);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 5; i++)
					status = xdr_double(xdrs, &data->roll_rx[i]);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->pitch_tx);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 5; i++)
					status = xdr_double(xdrs, &data->pitch_rx[i]);

			/* Establish the time of day for this
			 * ping. "Raw" (travel time) data records
			 * do not contain time of day, only the
			 * internal Reference time. The interrupt
			 * records contain a unix epoch time used
			 * to convert to UTC. */
			if (status == MB_SUCCESS) {
				PingTime = data->datuhr + (data->reftime - *FirstReftime);
				status = mb_get_date(verbose, PingTime, time_i);

				data->PingTime = PingTime;
				data->year = time_i[0];
				data->month = time_i[1];
				data->day = time_i[2];
				data->hour = time_i[3];
				data->minute = time_i[4];
				data->second = time_i[5];
				data->millisecond = time_i[6] / 1000;
			}

			/* output some debug messages */
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "\ndgb2: Setting time of Ping in RAW\n:");
				fprintf(stderr, "dbg2: \t->year:   \t%4d\n", data->year);
				fprintf(stderr, "dbg2: \t->month:  \t%2d\n", data->month);
				fprintf(stderr, "dgb2: \t->day:    \t%2d\n", data->day);
				fprintf(stderr, "dgb2: \t->hour:   \t%2d\n", data->hour);
				fprintf(stderr, "dbg2: \t->minute: \t%2d\n", data->minute);
				fprintf(stderr, "dbg2: \t->second: \t%2d\n", data->second);
				fprintf(stderr, "dbg2: \t->millisecond: \t%3d\n", data->millisecond);
				fprintf(stderr, "\ndbg2: \t->Lat:   \t%.4lf\n", data->lat);
				fprintf(stderr, "\ndbg2: \t->Lon:   \t%.4lf\n", data->lon);
			}
			if (verbose >= 2 && status == MB_SUCCESS)
				fprintf(stderr, "\ndbg2: RAW (1) \t%3d\t%4d %2d %2d %2d:%2d:%2d.%3d\n", data->Port, data->year, data->month,
				        data->day, data->hour, data->minute, data->second, data->millisecond);
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "\ndgb2: Raw\n");
				fprintf(stderr, "dbg2: \tckeel\t%8.2lf\n", data->ckeel);
				fprintf(stderr, "dbg2: \tcmean\t%8.2lf\n", data->cmean);
				fprintf(stderr, "dgb2: \tPort\t%d\n", data->Port);
				fprintf(stderr, "\tnoho\t%d\n", data->noho);
				fprintf(stderr, "\tskals\t%d\n", data->skals);
				fprintf(stderr, "\tspfbs\n");
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i = i + 4) {
					fprintf(stderr, "\t(%02d) %10d (%02d) %10d (%02d) %10d (%02d) %10d\n", i, data->spfb[i], i + 1,
					        data->spfb[i + 1], i + 2, data->spfb[i + 2], i + 3, data->spfb[i + 3]);
				}
				fprintf(stderr, "\tss_range\t%lf\n", data->ss_range);
				fprintf(stderr, "\tampl\n");
				for (int i = 0; i < MBF_HSMDARAW_PIXELS_PING; i = i + 4) {
					fprintf(stderr, "\t%d\t%d\t%d\t%d\n", data->ss[i], data->ss[i + 1], data->ss[i + 2], data->ss[i + 3]);
				}

				fprintf(stderr, "\theading_tx\t%8.3lf\n", data->heading_tx);
				fprintf(stderr, "\theading_rx:\t");
				fprintf(stderr, "%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", data->heading_rx[0], data->heading_rx[1],
				        data->heading_rx[2], data->heading_rx[3], data->heading_rx[4]);

				fprintf(stderr, "\troll_tx\t%8.3lf\n", data->roll_tx);
				fprintf(stderr, "\troll_rx:\t");
				fprintf(stderr, "%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", data->roll_rx[0], data->roll_rx[1], data->roll_rx[2],
				        data->roll_rx[3], data->roll_rx[4]);

				fprintf(stderr, "\tpitch_tx\t%8.3lf\n", data->pitch_tx);
				fprintf(stderr, "\tpitch_rx:\t");
				fprintf(stderr, "%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", data->pitch_rx[0], data->pitch_rx[1], data->pitch_rx[2],
				        data->pitch_rx[3], data->pitch_rx[4]);
			}

			/* check status */
			if (status == MB_SUCCESS) {
				*error = MB_ERROR_NO_ERROR;
			}
			else
				*error = MB_ERROR_EOF;
			break;
		}

		case (MBF_HSMDARAW_BAT): /* 8, LDEO bath data record */
		{
			data->kind = MB_DATA_DATA;

			(*Raw_count)++; /* the number of this kind of record */

			/* get time and position */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->PingTime);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->lon);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->lat);

			/* get water velocity and travel time data */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->ckeel);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->cmean);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->Port);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->noho);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->skals);
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++)
					status = xdr_int(xdrs, &data->spfb[i]);
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++)
					status = xdr_double(xdrs, &data->depth[i]);
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++)
					status = xdr_double(xdrs, &data->distance[i]);

			/* get sidescan data */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->ss_range);
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_PIXELS_PING; i++)
					status = xdr_char(xdrs, (char *)&data->ss[i]);

			/* get attitude data */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->heading_tx);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 5; i++)
					status = xdr_double(xdrs, &data->heading_rx[i]);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->roll_tx);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 5; i++)
					status = xdr_double(xdrs, &data->roll_rx[i]);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->pitch_tx);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 5; i++)
					status = xdr_double(xdrs, &data->pitch_rx[i]);

			/* Establish the time of day for this
			 * ping. "Raw" (travel time) data records
			 * do not contain time of day, only the
			 * internal Reference time. The interrupt
			 * records contain a unix epoch time used
			 * to convert to UTC. */
			if (status == MB_SUCCESS) {
				status = mb_get_date(verbose, data->PingTime, time_i);

				data->year = time_i[0];
				data->month = time_i[1];
				data->day = time_i[2];
				data->hour = time_i[3];
				data->minute = time_i[4];
				data->second = time_i[5];
				data->millisecond = time_i[6] / 1000;
			}

			/* output some debug messages */
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "\ndgb2: Setting time of Ping in RAW\n:");
				fprintf(stderr, "dbg2: \t->year:   \t%4d\n", data->year);
				fprintf(stderr, "dbg2: \t->month:  \t%2d\n", data->month);
				fprintf(stderr, "dgb2: \t->day:    \t%2d\n", data->day);
				fprintf(stderr, "dgb2: \t->hour:   \t%2d\n", data->hour);
				fprintf(stderr, "dbg2: \t->minute: \t%2d\n", data->minute);
				fprintf(stderr, "dbg2: \t->second: \t%2d\n", data->second);
				fprintf(stderr, "dbg2: \t->millisecond: \t%3d\n", data->millisecond);
				fprintf(stderr, "\ndbg2: \t->Lat:   \t%.4lf\n", data->lat);
				fprintf(stderr, "\ndbg2: \t->Lon:   \t%.4lf\n", data->lon);
			}
			if (verbose >= 2 && status == MB_SUCCESS)
				fprintf(stderr, "\ndbg2: RAW (1) \t%3d\t%4d %2d %2d %2d:%2d:%2d.%3d\n", data->Port, data->year, data->month,
				        data->day, data->hour, data->minute, data->second, data->millisecond);
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "\ndgb2: Raw\n");
				fprintf(stderr, "dbg2: \tckeel\t%8.2lf\n", data->ckeel);
				fprintf(stderr, "dbg2: \tcmean\t%8.2lf\n", data->cmean);
				fprintf(stderr, "dgb2: \tPort\t%d\n", data->Port);
				fprintf(stderr, "\tnoho\t%d\n", data->noho);
				fprintf(stderr, "\tskals\t%d\n", data->skals);
				fprintf(stderr, "\tspfbs\n");
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i = i + 4) {
					fprintf(stderr, "\t(%02d) %10d (%02d) %10d (%02d) %10d (%02d) %10d\n", i, data->spfb[i], i + 1,
					        data->spfb[i + 1], i + 2, data->spfb[i + 2], i + 3, data->spfb[i + 3]);
				}
				fprintf(stderr, "\tss_range\t%lf\n", data->ss_range);
				fprintf(stderr, "\tampl\n");
				for (int i = 0; i < MBF_HSMDARAW_PIXELS_PING; i = i + 4) {
					fprintf(stderr, "\t%d\t%d\t%d\t%d\n", data->ss[i], data->ss[i + 1], data->ss[i + 2], data->ss[i + 3]);
				}

				fprintf(stderr, "\theading_tx\t%8.3lf\n", data->heading_tx);
				fprintf(stderr, "\theading_rx:\t");
				fprintf(stderr, "%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", data->heading_rx[0], data->heading_rx[1],
				        data->heading_rx[2], data->heading_rx[3], data->heading_rx[4]);

				fprintf(stderr, "\troll_tx\t%8.3lf\n", data->roll_tx);
				fprintf(stderr, "\troll_rx:\t");
				fprintf(stderr, "%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", data->roll_rx[0], data->roll_rx[1], data->roll_rx[2],
				        data->roll_rx[3], data->roll_rx[4]);

				fprintf(stderr, "\tpitch_tx\t%8.3lf\n", data->pitch_tx);
				fprintf(stderr, "\tpitch_rx:\t");
				fprintf(stderr, "%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", data->pitch_rx[0], data->pitch_rx[1], data->pitch_rx[2],
				        data->pitch_rx[3], data->pitch_rx[4]);
			}

			/* check status */
			if (status == MB_SUCCESS) {
				*error = MB_ERROR_NO_ERROR;
			}
			else
				*error = MB_ERROR_EOF;
			break;
		}

		case (MBF_HSMDARAW_NAV): /* 2, Navigation data record */
		{
			(*Nav_count)++;
			data->kind = MB_DATA_NAV;

			/* get nav data */
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->navid);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->year);
			if (status == MB_SUCCESS)
				mb_fix_y2k(verbose, data->year, &data->year);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->month);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->day);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->hour);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->minute);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->secf);
			if (status == MB_SUCCESS) {
				/* break decimal seconds into integer
				    seconds and msec */
				data->second = (int)data->secf;
				data->millisecond = (int)(1000 * (data->secf - data->second));
			}

			/* get position */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->lat);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->lon);
			if (status == MB_SUCCESS)
				status = xdr_char(xdrs, &data->pos_sens[0]);
			if (status == MB_SUCCESS)
				status = xdr_char(xdrs, &data->pos_sens[1]);

			/* Establish the time of day for this
			 * nav record. Nav data records
			 * do contain time of day, but the values
			 * seem unreliable. Thus, we use the
			 * internal Reference time. The interrupt
			 * records contain a unix epoch time used
			 * to convert to UTC. */
			if (status == MB_SUCCESS) {
				PingTime = data->datuhr + (data->reftime - *FirstReftime);
				status = mb_get_date(verbose, PingTime, time_i);

				data->PingTime = PingTime;
				data->year = time_i[0];
				data->month = time_i[1];
				data->day = time_i[2];
				data->hour = time_i[3];
				data->minute = time_i[4];
				data->second = time_i[5];
				data->millisecond = time_i[6] / 1000;
			}

			/* output some debug messages */
			if (verbose >= 2 && status == MB_SUCCESS)
				fprintf(stderr, "\ndbg2: NAV (2) # %3d\t%4d %2d %2d %2d:%2d:%2d.%3d\n", *Nav_count, data->year, data->month,
				        data->day, data->hour, data->minute, data->second, data->millisecond);
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "dbg2: \nNav\n");
				fprintf(stderr, "dbg2: \t->navid:  \t%d\n", data->navid);
				fprintf(stderr, "dbg2: \t->year:   \t%4d\n", data->year);
				fprintf(stderr, "dbg2: \t->month:  \t%2d\n", data->month);
				fprintf(stderr, "dbg2: \t->day:    \t%2d\n", data->day);
				fprintf(stderr, "dbg2: \t->hour:   \t%2d\n", data->hour);
				fprintf(stderr, "dbg2: \t->minute: \t%2d\n", data->minute);
				fprintf(stderr, "dbg2: \t->second: \t%2d\n", data->second);
				fprintf(stderr, "dbg2: \t->millisec::\t%.3f\n", data->secf);

				fprintf(stderr, "dbg2: \t->lat:    \t%lf\n", data->lat);
				fprintf(stderr, "dbg2: \t->lon:    \t%lf\n", data->lon);
				fprintf(stderr, "dbg2: \t->pos_sens:\t%s\n", data->pos_sens);
			}
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "dbg2: %4d %2d %3d %2d %2d %2d %d %10.5lf %10.5lf %2d %2s \n", data->year, data->month, data->day,
				        data->hour, data->minute, data->second, data->millisecond, data->lat, data->lon, data->navid,
				        data->pos_sens);
			}

			/* check status */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_EOF;
			break;
		}

		case (MBF_HSMDARAW_MDE): /* 3, MD Event */
		{
			(*MDevent_count)++;
			data->kind = MB_DATA_EVENT;

			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->evid);

			if (status == MB_SUCCESS)
				for (int i = 0; i < 84; i++)
					status = xdr_char(xdrs, &data->evtext[i]);

			/* Establish the time of day for this
			 * record. Event data records
			 * do not contain time of day, only the
			 * internal Reference time. The interrupt
			 * records contain a unix epoch time used
			 * to convert to UTC. */
			if (status == MB_SUCCESS) {
				PingTime = data->datuhr + (data->reftime - *FirstReftime);
				status = mb_get_date(verbose, PingTime, time_i);

				data->PingTime = PingTime;
				data->year = time_i[0];
				data->month = time_i[1];
				data->day = time_i[2];
				data->hour = time_i[3];
				data->minute = time_i[4];
				data->second = time_i[5];
				data->millisecond = time_i[6] / 1000;
			}

			/* output some debug messages */
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "MDE (3) # %d\n", *MDevent_count);
			}
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "MDE Event->\n");
				fprintf(stderr, "\t->evid:\t%d\n", data->evid);
				fprintf(stderr, "\t->evtxt:\t%s\n", data->evtext);
			}

			/* check status */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_EOF;
			break;
		}

		case (MBF_HSMDARAW_ANG): /* Transid == 4, Beam Angles */
		{
			(*Angle_count)++;
			data->kind = MB_DATA_ANGLE;

			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->noho);

			if (status == MB_SUCCESS && status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++) {
					status = xdr_double(xdrs, &data->angle[i]);
					mbf_hsmdaraw_beamangle[i] = data->angle[i];
				}

			/* Establish the time of day for this
			 * record. Angle data records
			 * do not contain time of day, only the
			 * internal Reference time. The interrupt
			 * records contain a unix epoch time used
			 * to convert to UTC. */
			if (status == MB_SUCCESS) {
				PingTime = data->datuhr + (data->reftime - *FirstReftime);
				status = mb_get_date(verbose, PingTime, time_i);

				data->PingTime = PingTime;
				data->year = time_i[0];
				data->month = time_i[1];
				data->day = time_i[2];
				data->hour = time_i[3];
				data->minute = time_i[4];
				data->second = time_i[5];
				data->millisecond = time_i[6] / 1000;
			}

			/* output some debug messages */
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "\ndbg2: ANG (4) # %d\n", *Angle_count);
			}
			if (verbose >= 5 && status == MB_SUCCESS) {
				fprintf(stderr, "\ndgb5: Ang");
				fprintf(stderr, "dbg5:\tnoho:\t%d\n", data->noho);
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i = i + 4) {
					fprintf(stderr, "\t%02d: %8.3lf\t%02d: %8.3lf\t%02d: %8.3lf\t%02d: %8.3lf\n", i, data->angle[i], i + 1,
					        data->angle[i + 1], i + 2, data->angle[i + 2], i + 3, data->angle[i + 3]);
				}
			}

			/* check status */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_EOF;
			break;
		}

		case (MBF_HSMDARAW_SVP): /* 5, Sound Velocity Profile */
		{
			(*Svp_count)++;
			data->kind = MB_DATA_VELOCITY_PROFILE;

			data->num_vel = 20;
			for (int i = 0; i < data->num_vel; i++) {
				status = xdr_double(xdrs, &data->vdepth[i]);
				status = xdr_double(xdrs, &data->velocity[i]);
			}

			/* Establish the time of day for this
			 * record. SVP data records
			 * do not contain time of day, only the
			 * internal Reference time. The interrupt
			 * records contain a unix epoch time used
			 * to convert to UTC. */
			if (status == MB_SUCCESS) {
				PingTime = data->datuhr + (data->reftime - *FirstReftime);
				status = mb_get_date(verbose, PingTime, time_i);

				data->PingTime = PingTime;
				data->year = time_i[0];
				data->month = time_i[1];
				data->day = time_i[2];
				data->hour = time_i[3];
				data->minute = time_i[4];
				data->second = time_i[5];
				data->millisecond = time_i[6] / 1000;
			}

			/* output some debug messages */
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "\ndbg2: SVP (5) # %d\n", *Svp_count);
			}

			/* check status */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_EOF;
			break;
		}

		case (MBF_HSMDARAW_REV): /* 6, An Interrupt event? */
		{
			(*Rev_count)++;

			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->datuhr);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 8; i++)
					status = xdr_char(xdrs, &data->mksysint[i]);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 84; i++)
					status = xdr_char(xdrs, &data->mktext[i]);

			/* Establish the time of day for this
			 * record. Interrupt data records
			 * contain a unix time which is used
			 * to get time of day. */
			if (status == MB_SUCCESS) {
				PingTime = data->datuhr + (data->reftime - *FirstReftime);
				status = mb_get_date(verbose, PingTime, time_i);

				data->PingTime = PingTime;
				data->year = time_i[0];
				data->month = time_i[1];
				data->day = time_i[2];
				data->hour = time_i[3];
				data->minute = time_i[4];
				data->second = time_i[5];
				data->millisecond = time_i[6] / 1000;
			}

			/* output some debug messages */
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "dbg2:\n REV (6) # %d\t%.3lf", *Rev_count, data->datuhr);
			}
			if (verbose >= 5 && status == MB_SUCCESS) {
				fprintf(stderr, "\nIntevent");
				fprintf(stderr, "->datuhr:  \t%lf\n", data->datuhr);
				fprintf(stderr, "\t->mksysint:\t%s\n", data->mksysint);
				fprintf(stderr, "\t->mktext:  \t%s\n", data->mktext);
			}

			/* Check to see if this Raw Event is
			 * indicating the start or end of the file. */
			if (status == MB_SUCCESS && strncmp(data->mksysint, "STOP", 4) == 0) {
				data->kind = MB_DATA_STOP;
				*error = MB_ERROR_NO_ERROR;
			}
			else if (status == MB_SUCCESS) {
				data->kind = MB_DATA_START;
				*error = MB_ERROR_NO_ERROR;
			}
			else {
				*error = MB_ERROR_EOF;
			}
			break;
		}

		case (MBF_HSMDARAW_COM): /* 7, Comment */
		{
			data->kind = MB_DATA_COMMENT;

			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_COMMENT; i++)
					status = xdr_char(xdrs, &data->comment[i]);

			/* Check status. */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_EOF;
			break;
		}

		default: {
			/* Should never get here, so fail! */
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;

			if (verbose >= 2) {
				fprintf(stderr, "dbg2: data->transid=%d not parsed\n", data->transid);
			}
			break;
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
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_hsmdaraw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	int time_i[7];
	double time_d;
	double lon, lat, heading, speed;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbf_hsmdaraw_struct *data = (struct mbf_hsmdaraw_struct *)mb_io_ptr->raw_data;
	struct mbsys_hsmd_struct *store = (struct mbsys_hsmd_struct *)store_ptr;

	/* read next (record of) data from file */
	const int status = mbr_hsmdaraw_rd_data(verbose, mbio_ptr, error);

	if (verbose >= 5) {
		fprintf(stderr, "dbg5: In function name:\t%s\n", __func__);
		fprintf(stderr, "dbg5:\t Returned from  mbr_hsmdaraw_rd_data()\n");
		fprintf(stderr, "dbg5:\t Status:\t%d\n", status);
		fprintf(stderr, "dbg5:\t data->kind:\t%d\n", data->kind);
		fprintf(stderr, "dbg5:\t store_ptr: \t%p\n", (void *)store_ptr);
	}

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* add nav records to list for interpolation */
	if (status == MB_SUCCESS && data->kind == MB_DATA_NAV) {
		time_i[0] = data->year;
		time_i[1] = data->month;
		time_i[2] = data->day;
		time_i[3] = data->hour;
		time_i[4] = data->minute;
		time_i[5] = data->second;
		time_i[6] = 1000 * data->millisecond;
		mb_get_time(verbose, time_i, &time_d);
		lon = data->lon;
		lat = data->lat;
		mb_navint_add(verbose, mbio_ptr, time_d, lon, lat, error);
	}

	/* interpolate navigation for survey pings if needed */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA && mb_io_ptr->nfix >= 1) {
		time_i[0] = data->year;
		time_i[1] = data->month;
		time_i[2] = data->day;
		time_i[3] = data->hour;
		time_i[4] = data->minute;
		time_i[5] = data->second;
		time_i[6] = 1000 * data->millisecond;
		mb_get_time(verbose, time_i, &time_d);
		heading = data->heading_tx;
		mb_navint_interp(verbose, mbio_ptr, time_d, heading, 0.0, &lon, &lat, &speed, error);
		data->lon = lon;
		data->lat = lat;
		data->speed = speed;
	}

	/* translate values to data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		/* type of data record */
		store->kind = data->kind;

		/* header values */
		for (int i = 0; i < 4; i++) {
			store->scsid[i] = data->scsid[i];
			store->scsart[i] = data->scsart[i];
		}
		store->scslng = data->scslng;
		store->scsext = data->scsext;
		store->scsblcnt = data->scsblcnt;
		store->scsres1 = data->scsres1;
		store->transid = data->transid;
		store->reftime = data->reftime;

		/* event data */
		store->datuhr = data->datuhr;
		for (int i = 0; i < 8; i++)
			store->mksysint[i] = data->mksysint[i];
		for (int i = 0; i < 84; i++)
			store->mktext[i] = data->mktext[i];

		/* navigation data */
		store->navid = data->navid;
		store->year = data->year;
		store->month = data->month;
		store->day = data->day;
		store->hour = data->hour;
		store->minute = data->minute;
		store->second = data->second;
		store->secf = data->secf;
		store->millisecond = data->millisecond;
		store->PingTime = data->PingTime;
		store->lon = data->lon;
		store->lat = data->lat;
		store->pos_sens[0] = data->pos_sens[0];
		store->pos_sens[1] = data->pos_sens[1];

		/* travel time, bathymetry and sidescan data */
		store->ckeel = data->ckeel;
		store->cmean = data->cmean;
		store->Port = data->Port;
		store->noho = data->noho;
		store->skals = data->skals;
		for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++) {
			store->spfb[i] = data->spfb[i];
			store->depth[i] = data->depth[i];
			store->distance[i] = data->distance[i];
			store->angle[i] = data->angle[i];
		}
		store->ss_range = data->ss_range;
		for (int i = 0; i < MBF_HSMDARAW_PIXELS_PING; i++) {
			store->ss[i] = data->ss[i];
		}
		store->heading_tx = data->heading_tx;
		store->roll_tx = data->roll_tx;
		store->pitch_tx = data->pitch_tx;
		for (int i = 0; i < 5; i++) {
			store->heading_rx[i] = data->heading_rx[i];
			store->pitch_rx[i] = data->pitch_rx[i];
			store->roll_rx[i] = data->roll_rx[i];
		}

		/* MD event data */
		store->evid = data->evid;
		for (int i = 0; i < 84; i++)
			store->evtext[i] = data->evtext[i];

		store->num_vel = data->num_vel;
		for (int i = 0; i < data->num_vel; i++) {
			store->vdepth[i] = data->vdepth[i];
			store->velocity[i] = data->velocity[i];
		}

		/* comment */
		strncpy(store->comment, data->comment, MBSYS_HSMD_COMMENT);
		store->heave = data->heave;
		store->speed = data->speed;
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
int mbr_hsmdaraw_wr_data(int verbose, void *mbio_ptr, char *data_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       data_ptr:   %p\n", (void *)data_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_hsmdaraw_struct *data = (struct mbf_hsmdaraw_struct *)data_ptr;
	XDR *xdrs = mb_io_ptr->xdrs; /* xdr i/o pointer */

	/* make sure transid is correct */
	if (data->transid == MBF_HSMDARAW_BAT)
		data->transid = MBF_HSMDARAW_RAW;

	int status = MB_SUCCESS;

	/* Start writing an HSMD Header structure */
	for (int i = 0; i < 4; i++)
		status = xdr_char(xdrs, &data->scsid[i]);
	if (status == MB_SUCCESS)
		for (int i = 0; i < 4; i++)
			status = xdr_char(xdrs, &data->scsart[i]);
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->scslng);
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->scsext);
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->scsblcnt);
	if (status == MB_SUCCESS)
		status = xdr_double(xdrs, &data->scsres1);
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->transid);
	if (status == MB_SUCCESS)
		status = xdr_double(xdrs, &data->reftime);

	/* write the appropriate data record */
	if (status == MB_SUCCESS) {
		switch (data->transid) {
		case (MBF_HSMDARAW_RAW): /* 1, Raw data record */
		{
			/* make sure transid is correct */
			data->transid = MBF_HSMDARAW_RAW;

			/* First make sure bathymetry edits are
			    carried over into travel times */
			for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++) {
				if (data->depth[i] < 0.0 && data->spfb[i] > 0.0)
					data->spfb[i] = -data->spfb[i];
				else if (data->depth[i] > 0.0 && data->spfb[i] < 0.0)
					data->spfb[i] = -data->spfb[i];
			}

			/* output some debug messages */
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "\ndgb2: Setting time of Ping in RAW\n:");
				fprintf(stderr, "dbg2: \t->year:   \t%4d\n", data->year);
				fprintf(stderr, "dbg2: \t->month:  \t%2d\n", data->month);
				fprintf(stderr, "dgb2: \t->day:    \t%2d\n", data->day);
				fprintf(stderr, "dgb2: \t->hour:   \t%2d\n", data->hour);
				fprintf(stderr, "dbg2: \t->minute: \t%2d\n", data->minute);
				fprintf(stderr, "dbg2: \t->second: \t%2d\n", data->second);
				fprintf(stderr, "dbg2: \t->millisecond: \t%3d\n", data->millisecond);
				fprintf(stderr, "\ndbg2: \t->Lat:   \t%.4lf\n", data->lat);
				fprintf(stderr, "\ndbg2: \t->Lon:   \t%.4lf\n", data->lon);
			}
			if (verbose >= 2 && status == MB_SUCCESS)
				fprintf(stderr, "\ndbg2: RAW (1) \t%3d\t%4d %2d %2d %2d:%2d:%2d.%3d\n", data->Port, data->year, data->month,
				        data->day, data->hour, data->minute, data->second, data->millisecond);
			if (verbose >= 2 && status == MB_SUCCESS) {
				fprintf(stderr, "\ndgb2: Raw\n");
				fprintf(stderr, "dbg2: \tckeel\t%8.2lf\n", data->ckeel);
				fprintf(stderr, "dbg2: \tcmean\t%8.2lf\n", data->cmean);
				fprintf(stderr, "dgb2: \tPort\t%d\n", data->Port);
				fprintf(stderr, "\tnoho\t%d\n", data->noho);
				fprintf(stderr, "\tskals\t%d\n", data->skals);
				fprintf(stderr, "\tspfbs\n");
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i = i + 4) {
					fprintf(stderr, "\t(%02d) %10d (%02d) %10d (%02d) %10d (%02d) %10d\n", i, data->spfb[i], i + 1,
					        data->spfb[i + 1], i + 2, data->spfb[i + 2], i + 3, data->spfb[i + 3]);
				}
				fprintf(stderr, "\tss_range\t%lf\n", data->ss_range);
				fprintf(stderr, "\tampl\n");
				for (int i = 0; i < MBF_HSMDARAW_PIXELS_PING; i = i + 4) {
					fprintf(stderr, "\t%d\t%d\t%d\t%d\n", data->ss[i], data->ss[i + 1], data->ss[i + 2], data->ss[i + 3]);
				}

				fprintf(stderr, "\theading_tx\t%8.3lf\n", data->heading_tx);
				fprintf(stderr, "\theading_rx:\t");
				fprintf(stderr, "%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", data->heading_rx[0], data->heading_rx[1],
				        data->heading_rx[2], data->heading_rx[3], data->heading_rx[4]);

				fprintf(stderr, "\troll_tx\t%8.3lf\n", data->roll_tx);
				fprintf(stderr, "\troll_rx:\t");
				fprintf(stderr, "%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", data->roll_rx[0], data->roll_rx[1], data->roll_rx[2],
				        data->roll_rx[3], data->roll_rx[4]);

				fprintf(stderr, "\tpitch_tx\t%8.3lf\n", data->pitch_tx);
				fprintf(stderr, "\tpitch_rx:\t");
				fprintf(stderr, "%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", data->pitch_rx[0], data->pitch_rx[1], data->pitch_rx[2],
				        data->pitch_rx[3], data->pitch_rx[4]);
			}

			/* set water velocity and travel time data */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->ckeel);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->cmean);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->Port);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->noho);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->skals);
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++)
					status = xdr_int(xdrs, &data->spfb[i]);

			/* set sidescan data */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->ss_range);
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_PIXELS_PING; i++)
					status = xdr_char(xdrs, (char *)&data->ss[i]);

			/* set attitude data */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->heading_tx);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 5; i++)
					status = xdr_double(xdrs, &data->heading_rx[i]);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->roll_tx);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 5; i++)
					status = xdr_double(xdrs, &data->roll_rx[i]);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->pitch_tx);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 5; i++)
					status = xdr_double(xdrs, &data->pitch_rx[i]);

			/* check status */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_WRITE_FAIL;
			break;
		}

		case (MBF_HSMDARAW_NAV): /* 2, Navigation data record */
		{
			/* set nav data */
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->navid);
			if (status == MB_SUCCESS)
				mb_unfix_y2k(verbose, data->year, &data->year);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->year);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->month);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->day);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->hour);
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->minute);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->secf);

			/* set position */
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->lat);
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->lon);
			if (status == MB_SUCCESS)
				status = xdr_char(xdrs, &data->pos_sens[0]);
			if (status == MB_SUCCESS)
				status = xdr_char(xdrs, &data->pos_sens[1]);

			/* check status */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_WRITE_FAIL;
			break;
		}

		case (MBF_HSMDARAW_MDE): /* 3, MD Event */
		{
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->evid);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 84; i++)
					status = xdr_char(xdrs, &data->evtext[i]);

			/* check status */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_WRITE_FAIL;
			break;
		}

		case (MBF_HSMDARAW_ANG): /* Transid == 4, Beam Angles */
		{
			if (status == MB_SUCCESS)
				status = xdr_int(xdrs, &data->noho);
			if (status == MB_SUCCESS && status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++)
					status = xdr_double(xdrs, &data->angle[i]);

			/* check status */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_WRITE_FAIL;
			break;
		}

		case (MBF_HSMDARAW_SVP): /* 5, Sound Velocity Profile */
		{

			data->num_vel = 20;
			for (int i = 0; i < data->num_vel; i++) {
				status = xdr_double(xdrs, &data->vdepth[i]);
				status = xdr_double(xdrs, &data->velocity[i]);
			}

			/* check status */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_WRITE_FAIL;
			break;
		}

		case (MBF_HSMDARAW_REV): /* 6, An Interrupt event? */
		{
			if (status == MB_SUCCESS)
				status = xdr_double(xdrs, &data->datuhr);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 8; i++)
					status = xdr_char(xdrs, &data->mksysint[i]);
			if (status == MB_SUCCESS)
				for (int i = 0; i < 84; i++)
					status = xdr_char(xdrs, &data->mktext[i]);

			/* Check status. */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_WRITE_FAIL;
			break;
		}

		case (MBF_HSMDARAW_COM): /* 7, Comment */
		{
			if (status == MB_SUCCESS)
				for (int i = 0; i < MBF_HSMDARAW_COMMENT; i++)
					status = xdr_char(xdrs, &data->comment[i]);

			/* Check status. */
			if (status == MB_SUCCESS)
				*error = MB_ERROR_NO_ERROR;
			else
				*error = MB_ERROR_WRITE_FAIL;
			break;
		}

		default: {
			/* Should never get here, so fail! */
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			break;
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
int mbr_wt_hsmdaraw(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_hsmdaraw_struct *data = (struct mbf_hsmdaraw_struct *)mb_io_ptr->raw_data;
	char *data_ptr = (char *)data;
	struct mbsys_hsmd_struct *store = (struct mbsys_hsmd_struct *)store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL) {
		/* type of data record */
		data->kind = store->kind;

		/* header values */
		for (int i = 0; i < 4; i++) {
			data->scsid[i] = store->scsid[i];
			data->scsart[i] = store->scsart[i];
		}
		data->scslng = store->scslng;
		data->scsext = store->scsext;
		data->scsblcnt = store->scsblcnt;
		data->scsres1 = store->scsres1;
		data->transid = store->transid;
		data->reftime = store->reftime;

		/* event data */
		data->datuhr = store->datuhr;
		for (int i = 0; i < 8; i++)
			data->mksysint[i] = store->mksysint[i];
		for (int i = 0; i < 84; i++)
			data->mktext[i] = store->mktext[i];

		/* navigation data */
		data->navid = store->navid;
		data->year = store->year;
		data->month = store->month;
		data->day = store->day;
		data->hour = store->hour;
		data->minute = store->minute;
		data->second = store->second;
		data->secf = store->secf;
		data->millisecond = store->millisecond;
		data->PingTime = store->PingTime;
		data->lon = store->lon;
		data->lat = store->lat;
		data->pos_sens[0] = store->pos_sens[0];
		data->pos_sens[1] = store->pos_sens[1];

		/* travel time, bathymetry and sidescan data */
		data->ckeel = store->ckeel;
		data->cmean = store->cmean;
		data->Port = store->Port;
		data->noho = store->noho;
		data->skals = store->skals;
		for (int i = 0; i < MBF_HSMDARAW_BEAMS_PING; i++) {
			data->spfb[i] = store->spfb[i];
			data->depth[i] = store->depth[i];
			data->distance[i] = store->distance[i];
			data->angle[i] = store->angle[i];
		}
		data->ss_range = store->ss_range;
		for (int i = 0; i < MBF_HSMDARAW_PIXELS_PING; i++) {
			data->ss[i] = store->ss[i];
		}
		data->heading_tx = store->heading_tx;
		data->roll_tx = store->roll_tx;
		data->pitch_tx = store->pitch_tx;
		for (int i = 0; i < 5; i++) {
			data->heading_rx[i] = store->heading_rx[i];
			data->pitch_rx[i] = store->pitch_rx[i];
			data->roll_rx[i] = store->roll_rx[i];
		}

		/* MD event data */
		data->evid = store->evid;
		for (int i = 0; i < 84; i++)
			data->evtext[i] = store->evtext[i];

		data->num_vel = store->num_vel;
		for (int i = 0; i < store->num_vel; i++) {
			data->vdepth[i] = store->vdepth[i];
			data->velocity[i] = store->velocity[i];
		}

		/* comment */
		strncpy(data->comment, store->comment, MBSYS_HSMD_COMMENT);
		data->heave = store->heave;
		data->speed = store->speed;
	}

	/* write next data to file */
	const int status = mbr_hsmdaraw_wr_data(verbose, mbio_ptr, data_ptr, error);

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
int mbr_register_hsmdaraw(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_hsmdaraw(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hsmdaraw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hsmdaraw;
	mb_io_ptr->mb_io_store_alloc = &mbsys_hsmd_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_hsmd_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hsmdaraw;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hsmdaraw;
	mb_io_ptr->mb_io_dimensions = &mbsys_hsmd_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_hsmd_extract;
	mb_io_ptr->mb_io_insert = &mbsys_hsmd_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_hsmd_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_hsmd_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_hsmd_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_hsmd_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_hsmd_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_hsmd_copy;
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
