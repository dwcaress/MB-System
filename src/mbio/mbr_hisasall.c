/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hisasall.c	11/22/2026
 *
 *    Copyright (c) 2026 by
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
 * mbr_hisasall.c contains the functions for reading sidescan data from
 * Kongsberg HISAS synthetic aperture sonars recorded in Kongsberg *.all
 * files (format MBF_HISASALL, MBIO ID 212). The format is read only.
 * These functions include:
 *   mbr_alm_hisasall	- allocate read/write memory
 *   mbr_dem_hisasall	- deallocate read/write memory
 *   mbr_rt_hisasall	- read and translate data
 *   mbr_wt_hisasall	- translate and write data (fails: read only)
 *
 * Notes on the format:
 *   Each datagram starts with a 4-byte length N (the number of bytes that
 *   follow), then STX (0x02), the datagram type, the model number
 *   (12040 = HISAS 2040), the date (yyyymmdd), the time (ms since
 *   midnight), a counter and the serial number; the datagram ends with
 *   spare, ETX (0x03) and a checksum. All values are little-endian.
 *   The datagrams used here are:
 *     '>' navigation: vehicle position, depth, attitude, velocity,
 *         altitude and sound speed (about 10 Hz)
 *     '"' sidescan status: a 1024-byte XTF file header with the channel
 *         information
 *     '%' sidescan ping: an XTF sidescan ping record; its time, position
 *         and attitude fields are zero, so the ping time comes from the
 *         datagram header and the navigation is interpolated from the
 *         '>' datagrams. Both sides are stored near range to far range.
 *         The ranges use the last sound speed measured by the vehicle
 *         ('>' datagrams); the sound speed of the ping record (a sonar
 *         setting, e.g. 1480 m/s) is used only when none is available.
 *   Other datagrams (including the 4.5 MB 'd' raw element records) are
 *   skipped. The raw per-side files HISAS-P-*.all and HISAS-S-*.all hold
 *   no navigation; it is read from the file of the same name starting
 *   with HISAS-N- in the same directory, if that file exists.
 *   The positions are those of the vehicle navigation; no lever arm
 *   correction to the sonar is applied in this version.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_hisas.h"

/* datagram layout */
#define MBF_HISASALL_HEADER_SIZE 16          /* STX to serial number */
#define MBF_HISASALL_TRAILER_SIZE 4          /* spare, ETX, checksum */
#define MBF_HISASALL_DATAGRAM_MIN 20
#define MBF_HISASALL_DATAGRAM_MAX 50000000
#define MBF_HISASALL_STX 0x02
#define MBF_HISASALL_ETX 0x03
#define MBF_HISASALL_NAV '>'
#define MBF_HISASALL_STATUS '"'
#define MBF_HISASALL_PING '%'

/* XTF structures inside the '"' and '%' datagrams */
#define MBF_HISASALL_XTFFILEHEADER_SIZE 1024
#define MBF_HISASALL_XTFCHANINFO_SIZE 128
#define MBF_HISASALL_XTFPINGHEADER_SIZE 256
#define MBF_HISASALL_XTFCHANHEADER_SIZE 64
#define MBF_HISASALL_XTFMAGIC 0xFACE
#define MBF_HISASALL_MAXCHANNELS 6
#define MBF_HISASALL_MAXPINGCHANNELS 64

/* '>' navigation datagram */
#define MBF_HISASALL_NAV_FIRST_ELEMENT 64
#define MBF_HISASALL_NAV_ELEMENT_SIZE 64

/* the read buffer holds the largest datagram read into memory: a ping with
   MBF_HISASALL_MAXCHANNELS channels of MBSYS_HISAS_MAXRAWPIXELS complex samples */
#define MBF_HISASALL_BUFFER_SIZE                                                                                          \
	(MBF_HISASALL_HEADER_SIZE + MBF_HISASALL_XTFPINGHEADER_SIZE +                                                         \
	 MBF_HISASALL_MAXCHANNELS * (MBF_HISASALL_XTFCHANHEADER_SIZE + 8 * MBSYS_HISAS_MAXRAWPIXELS) + MBF_HISASALL_TRAILER_SIZE)

/* navigation of the HISAS-N- file is read up to this time after each ping (s) */
#define MBF_HISASALL_NAV_LOOKAHEAD 1.0

/* gaps of up to this number of pixels are interpolated in the processed sidescan */
#define MBF_HISASALL_PIXEL_INT 10

struct mbf_hisasall_chaninfo {
	int TypeOfChannel;    /* 1 = port, 2 = starboard */
	int SubChannelNumber;
	int BytesPerSample;   /* 2 = unsigned short, 4 = float, 8 = complex float */
	char ChannelName[MBSYS_HISAS_CHANNELNAME_LENGTH];
	float Frequency;
	float HorizBeamAngle;
	float TiltAngle;
	float BeamWidth;
	float OffsetX;
	float OffsetY;
	float OffsetZ;
};

