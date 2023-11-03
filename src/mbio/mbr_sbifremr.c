/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sbifremr.c	3/29/96
 *
 *    Copyright (c) 1996-2023 by
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
 * mbr_sbifremr.c contains the functions for reading and writing
 * multibeam data in the SBIFREMR format.
 * These functions include:
 *   mbr_alm_sbifremr	- allocate read/write memory
 *   mbr_dem_sbifremr	- deallocate read/write memory
 *   mbr_rt_sbifremr	- read and translate data
 *   mbr_wt_sbifremr	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 29, 1996
 * Location:	152 39.061W; 34 09.150S on R/V Ewing
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
#include "mbf_sbifremr.h"
#include "mbsys_sb.h"

/*--------------------------------------------------------------------*/
int mbr_info_sbifremr(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_SB;
	*beams_bath_max = 19;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "SBIFREMR", MB_NAME_LENGTH);
	strncpy(system_name, "SB", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_SBIFREMR\nInformal Description: IFREMER Archive SeaBeam\nAttributes:           Sea Beam, "
	        "bathymetry, 19 beams, ascii, centered,\n                      IFREMER.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = false;
	*traveltime = false;
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
int mbr_alm_sbifremr(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_sbifremr_struct);
	mb_io_ptr->data_structure_size = 0;
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status &= mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_sb_struct), &mb_io_ptr->store_data, error);
	status &= mb_mallocd(verbose, __FILE__, __LINE__, MBF_SBIFREMR_MAXLINE, &mb_io_ptr->saveptr1, error);

  int *ping_num_save = &mb_io_ptr->save1;
  double *heading_save = &mb_io_ptr->saved1;
  int *sounding_num_save = &mb_io_ptr->save2;
  bool *first = &mb_io_ptr->saveb1;
  bool *line_save = &mb_io_ptr->saveb2;
  *ping_num_save = 0;
  *heading_save = 0.0;
  *sounding_num_save = 0;
  *first = true;
  *line_save = false;

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
int mbr_dem_sbifremr(int verbose, void *mbio_ptr, int *error) {
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
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->saveptr1, error);

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
int mbr_sbifremr_rd_data(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_sbifremr_struct *data = (struct mbf_sbifremr_struct *)mb_io_ptr->raw_data;
	FILE *mbfp = mb_io_ptr->mbfp;
	char *line = (char *) mb_io_ptr->saveptr1;
  int *ping_num_save = &mb_io_ptr->save1;
  double *heading_save = &mb_io_ptr->saved1;
  bool *first = &mb_io_ptr->saveb1;
  bool *line_save = &mb_io_ptr->saveb2;

	/* initialize beams to zeros */
	for (int i = 0; i < MBF_SBIFREMR_NUM_BEAMS; i++) {
		data->deph[i] = 0;
		data->dist[i] = 0;
	}

	int day;
	int month;
	int year;
	int hour;
	int minute;
	int second;
	int tsecond;

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	int center = MBF_SBIFREMR_NUM_BEAMS / 2;
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	bool done = false;
	while (!done) {

		char *result = NULL;
		/* get next line */
		if (!(*line_save)) {
			mb_io_ptr->file_bytes = ftell(mbfp);
			strncpy(line, "", MBF_SBIFREMR_MAXLINE);
			result = fgets(line, MBF_SBIFREMR_MAXLINE, mbfp);
		}
		else {
			*line_save = false;
			result = line;
		}

		/* check size of line */
		const int nchars = strlen(line);

		/* deal with end of file */
		if (result == NULL) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			done = true;
		}

		/* deal with comment */
		else if (nchars > 2 && line[0] == '#' && line[1] == '#') {
			strncpy(data->comment, &line[2], MBF_SBIFREMR_MAXLINE - 3);
			data->kind = MB_DATA_COMMENT;
			done = true;
			*first = true;
		}

		/* deal with good line */
		else if (nchars > 96) {
			/* get ping number */
			int ping_num;
			mb_get_int(&ping_num, line + 52, 7);
			int beam_num;
			mb_get_int(&beam_num, line + 59, 4);
			beam_num = 19 - beam_num;

			/* check if new ping */
			if (ping_num != *ping_num_save && !(*first)) {
				done = true;
				*line_save = true;
				*first = true;
			}

			/* else convert and store the data */
			else if (beam_num > -1 && beam_num < 19) {
				/* parse the line */
				const char NorS = line[0];
				int lat_deg;
				mb_get_int(&lat_deg, line + 1, 2);
				double lat_min;
				mb_get_double(&lat_min, line + 3, 8);
				const char EorW = line[12];
				int lon_deg;
				mb_get_int(&lon_deg, line + 13, 3);
				double lon_min;
				mb_get_double(&lon_min, line + 16, 8);
				double depth;
				mb_get_double(&depth, line + 24, 11);
				mb_get_int(&day, line + 76, 2);
				mb_get_int(&month, line + 79, 2);
				mb_get_int(&year, line + 82, 2);
				mb_get_int(&hour, line + 85, 2);
				mb_get_int(&minute, line + 88, 2);
				mb_get_int(&second, line + 91, 2);
				mb_get_int(&tsecond, line + 94, 2);

				/* convert data */
				data->kind = MB_DATA_DATA;
				data->lon[beam_num] = lon_deg + lon_min / 60.0;
				if (EorW == 'W')
					data->lon[beam_num] = -1.0 * data->lon[beam_num];
				data->lat[beam_num] = lat_deg + lat_min / 60.0;
				if (NorS == 'S')
					data->lat[beam_num] = -1.0 * data->lat[beam_num];
				data->deph[beam_num] = -depth;
				*first = false;
				*ping_num_save = ping_num;
			}
		}
	}

	/* if success convert the data */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA) {
		/* do time */
		int time_i[7];
		mb_fix_y2k(verbose, year, &time_i[0]);
		time_i[1] = month;
		time_i[2] = day;
		time_i[3] = hour;
		time_i[4] = minute;
		time_i[5] = second;
		time_i[6] = 0;
		int time_j[5];
		mb_get_jtime(verbose, time_i, time_j);
		data->year = time_j[0];
		data->day = time_j[1];
		data->min = time_j[2];
		data->sec = time_j[3];

		/* do nav */
		data->lon2u = (unsigned short)60.0 * data->lon[center];
		data->lon2b = (unsigned short)(600000.0 * (data->lon[center] - data->lon2u / 60.0));
		data->lat2u = (unsigned short)60.0 * (90.0 + data->lat[center]);
		data->lat2b = (unsigned short)(600000.0 * (data->lat[center] + 90.0 - data->lat2u / 60.0));

		/* get coordinate scaling */
		double mtodeglon;
		double mtodeglat;
		mb_coor_scale(verbose, data->lat[center], &mtodeglon, &mtodeglat);

		/* find port-most and starboard-most beams */
		int beam_port = MBF_SBIFREMR_NUM_BEAMS;
		int beam_starboard = -1;
		for (int i = 0; i < MBF_SBIFREMR_NUM_BEAMS; i++) {
			if (data->deph[i] != 0) {
				if (beam_port > i)
					beam_port = i;
				if (beam_starboard < i)
					beam_starboard = i;
			}
		}
		double heading;
		if (beam_starboard > beam_port) {
			double dx = (data->lon[beam_port] - data->lon[beam_starboard]) / mtodeglon;
			double dy = (data->lat[beam_port] - data->lat[beam_starboard]) / mtodeglat;
			const double denom = sqrt(dx * dx + dy * dy);
			if (denom > 0.0) {
				dx = dx / denom;
				dy = dy / denom;
				heading = RTD * atan2(dx, dy) - 90.0;
				if (heading < 0.0)
					heading = heading + 360.0;
				if (heading > 360.0)
					heading = heading - 360.0;
			}
			else
				heading = *heading_save;
		}
		else
			heading = *heading_save;

		/* do heading */
		*heading_save = heading;
		data->sbhdg = (short int)(heading * 182.044444);

		/* if needed try to get center beam lon and lat */
		if (data->deph[center] == 0) {
			/* the code below commented out because it does not work well enough
			    - simply ignore pings without center beams */
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;

			/*			headingx = sin(heading * DTR);
			            headingy = cos(heading * DTR);
			            data->lon[center] = 0.0;
			            data->lat[center] = 0.0;
			            count = 0;
			            for (i=0;i<MBF_SBIFREMR_NUM_BEAMS;i++)
			                {
			                if (data->deph[i] != 0)
			                    {
			                    depth = fabs((double)data->deph[i]);
			                    angle = DTR * fabs(MBF_SBIFREMR_ANGLE_SPACING * (i - center));
			                    distance = depth * tan(angle);
			                    data->lon[center] +=
			                        data->lon[i]
			                        - mtodeglon * headingy
			                        * distance;
			                    data->lat[center] +=
			                        data->lat[i]
			                        + mtodeglat * headingx
			                        * distance;
			                    count++;
			                    }
			                }
			            if (count > 0)
			                {
			                data->lon[center] =
			                    data->lon[center] / count;
			                data->lat[center] =
			                    data->lat[center] / count;
			                }
			            else
			                {
			                status = MB_FAILURE;
			                *error = MB_ERROR_UNINTELLIGIBLE;
			                }
			*/
			/* end code commented out */
		}

		/* do acrosstrack distances */
		if (status == MB_SUCCESS) {
			for (int i = 0; i < MBF_SBIFREMR_NUM_BEAMS; i++)
				if (data->deph[i] != 0) {
					const double dx = (data->lon[i] - data->lon[center]) / mtodeglon;
					const double dy = (data->lat[i] - data->lat[center]) / mtodeglat;
					double distance = sqrt(dx * dx + dy * dy);
					if (i > center)
						distance = -distance;
					data->dist[i] = distance;
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
int mbr_rt_sbifremr(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_sbifremr_struct *data = (struct mbf_sbifremr_struct *)mb_io_ptr->raw_data;
	data->kind = MB_DATA_DATA;
	struct mbsys_sb_struct *store = (struct mbsys_sb_struct *)store_ptr;

	/* read next record from file */
	const int status = mbr_sbifremr_rd_data(verbose, mbio_ptr, error);

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = data->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to sea beam data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		/* type of data record */
		store->kind = data->kind;

		/* position */
		store->lon2u = data->lon2u;
		store->lon2b = data->lon2b;
		store->lat2u = data->lat2u;
		store->lat2b = data->lat2b;

		/* time stamp */
		store->year = data->year;
		store->day = data->day;
		store->min = data->min;
		store->sec = data->sec;

		/* depths and distances */
		for (int i = 0; i < MBSYS_SB_BEAMS; i++) {
			store->dist[i] = data->dist[i];
			store->deph[i] = data->deph[i];
		}

		/* additional values */
		store->sbtim = 0;
		store->sbhdg = data->sbhdg;
		store->axis = 0;
		store->major = 0;
		store->minor = 0;

		/* comment */
		strncpy(store->comment, data->comment, MBSYS_SB_MAXLINE);
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
int mbr_sbifremr_wr_data(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	struct mbf_sbifremr_struct *data = (struct mbf_sbifremr_struct *)mb_io_ptr->raw_data;
	FILE *mbfp = mb_io_ptr->mbfp;

  int *ping_num_save = &mb_io_ptr->save1;
  int *sounding_num_save = &mb_io_ptr->save2;

	int status = MB_SUCCESS;

	/* write comment */
	if (data->kind == MB_DATA_COMMENT) {
		/* output the line */
		status = fprintf(mbfp, "##%s\n", data->comment);
		if (status >= 0) {
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}
		else {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}

	/* write data */
	else if (data->kind == MB_DATA_DATA) {
		/* increment ping counter */
		(*ping_num_save)++;

		/* get time */
		int time_j[5];
		time_j[0] = data->year;
		time_j[1] = data->day;
		time_j[2] = data->min;
		time_j[3] = data->sec;
		time_j[4] = 0;
		int time_i[7];
		mb_get_itime(verbose, time_j, time_i);
		int year;
		mb_unfix_y2k(verbose, time_i[0], &year);
		const int month = time_i[1];
		const int day = time_i[2];
		const int hour = time_i[3];
		const int minute = time_i[4];
		const int second = time_i[5];

		/* get lon lat */
		double lon = data->lon2u / 60. + data->lon2b / 600000.;
		double lat = data->lat2u / 60. + data->lat2b / 600000. - 90.;
		if (lon > 180.0)
			lon = lon - 360.0;
		else if (lon < -180.0)
			lon = lon + 360.0;

		/* get coordinate scaling */
		const double heading = 0.0054932 * data->sbhdg;
		data->sbhdg = (short int)(heading * 182.044444);
		double mtodeglon;
		double mtodeglat;
		mb_coor_scale(verbose, lat, &mtodeglon, &mtodeglat);
		const double headingx = sin(heading * DTR);
		const double headingy = cos(heading * DTR);

		/* write beams */
		for (int i = 0; i < MBF_SBIFREMR_NUM_BEAMS; i++) {
			if (data->deph[i] != 0) {
				/* increment sounding counter */
				(*sounding_num_save)++;

				/* get lon lat for beam */
				data->lon[i] = lon + headingy * mtodeglon * data->dist[i];
				data->lat[i] = lat - headingx * mtodeglat * data->dist[i];

				/* get printing values */
				const int beam_num = 19 - i;
				if (data->lon[i] > 180.0)
					data->lon[i] = data->lon[i] - 360.0;
				else if (data->lon[i] < -180.0)
					data->lon[i] = data->lon[i] + 360.0;
				char EorW;
				if (data->lon[i] < 0.0) {
					EorW = 'W';
					data->lon[i] = -data->lon[i];
				}
				else
					EorW = 'E';
				const int lon_deg = (int)data->lon[i];
				const double lon_min = (data->lon[i] - lon_deg) * 60.0;
				char NorS;
				if (data->lat[i] < 0.0) {
					NorS = 'S';
					data->lat[i] = -data->lat[i];
				}
				else
					NorS = 'N';
				const int lat_deg = (int)data->lat[i];
				const double lat_min = (data->lat[i] - lat_deg) * 60.0;

				/* print out beam */
				fprintf(mbfp, "%c%2.2d%8.4f %c%3.3d%8.4f", NorS, lat_deg, lat_min, EorW, lon_deg, lon_min);
				const double depth = -data->deph[i];
				fprintf(mbfp, "%11.3f ****************", depth);
				fprintf(mbfp, "%7d%4d%7d    0 ", *ping_num_save, beam_num, *sounding_num_save);
				fprintf(mbfp, "%2d/%2d/%2d %2dh%2dm%2ds00\n", day, month, year, hour, minute, second);
			}
		}
	}

	/* else fail */
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
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
int mbr_wt_sbifremr(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
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
	struct mbf_sbifremr_struct *data = (struct mbf_sbifremr_struct *)mb_io_ptr->raw_data;
	struct mbsys_sb_struct *store = (struct mbsys_sb_struct *)store_ptr;

	/* second translate values from seabeam data storage structure */
	if (store != NULL) {
		data->kind = store->kind;
		if (store->kind == MB_DATA_DATA) {
			/* position */
			data->lon2u = store->lon2u;
			data->lon2b = store->lon2b;
			data->lat2u = store->lat2u;
			data->lat2b = store->lat2b;

			/* time stamp */
			data->year = store->year;
			data->day = store->day;
			data->min = store->min;
			data->sec = store->sec;

			/* depths and distances */
			for (int i = 0; i < MBSYS_SB_BEAMS; i++) {
				data->dist[i] = store->dist[i];
				data->deph[i] = store->deph[i];
			}

			/* additional values */
			data->sbhdg = store->sbhdg;
		}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT) {
			strncpy(data->comment, store->comment, MBSYS_SB_MAXLINE - 1);
		}
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Ready to write data in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       kind:       %d\n", data->kind);
		fprintf(stderr, "dbg5       error:      %d\n", *error);
	}

	int status = MB_SUCCESS;

	/* write next record to file */
	if (data->kind == MB_DATA_DATA || data->kind == MB_DATA_COMMENT) {
		status = mbr_sbifremr_wr_data(verbose, mbio_ptr, error);
	}
	else {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (verbose >= 5)
			fprintf(stderr, "\ndbg5  No data written in MBIO function <%s>\n", __func__);
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
int mbr_register_sbifremr(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_sbifremr(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sbifremr;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sbifremr;
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_sb_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sbifremr;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sbifremr;
	mb_io_ptr->mb_io_dimensions = &mbsys_sb_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_sb_extract;
	mb_io_ptr->mb_io_insert = &mbsys_sb_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_sb_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_sb_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb_copy;
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
