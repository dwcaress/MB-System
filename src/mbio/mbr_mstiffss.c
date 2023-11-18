/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mstiffss.c	4/7/98
 *
 *    Copyright (c) 1998-2023 by
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
 * mbr_mstiffss.c contains the functions for reading
 * sidescan data in the MSTIFFSS format.
 * These functions include:
 *   mbr_alm_mstiffss	- allocate read/write memory
 *   mbr_dem_mstiffss	- deallocate read/write memory
 *   mbr_rt_mstiffss	- read and translate data
 *   mbr_wt_mstiffss	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	April 7, 1998
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbf_mstiffss.h"
#include "mbsys_mstiff.h"

/*--------------------------------------------------------------------*/
int mbr_info_mstiffss(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
	*system = MB_SYS_MSTIFF;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 1024;
	strncpy(format_name, "MSTIFFSS", MB_NAME_LENGTH);
	strncpy(system_name, "MSTIFF", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_MSTIFFSS\nInformal Description: MSTIFF sidescan format\nAttributes:           variable "
	        "pixels,  sidescan,\n                      binary TIFF variant, single files, Sea Scan. \n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = false;
	*traveltime = false;
	*beam_flagging = false;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
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
int mbr_alm_mstiffss(int verbose, void *mbio_ptr, int *error) {
	int *n_read;
	int *n_nav_use;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_mstiffss_struct);
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	status &= mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_mstiff_struct), &mb_io_ptr->store_data, error);

	/* set number read counter */
	n_read = &(mb_io_ptr->save_label_flag);
	n_nav_use = &(mb_io_ptr->save8);
	*n_read = 0;
	*n_nav_use = 0;

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
int mbr_dem_mstiffss(int verbose, void *mbio_ptr, int *error) {
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
int mbr_rt_mstiffss(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char buffer[MBF_MSTIFFSS_BUFFERSIZE];
	int ifd_offset = 0;
	short nentry = 0;
	short tag = 0;
	short type = 0;
	int count = 0;
	int value_offset = 0;
	int *n_read;
	int *bits_per_pixel;
	int *n_ping_file;
	int *n_pixel_channel;
	int *left_channel_offset;
	int *right_channel_offset;
	int *sonar_data_info_offset;
	int *sonar_data_info_tag;
	int *n_nav;
	int *n_nav_use;
	int *nav_info_offset;
	int *nav_info_tag;
	int timecorr_tag;
	int timecorr_offset;
	short corr_time[9];
	int *ref_windows_time;
	int corr_time_i[7];
	double *corr_time_d;
	int date = 0;
  int time = 0;
	int pingtime = 0;
	short range_code = 0;;
	short frequency_code = 0;;
	short range_delay_bin = 0;;
	short altitude_bin = 0;;
	int range_mode = 0;;
	int channel_mode = 0;;
	double range_per_bin = 0.0;
	double range = 0.0;
	double range_delay = 0.0;
	double altitude = 0.0;
	double frequency = 0.0;
	short sonar_gain[16];
	int navsize = 0;
	int navtime1 = 0;
  int navtime2 = 0;;
	float lon1 = 0.0;
  float lon2 = 0.0;
	float lat1 = 0.0;
  float lat2 = 0.0;
	float speed1 = 0.0;
  float speed2 = 0.0;
	float course1 = 0.0;
  float course2 = 0.0;
	float heading1 = 0.0;
  float heading2 = 0.0;
	double lon = 0.0;
	double lat = 0.0;
	double speed = 0.0;
	double course = 0.0;
	double heading = 0.0;
	double factor = 0.0;
	double range_tot = 0.0;
	unsigned char left_channel[MBF_MSTIFFSS_PIXELS / 2];
	unsigned char right_channel[MBF_MSTIFFSS_PIXELS / 2];
	int ibottom = 0;
	int istart = 0;
	int index = 0;

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
	struct mbf_mstiffss_struct *data = (struct mbf_mstiffss_struct *)mb_io_ptr->raw_data;
	struct mbsys_mstiff_struct *store = (struct mbsys_mstiff_struct *)store_ptr;

	/* get pointers to saved values in mb_io_ptr */
	n_read = &(mb_io_ptr->save_label_flag);
	bits_per_pixel = &(mb_io_ptr->save_flag);
	n_ping_file = &(mb_io_ptr->save1);
	n_pixel_channel = &(mb_io_ptr->save2);
	left_channel_offset = &(mb_io_ptr->save3);
	right_channel_offset = &(mb_io_ptr->save4);
	sonar_data_info_offset = &(mb_io_ptr->save5);
	sonar_data_info_tag = &(mb_io_ptr->save6);
	n_nav = &(mb_io_ptr->save7);
	n_nav_use = &(mb_io_ptr->save8);
	nav_info_offset = &(mb_io_ptr->save9);
	nav_info_tag = &(mb_io_ptr->save10);
	ref_windows_time = &(mb_io_ptr->save11);
	corr_time_d = &(mb_io_ptr->saved1);

	int status = MB_SUCCESS;

	/* if first time through, read file header and
	    offsets, setting up for later reads */
	if (*n_read <= 0) {
		/* set defaults */
		*bits_per_pixel = 8;
		/* check for proper file tag */
		if ((status = fread(buffer, 4 * sizeof(char), 1, mb_io_ptr->mbfp)) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}
		else if (strncmp(buffer, "MSTL", 4) != 0) {
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_DATA;
		}
		else {
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}

		/* now get the image file directory offset */
		if ((status = fread(buffer, sizeof(int), 1, mb_io_ptr->mbfp)) == 1) {
			mb_get_binary_int(true, buffer, &ifd_offset);
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
		}

		/* now parse through the image file directory
		and grab the important values */
		if (status == MB_SUCCESS) {
			status = fseek(mb_io_ptr->mbfp, (size_t)ifd_offset, SEEK_SET);
			status = ftell(mb_io_ptr->mbfp);
			if ((status = fread(buffer, sizeof(short), 1, mb_io_ptr->mbfp)) == 1) {
				mb_get_binary_short(true, buffer, &nentry);

				/* loop over all entries in the directory */
				if ((status = fread(buffer, 6 * nentry * sizeof(short), 1, mb_io_ptr->mbfp)) == 1) {
					index = 0;
					for (int i = 0; i < nentry && status == MB_SUCCESS; i++) {
						mb_get_binary_short(true, &(buffer[index]), &tag);
						index += sizeof(short);
						mb_get_binary_short(true, &(buffer[index]), &type);
						index += sizeof(short);
						mb_get_binary_int(true, &(buffer[index]), &count);
						index += sizeof(int);
						mb_get_binary_int(true, &(buffer[index]), &value_offset);
						index += sizeof(int);

						/* set values for important entries */

						/* BitsPerBin */
						if (tag == BitsPerBin) {
							*bits_per_pixel = value_offset;
						}

						/* SonarLines */
						else if (tag == SonarLines) {
							*n_ping_file = value_offset;
						}

						/* BinsPerChannel */
						else if (tag == BinsPerChannel) {
							*n_pixel_channel = value_offset;
						}

						/* TimeCorrelation */
						else if (tag == TimeCorrelation) {
							timecorr_tag = tag;
							timecorr_offset = value_offset;
						}

						/* Y2KTimeCorrelation */
						else if (tag == Y2KTimeCorrelation) {
							timecorr_tag = tag;
							timecorr_offset = value_offset;
						}

						/* LeftChannel */
						else if (tag == LeftChannel) {
							*left_channel_offset = value_offset;
						}

						/* LeftChannel2 */
						else if (tag == LeftChannel2) {
							*left_channel_offset = value_offset;
						}

						/* RightChannel */
						else if (tag == RightChannel) {
							*right_channel_offset = value_offset;
						}

						/* RightChannel2 */
						else if (tag == RightChannel2) {
							*right_channel_offset = value_offset;
						}

						/* SonarDataInfo */
						else if (tag == SonarDataInfo) {
							*sonar_data_info_offset = value_offset;
							*sonar_data_info_tag = tag;
						}

						/* SonarDataInfo2 */
						else if (tag == SonarDataInfo2) {
							*sonar_data_info_offset = value_offset;
							*sonar_data_info_tag = tag;
						}

						/* SonarDataInfo3 */
						else if (tag == SonarDataInfo3) {
							*sonar_data_info_offset = value_offset;
							*sonar_data_info_tag = tag;
						}

						/* NavInfoCount */
						else if (tag == NavInfoCount) {
							*n_nav = value_offset;
						}

						/* NavInfo */
						else if (tag == NavInfo) {
							*nav_info_offset = value_offset;
							*nav_info_tag = tag;
						}

						/* NavInfo2 */
						else if (tag == NavInfo2) {
							*nav_info_offset = value_offset;
							*nav_info_tag = tag;
						}

						/* NavInfo3 */
						else if (tag == NavInfo3) {
							*nav_info_offset = value_offset;
							*nav_info_tag = tag;
						}

						/* NavInfo4 */
						else if (tag == NavInfo4) {
							*nav_info_offset = value_offset;
							*nav_info_tag = tag;
						}

						/* NavInfo5 */
						else if (tag == NavInfo5) {
							*nav_info_offset = value_offset;
							*nav_info_tag = tag;
						}

						/* NavInfo6 */
						else if (tag == NavInfo6) {
							*nav_info_offset = value_offset;
							*nav_info_tag = tag;
						}
					}
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}

		/* set the time correlation */
		if (status == MB_SUCCESS) {
			/* TimeCorrelation */
			if (timecorr_tag == TimeCorrelation) {
				/* get time correlation values */
				fseek(mb_io_ptr->mbfp, timecorr_offset, SEEK_SET);
				if ((status = fread(buffer, sizeof(int) + 9 * sizeof(short), 1, mb_io_ptr->mbfp)) == 1) {
					index = 0;
					mb_get_binary_int(true, &(buffer[index]), ref_windows_time);
					index += sizeof(int);
					for (int i = 0; i < 9; i++) {
						mb_get_binary_short(true, &(buffer[index]), &(corr_time[i]));
						index += sizeof(short);
					}
					mb_fix_y2k(verbose, (int)corr_time[5], &corr_time_i[0]);
					corr_time_i[1] = corr_time[4] + 1;
					corr_time_i[2] = corr_time[3];
					corr_time_i[3] = corr_time[2];
					corr_time_i[4] = corr_time[1];
					corr_time_i[5] = corr_time[0];
					corr_time_i[6] = 0;
					mb_get_time(verbose, corr_time_i, corr_time_d);

					status = MB_SUCCESS;
					*error = MB_ERROR_NO_ERROR;
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
			}

			/* Y2KTimeCorrelation */
			else if (timecorr_tag == Y2KTimeCorrelation) {
				/* get time correlation values */
				fseek(mb_io_ptr->mbfp, timecorr_offset, SEEK_SET);
				if ((status = fread(buffer, 3 * sizeof(int), 1, mb_io_ptr->mbfp)) == 1) {
					index = 0;
					mb_get_binary_int(true, &(buffer[index]), ref_windows_time);
					index += sizeof(int);
					mb_get_binary_int(true, &(buffer[index]), &date);
					index += sizeof(int);
					mb_get_binary_int(true, &(buffer[index]), &time);
					index += sizeof(int);
					corr_time_i[0] = date / 10000;
					corr_time_i[1] = (date - corr_time_i[0] * 10000) / 100;
					corr_time_i[2] = (date - corr_time_i[0] * 10000 - corr_time_i[1] * 100);
					corr_time_i[3] = (time / 3600);
					corr_time_i[4] = (time - corr_time_i[3] * 3600) / 60;
					corr_time_i[5] = (time - corr_time_i[3] * 3600 - corr_time_i[4] * 60);
					corr_time_i[6] = 0;
					mb_get_time(verbose, corr_time_i, corr_time_d);
					status = MB_SUCCESS;
					*error = MB_ERROR_NO_ERROR;
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}
	}

	/* if all pings already read then return EOF */
	if (status == MB_SUCCESS && *n_read >= *n_ping_file) {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
	}

	/* else read next ping */
	else if (status == MB_SUCCESS) {
		/* get sonar data info */
		if (*sonar_data_info_tag == SonarDataInfo) {
			fseek(mb_io_ptr->mbfp, *sonar_data_info_offset + (*n_read) * (sizeof(int) + 3 * sizeof(short)), SEEK_SET);
			if ((status = fread(buffer, sizeof(int) + 3 * sizeof(short), 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(true, &(buffer[index]), &pingtime);
				index += sizeof(int);
				mb_get_binary_short(true, &(buffer[index]), &range_code);
				index += sizeof(short);
				frequency_code = FREQ_UNKNOWN;
				mb_get_binary_short(true, &(buffer[index]), &range_delay_bin);
				index += sizeof(short);
				mb_get_binary_short(true, &(buffer[index]), &altitude_bin);
				index += sizeof(short);
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
				for (int i = 0; i < 16; i++)
					sonar_gain[i] = 0;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}

		/* get sonar data info */
		else if (*sonar_data_info_tag == SonarDataInfo2) {
			fseek(mb_io_ptr->mbfp, *sonar_data_info_offset + (*n_read) * (sizeof(int) + 4 * sizeof(short)), SEEK_SET);
			if ((status = fread(buffer, sizeof(int) + 4 * sizeof(short), 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(true, &(buffer[index]), &pingtime);
				index += sizeof(int);
				mb_get_binary_short(true, &(buffer[index]), &range_code);
				index += sizeof(short);
				mb_get_binary_short(true, &(buffer[index]), &frequency_code);
				index += sizeof(short);
				mb_get_binary_short(true, &(buffer[index]), &range_delay_bin);
				index += sizeof(short);
				mb_get_binary_short(true, &(buffer[index]), &altitude_bin);
				index += sizeof(short);
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
				for (int i = 0; i < 16; i++)
					sonar_gain[i] = 0;
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}

		/* get sonar data info */
		else if (*sonar_data_info_tag == SonarDataInfo3) {
			fseek(mb_io_ptr->mbfp, *sonar_data_info_offset + (*n_read) * (sizeof(int) + 20 * sizeof(short)), SEEK_SET);
			if ((status = fread(buffer, sizeof(int) + 20 * sizeof(short), 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(true, &(buffer[index]), &pingtime);
				index += sizeof(int);
				mb_get_binary_short(true, &(buffer[index]), &range_code);
				index += sizeof(short);
				mb_get_binary_short(true, &(buffer[index]), &frequency_code);
				index += sizeof(short);
				mb_get_binary_short(true, &(buffer[index]), &range_delay_bin);
				index += sizeof(short);
				mb_get_binary_short(true, &(buffer[index]), &altitude_bin);
				index += sizeof(short);
				for (int i = 0; i < 16; i++) {
					mb_get_binary_short(true, &(buffer[index]), &(sonar_gain[i]));
					index += sizeof(short);
				}
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}

		/* make sense of sonar data info */
		if (status == MB_SUCCESS) {
			channel_mode = (range_code & ~63) >> 6;
			if (channel_mode == 3)
				channel_mode = 0;
			range_mode = range_code & 15;
			switch (range_mode) {
			case 1:
				range = 5;
				break;
			case 2:
				range = 10;
				break;
			case 3:
				range = 20;
				break;
			case 4:
				range = 50;
				break;
			case 5:
				range = 75;
				break;
			case 6:
				range = 100;
				break;
			case 7:
				range = 150;
				break;
			case 8:
				range = 200;
				break;
			case 9:
				range = 300;
				break;
			case 10:
				range = 500;
				break;
			default:
				range = 5;
				break;
			}
			range_per_bin = range / *n_pixel_channel;
			range_delay = range_delay_bin * range_per_bin;
			altitude = altitude_bin * range_per_bin;
			switch (frequency_code) {
			case 1:
				frequency = 150.0;
				break;
			case 2:
				frequency = 300.0;
				break;
			case 3:
				frequency = 600.0;
				break;
			case 4:
				frequency = 1200.0;
				break;
			case 5:
				frequency = 0.0;
				break;
			}
		}

		/* now get navigation */
		if (status == MB_SUCCESS && *n_nav_use < *n_nav) {
			switch (*nav_info_tag) {
			case NavInfo:
				navsize = 16 * sizeof(int);
				break;
			case NavInfo2:
				navsize = 19 * sizeof(int);
				break;
			case NavInfo3:
				navsize = 16 * sizeof(int);
				break;
			case NavInfo4:
				navsize = 19 * sizeof(int);
				break;
			case NavInfo5:
				navsize = 20 * sizeof(int);
				break;
			case NavInfo6:
				navsize = 20 * sizeof(int);
				break;
			default:
				navsize = 20 * sizeof(int);
				break;
			}

			/* read first nav point starting with last
			   nav used */
			fseek(mb_io_ptr->mbfp, *nav_info_offset + (*n_nav_use) * navsize, SEEK_SET);
			if ((status = fread(buffer, navsize, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(true, &(buffer[index]), &navtime1);
				index += sizeof(int);
				mb_get_binary_float(true, &(buffer[index]), &lat1);
				index += sizeof(float);
				mb_get_binary_float(true, &(buffer[index]), &lon1);
				index += sizeof(float);
				mb_get_binary_float(true, &(buffer[index]), &speed1);
				index += sizeof(float);
				mb_get_binary_float(true, &(buffer[index]), &course1);
				if (*nav_info_tag == NavInfo6) {
					index += 3 * sizeof(float);
					mb_get_binary_float(true, &(buffer[index]), &heading1);
				}
				else {
					heading1 = course1;
				}
				lon1 /= 60.;
				lat1 /= 60.;
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}

			/* read second nav point starting with last
			   nav used */
			fseek(mb_io_ptr->mbfp, *nav_info_offset + (*n_nav_use + 1) * navsize, SEEK_SET);
			if ((status = fread(buffer, navsize, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(true, &(buffer[index]), &navtime2);
				index += sizeof(int);
				mb_get_binary_float(true, &(buffer[index]), &lat2);
				index += sizeof(float);
				mb_get_binary_float(true, &(buffer[index]), &lon2);
				index += sizeof(float);
				mb_get_binary_float(true, &(buffer[index]), &speed2);
				index += sizeof(float);
				mb_get_binary_float(true, &(buffer[index]), &course2);
				if (*nav_info_tag == NavInfo6) {
					index += 3 * sizeof(float);
					mb_get_binary_float(true, &(buffer[index]), &heading2);
				}
				else {
					heading2 = course2;
				}
				lon2 /= 60.;
				lat2 /= 60.;
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				navtime2 = 0.0;
			}

			/* if first two nav points don't bracket ping
			   in time keep reading until two found that
			   do or end of nav reached */
			while (pingtime > navtime2 && *n_nav_use < *n_nav - 2) {
				/* move nav 2 to nav 1 */
				navtime1 = navtime2;
				lat1 = lat2;
				lon1 = lon2;
				speed1 = speed2;
				course1 = course2;

				/* read second nav point starting with last
				   nav used */
				fseek(mb_io_ptr->mbfp, *nav_info_offset + (*n_nav_use + 2) * navsize, SEEK_SET);
				if ((status = fread(buffer, navsize, 1, mb_io_ptr->mbfp)) == 1) {
					index = 0;
					mb_get_binary_int(true, &(buffer[index]), &navtime2);
					index += sizeof(int);
					mb_get_binary_float(true, &(buffer[index]), &lat2);
					index += sizeof(float);
					mb_get_binary_float(true, &(buffer[index]), &lon2);
					index += sizeof(float);
					mb_get_binary_float(true, &(buffer[index]), &speed2);
					index += sizeof(float);
					mb_get_binary_float(true, &(buffer[index]), &course2);
					if (*nav_info_tag == NavInfo6) {
						index += 3 * sizeof(float);
						mb_get_binary_float(true, &(buffer[index]), &heading2);
					}
					else {
						heading2 = course2;
					}
					lon2 /= 60.;
					lat2 /= 60.;
					(*n_nav_use)++;
					status = MB_SUCCESS;
					*error = MB_ERROR_NO_ERROR;
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
				}
			}

			/* now interpolate nav */
			factor = ((double)(pingtime - navtime1)) / ((double)(navtime2 - navtime1));
			lon = lon1 + factor * (lon2 - lon1);
			lat = lat1 + factor * (lat2 - lat1);
			speed = speed1 + factor * (speed2 - speed1);
			if (course2 - course1 > 180.0) {
				course = course1 + factor * (course2 - course1 - 360.0);
			}
			else if (course2 - course1 < -180.0) {
				course = course1 + factor * (course2 - course1 + 360.0);
			}
			else {
				course = course1 + factor * (course2 - course1);
			}
			if (heading2 - heading1 > 180.0) {
				heading = heading1 + factor * (heading2 - heading1 - 360.0);
			}
			else if (heading2 - heading1 < -180.0) {
				heading = heading1 + factor * (heading2 - heading1 + 360.0);
			}
			else {
				heading = heading1 + factor * (heading2 - heading1);
			}
		}

		/* now get left channel sonar data */
		if (status == MB_SUCCESS) {
			fseek(mb_io_ptr->mbfp, *left_channel_offset + (*n_read) * (*n_pixel_channel), SEEK_SET);
			if ((status = fread(left_channel, sizeof(unsigned char), *n_pixel_channel, mb_io_ptr->mbfp)) == *n_pixel_channel) {
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}

		/* now get right channel sonar data */
		if (status == MB_SUCCESS) {
			fseek(mb_io_ptr->mbfp, *right_channel_offset + (*n_read) * (*n_pixel_channel), SEEK_SET);
			if ((status = fread(right_channel, sizeof(unsigned char), *n_pixel_channel, mb_io_ptr->mbfp)) == *n_pixel_channel) {
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
			}
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
		}

		/* if no altitude do bottom detect */
		if (status == MB_SUCCESS && altitude <= 0.0) {
			ibottom = 0;
			for (int i = 0; i < *n_pixel_channel && ibottom == 0; i++) {
				range_tot = range_delay + i * range_per_bin;
				if (range_tot > MBF_MSTIFF_TRANSMIT_BINS * range_per_bin && right_channel[i] > MBF_MSTIFF_BOTTOM_THRESHOLD &&
				    left_channel[i] > MBF_MSTIFF_BOTTOM_THRESHOLD) {
					ibottom = i;
					altitude = range_tot;
				}
			}
			if (altitude != range_tot) {
				/* There's either no amplitude data or the bottom, VES */
				/* threshold is too high. Set a default value */
				altitude = range_delay;
			}
		}

		/* increment reading counter */
		if (status == MB_SUCCESS)
			(*n_read)++;

		/* copy to proper storage, doing slant range correction */
		if (status == MB_SUCCESS) {
			data->time_d = *corr_time_d + 0.001 * (pingtime - *ref_windows_time);
			data->lon = lon;
			data->lat = lat;
			data->heading = heading;
			data->course = course;
			data->speed = speed;
			data->altitude = altitude;
			data->slant_range_max = range;
			data->range_delay = range_delay;
			data->sample_interval = range_per_bin;
			data->sonar_depth = 0.0;
			data->frequency = frequency;
			data->pixels_ss = 2 * (*n_pixel_channel);
			istart = (altitude - range_delay) / range_per_bin + 1;

			/* port and starboard channels */
			if (channel_mode == 0) {
				for (int i = 0; i < istart; i++) {
					int j = (*n_pixel_channel) - 1 - i;
					data->ss[j] = left_channel[i];
					data->ssacrosstrack[j] = 0.0;
					j = (*n_pixel_channel) + i;
					data->ss[j] = right_channel[i];
					data->ssacrosstrack[j] = 0.0;
				}
				for (int i = istart; i < *n_pixel_channel; i++) {
					range_tot = range_delay + i * range_per_bin;
					int j = (*n_pixel_channel) - 1 - i;
					data->ss[j] = left_channel[i];
					data->ssacrosstrack[j] = -sqrt(range_tot * range_tot - altitude * altitude);
					j = (*n_pixel_channel) + i;
					data->ss[j] = right_channel[i];
					data->ssacrosstrack[j] = sqrt(range_tot * range_tot - altitude * altitude);
				}
			}

			/* port channel only */
			else if (channel_mode == 1) {
				for (int i = 0; i < istart; i++) {
					int j = 2 * (*n_pixel_channel) - 1 - 2 * i;
					data->ss[j] = left_channel[i];
					data->ssacrosstrack[j] = 0.0;
					j = 2 * (*n_pixel_channel) - 2 - 2 * i;
					data->ss[j] = right_channel[i];
					data->ssacrosstrack[j] = 0.0;
				}
				for (int i = istart; i < *n_pixel_channel; i++) {
					range_tot = range_delay + (i - 0.5) * range_per_bin;
					int j = 2 * (*n_pixel_channel) - 1 - 2 * i;
					data->ss[j] = left_channel[i];
					data->ssacrosstrack[j] = -sqrt(range_tot * range_tot - altitude * altitude);
					range_tot = range_delay + i * range_per_bin;
					j = 2 * (*n_pixel_channel) - 2 - 2 * i;
					data->ss[j] = right_channel[i];
					data->ssacrosstrack[j] = -sqrt(range_tot * range_tot - altitude * altitude);
				}
			}

			/* starboard channel only */
			else if (channel_mode == 2) {
				for (int i = 0; i < istart; i++) {
					int j = 2 * i;
					data->ss[j] = right_channel[i];
					data->ssacrosstrack[j] = 0.0;
					j = 2 * i + 1;
					data->ss[j] = left_channel[i];
					data->ssacrosstrack[j] = 0.0;
				}
				for (int i = istart; i < *n_pixel_channel; i++) {
					range_tot = range_delay + (i - 0.5) * range_per_bin;
					int j = 2 * i;
					data->ss[j] = right_channel[i];
					data->ssacrosstrack[j] = sqrt(range_tot * range_tot - altitude * altitude);
					range_tot = range_delay + i * range_per_bin;
					j = 2 * i + 1;
					data->ss[j] = left_channel[i];
					data->ssacrosstrack[j] = sqrt(range_tot * range_tot - altitude * altitude);
				}
			}
		}
	}

	if (status == MB_SUCCESS && verbose >= 5) {
		fprintf(stderr, "\ndbg5  New data record read by MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5  Raw values:\n");
		fprintf(stderr, "dbg5       n_ping_file:      %d\n", *n_ping_file);
		fprintf(stderr, "dbg5       bits_per_pixel:   %d\n", *bits_per_pixel);
		fprintf(stderr, "dbg5       n_pixel_channel:  %d\n", *n_pixel_channel);
		fprintf(stderr, "dbg5       n_nav:            %d\n", *n_nav);
		fprintf(stderr, "dbg5       n_nav_use:        %d\n", *n_nav_use);
		fprintf(stderr, "dbg5       corr_time_d:      %f\n", *corr_time_d);
		fprintf(stderr, "dbg5       ref_windows_time: %d\n", *ref_windows_time);
		fprintf(stderr, "dbg5       pingtime:         %d\n", pingtime);
		fprintf(stderr, "dbg5       range_code:       %d\n", range_code);
		fprintf(stderr, "dbg5       channel_mode:     %d\n", channel_mode);
		fprintf(stderr, "dbg5       range_mode:       %d\n", range_mode);
		fprintf(stderr, "dbg5       range:            %f\n", range);
		fprintf(stderr, "dbg5       range_delay_bin:  %d\n", range_delay_bin);
		fprintf(stderr, "dbg5       range_delay:      %f\n", range_delay);
		fprintf(stderr, "dbg5       altitude_bin:     %d\n", altitude_bin);
		fprintf(stderr, "dbg5       altitude:         %f\n", altitude);
		for (int i = 0; i < *n_pixel_channel; i++)
			fprintf(stderr, "dbg5       %4d  ss_left: %d  ss_right: %d\n", i, left_channel[i], right_channel[i]);
		fprintf(stderr, "dbg5  Stored data values:\n");
		fprintf(stderr, "dbg5       time:       %f\n", data->time_d);
		fprintf(stderr, "dbg5       lon:        %f\n", data->lon);
		fprintf(stderr, "dbg5       lat:        %f\n", data->lat);
		fprintf(stderr, "dbg5       heading:    %f\n", data->heading);
		fprintf(stderr, "dbg5       speed:      %f\n", data->speed);
		fprintf(stderr, "dbg5       altitude:   %f\n", data->altitude);
		fprintf(stderr, "dbg5       pixels_ss:  %d\n", data->pixels_ss);
		for (int i = 0; i < data->pixels_ss; i++)
			fprintf(stderr, "dbg5       ss[%4d]: %d  xtrack:%f\n", i, data->ss[i], data->ssacrosstrack[i]);
	}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = MB_DATA_DATA;
	mb_io_ptr->new_error = *error;

	/* translate values to mstiff data storage structure */
	if (status == MB_SUCCESS && store != NULL) {
		/* time stamp */
		store->time_d = data->time_d;

		/* position */
		store->lon = data->lon;
		store->lat = data->lat;

		/* heading and speed */
		store->heading = data->heading;
		store->course = data->course;
		store->speed = data->speed;
		store->altitude = data->altitude;
		store->slant_range_max = data->slant_range_max;
		store->range_delay = data->range_delay;
		store->sample_interval = data->sample_interval;
		store->sonar_depth = data->sonar_depth;
		store->frequency = data->frequency;

		/* sidescan data */
		store->pixels_ss = data->pixels_ss;
		for (int i = 0; i < data->pixels_ss; i++) {
			store->ss[i] = data->ss[i];
			store->ssacrosstrack[i] = data->ssacrosstrack[i];
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
int mbr_wt_mstiffss(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* set error as this is a read only format */
	const int status = MB_SUCCESS;
	*error = MB_ERROR_WRITE_FAIL;

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
int mbr_register_mstiffss(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_mstiffss(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_mstiffss;
	mb_io_ptr->mb_io_format_free = &mbr_dem_mstiffss;
	mb_io_ptr->mb_io_store_alloc = &mbsys_mstiff_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_mstiff_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_mstiffss;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_mstiffss;
	mb_io_ptr->mb_io_dimensions = &mbsys_mstiff_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_mstiff_extract;
	mb_io_ptr->mb_io_insert = &mbsys_mstiff_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_mstiff_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_mstiff_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_mstiff_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_mstiff_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_mstiff_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_mstiff_copy;
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