struct mbf_hisasall_struct {
	/* type of data record */
	int kind;

	/* sidescan channel information from the '"' datagram */
	bool chaninfo_read;
	int sonartype;
	int nchannels;
	struct mbf_hisasall_chaninfo chaninfo[MBF_HISASALL_MAXCHANNELS];

	/* last sound speed of the '>' datagrams */
	double soundspeed;

	/* number of '>' navigation records used */
	int nnav;

	/* navigation file of the raw per-side files (HISAS-N-) */
	bool nfile_checked;
	FILE *nfp;
	mb_path nfile;
	bool nfile_done;
	double nfile_time_d;
	int nnav_nfile;

	/* read buffer */
	char *buffer;
};

/*--------------------------------------------------------------------*/
int mbr_info_hisasall(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, bool *variable_beams,
                      bool *traveltime, bool *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* set format info parameters */
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_HISAS;
	*beams_bath_max = MBSYS_HISAS_MAXBEAMS;
	*beams_amp_max = 0;
	*pixels_ss_max = MBSYS_HISAS_MAXPIXELS;
	strncpy(format_name, "HISASALL", MB_NAME_LENGTH);
	strncpy(system_name, "HISAS", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_HISASALL\nInformal Description: Kongsberg HISAS sidescan in *.all files\n"
	        "Attributes:           Kongsberg HISAS synthetic aperture sonar,\n"
	        "                      sidescan, variable pixels, binary datagrams,\n"
	        "                      single files, raw per-side files use the\n"
	        "                      navigation of the HISAS-N- file,\n"
	        "                      read only, Kongsberg.\n",
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
int mbr_alm_hisasall(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_hisasall_struct);
	mb_io_ptr->data_structure_size = 0;
	int status = mb_mallocd(verbose, __FILE__, __LINE__, mb_io_ptr->structure_size, &mb_io_ptr->raw_data, error);
	if (status == MB_SUCCESS)
		status = mbsys_hisas_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* initialize the reader state and allocate the read buffer */
	if (status == MB_SUCCESS) {
		struct mbf_hisasall_struct *data = (struct mbf_hisasall_struct *)mb_io_ptr->raw_data;
		memset(data, 0, sizeof(struct mbf_hisasall_struct));
		data->kind = MB_DATA_NONE;
		data->nfp = NULL;
		data->buffer = NULL;
		status = mb_mallocd(verbose, __FILE__, __LINE__, MBF_HISASALL_BUFFER_SIZE, (void **)&data->buffer, error);
	}

	/* set saved values */
	double *pixel_size = &mb_io_ptr->saved1;
	double *swath_width = &mb_io_ptr->saved2;
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
int mbr_dem_hisasall(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointers to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* close the navigation file and free the read buffer */
	int status = MB_SUCCESS;
	struct mbf_hisasall_struct *data = (struct mbf_hisasall_struct *)mb_io_ptr->raw_data;
	if (data != NULL) {
		if (data->nfp != NULL) {
			fclose(data->nfp);
			data->nfp = NULL;
		}
		if (data->buffer != NULL)
			status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&data->buffer, error);
	}

	/* deallocate memory for data descriptor */
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->raw_data, error);
	status &= mbsys_hisas_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
/* Check a datagram start: length, STX and a HISAS model number. */
static bool mbr_hisasall_header_ok(char *header) {
	unsigned int size;
	unsigned short model;
	mb_get_binary_int(true, &header[0], (int *)&size);
	mb_get_binary_short(true, &header[6], (short *)&model);
	return (size >= MBF_HISASALL_DATAGRAM_MIN && size <= MBF_HISASALL_DATAGRAM_MAX &&
	        (mb_u_char)header[4] == MBF_HISASALL_STX &&
	        (model == MBSYS_HISAS_HISAS2040 || model == MBSYS_HISAS_HISAS1032 || model == MBSYS_HISAS_HISAS1034));
}
/*--------------------------------------------------------------------*/
/* Read the next datagram. The navigation, status and ping datagrams are
   read whole into buffer (from STX on, so that buffer offsets are datagram
   offsets); all other datagrams are skipped with fseek. If a datagram start
   is not valid, the file is scanned forward one byte at a time for the next
   valid one. Returns MB_FAILURE with MB_ERROR_EOF at the end of the file. */
static int mbr_hisasall_read_datagram(int verbose, FILE *fp, char *buffer, unsigned int *size, mb_u_char *type,
                                      bool *in_buffer, int *error) {
	/* 4-byte length plus the datagram header */
	char header[4 + MBF_HISASALL_HEADER_SIZE];
	const size_t header_len = sizeof(header);
	*in_buffer = false;
	*size = 0;
	*type = 0;

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	bool done = false;
	if (fread(header, 1, header_len, fp) != header_len) {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		done = true;
	}
	long skipped = 0;
	while (!done) {
		/* resynchronize on a bad datagram start */
		while (status == MB_SUCCESS && !mbr_hisasall_header_ok(header)) {
			memmove(header, &header[1], header_len - 1);
			if (fread(&header[header_len - 1], 1, 1, fp) != 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
			skipped++;
		}
		if (status == MB_FAILURE)
			break;

		mb_get_binary_int(true, &header[0], (int *)size);
		*type = (mb_u_char)header[5];

		/* read datagrams of interest into the buffer */
		if ((*type == MBF_HISASALL_NAV || *type == MBF_HISASALL_STATUS || *type == MBF_HISASALL_PING) &&
		    *size <= MBF_HISASALL_BUFFER_SIZE) {
			memcpy(buffer, &header[4], MBF_HISASALL_HEADER_SIZE);
			const size_t remaining = *size - MBF_HISASALL_HEADER_SIZE;
			if (fread(&buffer[MBF_HISASALL_HEADER_SIZE], 1, remaining, fp) != remaining) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				done = true;
			}
			else if ((mb_u_char)buffer[*size - 3] != MBF_HISASALL_ETX) {
				/* no ETX: go back to the byte after this datagram start and scan */
				if (verbose > 0)
					fprintf(stderr, "HISAS datagram type 0x%2.2x of %u bytes without ETX - resynchronizing\n", *type,
					        *size);
				if (fseek(fp, -((long)(*size + 4) - 1), SEEK_CUR) != 0 ||
				    fread(header, 1, header_len, fp) != header_len) {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					done = true;
				}
			}
			else {
				*in_buffer = true;
				done = true;
			}
		}

		/* skip all other datagrams */
		else {
			if (fseek(fp, (long)(*size - MBF_HISASALL_HEADER_SIZE), SEEK_CUR) != 0) {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
			}
			done = true;
		}
	}

	if (verbose > 0 && skipped > 0)
		fprintf(stderr, "HISAS reader skipped %ld bytes to find the next datagram\n", skipped);

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Datagram read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       size:       %u\n", *size);
		fprintf(stderr, "dbg5       type:       0x%2.2x\n", *type);
		fprintf(stderr, "dbg5       in_buffer:  %d\n", *in_buffer);
		fprintf(stderr, "dbg5       skipped:    %ld\n", skipped);
		fprintf(stderr, "dbg5       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* Time from a date (yyyymmdd) and milliseconds since midnight; false if the
   date is not valid. */
static bool mbr_hisasall_get_time(int verbose, unsigned int date, double msec, double *time_d) {
	int time_i[7];
	time_i[0] = date / 10000;
	time_i[1] = (date % 10000) / 100;
	time_i[2] = date % 100;
	time_i[3] = 0;
	time_i[4] = 0;
	time_i[5] = 0;
	time_i[6] = 0;
	if (time_i[0] < 1970 || time_i[0] > 2100 || time_i[1] < 1 || time_i[1] > 12 || time_i[2] < 1 || time_i[2] > 31)
		return false;
	mb_get_time(verbose, time_i, time_d);
	*time_d += 0.001 * msec;
	return true;
}
/*--------------------------------------------------------------------*/
/* Add the elements of a '>' navigation datagram to the interpolation lists;
   returns the time of the last valid element, or 0.0. */
static double mbr_hisasall_parse_nav(int verbose, void *mbio_ptr, struct mbf_hisasall_struct *data, char *buffer,
                                     unsigned int size, int *error) {
	double last_time_d = 0.0;
	if (size < MBF_HISASALL_NAV_FIRST_ELEMENT + MBF_HISASALL_TRAILER_SIZE)
		return last_time_d;

	unsigned short bytes_per_element;
	unsigned int nelements;
	unsigned int date;
	unsigned int msec;
	mb_get_binary_short(true, &buffer[18], (short *)&bytes_per_element);
	mb_get_binary_int(true, &buffer[20], (int *)&nelements);
	mb_get_binary_int(true, &buffer[24], (int *)&date);
	mb_get_binary_int(true, &buffer[28], (int *)&msec);
	if (bytes_per_element < MBF_HISASALL_NAV_ELEMENT_SIZE)
		return last_time_d;

	/* elements that fit in the datagram */
	const unsigned int nfit =
	    (size - MBF_HISASALL_NAV_FIRST_ELEMENT - MBF_HISASALL_TRAILER_SIZE) / bytes_per_element;
	if (nelements > nfit)
		nelements = nfit;

	for (unsigned int ielement = 0; ielement < nelements; ielement++) {
		char *element = &buffer[MBF_HISASALL_NAV_FIRST_ELEMENT + ielement * bytes_per_element];
		unsigned int time_offset;
		double latitude, longitude;
		float depth, heading, pitch, roll, vforward, vstarboard, vdown, stddev, altitude, soundspeed;
		unsigned short validity;
		mb_get_binary_int(true, &element[0], (int *)&time_offset);
		mb_get_binary_double(true, &element[4], &latitude);
		mb_get_binary_double(true, &element[12], &longitude);
		mb_get_binary_float(true, &element[20], &depth);
		mb_get_binary_float(true, &element[24], &heading);
		mb_get_binary_float(true, &element[28], &pitch);
		mb_get_binary_float(true, &element[32], &roll);
		mb_get_binary_float(true, &element[36], &vforward);
		mb_get_binary_float(true, &element[40], &vstarboard);
		mb_get_binary_float(true, &element[44], &vdown);
		mb_get_binary_float(true, &element[48], &stddev);
		mb_get_binary_float(true, &element[52], &altitude);
		mb_get_binary_float(true, &element[56], &soundspeed);
		mb_get_binary_short(true, &element[62], (short *)&validity);

		double time_d;
		if (!mbr_hisasall_get_time(verbose, date, (double)msec + (double)time_offset, &time_d))
			continue;

		if ((validity & 0x0001) && fabs(latitude) <= 90.0 && fabs(longitude) <= 360.0)
			mb_navint_add(verbose, mbio_ptr, time_d, longitude, latitude, error);
		if (validity & 0x0002)
			mb_depint_add(verbose, mbio_ptr, time_d, depth, error);
		if (validity & 0x0004)
			mb_hedint_add(verbose, mbio_ptr, time_d, heading, error);
		if (validity & 0x0008)
			mb_attint_add(verbose, mbio_ptr, time_d, 0.0, roll, pitch, error);
		if (validity & 0x0080)
			mb_altint_add(verbose, mbio_ptr, time_d, altitude, error);
		if ((validity & 0x0100) && soundspeed > 0.0)
			data->soundspeed = soundspeed;
		data->nnav++;
		last_time_d = time_d;

		if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Navigation element read in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg5       time_d:      %f\n", time_d);
			fprintf(stderr, "dbg5       latitude:    %f\n", latitude);
			fprintf(stderr, "dbg5       longitude:   %f\n", longitude);
			fprintf(stderr, "dbg5       depth:       %f\n", depth);
			fprintf(stderr, "dbg5       heading:     %f\n", heading);
			fprintf(stderr, "dbg5       pitch:       %f\n", pitch);
			fprintf(stderr, "dbg5       roll:        %f\n", roll);
			fprintf(stderr, "dbg5       vforward:    %f\n", vforward);
			fprintf(stderr, "dbg5       vstarboard:  %f\n", vstarboard);
			fprintf(stderr, "dbg5       vdown:       %f\n", vdown);
			fprintf(stderr, "dbg5       stddev:      %f\n", stddev);
			fprintf(stderr, "dbg5       altitude:    %f\n", altitude);
			fprintf(stderr, "dbg5       soundspeed:  %f\n", soundspeed);
			fprintf(stderr, "dbg5       validity:    0x%4.4x\n", validity);
		}
	}

	return last_time_d;
}
/*--------------------------------------------------------------------*/
/* Parse the XTF file header of a '"' sidescan status datagram. */
static void mbr_hisasall_parse_status(int verbose, struct mbf_hisasall_struct *data, char *buffer, unsigned int size) {
	if (size < MBF_HISASALL_HEADER_SIZE + MBF_HISASALL_XTFFILEHEADER_SIZE + MBF_HISASALL_TRAILER_SIZE)
		return;
	char *header = &buffer[MBF_HISASALL_HEADER_SIZE];
	unsigned short sonartype, nchannels;
	mb_get_binary_short(true, &header[34], (short *)&sonartype);
	mb_get_binary_short(true, &header[166], (short *)&nchannels);
	data->sonartype = sonartype;
	data->nchannels = MIN((int)nchannels, MBF_HISASALL_MAXCHANNELS);
	for (int i = 0; i < data->nchannels; i++) {
		char *chaninfo = &header[256 + MBF_HISASALL_XTFCHANINFO_SIZE * i];
		struct mbf_hisasall_chaninfo *info = &data->chaninfo[i];
		unsigned short bytes_per_sample;
		info->TypeOfChannel = (mb_u_char)chaninfo[0];
		info->SubChannelNumber = (mb_u_char)chaninfo[1];
		mb_get_binary_short(true, &chaninfo[6], (short *)&bytes_per_sample);
		info->BytesPerSample = bytes_per_sample;
		memcpy(info->ChannelName, &chaninfo[12], MBSYS_HISAS_CHANNELNAME_LENGTH - 1);
		info->ChannelName[MBSYS_HISAS_CHANNELNAME_LENGTH - 1] = '\0';
		mb_get_binary_float(true, &chaninfo[32], &info->Frequency);
		mb_get_binary_float(true, &chaninfo[36], &info->HorizBeamAngle);
		mb_get_binary_float(true, &chaninfo[40], &info->TiltAngle);
		mb_get_binary_float(true, &chaninfo[44], &info->BeamWidth);
		mb_get_binary_float(true, &chaninfo[48], &info->OffsetX);
		mb_get_binary_float(true, &chaninfo[52], &info->OffsetY);
		mb_get_binary_float(true, &chaninfo[56], &info->OffsetZ);
	}
	data->chaninfo_read = true;

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Sidescan status read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sonartype:  %d\n", data->sonartype);
		fprintf(stderr, "dbg5       nchannels:  %d\n", data->nchannels);
		for (int i = 0; i < data->nchannels; i++)
			fprintf(stderr, "dbg5       channel %d: type:%d sub:%d bytes:%d name:%s frequency:%f offsets:%f %f %f\n", i,
			        data->chaninfo[i].TypeOfChannel, data->chaninfo[i].SubChannelNumber, data->chaninfo[i].BytesPerSample,
			        data->chaninfo[i].ChannelName, data->chaninfo[i].Frequency, data->chaninfo[i].OffsetX,
			        data->chaninfo[i].OffsetY, data->chaninfo[i].OffsetZ);
	}
}
/*--------------------------------------------------------------------*/
/* Bytes per sample of a ping channel from the channel information, or 0 if
   unknown. */
static int mbr_hisasall_bytes_per_sample(const struct mbf_hisasall_struct *data, int channel) {
	if (!data->chaninfo_read || channel < 0 || channel >= data->nchannels)
		return 0;
	const int bytes_per_sample = data->chaninfo[channel].BytesPerSample;
	if (bytes_per_sample == 1 || bytes_per_sample == 2 || bytes_per_sample == 4 || bytes_per_sample == 8)
		return bytes_per_sample;
	return 0;
}
/*--------------------------------------------------------------------*/
/* Sum of the sizes of the ping channels, using guess as the number of bytes
   per sample of the channels without channel information; -1 if a channel
   header does not fit. */
static long long mbr_hisasall_ping_channels_size(const struct mbf_hisasall_struct *data, char *body, long long bodylen,
                                                 int nchans, int guess) {
	long long offset = MBF_HISASALL_XTFPINGHEADER_SIZE;
	for (int k = 0; k < nchans; k++) {
		if (offset + MBF_HISASALL_XTFCHANHEADER_SIZE > bodylen)
			return -1;
		unsigned short channel;
		unsigned int nsamples;
		mb_get_binary_short(true, &body[offset], (short *)&channel);
		mb_get_binary_int(true, &body[offset + 42], (int *)&nsamples);
		int bytes_per_sample = mbr_hisasall_bytes_per_sample(data, channel);
		if (bytes_per_sample == 0)
			bytes_per_sample = guess;
		offset += MBF_HISASALL_XTFCHANHEADER_SIZE + (long long)nsamples * bytes_per_sample;
		if (offset > bodylen)
			return -1;
	}
	return offset;
}
/*--------------------------------------------------------------------*/
/* Parse a '%' sidescan ping datagram into the store; returns MB_FAILURE with
   MB_ERROR_UNINTELLIGIBLE if the ping cannot be used. */
static int mbr_hisasall_parse_ping(int verbose, struct mbf_hisasall_struct *data, char *buffer, unsigned int size,
                                   struct mbsys_hisas_struct *store, int *error) {
	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	for (int side = 0; side < 2; side++) {
		store->ss_numsamples[side] = 0;
		store->ss_channel[side] = -1;
		store->ss_slantrange[side] = 0.0;
		store->ss_duration[side] = 0.0;
		store->ss_frequency[side] = 0.0;
		store->ss_channelname[side][0] = '\0';
	}

	char *body = &buffer[MBF_HISASALL_HEADER_SIZE];
	const long long bodylen = (long long)size - MBF_HISASALL_HEADER_SIZE - MBF_HISASALL_TRAILER_SIZE;
	unsigned short magic = 0;
	unsigned short nchans = 0;
	if (bodylen >= MBF_HISASALL_XTFPINGHEADER_SIZE) {
		mb_get_binary_short(true, &body[0], (short *)&magic);
		mb_get_binary_short(true, &body[4], (short *)&nchans);
	}
	if (magic != MBF_HISASALL_XTFMAGIC || body[2] != 0 || nchans == 0 || nchans > MBF_HISASALL_MAXPINGCHANNELS) {
		if (verbose > 0)
			fprintf(stderr, "HISAS ping datagram with a bad XTF ping header - skipped\n");
		*error = MB_ERROR_UNINTELLIGIBLE;
		return (MB_FAILURE);
	}

	unsigned int ping_number;
	float half_soundspeed, soundspeed;
	mb_get_binary_int(true, &body[28], (int *)&ping_number);
	mb_get_binary_float(true, &body[32], &half_soundspeed);
	mb_get_binary_float(true, &body[72], &soundspeed);
	store->ping_number = ping_number;
	if (soundspeed > 1000.0 && soundspeed < 2000.0)
		store->soundspeed = soundspeed;
	else if (half_soundspeed > 500.0 && half_soundspeed < 1000.0)
		store->soundspeed = 2.0 * half_soundspeed;
	else
		store->soundspeed = 1500.0;

	/* find the number of bytes per sample of channels without channel
	    information: the one that makes the channels fill the ping record */
	int guess = 0;
	if (mbr_hisasall_ping_channels_size(data, body, bodylen, nchans, 0) != bodylen) {
		const int guesses[3] = {2, 4, 8};
		for (int i = 0; i < 3 && guess == 0; i++) {
			if (mbr_hisasall_ping_channels_size(data, body, bodylen, nchans, guesses[i]) == bodylen)
				guess = guesses[i];
		}
		if (guess == 0) {
			if (verbose > 0)
				fprintf(stderr, "HISAS ping %u: channel sizes do not match the datagram - skipped\n", ping_number);
			*error = MB_ERROR_UNINTELLIGIBLE;
			return (MB_FAILURE);
		}
	}

	/* keep the first channel of each side */
	long long offset = MBF_HISASALL_XTFPINGHEADER_SIZE;
	for (int k = 0; k < nchans && status == MB_SUCCESS; k++) {
		char *chanheader = &body[offset];
		unsigned short channel;
		float slantrange, duration;
		unsigned int nsamples;
		short weight;
		mb_get_binary_short(true, &chanheader[0], (short *)&channel);
		mb_get_binary_float(true, &chanheader[4], &slantrange);
		mb_get_binary_float(true, &chanheader[16], &duration);
		mb_get_binary_int(true, &chanheader[42], (int *)&nsamples);
		mb_get_binary_short(true, &chanheader[58], &weight);
		int bytes_per_sample = mbr_hisasall_bytes_per_sample(data, channel);
		if (bytes_per_sample == 0)
			bytes_per_sample = guess;
		char *samples = &chanheader[MBF_HISASALL_XTFCHANHEADER_SIZE];
		offset += MBF_HISASALL_XTFCHANHEADER_SIZE + (long long)nsamples * bytes_per_sample;

		/* side from the channel type, or from the position in the ping */
		int side = -1;
		if (data->chaninfo_read && channel < data->nchannels) {
			if (data->chaninfo[channel].TypeOfChannel == 1)
				side = MBSYS_HISAS_PORT;
			else if (data->chaninfo[channel].TypeOfChannel == 2)
				side = MBSYS_HISAS_STBD;
		}
		else if (!data->chaninfo_read && k < 2) {
			side = (k == 0) ? MBSYS_HISAS_PORT : MBSYS_HISAS_STBD;
		}
		if (side < 0 || store->ss_channel[side] >= 0)
			continue;

		if (nsamples > MBSYS_HISAS_MAXRAWPIXELS) {
			if (verbose > 0)
				fprintf(stderr, "HISAS ping %u: %u samples in channel %d exceed the maximum %d - skipped\n", ping_number,
				        nsamples, channel, MBSYS_HISAS_MAXRAWPIXELS);
			*error = MB_ERROR_UNINTELLIGIBLE;
			status = MB_FAILURE;
			break;
		}

		/* sample magnitudes */
		float *ssraw = store->ssraw[side];
		if (bytes_per_sample == 1) {
			for (unsigned int i = 0; i < nsamples; i++)
				ssraw[i] = (float)((mb_u_char)samples[i]);
		}
		else if (bytes_per_sample == 2) {
			const double scale = ldexp(1.0, -weight);
			for (unsigned int i = 0; i < nsamples; i++) {
				unsigned short value;
				mb_get_binary_short(true, &samples[2 * i], (short *)&value);
				ssraw[i] = (float)(scale * value);
			}
		}
		else if (bytes_per_sample == 4) {
			for (unsigned int i = 0; i < nsamples; i++)
				mb_get_binary_float(true, &samples[4 * i], &ssraw[i]);
		}
		else if (bytes_per_sample == 8) {
			for (unsigned int i = 0; i < nsamples; i++) {
				float re, im;
				mb_get_binary_float(true, &samples[8 * i], &re);
				mb_get_binary_float(true, &samples[8 * i + 4], &im);
				ssraw[i] = (float)sqrt((double)re * re + (double)im * im);
			}
		}
		store->ss_numsamples[side] = (int)nsamples;
		store->ss_channel[side] = channel;
		store->ss_slantrange[side] = slantrange;
		store->ss_duration[side] = duration;
		if (data->chaninfo_read && channel < data->nchannels) {
			store->ss_frequency[side] = data->chaninfo[channel].Frequency;
			memcpy(store->ss_channelname[side], data->chaninfo[channel].ChannelName, MBSYS_HISAS_CHANNELNAME_LENGTH);
		}
	}

	if (status == MB_SUCCESS && store->ss_numsamples[MBSYS_HISAS_PORT] == 0 &&
	    store->ss_numsamples[MBSYS_HISAS_STBD] == 0) {
		if (verbose > 0)
			fprintf(stderr, "HISAS ping %u: no port or starboard sidescan channel - skipped\n", ping_number);
		*error = MB_ERROR_UNINTELLIGIBLE;
		status = MB_FAILURE;
	}

	if (verbose >= 5) {
		fprintf(stderr, "\ndbg5  Sidescan ping read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       ping_number:  %u\n", store->ping_number);
		fprintf(stderr, "dbg5       soundspeed:   %f\n", store->soundspeed);
		fprintf(stderr, "dbg5       nchans:       %d\n", nchans);
		for (int side = 0; side < 2; side++)
			fprintf(stderr, "dbg5       side %d: channel:%d samples:%d slantrange:%f duration:%f name:%s\n", side,
			        store->ss_channel[side], store->ss_numsamples[side], store->ss_slantrange[side], store->ss_duration[side],
			        store->ss_channelname[side]);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* For the raw per-side files HISAS-P-*.all and HISAS-S-*.all, open the
   navigation file HISAS-N-*.all of the same name in the same directory. */
static void mbr_hisasall_open_nfile(int verbose, struct mb_io_struct *mb_io_ptr, struct mbf_hisasall_struct *data) {
	data->nfile_checked = true;
	const char *path = mb_io_ptr->file;
	const char *basename = strrchr(path, '/');
#ifdef _WIN32
	const char *basename2 = strrchr(path, '\\');
	if (basename2 != NULL && (basename == NULL || basename2 > basename))
		basename = basename2;
#endif
	basename = (basename == NULL) ? path : basename + 1;
	if (strncmp(basename, "HISAS-P-", 8) != 0 && strncmp(basename, "HISAS-S-", 8) != 0)
		return;

	const int dirlen = (int)(basename - path);
	const int n = snprintf(data->nfile, sizeof(data->nfile), "%.*sHISAS-N-%s", dirlen, path, basename + 8);
	if (n < 0 || n >= (int)sizeof(data->nfile)) {
		data->nfile[0] = '\0';
		return;
	}
	data->nfp = fopen(data->nfile, "rb");
	if (data->nfp == NULL) {
		data->nfile_done = true;
		if (verbose > 0)
			fprintf(stderr, "HISAS navigation file %s not found - pings are read without navigation\n", data->nfile);
	}
	else if (verbose >= 2) {
		fprintf(stderr, "dbg2  HISAS navigation read from %s\n", data->nfile);
	}
}
/*--------------------------------------------------------------------*/
/* Feed the '>' records of the navigation file to the interpolation lists up
   to time_d plus a look-ahead, so that the lists cover the ping times while
   they stay within their limited size. */
static void mbr_hisasall_feed_nfile(int verbose, void *mbio_ptr, struct mbf_hisasall_struct *data, double time_d,
                                    int *error) {
	if (data->nfp == NULL || data->nfile_done)
		return;
	char *buffer = data->buffer;
	while (!data->nfile_done && data->nfile_time_d <= time_d + MBF_HISASALL_NAV_LOOKAHEAD) {
		unsigned int size;
		mb_u_char type;
		bool in_buffer;
		int nerror = MB_ERROR_NO_ERROR;
		const int nstatus = mbr_hisasall_read_datagram(verbose, data->nfp, buffer, &size, &type, &in_buffer, &nerror);
		if (nstatus == MB_FAILURE) {
			data->nfile_done = true;
		}
		else if (in_buffer && type == MBF_HISASALL_NAV) {
			const int nnav = data->nnav;
			const double nav_time_d = mbr_hisasall_parse_nav(verbose, mbio_ptr, data, buffer, size, error);
			data->nnav_nfile += data->nnav - nnav;
			if (nav_time_d > 0.0)
				data->nfile_time_d = nav_time_d;
		}
	}
}
/*--------------------------------------------------------------------*/
int mbr_rt_hisasall(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointers to mbio descriptor and data structures */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbf_hisasall_struct *data = (struct mbf_hisasall_struct *)mb_io_ptr->raw_data;
	struct mbsys_hisas_struct *store = (struct mbsys_hisas_struct *)store_ptr;
	double *pixel_size = (double *)&mb_io_ptr->saved1;
	double *swath_width = (double *)&mb_io_ptr->saved2;
	char *buffer = data->buffer;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	data->kind = MB_DATA_NONE;

	/* read datagrams until a sidescan ping is found */
	bool done = false;
	while (!done) {
		unsigned int size;
		mb_u_char type;
		bool in_buffer;
		status = mbr_hisasall_read_datagram(verbose, mb_io_ptr->mbfp, buffer, &size, &type, &in_buffer, error);
		if (status == MB_FAILURE) {
			done = true;
		}
		else if (!in_buffer) {
			if (type == MBF_HISASALL_PING && verbose > 0)
				fprintf(stderr, "HISAS ping datagram of %u bytes exceeds the read buffer - skipped\n", size);
		}
		else if (type == MBF_HISASALL_NAV) {
			mbr_hisasall_parse_nav(verbose, mbio_ptr, data, buffer, size, error);
		}
		else if (type == MBF_HISASALL_STATUS) {
			mbr_hisasall_parse_status(verbose, data, buffer, size);
		}
		else if (type == MBF_HISASALL_PING && store != NULL) {
			done = true;
			unsigned short model, serial;
			unsigned int date, msec;
			mb_get_binary_short(true, &buffer[2], (short *)&model);
			mb_get_binary_int(true, &buffer[4], (int *)&date);
			mb_get_binary_int(true, &buffer[8], (int *)&msec);
			mb_get_binary_short(true, &buffer[14], (short *)&serial);

			/* ping time from the datagram header */
			double time_d;
			if (!mbr_hisasall_get_time(verbose, date, (double)msec, &time_d)) {
				if (verbose > 0)
					fprintf(stderr, "HISAS ping datagram with a bad date %u - skipped\n", date);
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
			}
			else {
				status = mbr_hisasall_parse_ping(verbose, data, buffer, size, store, error);
			}

			if (status == MB_SUCCESS) {
				/* navigation from the HISAS-N- file for raw per-side files */
				if (!data->nfile_checked)
					mbr_hisasall_open_nfile(verbose, mb_io_ptr, data);
				mbr_hisasall_feed_nfile(verbose, mbio_ptr, data, time_d, error);

				/* the sound speed measured by the vehicle ('>' datagrams) is
				    preferred to the sound speed set in the sonar (ping record),
				    which may be a fixed setting */
				if (data->soundspeed > 1300.0 && data->soundspeed < 1700.0)
					store->soundspeed = data->soundspeed;

				store->kind = MB_DATA_DATA;
				store->sonar = model;
				store->serial = serial;
				store->time_d = time_d;

				/* interpolate the vehicle navigation at the ping time; the
				    positions are not moved to the sonar (no lever arm) */
				int ierror = MB_ERROR_NO_ERROR;
				store->heading = 0.0;
				if (mb_io_ptr->nheading > 0)
					mb_hedint_interp(verbose, mbio_ptr, time_d, &store->heading, &ierror);
				store->longitude = 0.0;
				store->latitude = 0.0;
				store->speed = 0.0;
				if (mb_io_ptr->nfix > 0)
					mb_navint_interp(verbose, mbio_ptr, time_d, store->heading, 0.0, &store->longitude, &store->latitude,
					                 &store->speed, &ierror);
				store->roll = 0.0;
				store->pitch = 0.0;
				store->heave = 0.0;
				if (mb_io_ptr->nattitude > 0)
					mb_attint_interp(verbose, mbio_ptr, time_d, &store->heave, &store->roll, &store->pitch, &ierror);
				store->sensordepth = 0.0;
				if (mb_io_ptr->nsensordepth > 0)
					mb_depint_interp(verbose, mbio_ptr, time_d, &store->sensordepth, &ierror);
				store->altitude = 0.0;
				if (mb_io_ptr->naltitude > 0)
					mb_altint_interp(verbose, mbio_ptr, time_d, &store->altitude, &ierror);

				/* one bathymetry value: the seafloor depth below the sonar */
				store->beams_bath = 1;
				if (store->altitude > 0.0) {
					store->bath[0] = store->sensordepth + store->altitude;
					store->beamflag[0] = MB_FLAG_NONE;
				}
				else {
					store->bath[0] = 0.0;
					store->beamflag[0] = MB_FLAG_NULL;
				}

				/* generate processed sidescan */
				status = mbsys_hisas_makess(verbose, mbio_ptr, store_ptr, false, pixel_size, false, swath_width,
				                            MBF_HISASALL_PIXEL_INT, error);
				data->kind = MB_DATA_DATA;
			}
			else {
				store->kind = MB_DATA_NONE;
			}
		}
	}

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

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
int mbr_wt_hisasall(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* set error as this is a read only format */
	const int status = MB_FAILURE;
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
int mbr_register_hisasall(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_hisasall(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hisasall;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hisasall;
	mb_io_ptr->mb_io_store_alloc = &mbsys_hisas_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_hisas_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hisasall;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hisasall;
	mb_io_ptr->mb_io_dimensions = &mbsys_hisas_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_hisas_extract;
	mb_io_ptr->mb_io_insert = &mbsys_hisas_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_hisas_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_hisas_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_hisas_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_hisas_ttimes;
	mb_io_ptr->mb_io_copyrecord = &mbsys_hisas_copy;
	mb_io_ptr->mb_io_makess = &mbsys_hisas_makess;
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
