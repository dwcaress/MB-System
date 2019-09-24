/*--------------------------------------------------------------------
 *    The MB-system:	mblist.c	2/1/93
 *
 *    Copyright (c) 1993-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBlist prints the specified contents of a swath sonar data
 * file to stdout. The form of the output is quite flexible;
 * MBlist is tailored to produce ascii files in spreadsheet
 * style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	February 1, 1993
 *
 * Note:	This program is based on the program mblist created
 *		by A. Malinverno (currently at Schlumberger, formerly
 *		at L-DEO) in August 1991.  It also includes elements
 *		derived from the program mbdump created by D. Caress
 *		in 1990.
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_simrad2.h"
#include "mbsys_simrad3.h"

#define MAX_OPTIONS 25
#define DUMP_MODE_LIST 1
#define DUMP_MODE_BATH 2
#define DUMP_MODE_TOPO 3
#define DUMP_MODE_AMP 4
#define DUMP_MODE_SS 5
#define MBLIST_CHECK_ON 0
#define MBLIST_CHECK_ON_NULL 1
#define MBLIST_CHECK_OFF_RAW 2
#define MBLIST_CHECK_OFF_NAN 3
#define MBLIST_CHECK_OFF_FLAGNAN 4
#define MBLIST_SET_OFF 0
#define MBLIST_SET_ON 1
#define MBLIST_SET_ALL 2
#define MBLIST_SET_EXCLUDE_OUTER 3
#define MBLIST_SEGMENT_MODE_NONE 0
#define MBLIST_SEGMENT_MODE_TAG 1
#define MBLIST_SEGMENT_MODE_SWATHFILE 2
#define MBLIST_SEGMENT_MODE_DATALIST 3

/* NaN value */
double NaN;

static const char program_name[] = "MBLIST";
static const char help_message[] =
    "MBLIST prints the specified contents of a swath data \nfile to stdout. The form of the output is "
    "quite flexible; \nMBLIST is tailored to produce ascii files in spreadsheet \nstyle with data columns "
    "separated by tabs.";
static const char usage_message[] =
    "mblist [-Byr/mo/da/hr/mn/sc -C -Ddump_mode -Eyr/mo/da/hr/mn/sc \n-Fformat -Gdelimiter -H -Ifile "
    "-Kdecimate -Llonflip -M[beam_start/beam_end | A | X%] -Npixel_start/pixel_end \n-Ooptions -Ppings "
    "-Rw/e/s/n -Sspeed -Ttimegap -Ucheck -Xoutfile -V -W -Zsegment]";

/*--------------------------------------------------------------------*/
int set_output(int verbose, int beams_bath, int beams_amp, int pixels_ss, int use_bath, int use_amp, int use_ss, int dump_mode,
               int beam_set, int pixel_set, int beam_vertical, int pixel_vertical, int *beam_start, int *beam_end,
               int *beam_exclude_percent, int *pixel_start, int *pixel_end, int *n_list, char *list, int *error) {
	int status = MB_SUCCESS;
	int i;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBLIST function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       beams_bath:      %d\n", beams_bath);
		fprintf(stderr, "dbg2       beams_amp:       %d\n", beams_amp);
		fprintf(stderr, "dbg2       pixels_ss:       %d\n", pixels_ss);
		fprintf(stderr, "dbg2       use_bath:        %d\n", use_bath);
		fprintf(stderr, "dbg2       use_amp:         %d\n", use_amp);
		fprintf(stderr, "dbg2       use_ss:          %d\n", use_ss);
		fprintf(stderr, "dbg2       dump_mode:       %d\n", dump_mode);
		fprintf(stderr, "dbg2       :        %d\n", beam_set);
		fprintf(stderr, "dbg2       pixel_set:       %d\n", pixel_set);
		fprintf(stderr, "dbg2       beam_vertical:   %d\n", beam_vertical);
		fprintf(stderr, "dbg2       pixel_vertical:  %d\n", pixel_vertical);
		fprintf(stderr, "dbg2       beam_start:      %d\n", *beam_start);
		fprintf(stderr, "dbg2       beam_end:        %d\n", *beam_end);
		fprintf(stderr, "dbg2       beam_exclude_percent: %d\n", *beam_exclude_percent);
		fprintf(stderr, "dbg2       pixel_start:     %d\n", *pixel_start);
		fprintf(stderr, "dbg2       pixel_end:       %d\n", *pixel_end);
		fprintf(stderr, "dbg2       n_list:          %d\n", *n_list);
		for (i = 0; i < *n_list; i++)
			fprintf(stderr, "dbg2       list[%2d]:        %c\n", i, list[i]);
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	if (beam_set == MBLIST_SET_OFF && pixel_set == MBLIST_SET_OFF && beams_bath <= 0 && pixels_ss <= 0) {
		*beam_start = 0;
		*beam_end = 1;
		*pixel_start = 0;
		*pixel_end = -1;
	}
	else if (beam_set == MBLIST_SET_OFF && pixel_set != MBLIST_SET_OFF) {
		*beam_start = 0;
		*beam_end = -1;
	}
	else if (beam_set == MBLIST_SET_OFF && beams_bath <= 0) {
		*beam_start = 0;
		*beam_end = -1;
		*pixel_start = pixel_vertical;
		*pixel_end = pixel_vertical;
	}
	else if (beam_set == MBLIST_SET_OFF) {
		*beam_start = beam_vertical;
		*beam_end = beam_vertical;
	}
	else if (beam_set == MBLIST_SET_ALL) {
		*beam_start = 0;
		*beam_end = beams_bath - 1;
	}
	else if (beam_set == MBLIST_SET_EXCLUDE_OUTER) {
		*beam_start = (beams_bath * *beam_exclude_percent) / 100;
		*beam_end = beams_bath - (*beam_start + 1);
	}
	if (pixel_set == MBLIST_SET_OFF && beams_bath > 0) {
		*pixel_start = 0;
		*pixel_end = -1;
	}
	else if (pixel_set == MBLIST_SET_ALL) {
		*pixel_start = 0;
		*pixel_end = pixels_ss - 1;
	}

	/* deal with dump_mode if set */
	if (dump_mode == DUMP_MODE_BATH) {
		*beam_start = 0;
		*beam_end = beams_bath - 1;
		*pixel_start = 0;
		*pixel_end = -1;
		strcpy(list, "XYz");
		*n_list = 3;
	}
	else if (dump_mode == DUMP_MODE_TOPO) {
		*beam_start = 0;
		*beam_end = beams_bath - 1;
		*pixel_start = 0;
		*pixel_end = -1;
		strcpy(list, "XYZ");
		*n_list = 3;
	}
	else if (dump_mode == DUMP_MODE_AMP) {
		*beam_start = 0;
		*beam_end = beams_bath - 1;
		*pixel_start = 0;
		*pixel_end = -1;
		strcpy(list, "XYB");
		*n_list = 3;
	}
	else if (dump_mode == DUMP_MODE_SS) {
		*beam_start = 0;
		*beam_end = -1;
		*pixel_start = 0;
		*pixel_end = pixels_ss - 1;
		strcpy(list, "XYb");
		*n_list = 3;
	}

	/* check if beam and pixel range is ok */
	if ((use_bath == MB_YES && *beam_end >= *beam_start) && beams_bath <= 0) {
		fprintf(stderr, "\nBathymetry data not available\n");
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
	}
	else if (use_bath == MB_YES && *beam_end >= *beam_start && (*beam_start < 0 || *beam_end >= beams_bath)) {
		fprintf(stderr, "\nBeam range %d to %d exceeds available beams 0 to %d\n", *beam_start, *beam_end, beams_bath - 1);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
	}
	if (*error == MB_ERROR_NO_ERROR && use_amp == MB_YES && beams_amp <= 0) {
		fprintf(stderr, "\nAmplitude data not available\n");
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
	}
	else if (*error == MB_ERROR_NO_ERROR && *beam_end >= *beam_start && use_amp == MB_YES &&
	         (*beam_start < 0 || *beam_end >= beams_amp)) {
		fprintf(stderr, "\nAmplitude beam range %d to %d exceeds available beams 0 to %d\n", *beam_start, *beam_end,
		        beams_amp - 1);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
	}
	if (*error == MB_ERROR_NO_ERROR && (use_ss == MB_YES || *pixel_end >= *pixel_start) && pixels_ss <= 0) {
		fprintf(stderr, "\nSidescan data not available\n");
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
	}
	else if (*error == MB_ERROR_NO_ERROR && *pixel_end >= *pixel_start && (*pixel_start < 0 || *pixel_end >= pixels_ss)) {
		fprintf(stderr, "\nPixels range %d to %d exceeds available pixels 0 to %d\n", *pixel_start, *pixel_end, pixels_ss - 1);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBCOPY function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       beam_start:    %d\n", *beam_start);
		fprintf(stderr, "dbg2       beam_end:      %d\n", *beam_end);
		fprintf(stderr, "dbg2       pixel_start:   %d\n", *pixel_start);
		fprintf(stderr, "dbg2       pixel_end:     %d\n", *pixel_end);
		fprintf(stderr, "dbg2       n_list:        %d\n", *n_list);
		for (i = 0; i < *n_list; i++)
			fprintf(stderr, "dbg2       list[%2d]:      %c\n", i, list[i]);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int set_bathyslope(int verbose, int nbath, char *beamflag, double *bath, double *bathacrosstrack, int *ndepths, double *depths,
                   double *depthacrosstrack, int *nslopes, double *slopes, double *slopeacrosstrack, int *error) {
	int status = MB_SUCCESS;
	int i;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       nbath:           %d\n", nbath);
		fprintf(stderr, "dbg2       bath:            %p\n", (void *)bath);
		fprintf(stderr, "dbg2       bathacrosstrack: %p\n", (void *)bathacrosstrack);
		fprintf(stderr, "dbg2       bath:\n");
		for (i = 0; i < nbath; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, bath[i], bathacrosstrack[i]);
	}

	/* first find all depths */
	*ndepths = 0;
	for (i = 0; i < nbath; i++) {
		if (mb_beam_ok(beamflag[i])) {
			depths[*ndepths] = bath[i];
			depthacrosstrack[*ndepths] = bathacrosstrack[i];
			(*ndepths)++;
		}
	}

	/* now calculate slopes */
	*nslopes = *ndepths + 1;
	for (i = 0; i < *ndepths - 1; i++) {
		slopes[i + 1] = (depths[i + 1] - depths[i]) / (depthacrosstrack[i + 1] - depthacrosstrack[i]);
		slopeacrosstrack[i + 1] = 0.5 * (depthacrosstrack[i + 1] + depthacrosstrack[i]);
	}
	if (*ndepths > 1) {
		slopes[0] = 0.0;
		slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[*ndepths] = 0.0;
		slopeacrosstrack[*ndepths] = depthacrosstrack[*ndepths - 1];
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndepths:         %d\n", *ndepths);
		fprintf(stderr, "dbg2       depths:\n");
		for (i = 0; i < *ndepths; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, depths[i], depthacrosstrack[i]);
		fprintf(stderr, "dbg2       nslopes:         %d\n", *nslopes);
		fprintf(stderr, "dbg2       slopes:\n");
		for (i = 0; i < *nslopes; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int get_bathyslope(int verbose, int ndepths, double *depths, double *depthacrosstrack, int nslopes, double *slopes,
                   double *slopeacrosstrack, double acrosstrack, double *depth, double *slope, int *error) {
	int status = MB_SUCCESS;
	int found_depth, found_slope;
	int idepth, islope;
	int i;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       ndepths:         %d\n", ndepths);
		fprintf(stderr, "dbg2       depths:\n");
		for (i = 0; i < ndepths; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, depths[i], depthacrosstrack[i]);
		fprintf(stderr, "dbg2       nslopes:         %d\n", nslopes);
		fprintf(stderr, "dbg2       slopes:\n");
		for (i = 0; i < nslopes; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr, "dbg2       acrosstrack:     %f\n", acrosstrack);
	}

	/* check if acrosstrack is in defined interval */
	found_depth = MB_NO;
	found_slope = MB_NO;
	if (ndepths > 1)
		if (acrosstrack >= depthacrosstrack[0] && acrosstrack <= depthacrosstrack[ndepths - 1]) {

			/* look for depth */
			idepth = -1;
			while (found_depth == MB_NO && idepth < ndepths - 2) {
				idepth++;
				if (acrosstrack >= depthacrosstrack[idepth] && acrosstrack <= depthacrosstrack[idepth + 1]) {
					*depth = depths[idepth] + (acrosstrack - depthacrosstrack[idepth]) /
					                              (depthacrosstrack[idepth + 1] - depthacrosstrack[idepth]) *
					                              (depths[idepth + 1] - depths[idepth]);
					found_depth = MB_YES;
					*error = MB_ERROR_NO_ERROR;
				}
			}

			/* look for slope */
			islope = -1;
			while (found_slope == MB_NO && islope < nslopes - 2) {
				islope++;
				if (acrosstrack >= slopeacrosstrack[islope] && acrosstrack <= slopeacrosstrack[islope + 1]) {
					*slope = slopes[islope] + (acrosstrack - slopeacrosstrack[islope]) /
					                              (slopeacrosstrack[islope + 1] - slopeacrosstrack[islope]) *
					                              (slopes[islope + 1] - slopes[islope]);
					found_slope = MB_YES;
					*error = MB_ERROR_NO_ERROR;
				}
			}
		}

	/* translate slope to degrees */
	if (found_slope == MB_YES)
		*slope = RTD * atan(*slope);

	/* check for failure */
	if (found_depth != MB_YES || found_slope != MB_YES) {
		status = MB_FAILURE;
		*error = MB_ERROR_OTHER;
		*depth = 0.0;
		*slope = 0.0;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       depth:           %f\n", *depth);
		fprintf(stderr, "dbg2       slope:           %f\n", *slope);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int printsimplevalue(int verbose, FILE *output, double value, int width, int precision, int ascii, int *invert, int *flipsign,
                     int *error) {
	int status = MB_SUCCESS;
	char format[24];

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       value:           %f\n", value);
		fprintf(stderr, "dbg2       width:           %d\n", width);
		fprintf(stderr, "dbg2       precision:       %d\n", precision);
		fprintf(stderr, "dbg2       ascii:           %d\n", ascii);
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       flipsign:        %d\n", *flipsign);
	}

	/* make print format */
	format[0] = '%';
	if (*invert == MB_YES)
		strcpy(format, "%g");
	else if (width > 0)
		sprintf(&format[1], "%d.%df", width, precision);
	else
		sprintf(&format[1], ".%df", precision);

	/* invert value if desired */
	if (*invert == MB_YES) {
		*invert = MB_NO;
		if (value != 0.0)
			value = 1.0 / value;
	}

	/* flip sign value if desired */
	if (*flipsign == MB_YES) {
		*flipsign = MB_NO;
		value = -value;
	}

	/* print value */
	if (ascii == MB_YES)
		fprintf(output, format, value);
	else
		fwrite(&value, sizeof(double), 1, output);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int printNaN(int verbose, FILE *output, int ascii, int *invert, int *flipsign, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       ascii:           %d\n", ascii);
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       flipsign:        %d\n", *flipsign);
	}

	/* reset invert flag */
	if (*invert == MB_YES)
		*invert = MB_NO;

	/* reset flipsign flag */
	if (*flipsign == MB_YES)
		*flipsign = MB_NO;

	/* print value */
	if (ascii == MB_YES)
		fprintf(output, "NaN");
	else
		fwrite(&NaN, sizeof(double), 1, output);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/*
Method to get fields from raw data, similar to mb_get_all.
*/
int mb_get_raw(int verbose, void *mbio_ptr, int *mode, int *ipulse_length, int *png_count, int *sample_rate, double *absorption,
               int *max_range, int *r_zero, int *r_zero_corr, int *tvg_start, int *tvg_stop, double *bsn, double *bso, int *tx,
               int *tvg_crossover, int *nbeams_ss, int *npixels, int *beam_samples, int *start_sample, int *range,
               double *depression, double *bs, double *ss_pixels, int *error) {
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int i;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
	}

	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	*mode = -1;
	*ipulse_length = 0;
	*png_count = 0;
	*sample_rate = 0;
	*absorption = 0;
	*max_range = 0;
	*r_zero = 0;
	*r_zero_corr = 0;
	*tvg_start = 0;
	*tvg_stop = 0;
	*bsn = 0;
	*bso = 0;
	*tx = 0;
	*tvg_crossover = 0;
	*nbeams_ss = 0;
	*npixels = 0;

	for (i = 0; i < mb_io_ptr->beams_bath_max; i++) {
		beam_samples[i] = 0;
		start_sample[i] = 0;
		range[i] = 0;
		depression[i] = 0.0;
		bs[i] = 0.0;
	}

	switch (mb_io_ptr->format) {
	case MBF_EM300MBA:
	case MBF_EM300RAW:
		mb_get_raw_simrad2(verbose, mbio_ptr, mode, ipulse_length, png_count, sample_rate, absorption, max_range, r_zero,
		                   r_zero_corr, tvg_start, tvg_stop, bsn, bso, tx, tvg_crossover, nbeams_ss, npixels, beam_samples,
		                   start_sample, range, depression, bs, ss_pixels, error);

		break;
	case MBF_EM710MBA:
	case MBF_EM710RAW:
		mb_get_raw_simrad3(verbose, mbio_ptr, mode, ipulse_length, png_count, sample_rate, absorption, max_range, r_zero,
		                   r_zero_corr, tvg_start, tvg_stop, bsn, bso, tx, tvg_crossover, nbeams_ss, npixels, beam_samples,
		                   start_sample, range, depression, bs, ss_pixels, error);

		break;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       mode:            %d\n", *mode);
		fprintf(stderr, "dbg2       ipulse_length:   %d\n", *ipulse_length);
		fprintf(stderr, "dbg2       png_count:       %d\n", *png_count);
		fprintf(stderr, "dbg2       sample_rate:     %d\n", *sample_rate);
		fprintf(stderr, "dbg2       absorption:      %f\n", *absorption);
		fprintf(stderr, "dbg2       max_range:       %d\n", *max_range);
		fprintf(stderr, "dbg2       r_zero:          %d\n", *r_zero);
		fprintf(stderr, "dbg2       r_zero_corr:     %d\n", *r_zero_corr);
		fprintf(stderr, "dbg2       tvg_start:       %d\n", *tvg_start);
		fprintf(stderr, "dbg2       tvg_stop:        %d\n", *tvg_stop);
		fprintf(stderr, "dbg2       bsn:             %f\n", *bsn);
		fprintf(stderr, "dbg2       bso:             %f\n", *bso);
		fprintf(stderr, "dbg2       tx:              %d\n", *tx);
		fprintf(stderr, "dbg2       tvg_crossover:   %d\n", *tvg_crossover);
		fprintf(stderr, "dbg2       nbeams_ss:       %d\n", *nbeams_ss);
		fprintf(stderr, "dbg2       npixels:         %d\n", *npixels);
		for (i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d range:%d depression:%f bs:%f\n", i, range[i], depression[i], bs[i]);
		}
		for (i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d samples:%d start:%d\n", i, beam_samples[i], start_sample[i]);
		}
		for (i = 0; i < *npixels; i++) {
			fprintf(stderr, "dbg2       pixel:%d ss:%f\n", i, ss_pixels[i]);
		}
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}

/*--------------------------------------------------------------------*/
/*
Method to get fields from simrad2 raw data.
*/

int mb_get_raw_simrad2(int verbose, void *mbio_ptr, int *mode, int *ipulse_length, int *png_count, int *sample_rate,
                       double *absorption, int *max_range, int *r_zero, int *r_zero_corr, int *tvg_start, int *tvg_stop,
                       double *bsn, double *bso, int *tx, int *tvg_crossover, int *nbeams_ss, int *npixels, int *beam_samples,
                       int *start_sample, int *range, double *depression, double *bs, double *ss_pixels, int *error) {
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad2_struct *store_ptr;
	struct mbsys_simrad2_ping_struct *ping_ptr;
	int i;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
	}

	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	store_ptr = (struct mbsys_simrad2_struct *)mb_io_ptr->store_data;
	ping_ptr = store_ptr->ping;

	if (store_ptr->kind == MB_DATA_DATA) {
		*mode = store_ptr->run_mode;
		*ipulse_length = store_ptr->run_tran_pulse;
		*png_count = ping_ptr->png_count;
		*sample_rate = ping_ptr->png_sample_rate;
		*absorption = ping_ptr->png_max_range * 0.01;
		*max_range = ping_ptr->png_max_range;
		*r_zero = ping_ptr->png_r_zero;
		*r_zero_corr = ping_ptr->png_r_zero_corr;
		*tvg_start = ping_ptr->png_tvg_start;
		*tvg_stop = ping_ptr->png_tvg_stop;
		*bsn = ping_ptr->png_bsn * 0.5;
		*bso = ping_ptr->png_bso * 0.5;
		*tx = ping_ptr->png_tx;
		*tvg_crossover = ping_ptr->png_tvg_crossover;
		*nbeams_ss = ping_ptr->png_nbeams_ss;
		*npixels = ping_ptr->png_npixels;

		for (i = 0; i < ping_ptr->png_nbeams; i++) {
			range[ping_ptr->png_beam_num[i] - 1] = ping_ptr->png_range[i];
			depression[ping_ptr->png_beam_num[i] - 1] = ping_ptr->png_depression[i] * .01;
			bs[ping_ptr->png_beam_num[i] - 1] = ping_ptr->png_amp[i] * 0.5;
		}
		for (i = 0; i < ping_ptr->png_nbeams_ss; i++) {
			beam_samples[ping_ptr->png_beam_index[i]] = ping_ptr->png_beam_samples[i];
			start_sample[ping_ptr->png_beam_index[i]] = ping_ptr->png_start_sample[i];
		}
		for (i = 0; i < ping_ptr->png_npixels; i++)
			ss_pixels[i] = ping_ptr->png_ssraw[i] * 0.5;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       mode:            %d\n", *mode);
		fprintf(stderr, "dbg2       ipulse_length:   %d\n", *ipulse_length);
		fprintf(stderr, "dbg2       png_count:       %d\n", *png_count);
		fprintf(stderr, "dbg2       sample_rate:     %d\n", *sample_rate);
		fprintf(stderr, "dbg2       absorption:      %f\n", *absorption);
		fprintf(stderr, "dbg2       max_range:       %d\n", *max_range);
		fprintf(stderr, "dbg2       r_zero:          %d\n", *r_zero);
		fprintf(stderr, "dbg2       r_zero_corr:     %d\n", *r_zero_corr);
		fprintf(stderr, "dbg2       tvg_start:       %d\n", *tvg_start);
		fprintf(stderr, "dbg2       tvg_stop:        %d\n", *tvg_stop);
		fprintf(stderr, "dbg2       bsn:             %f\n", *bsn);
		fprintf(stderr, "dbg2       bso:             %f\n", *bso);
		fprintf(stderr, "dbg2       tx:              %d\n", *tx);
		fprintf(stderr, "dbg2       tvg_crossover:   %d\n", *tvg_crossover);
		fprintf(stderr, "dbg2       nbeams_ss:       %d\n", *nbeams_ss);
		fprintf(stderr, "dbg2       npixels:         %d\n", *npixels);
		for (i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d range:%d depression:%f bs:%f\n", i, range[i], depression[i], bs[i]);
		}
		for (i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d samples:%d start:%d\n", i, beam_samples[i], start_sample[i]);
		}
		for (i = 0; i < *npixels; i++) {
			fprintf(stderr, "dbg2       pixel:%d ss:%f\n", i, ss_pixels[i]);
		}
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}
/*--------------------------------------------------------------------*/
/*
Method to get fields from simrad3 raw data.
*/

int mb_get_raw_simrad3(int verbose, void *mbio_ptr, int *mode, int *ipulse_length, int *png_count, int *sample_rate,
                       double *absorption, int *max_range, int *r_zero, int *r_zero_corr, int *tvg_start, int *tvg_stop,
                       double *bsn, double *bso, int *tx, int *tvg_crossover, int *nbeams_ss, int *npixels, int *beam_samples,
                       int *start_sample, int *range, double *depression, double *bs, double *ss_pixels, int *error) {
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad3_struct *store_ptr;
	struct mbsys_simrad3_ping_struct *ping_ptr;
	int i;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
	}

	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	store_ptr = (struct mbsys_simrad3_struct *)mb_io_ptr->store_data;
	ping_ptr = (struct mbsys_simrad3_ping_struct *)&(store_ptr->pings[store_ptr->ping_index]);

	if (store_ptr->kind == MB_DATA_DATA) {
		*mode = store_ptr->run_mode;
		*ipulse_length = store_ptr->run_tran_pulse;
		*png_count = ping_ptr->png_count;
		*sample_rate = ping_ptr->png_sample_rate;
		*absorption = store_ptr->run_absorption * 0.01;
		*max_range = 0;
		*r_zero = ping_ptr->png_r_zero;
		*r_zero_corr = 0;
		*tvg_start = 0;
		*tvg_stop = 0;
		*bsn = ping_ptr->png_bsn * 0.1;
		*bso = ping_ptr->png_bso * 0.1;
		*tx = ping_ptr->png_tx * 0.1;
		*tvg_crossover = ping_ptr->png_tvg_crossover;
		*nbeams_ss = ping_ptr->png_nbeams_ss;
		*npixels = ping_ptr->png_npixels;

		for (i = 0; i < ping_ptr->png_nbeams; i++) {
			range[i] = ping_ptr->png_range[i];
			depression[i] = ping_ptr->png_depression[i] * .01;
			bs[i] = ping_ptr->png_amp[i] * 0.5;
		}
		for (i = 0; i < ping_ptr->png_nbeams_ss; i++) {
			beam_samples[i] = ping_ptr->png_beam_samples[i];
			start_sample[i] = ping_ptr->png_start_sample[i];
		}
		for (i = 0; i < ping_ptr->png_npixels; i++)
			ss_pixels[i] = ping_ptr->png_ssraw[i] * 0.5;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       mode:            %d\n", *mode);
		fprintf(stderr, "dbg2       ipulse_length:   %d\n", *ipulse_length);
		fprintf(stderr, "dbg2       png_count:       %d\n", *png_count);
		fprintf(stderr, "dbg2       sample_rate:     %d\n", *sample_rate);
		fprintf(stderr, "dbg2       absorption:      %f\n", *absorption);
		fprintf(stderr, "dbg2       max_range:       %d\n", *max_range);
		fprintf(stderr, "dbg2       r_zero:          %d\n", *r_zero);
		fprintf(stderr, "dbg2       r_zero_corr:     %d\n", *r_zero_corr);
		fprintf(stderr, "dbg2       tvg_start:       %d\n", *tvg_start);
		fprintf(stderr, "dbg2       tvg_stop:        %d\n", *tvg_stop);
		fprintf(stderr, "dbg2       bsn:             %f\n", *bsn);
		fprintf(stderr, "dbg2       bso:             %f\n", *bso);
		fprintf(stderr, "dbg2       tx:              %d\n", *tx);
		fprintf(stderr, "dbg2       tvg_crossover:   %d\n", *tvg_crossover);
		fprintf(stderr, "dbg2       nbeams_ss:       %d\n", *nbeams_ss);
		fprintf(stderr, "dbg2       npixels:         %d\n", *npixels);
		for (i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d range:%d depression:%f bs:%f\n", i, range[i], depression[i], bs[i]);
		}
		for (i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d samples:%d start:%d\n", i, beam_samples[i], start_sample[i]);
		}
		for (i = 0; i < *npixels; i++) {
			fprintf(stderr, "dbg2       pixel:%d ss:%f\n", i, ss_pixels[i]);
		}
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;

	/* MBIO status variables */
	int status = MB_SUCCESS;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	char *message;

	/* MBIO read control parameters */
	int read_datalist = MB_NO;
	char read_file[MB_PATH_MAXLINE];
	void *datalist;
	int look_processed = MB_DATALIST_LOOK_UNSET;
	double file_weight;
	int format;
	int pings;
	int pings_read;
	int decimate;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double btime_d;
	double etime_d;
	double speedmin;
	double timegap;
	char file[MB_PATH_MAXLINE];
	char dfile[MB_PATH_MAXLINE];
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* output format list controls */
	char list[MAX_OPTIONS];
	int n_list;
	int beam_set = MBLIST_SET_OFF;
	int beam_start;
	int beam_end;
	int beam_exclude_percent;
	int beam_vertical = 0;
	int pixel_set = MBLIST_SET_OFF;
	int pixel_start;
	int pixel_end;
	int pixel_vertical = 0;
	int dump_mode = 1;
	double distance_total;
	int nread;
	int beam_status = MB_SUCCESS;
	int pixel_status = MB_SUCCESS;
	int time_j[5];
	int use_bath = MB_NO;
	int use_amp = MB_NO;
	int use_ss = MB_NO;
	int use_slope = MB_NO;
	int use_attitude = MB_NO;
	int use_nav = MB_NO;
	int use_gains = MB_NO;
	int use_detects = MB_YES;
	int use_pingnumber = MB_NO;
	int check_values = MBLIST_CHECK_ON;
	int check_nav = MB_NO;
	int check_bath = MB_NO;
	int check_amp = MB_NO;
	int check_ss = MB_NO;
	int invert_next_value = MB_NO;
	int signflip_next_value = MB_NO;
	int raw_next_value = MB_NO;
	int port_next_value = MB_NO;
	int stbd_next_value = MB_NO;
	int sensornav_next_value = MB_NO;
	int sensorrelative_next_value = MB_NO;
	int projectednav_next_value = MB_NO;
	int use_raw = MB_NO;
	int special_character = MB_NO;
	int first = MB_YES;
	int ascii = MB_YES;
	int netcdf = MB_NO;
	int netcdf_cdl = MB_YES;
	int segment = MB_NO;
	int segment_mode = MBLIST_SEGMENT_MODE_NONE;
	char segment_tag[MB_PATH_MAXLINE];
	char delimiter[MB_PATH_MAXLINE];

	/* MBIO read values */
	void *mbio_ptr = NULL;
	void *store_ptr = NULL;
	int kind;
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sonardepth;
	double draft;
	double roll;
	double pitch;
	double heave;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathacrosstrack = NULL;
	double *bathalongtrack = NULL;
	int *detect = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *ssacrosstrack = NULL;
	double *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];
	int icomment = 0;
	unsigned int pingnumber;

	/* additional time variables */
	int first_m = MB_YES;
	double time_d_ref;
	int first_u = MB_YES;
	time_t time_u;
	time_t time_u_ref;
	double seconds;

	/* crosstrack slope values */
	double avgslope;
	double sx, sy, sxx, sxy;
	int ns;
	double angle, depth, slope;
	int ndepths;
	double *depths = NULL;
	double *depthacrosstrack = NULL;
	int nslopes;
	double *slopes = NULL;
	double *slopeacrosstrack = NULL;

	/* course calculation variables */
	int use_course = MB_NO;
	int use_time_interval = MB_NO;
	double course, course_old;
	double time_d_old, dt;
	double time_interval;
	double speed_made_good, speed_made_good_old;
	double navlon_old, navlat_old;
	double dx, dy, dist;
	double delta, b;
	double dlon, dlat, minutes;
	int degrees;
	char hemi;
	double headingx, headingy, mtodeglon, mtodeglat;

	/* swathbounds variables */
	int use_swathbounds = MB_NO;
	int beam_port, beam_stbd;
	int pixel_port, pixel_stbd;

	/* projected coordinate system */
	int use_projection = MB_NO;
	char projection_pars[MB_PATH_MAXLINE];
	char projection_id[MB_PATH_MAXLINE];
	int proj_status;
	void *pjptr = NULL;
	double reference_lon, reference_lat;
	int utm_zone;
	double naveasting, navnorthing, deasting, dnorthing;

	/* bathymetry feet flag */
	int bathy_in_feet = MB_NO;
	double bathy_scale;

	/* raw data values */
	int count = 0;
	int invert;
	int flip;
	int mode;
	int ipulse_length;
	int png_count;
	int sample_rate;
	double absorption;
	int max_range;
	int r_zero;
	int r_zero_corr;
	int tvg_start;
	int tvg_stop;
	double bsn;
	double bso;
	double mback;
	int nback;
	int tx;
	int tvg_crossover;
	int nbeams_ss;
	int npixels;
	int *beam_samples = NULL;
	int *range = NULL;
	int *start_sample = NULL;
	double *depression = NULL;
	double *bs = NULL;
	double *ss_pixels = NULL;
	double transmit_gain;
	double pulse_length;
	double receive_gain;

	int shellstatus;
	int read_data;
	int nbeams;
	int i, j, k, m;

	/* output files */
	FILE **output;
	FILE *outfile;
	char output_file[MB_PATH_MAXLINE];
	char output_file_temp[MB_PATH_MAXLINE];
	char buffer[MB_BUFFER_MAX];

	/* netcdf variables */
	char variable[MB_PATH_MAXLINE];
	int lcount = 0;
	time_t right_now;
	char date[32], user[128], *user_ptr, host[128];

	/* get current default values */
	status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* set default input to datalist.mb-1 */
	strcpy(read_file, "datalist.mb-1");

	/* set up the default list controls
	    (Time, lon, lat, heading, speed, along-track distance, center beam depth) */
	list[0] = 'T';
	list[1] = 'X';
	list[2] = 'Y';
	list[3] = 'H';
	list[4] = 'S';
	list[5] = 'L';
	list[6] = 'Z';
	n_list = 7;
	delimiter[0] = '\t';
	delimiter[1] = '\0';
	projection_pars[0] = '\0';

	/* set dump mode flag to DUMP_MODE_LIST */
	dump_mode = DUMP_MODE_LIST;
	decimate = 1;

	/* get NaN value */
	MB_MAKE_DNAN(NaN);

	strcpy(output_file, "-");

	/* process argument list */
	while ((c = getopt(argc, argv, "AaB:b:CcD:d:E:e:F:f:G:g:I:i:J:j:K:k:L:l:M:m:N:n:O:o:P:p:QqR:r:S:s:T:t:U:u:X:x:Z:z:VvWwHh")) !=
	       -1)
		switch (c) {
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'A':
		case 'a':
			ascii = MB_NO;
			netcdf_cdl = MB_NO;
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'C':
		case 'c':
			netcdf = MB_YES;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf(optarg, "%d", &dump_mode);
			if (dump_mode == DUMP_MODE_BATH)
				beam_set = MBLIST_SET_ALL;
			else if (dump_mode == DUMP_MODE_TOPO)
				beam_set = MBLIST_SET_ALL;
			else if (dump_mode == DUMP_MODE_AMP)
				beam_set = MBLIST_SET_ALL;
			else if (dump_mode == DUMP_MODE_SS)
				pixel_set = MBLIST_SET_ALL;
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'G':
		case 'g':
			sscanf(optarg, "%s", delimiter);
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", read_file);
			flag++;
			break;
		case 'J':
		case 'j':
			sscanf(optarg, "%s", projection_pars);
			use_projection = MB_YES;
			flag++;
			break;
		case 'K':
		case 'k':
			sscanf(optarg, "%d", &decimate);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf(optarg, "%d", &lonflip);
			flag++;
			break;
		case 'M':
		case 'm':
			if (optarg[0] == 'a' || optarg[0] == 'A') {
				beam_set = MBLIST_SET_ALL;
			}
			else if (optarg[0] == 'x' || optarg[0] == 'X') {
				beam_set = MBLIST_SET_EXCLUDE_OUTER;
				sscanf(optarg, "%*c%d", &beam_exclude_percent);
			}
			else {
				sscanf(optarg, "%d/%d", &beam_start, &beam_end);
				beam_set = MBLIST_SET_ON;
			}
			flag++;
			break;
		case 'N':
		case 'n':
			if (optarg[0] == 'a' || optarg[0] == 'A') {
				pixel_set = MBLIST_SET_ALL;
			}
			else {
				sscanf(optarg, "%d/%d", &pixel_start, &pixel_end);
				pixel_set = MBLIST_SET_ON;
			}
			flag++;
			break;
		case 'O':
		case 'o':
			for (j = 0, n_list = 0; j < (int)strlen(optarg); j++, n_list++)
				if (n_list < MAX_OPTIONS) {
					list[n_list] = optarg[j];
					if (list[n_list] == '^')
						use_projection = MB_YES;
				}
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf(optarg, "%d", &pings);
			flag++;
			break;
		case 'Q':
		case 'q':
			check_values = MBLIST_CHECK_OFF_RAW;
			flag++;
			break;
		case 'R':
		case 'r':
			mb_get_bounds(optarg, bounds);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf(optarg, "%lf", &speedmin);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf(optarg, "%lf", &timegap);
			flag++;
			break;
		case 'U':
		case 'u':
			if (optarg[0] == 'N')
				check_nav = MB_YES;
			else {
				sscanf(optarg, "%d", &check_values);
				if (check_values < MBLIST_CHECK_ON || check_values > MBLIST_CHECK_OFF_FLAGNAN)
					check_values = MBLIST_CHECK_ON;
			}
			flag++;
			break;
		case 'W':
		case 'w':
			bathy_in_feet = MB_YES;
			break;
		case 'X':
		case 'x':
			sscanf(optarg, "%s", output_file);
			break;
		case 'Z':
		case 'z':
			segment = MB_YES;
			sscanf(optarg, "%s", segment_tag);
			if (strcmp(segment_tag, "swathfile") == 0)
				segment_mode = MBLIST_SEGMENT_MODE_SWATHFILE;
			else if (strcmp(segment_tag, "datalist") == 0)
				segment_mode = MBLIST_SEGMENT_MODE_DATALIST;
			else
				segment_mode = MBLIST_SEGMENT_MODE_TAG;
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       help:           %d\n", help);
		fprintf(stderr, "dbg2       format:         %d\n", format);
		fprintf(stderr, "dbg2       pings:          %d\n", pings);
		fprintf(stderr, "dbg2       decimate:       %d\n", decimate);
		fprintf(stderr, "dbg2       lonflip:        %d\n", lonflip);
		fprintf(stderr, "dbg2       bounds[0]:      %f\n", bounds[0]);
		fprintf(stderr, "dbg2       bounds[1]:      %f\n", bounds[1]);
		fprintf(stderr, "dbg2       bounds[2]:      %f\n", bounds[2]);
		fprintf(stderr, "dbg2       bounds[3]:      %f\n", bounds[3]);
		fprintf(stderr, "dbg2       btime_i[0]:     %d\n", btime_i[0]);
		fprintf(stderr, "dbg2       btime_i[1]:     %d\n", btime_i[1]);
		fprintf(stderr, "dbg2       btime_i[2]:     %d\n", btime_i[2]);
		fprintf(stderr, "dbg2       btime_i[3]:     %d\n", btime_i[3]);
		fprintf(stderr, "dbg2       btime_i[4]:     %d\n", btime_i[4]);
		fprintf(stderr, "dbg2       btime_i[5]:     %d\n", btime_i[5]);
		fprintf(stderr, "dbg2       btime_i[6]:     %d\n", btime_i[6]);
		fprintf(stderr, "dbg2       etime_i[0]:     %d\n", etime_i[0]);
		fprintf(stderr, "dbg2       etime_i[1]:     %d\n", etime_i[1]);
		fprintf(stderr, "dbg2       etime_i[2]:     %d\n", etime_i[2]);
		fprintf(stderr, "dbg2       etime_i[3]:     %d\n", etime_i[3]);
		fprintf(stderr, "dbg2       etime_i[4]:     %d\n", etime_i[4]);
		fprintf(stderr, "dbg2       etime_i[5]:     %d\n", etime_i[5]);
		fprintf(stderr, "dbg2       etime_i[6]:     %d\n", etime_i[6]);
		fprintf(stderr, "dbg2       speedmin:       %f\n", speedmin);
		fprintf(stderr, "dbg2       timegap:        %f\n", timegap);
		fprintf(stderr, "dbg2       file:           %s\n", file);
		fprintf(stderr, "dbg2       output_file:    %s\n", output_file);
		fprintf(stderr, "dbg2       ascii:          %d\n", ascii);
		fprintf(stderr, "dbg2       netcdf:         %d\n", netcdf);
		fprintf(stderr, "dbg2       netcdf_cdl:     %d\n", netcdf_cdl);
		fprintf(stderr, "dbg2       segment:        %d\n", segment);
		fprintf(stderr, "dbg2       segment_mode:   %d\n", segment_mode);
		fprintf(stderr, "dbg2       segment_tag:    %s\n", segment_tag);
		fprintf(stderr, "dbg2       delimiter:      %s\n", delimiter);
		fprintf(stderr, "dbg2       beam_set:       %d\n", beam_set);
		fprintf(stderr, "dbg2       beam_start:     %d\n", beam_start);
		fprintf(stderr, "dbg2       beam_end:       %d\n", beam_end);
		fprintf(stderr, "dbg2       beam_exclude_percent: %d\n", beam_exclude_percent);
		fprintf(stderr, "dbg2       pixel_set:      %d\n", pixel_set);
		fprintf(stderr, "dbg2       pixel_start:    %d\n", pixel_start);
		fprintf(stderr, "dbg2       pixel_end:      %d\n", pixel_end);
		fprintf(stderr, "dbg2       dump_mode:      %d\n", dump_mode);
		fprintf(stderr, "dbg2       check_values:   %d\n", check_values);
		fprintf(stderr, "dbg2       check_nav:      %d\n", check_nav);
		fprintf(stderr, "dbg2       use_projection: %d\n", use_projection);
		fprintf(stderr, "dbg2       projection_pars:%s\n", projection_pars);
		fprintf(stderr, "dbg2       n_list:         %d\n", n_list);
		for (i = 0; i < n_list; i++)
			fprintf(stderr, "dbg2         list[%d]:      %c\n", i, list[i]);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		exit(error);
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	/* set bathymetry scaling */
	if (bathy_in_feet == MB_YES)
		bathy_scale = 1.0 / 0.3048;
	else
		bathy_scale = 1.0;

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES) {
		if ((status = mb_datalist_open(verbose, &datalist, read_file, look_processed, &error)) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		if ((status = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
			read_data = MB_YES;
		else
			read_data = MB_NO;
	}
	/* else copy single filename to be read */
	else {
		strcpy(file, read_file);
		read_data = MB_YES;
	}

	/* set the initial along track distance here so */
	/* it's cummulative over multiple files*/
	distance_total = 0.0;

	/* initialize output files */
	status = mb_mallocd(verbose, __FILE__, __LINE__, n_list * sizeof(FILE *), (void **)&output, &error);

	if (netcdf == MB_NO) {
		/* open output file */
		if (0 == strncmp("-", output_file, 2))
			outfile = stdout;
		else
			outfile = fopen(output_file, "w");
		if (NULL == outfile) {
			fprintf(stderr, "Could not open file: %s\n", output_file);
			exit(1);
		}

		/* for non netcdf all output goes to the same file */
		for (i = 0; i < n_list; i++)
			output[i] = outfile;
	}
	else {
		/* netcdf must be ascii and must not be segmented */
		ascii = MB_YES;
		segment = MB_NO;

		/* open CDL file */
		if (0 == strncmp("-", output_file, 2) && netcdf_cdl == MB_NO)
			strcpy(output_file, "mblist.nc");
		if (0 == strncmp("-", output_file, 2)) {
			outfile = stdout;
		}
		else {
			strncpy(output_file_temp, output_file, MB_PATH_MAXLINE - 5);
			if (netcdf_cdl == MB_NO)
				strcat(output_file_temp, ".cdl");
			outfile = fopen(output_file_temp, "w+");
			if (outfile == NULL) {
				fprintf(stderr, "Unable to open file: %s\n", output_file_temp);
				exit(1);
			}
		}

		/* output CDL headers */
		fprintf(outfile, "netcdf mlist {\n\n\t// ");
		for (i = 0; i < argc; i++)
			fprintf(outfile, "%s ", argv[i]);
		fprintf(outfile, "\n");
		fprintf(outfile, "dimensions:\n\ttimestring = 26, timestring_J = 24, timestring_j = 23, \n\t");
		fprintf(outfile, "timefields_J = 6,  timefields_j = 5, timefields_t = 7, latm = 13, \n\t");

		/* find dimensions in format list */
		raw_next_value = MB_NO;
		for (i = 0; i < n_list; i++)
			if (list[i] == '/' || list[i] == '-' || list[i] == '=' || list[i] == '+') {
				// ignore
			}
			else if (raw_next_value == MB_NO) {
				if (list[i] == '.')
					raw_next_value = MB_YES;
			}
			else if (list[i] >= '0' && list[i] <= '9')
				count = count * 10 + list[i] - '0';
			else {
				raw_next_value = MB_NO;
				if (count > 0) {
					fprintf(outfile, "%c = %d,  ", list[i], count);
					count = 0;
				}
			}

		fprintf(outfile, "\n\tdata = unlimited ;\n\n");
		fprintf(outfile, "variables:\n\t");
		fprintf(outfile, ":command_line = \"");
		for (i = 0; i < argc; i++)
			fprintf(outfile, "%s ", argv[i]);
		fprintf(outfile, "\n");
		fprintf(outfile, "\t:mbsystem_version = \"%s\";\n", MB_VERSION);

		right_now = time((time_t *)0);
		strcpy(date, ctime(&right_now));
		date[strlen(date) - 1] = '\0';
		if ((user_ptr = (char *)getenv("USER")) == NULL)
			user_ptr = (char *)getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user, user_ptr);
		else
			strcpy(user, "unknown");
		gethostname(host, 128);

		fprintf(outfile, "\t:run = \"by <%s> on cpu <%s> at <%s>\";\n\n", user, host, date);

		/* get temporary output file for each variable */
		for (i = 0; i < n_list; i++) {
			output[i] = tmpfile();
			if (output[i] == NULL) {
				fprintf(stderr, "Unable to open temp files\n");
				exit(1);
			}

			if (raw_next_value == MB_NO) {
				switch (list[i]) {
				case '/': /* Inverts next simple value */
					invert_next_value = MB_YES;
					break;

				case '-': /* Flip sign on next simple value */
					signflip_next_value = MB_YES;
					break;

				case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
					sensornav_next_value = MB_YES;
					break;

				case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
					sensorrelative_next_value = MB_YES;
					break;

				case '^': /* Print position values in projected coordinates
				           * - easting northing rather than lon lat
				           * - applies to XY */
					projectednav_next_value = MB_YES;
					break;

				case '.': /* Raw value next field */
					raw_next_value = MB_YES;
					break;

				case '=': /* Port-most value next field -ignored here */
					break;

				case '+': /* Starboard-most value next field - ignored here*/
					break;

				case 'A': /* Average seafloor crosstrack slope */
					strcpy(variable, "aslope");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Average seafloor crosstrack slope\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "tangent of angle from seafloor to vertical\";\n");
					else
						fprintf(outfile, "tangent of angle from seafloor to horizontal\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'a': /* Per-beam seafloor crosstrack slope */
					strcpy(variable, "bslope");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Per-beam seafloor crosstrack slope\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "tangent of angle from seafloor to vertical\";\n");
					else
						fprintf(outfile, "tangent of angle from seafloor to horizontal\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'B': /* amplitude */
					strcpy(variable, "amplitude");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Amplitude\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					if (format == MBF_EM300RAW || format == MBF_EM300MBA)
						fprintf(outfile, "dB + 64\";\n");
					else
						fprintf(outfile, "backscatter\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;

					break;

				case 'b': /* sidescan */
					strcpy(variable, "sidescan");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"sidescan\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					if (format == MBF_EM300RAW || format == MBF_EM300MBA)
						fprintf(outfile, "dB + 64\";\n");
					else
						fprintf(outfile, "backscatter\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;

					break;

				case 'C': /* Sonar altitude (m) */
					strcpy(variable, "altitude");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Sonar altitude\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;

					break;

				case 'c': /* Sonar transducer depth (m) */
					strcpy(variable, "transducer");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Sonar transducer depth\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'D': /* acrosstrack dist. */
				case 'd':
					strcpy(variable, "acrosstrack");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Acrosstrack distance\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					if (bathy_in_feet == MB_YES)
						fprintf(outfile, "f\";\n");
					else
						fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'E': /* alongtrack dist. */
				case 'e':
					strcpy(variable, "alongtrack");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Alongtrack distance\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					if (bathy_in_feet == MB_YES)
						fprintf(outfile, "f\";\n");
					else
						fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;

					break;

				case 'F': /* beamflag (numeric only for netcdf) */
				case 'f':
					strcpy(variable, "beamflag");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Beamflag\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					if (bathy_in_feet == MB_YES)
						fprintf(outfile, "f\";\n");
					else
						fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;

					break;

				case 'G': /* flat bottom grazing angle */
					strcpy(variable, "flatgrazing");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Flat bottom grazing angle\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "tangent of angle from beam to vertical\";\n");
					else
						fprintf(outfile, "tangent of angle from beam to horizontal\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'g': /* grazing angle using slope */
					strcpy(variable, "grazing");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Grazing angle using slope\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "tangent of angle from beam to perpendicular to seafloor\";\n");
					else
						fprintf(outfile, "tangent of angle from beam to seafloor\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'H': /* heading */
					strcpy(variable, "heading");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Heading\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "degrees true\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'h': /* course */
					strcpy(variable, "course");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Course\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "degrees true\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'J': /* time string */
					strcpy(variable, "time_J");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data,timefields_J);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Time - year julian_day hour minute seconds\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);

					fprintf(outfile, "year, julian day, hour, minute, second, nanosecond\";\n");
					break;

				case 'j': /* time string */
					strcpy(variable, "time_j");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data,timefields_j);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Time - year julian_day minute seconds\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);

					fprintf(outfile, "year, julian day, minute, second, nanosecond\";\n");

					break;

				case 'L': /* along-track distance (km) */
					strcpy(variable, "along_track");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Alongtrack distance\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "km\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;

					break;

				case 'l': /* along-track distance (m) */
					strcpy(variable, "along_track_m");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Alongtrack distance\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'M': /* Decimal unix seconds since
				              1/1/70 00:00:00 */
					strcpy(variable, "unix_time");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tdouble %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Seconds since 1/1/70 00:00:00\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "s\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'm': /* time in decimal seconds since
				              first record */

					strcpy(variable, "survey_time");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tdouble %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Seconds since first record\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "s\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'N': /* ping counter */
					strcpy(variable, "ping");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Ping counter\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					fprintf(outfile, "pings\";\n");
					break;

				case 'P': /* pitch */
					strcpy(variable, "pitch");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Pitch\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "degrees from horizontal\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'p': /* draft */
					strcpy(variable, "draft");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Draft\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'q': /* bottom detect type */
				case 'Q': /* bottom detect type */
					strcpy(variable, "bottom_detect_type");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Bottom detect type\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					fprintf(outfile, "0=unknown,1=amplitude,2=phase\";\n");
					break;

				case 'R': /* roll */
					strcpy(variable, "roll");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Roll\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "degrees from horizontal\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'r': /* heave */
					strcpy(variable, "heave");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Heave\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'S': /* speed */
					strcpy(variable, "speed");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Speed\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "km/hr\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 's': /* speed made good */
					strcpy(variable, "speed_made_good");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Speed made good\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "km/hr\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
					strcpy(variable, "time_T");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tchar %s(data,timestring);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Time string - year/month/day/hour/minute/seconds\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);

					fprintf(outfile, "yyyy/MM/dd/hh/mm/ss.ssssss\";\n");
					break;

				case 't': /* yyyy mm dd hh mm ss time string */
					strcpy(variable, "time_t");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data,timefields_t);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Time - year month day hour minute seconds\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);

					fprintf(outfile, "year, month, day, hour, minute, second, nanosecond\";\n");
					break;

				case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
					strcpy(variable, "unix_time_s");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Integer seconds since 1/1/70 00:00:00\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					fprintf(outfile, "s\";\n");
					break;

				case 'u': /* time in seconds since first record */
					strcpy(variable, "survey_time_s");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Integer seconds since first record\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					fprintf(outfile, "s\";\n");
					break;

				case 'V': /* time in seconds since last ping */
				case 'v':
					strcpy(variable, "ping_time");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Seconds since last ping\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					fprintf(outfile, "s\";\n");
					break;

				case 'X': /* longitude decimal degrees */
					strcpy(variable, "longitude");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tdouble %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Longitude\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "degrees\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'x': /* longitude degress + decimal minutes */
					strcpy(variable, "longitude_minutes");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tchar %s(data,latm);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Longitude - decimal minutes\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);

					fprintf(outfile, "ddd mm.mmmmmH\";\n");
					break;

				case 'Y': /* latitude decimal degrees */
					strcpy(variable, "latitude");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tdouble %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Latitude\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "degrees\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'y': /* latitude degrees + decimal minutes */
					strcpy(variable, "latitude_minutes");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tchar %s(data,latm);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Latitude - decimal minutes\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);

					fprintf(outfile, "ddd mm.mmmmmH\";\n");
					break;

				case 'Z': /* topography */
					strcpy(variable, "topography");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Topography\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					if (bathy_in_feet == MB_YES)
						fprintf(outfile, "f\";\n");
					else
						fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case 'z': /* depth */
					strcpy(variable, "depth");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Depth\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					if (bathy_in_feet == MB_YES)
						fprintf(outfile, "f\";\n");
					else
						fprintf(outfile, "m\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					break;

				case '#': /* beam number */
					strcpy(variable, "beam");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Beam number\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					fprintf(outfile, "number\";\n");
					break;
				}
			}
			else {
				switch (list[i]) {
				case '/': /* Inverts next simple value */
					invert_next_value = MB_YES;
					break;

				case '-': /* Flip sign on next simple value */
					signflip_next_value = MB_YES;
					break;

				case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
					sensornav_next_value = MB_YES;
					break;

				case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
					sensorrelative_next_value = MB_YES;
					break;

				case '^': /* Print position values in projected coordinates
				           * - easting northing rather than lon lat
				           * - applies to XY */
					projectednav_next_value = MB_YES;
					break;

				case '.': /* Raw value next field */
					raw_next_value = MB_YES;
					count = 0;
					break;

				case '=': /* Port-most value next field -ignored here */
					break;

				case '+': /* Starboard-most value next field - ignored here*/
					break;

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					count = count * 10 + list[i] - '0';
					break;

				case 'A': /* backscatter */
					strcpy(variable, "backscatter");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Backscatter\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "dB\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'a': /* absorption */
					strcpy(variable, "absorption");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Mean absorption\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "dB/km\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'B': /* BSN - Normal incidence backscatter */
					strcpy(variable, "bsn");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Normal incidence backscatter\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "dB\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'b': /* BSO - Oblique backscatter */
					strcpy(variable, "bso");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Oblique backscatter\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "dB\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'c': /* mean backscatter */
					strcpy(variable, "mback");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Mean backscatter\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					if (format == MBF_EM300RAW || format == MBF_EM300MBA)
						fprintf(outfile, "dB + 64\";\n");
					else
						fprintf(outfile, "backscatter\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'd': /* beam depression angle */
					strcpy(variable, "depression");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Beam depression angle\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "degrees\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'F': /* filename */
					strcpy(variable, "filename");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tchar %s(data,pathsize);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Name of swath data file\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);

					fprintf(outfile, "file name\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'f': /* format */
					strcpy(variable, "format");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tshort %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"MBsystem file format number\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);

					fprintf(outfile, "see mbformat\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'G': /* TVG start */
					strcpy(variable, "tvg_start");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Start range of TVG ramp\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "samples\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'g': /* TVG stop */
					strcpy(variable, "tvg_stop");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Stop range of TVG ramp\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "samples\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'L': /* Pulse length */
					strcpy(variable, "pulse_length");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Pulse Length\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					fprintf(outfile, "us");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'l': /* Transmit pulse length */
					strcpy(variable, "pulse_length");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Pulse length\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "seconds\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'M': /* Mode */
					strcpy(variable, "mode");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Sounder mode\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					fprintf(outfile, "0=very shallow,1=shallow,2=medium,3=deep,4=very deep,5=extra deep\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'N': /* Ping number */
					strcpy(variable, "ping_no");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Sounder ping counter\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					fprintf(outfile, "pings\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'p': /* sidescan pixel */
					strcpy(variable, "sidescan");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					if (count == 0)
						fprintf(outfile, "\tfloat %s(data);\n", variable);
					else
						fprintf(outfile, "\tfloat %s(data, %c);\n", variable, list[i]);

					fprintf(outfile, "\t\t%s:long_name = \"Raw sidescan pixels\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "dB\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'R': /* range */
					strcpy(variable, "range");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Range \";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "samples\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'r': /* Sample rate */
					strcpy(variable, "sample_rate");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Sample Rate\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "Hertz\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'S': /* Sidescan pixels */
					strcpy(variable, "pixels");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Total sidescan pixels \";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "pixels\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 's': /* Sidescan pixels per beam */
					strcpy(variable, "beam_pixels");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Sidescan pixels per beam\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "pixels\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 'T': /* Transmit gain */
					strcpy(variable, "transmit_gain");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Transmit gain\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "dB\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				case 't': /* Receive gain */
					strcpy(variable, "receive_gain");
					if (signflip_next_value == MB_YES)
						strcat(variable, "-");
					if (invert_next_value == MB_YES)
						strcat(variable, "_");

					fprintf(output[i], "\t%s = ", variable);

					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Receive gain\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"", variable);
					if (signflip_next_value == MB_YES)
						fprintf(outfile, "-");
					if (invert_next_value == MB_YES)
						fprintf(outfile, "1/");
					fprintf(outfile, "dB\";\n");

					signflip_next_value = MB_NO;
					invert_next_value = MB_NO;
					raw_next_value = MB_NO;
					break;

				default:
					raw_next_value = MB_NO;
					break;
				}
			}
		}
		fprintf(outfile, "\n\ndata:\n");
	}

	/* loop over all files to be read */
	while (read_data == MB_YES) {

		/* initialize reading the swath file */
		if ((status = mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* figure out whether bath, amp, or ss will be used */
		if (dump_mode == DUMP_MODE_BATH || dump_mode == DUMP_MODE_TOPO)
			use_bath = MB_YES;
		else if (dump_mode == DUMP_MODE_AMP)
			use_amp = MB_YES;
		else if (dump_mode == DUMP_MODE_SS)
			use_ss = MB_YES;
		else
			for (i = 0; i < n_list; i++) {
				if (raw_next_value == MB_NO) {
					if (list[i] == 'Z' || list[i] == 'z' || list[i] == 'A' || list[i] == 'a' || list[i] == 'Q' || list[i] == 'q')
						use_bath = MB_YES;
					if (list[i] == 'B')
						use_amp = MB_YES;
					if (list[i] == 'b')
						use_ss = MB_YES;
					if (list[i] == 'h')
						use_course = MB_YES;
					if (list[i] == 's')
						use_course = MB_YES;
					if (list[i] == 'V' || list[i] == 'v')
						use_time_interval = MB_YES;
					if (list[i] == 'A' || list[i] == 'a' || list[i] == 'G' || list[i] == 'g')
						use_slope = MB_YES;
					if (list[i] == 'P' || list[i] == 'p' || list[i] == 'R' || list[i] == 'r')
						use_attitude = MB_YES;
					if (list[i] == 'Q' || list[i] == 'q')
						use_detects = MB_YES;
					if (list[i] == 'N' || list[i] == 'n')
						use_pingnumber = MB_YES;
					if (list[i] == 'X' || list[i] == 'x' || list[i] == 'Y' || list[i] == 'y')
						use_nav = MB_YES;
					if (list[i] == '.')
						raw_next_value = MB_YES;
					if (list[i] == '=')
						use_swathbounds = MB_YES;
					if (list[i] == '+')
						use_swathbounds = MB_YES;
				}
				else {
					if (list[i] == 'T' || list[i] == 't' || list[i] == 'U' || list[i] == 'l')
						use_gains = MB_YES;
					else if (list[i] == 'F' || list[i] == 'f')
						; // ignore
					else {
						use_raw = MB_YES;
						if (list[i] == 'R' || list[i] == 'd')
							use_bath = MB_YES;
						if (list[i] == 'B' || list[i] == 'b' || list[i] == 'c')
							use_amp = MB_YES;
					}
					if (list[i] != '/' && list[i] != '-' && list[i] != '.')
						raw_next_value = MB_NO;
				}
			}
		if (check_values == MBLIST_CHECK_ON || check_values == MBLIST_CHECK_ON_NULL) {
			if (use_bath == MB_YES)
				check_bath = MB_YES;
			if (use_amp == MB_YES)
				check_amp = MB_YES;
			if (use_ss == MB_YES)
				check_ss = MB_YES;
		}

		/* allocate memory for data arrays */
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depths, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depthacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(double), (void **)&slopes, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(double), (void **)&slopeacrosstrack,
			                           &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(int), (void **)&detect, &error);
		if (use_raw == MB_YES) {
			if (error == MB_ERROR_NO_ERROR)
				status =
				    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&beam_samples, &error);
			if (error == MB_ERROR_NO_ERROR)
				status =
				    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&start_sample, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&range, &error);
			if (error == MB_ERROR_NO_ERROR)
				status =
				    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depression, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bs, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, (MBSYS_SIMRAD2_MAXRAWPIXELS) * sizeof(double), (void **)&ss_pixels,
			                    &error);
		}

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* output separator for GMT style segment file output */
		if (segment == MB_YES && ascii == MB_YES && netcdf == MB_NO) {
			if (segment_mode == MBLIST_SEGMENT_MODE_TAG)
				fprintf(output[0], "%s\n", segment_tag);
			else if (segment_mode == MBLIST_SEGMENT_MODE_SWATHFILE)
				fprintf(output[0], "# %s\n", file);
			else if (segment_mode == MBLIST_SEGMENT_MODE_DATALIST)
				fprintf(output[0], "# %s\n", dfile);
		}

		/* read and print data */
		nread = 0;
		first = MB_YES;
		while (error <= MB_ERROR_NO_ERROR) {
			/* reset error */
			error = MB_ERROR_NO_ERROR;

			/* read a ping of data */
			if (pings == 1 || use_attitude == MB_YES || use_detects == MB_YES || use_pingnumber == MB_YES) {
				/* read next data record */
				status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
				                    &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
				                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

				/* time gaps are not a problem here */
				if (error == MB_ERROR_TIME_GAP) {
					error = MB_ERROR_NO_ERROR;
					status = MB_SUCCESS;
				}

				/* if survey data extract nav */
				if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
					status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
					                        &heading, &draft, &roll, &pitch, &heave, &error);

				/* if survey data extract detects */
				if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_detects) {
          nbeams = beams_bath;
					status = mb_detects(verbose, mbio_ptr, store_ptr, &kind, &nbeams, detect, &error);
        }

				/* if survey data extract pingnumber */
				if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_pingnumber)
					status = mb_pingnumber(verbose, mbio_ptr, &pingnumber, &error);
			}
			else {
				status = mb_get(verbose, mbio_ptr, &kind, &pings_read, time_i, &time_d, &navlon, &navlat, &speed, &heading,
				                &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
				                bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

				/* time gaps are not a problem here */
				if (error == MB_ERROR_TIME_GAP) {
					error = MB_ERROR_NO_ERROR;
					status = MB_SUCCESS;
				}
			}

			/* make sure non survey data records are ignored */
			if (error == MB_ERROR_NO_ERROR && kind != MB_DATA_DATA)
				error = MB_ERROR_OTHER;

			/* increment counter and set cumulative distance */
			if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
				nread++;
				if (use_pingnumber == MB_NO)
					pingnumber = nread;
				distance_total += distance;
			}

			/* get projected navigation if needed */
			if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_projection == MB_YES) {
				/* set up projection if this is the first data */
				if (pjptr == NULL) {
					/* Default projection is UTM */
					if (strlen(projection_pars) == 0)
						strcpy(projection_pars, "U");

					/* check for UTM with undefined zone */
					if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 ||
					    strcmp(projection_pars, "utm") == 0 || strcmp(projection_pars, "u") == 0) {
						reference_lon = navlon;
						if (reference_lon < 180.0)
							reference_lon += 360.0;
						if (reference_lon >= 180.0)
							reference_lon -= 360.0;
						utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
						reference_lat = navlat;
						if (reference_lat >= 0.0)
							sprintf(projection_id, "UTM%2.2dN", utm_zone);
						else
							sprintf(projection_id, "UTM%2.2dS", utm_zone);
					}
					else
						strcpy(projection_id, projection_pars);

					/* set projection flag */
					proj_status = mb_proj_init(verbose, projection_id, &(pjptr), &error);

					/* if projection not successfully initialized then quit */
					if (proj_status != MB_SUCCESS) {
						fprintf(stderr, "\nOutput projection %s not found in database\n", projection_id);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						error = MB_ERROR_BAD_PARAMETER;
						mb_memory_clear(verbose, &error);
						exit(error);
					}
				}

				/* get projected navigation */
				mb_proj_forward(verbose, pjptr, navlon, navlat, &naveasting, &navnorthing, &error);
			}

			if (verbose >= 2) {
				fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
				fprintf(stderr, "dbg2       kind:           %d\n", kind);
				fprintf(stderr, "dbg2       error:          %d\n", error);
				fprintf(stderr, "dbg2       status:         %d\n", status);
			}

			if (verbose >= 1 && kind == MB_DATA_COMMENT) {
				if (icomment == 0) {
					fprintf(stderr, "\nComments:\n");
					icomment++;
				}
				fprintf(stderr, "%s\n", comment);
			}

			/* set output beams and pixels */
			if (error == MB_ERROR_NO_ERROR) {
				/* find vertical-most non-null beam
				    and port and starboard-most good beams */
				status = mb_swathbounds(verbose, MB_YES, navlon, navlat, heading, beams_bath, pixels_ss, beamflag, bath,
				                        bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, &beam_port,
				                        &beam_vertical, &beam_stbd, &pixel_port, &pixel_vertical, &pixel_stbd, &error);

				/* set and/or check beams and pixels to be output */
				status = set_output(verbose, beams_bath, beams_amp, pixels_ss, use_bath, use_amp, use_ss, dump_mode, beam_set,
				                    pixel_set, beam_vertical, pixel_vertical, &beam_start, &beam_end, &beam_exclude_percent,
				                    &pixel_start, &pixel_end, &n_list, list, &error);

				if (status == MB_FAILURE) {
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
				}

				if (verbose >= 2) {
					fprintf(stderr, "\ndbg2  Beams set for output in <%s>\n", program_name);
					fprintf(stderr, "dbg2       status:       %d\n", status);
					fprintf(stderr, "dbg2       error:        %d\n", error);
					fprintf(stderr, "dbg2       use_bath:     %d\n", use_bath);
					fprintf(stderr, "dbg2       use_amp:      %d\n", use_amp);
					fprintf(stderr, "dbg2       use_ss:       %d\n", use_ss);
					fprintf(stderr, "dbg2       beam_start:   %d\n", beam_start);
					fprintf(stderr, "dbg2       beam_end:     %d\n", beam_end);
					fprintf(stderr, "dbg2       beam_exclude_percent: %d\n", beam_exclude_percent);
					fprintf(stderr, "dbg2       pixel_start:  %d\n", pixel_start);
					fprintf(stderr, "dbg2       pixel_end:    %d\n", pixel_end);
					fprintf(stderr, "dbg2       check_values: %d\n", check_values);
					fprintf(stderr, "dbg2       check_bath:   %d\n", check_bath);
					fprintf(stderr, "dbg2       check_amp:    %d\n", check_amp);
					fprintf(stderr, "dbg2       check_ss:     %d\n", check_ss);
					fprintf(stderr, "dbg2       n_list:       %d\n", n_list);
					for (i = 0; i < n_list; i++)
						fprintf(stderr, "dbg2       list[%d]:      %c\n", i, list[i]);
				}
			}

			/* get factors for lon lat calculations */
			if (error == MB_ERROR_NO_ERROR) {
				mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
				headingx = sin(DTR * heading);
				headingy = cos(DTR * heading);
			}

			/* get time interval since last ping */
			if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && first == MB_YES) {
				time_interval = 0.0;
			}
			else if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
				time_interval = time_d - time_d_old;
			}

			/* calculate course made good */
			if (error == MB_ERROR_NO_ERROR && use_course == MB_YES) {
				if (first == MB_YES) {
					course = heading;
					speed_made_good = speed;
					course_old = heading;
					speed_made_good_old = speed;
				}
				else {
					dx = (navlon - navlon_old) / mtodeglon;
					dy = (navlat - navlat_old) / mtodeglat;
					dist = sqrt(dx * dx + dy * dy);
					if (dist > 0.0)
						course = RTD * atan2(dx / dist, dy / dist);
					else
						course = course_old;
					if (course < 0.0)
						course = course + 360.0;
					dt = (time_d - time_d_old);
					if (dt > 0.0)
						speed_made_good = 3.6 * dist / dt;
					else
						speed_made_good = speed_made_good_old;
				}
			}

			/* calculate slopes if required */
			if (error == MB_ERROR_NO_ERROR && use_slope == MB_YES) {
				/* get average slope */
				ns = 0;
				sx = 0.0;
				sy = 0.0;
				sxx = 0.0;
				sxy = 0.0;
				for (k = 0; k < beams_bath; k++)
					if (mb_beam_ok(beamflag[k])) {
						sx += bathacrosstrack[k];
						sy += bath[k];
						sxx += bathacrosstrack[k] * bathacrosstrack[k];
						sxy += bathacrosstrack[k] * bath[k];
						ns++;
					}
				if (ns > 0) {
					delta = ns * sxx - sx * sx;
					/* a = (sxx*sy - sx*sxy)/delta; */
					b = (ns * sxy - sx * sy) / delta;
					avgslope = RTD * atan(b);
				}
				else
					avgslope = 0.0;

				/* get per beam slope */
				set_bathyslope(verbose, beams_bath, beamflag, bath, bathacrosstrack, &ndepths, depths, depthacrosstrack, &nslopes,
				               slopes, slopeacrosstrack, &error);
			}

			/* reset old values */
			if (error == MB_ERROR_NO_ERROR) {
				navlon_old = navlon;
				navlat_old = navlat;
				course_old = course;
				speed_made_good_old = speed_made_good;
				time_d_old = time_d;
			}

			/* get raw values if required */
			if (error == MB_ERROR_NO_ERROR && use_raw == MB_YES) {
				status = mb_get_raw(verbose, mbio_ptr, &mode, &ipulse_length, &png_count, &sample_rate, &absorption, &max_range,
				                    &r_zero, &r_zero_corr, &tvg_start, &tvg_stop, &bsn, &bso, &tx, &tvg_crossover, &nbeams_ss,
				                    &npixels, beam_samples, start_sample, range, depression, bs, ss_pixels, &error);
			}

			/* get gains values if required */
			if (error == MB_ERROR_NO_ERROR && use_gains == MB_YES) {
				status = mb_gains(verbose, mbio_ptr, store_ptr, &kind, &transmit_gain, &pulse_length, &receive_gain, &error);
			}

			/* now loop over beams */
			if (error == MB_ERROR_NO_ERROR && (nread - 1) % decimate == 0)
				for (j = beam_start; j <= beam_end; j++) {
					/* check beam status */
					beam_status = MB_SUCCESS;
					if (check_bath == MB_YES && check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[j]))
						beam_status = MB_FAILURE;
					else if (check_bath == MB_YES && check_values == MBLIST_CHECK_ON_NULL && beamflag[j] == MB_FLAG_NULL)
						beam_status = MB_FAILURE;
					if (check_amp == MB_YES && check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[j]))
						beam_status = MB_FAILURE;
					else if (check_amp == MB_YES && check_values == MBLIST_CHECK_ON_NULL && beamflag[j] == MB_FLAG_NULL)
						beam_status = MB_FAILURE;
					if (check_ss == MB_YES && j != beam_vertical)
						beam_status = MB_FAILURE;
					else if (check_ss == MB_YES && j == beam_vertical)
						if (ss[pixel_vertical] <= MB_SIDESCAN_NULL)
							beam_status = MB_FAILURE;
					if (use_time_interval == MB_YES && first == MB_YES)
						beam_status = MB_FAILURE;
					if (check_nav == MB_YES && (navlon == 0.0 || navlon == 0.0))
						beam_status = MB_FAILURE;

					/* print out good beams */
					if (beam_status == MB_SUCCESS) {
						signflip_next_value = MB_NO;
						invert_next_value = MB_NO;
						raw_next_value = MB_NO;
						sensornav_next_value = MB_NO;
						sensorrelative_next_value = MB_NO;
						projectednav_next_value = MB_NO;
						special_character = MB_NO;
						for (i = 0; i < n_list; i++) {
							if (netcdf == MB_YES && lcount > 0)
								fprintf(output[i], ", ");
							if (port_next_value == MB_YES) {
								k = beam_port;
								port_next_value = MB_NO;
							}
							else if (stbd_next_value == MB_YES) {
								k = beam_stbd;
								stbd_next_value = MB_NO;
							}
							else
								k = j;

							if (raw_next_value == MB_NO) {
								switch (list[i]) {
								case '/': /* Inverts next simple value */
									invert_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '-': /* Flip sign on next simple value */
									signflip_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
									sensornav_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
									sensorrelative_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '^': /* Print position values in projected coordinates
								           * - easting northing rather than lon lat
								           * - applies to XY */
									projectednav_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '.': /* Raw value next field */
									raw_next_value = MB_YES;
									special_character = MB_YES;
									count = 0;
									break;
								case '=': /* Port-most value next field -ignored here */
									port_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '+': /* Starboard-most value next field - ignored here*/
									stbd_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case 'A': /* Average seafloor crosstrack slope */
									printsimplevalue(verbose, output[i], avgslope, 0, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'a': /* Per-beam seafloor crosstrack slope */
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
										                        slopeacrosstrack, bathacrosstrack[k], &depth, &slope, &error);
										printsimplevalue(verbose, output[i], slope, 0, 4, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									break;
								case 'B': /* amplitude */
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										printsimplevalue(verbose, output[i], amp[k], 0, 3, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									break;
								case 'b': /* sidescan */
									printsimplevalue(verbose, output[i], ss[pixel_vertical], 0, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'C': /* Sonar altitude (m) */
									printsimplevalue(verbose, output[i], altitude, 0, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'c': /* Sonar transducer depth (m) */
									printsimplevalue(verbose, output[i], sonardepth, 0, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'D': /* acrosstrack dist. */
								case 'd':
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										b = bathy_scale * bathacrosstrack[k];
										printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									break;
								case 'E': /* alongtrack dist. */
								case 'e':
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										b = bathy_scale * bathalongtrack[k];
										printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									break;
								case 'F': /* Beamflag numeric value */
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "%u", beamflag[k]);
										else
											fprintf(output[i], "%u", beamflag[k]);
									}
									else {
										b = beamflag[k];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'f': /* Beamflag character value (ascii only) */
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "%u", beamflag[k]);
										else {
											if (mb_beam_check_flag_unusable(beamflag[k]))
												fprintf(output[i], "-");
											else if (mb_beam_ok(beamflag[k]))
												fprintf(output[i], "G");
											else if (mb_beam_check_flag_manual(beamflag[k]))
												fprintf(output[i], "M");
											else if (mb_beam_check_flag_filter(beamflag[k]))
												fprintf(output[i], "F");
											else if (mb_beam_check_flag_filter2(beamflag[k]))
												fprintf(output[i], "F");
											else if (mb_beam_check_flag_secondary(beamflag[k]))
												fprintf(output[i], "N");
											else if (mb_beam_check_flag_interpolate(beamflag[k]))
												fprintf(output[i], "I");
											else if (mb_beam_check_flag_sonar(beamflag[k]))
												fprintf(output[i], "S");
										}
									}
									else {
										b = beamflag[k];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'G': /* flat bottom grazing angle */
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										angle = RTD * (atan(bathacrosstrack[k] / (bath[k] - sonardepth)));
										printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									break;
								case 'g': /* grazing angle using slope */
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
										                        slopeacrosstrack, bathacrosstrack[k], &depth, &slope, &error);
										angle = RTD * (atan(bathacrosstrack[k] / (bath[k] - sonardepth))) + slope;
										printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									break;
								case 'H': /* heading */
									printsimplevalue(verbose, output[i], heading, 7, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'h': /* course */
									printsimplevalue(verbose, output[i], course, 7, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'J': /* time string */
									mb_get_jtime(verbose, time_i, time_j);
									seconds = time_i[5] + 1e-6 * time_i[6];
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "%d, %d, %d, %d, %d, %d", time_j[0], time_j[1], time_i[3],
											        time_i[4], time_i[5], time_i[6]);
										else
											fprintf(output[i], "%.4d %.3d %.2d %.2d %9.6f", time_j[0], time_j[1], time_i[3],
											        time_i[4], seconds);
									}
									else {
										b = time_j[0];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[1];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[3];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[4];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[5];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[6];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'j': /* time string */
									mb_get_jtime(verbose, time_i, time_j);
									seconds = time_i[5] + 1e-6 * time_i[6];
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "%d, %d, %d, %d, %d", time_j[0], time_j[1], time_j[2], time_j[3],
											        time_j[4]);
										else
											fprintf(output[i], "%.4d %.3d %.4d %9.6f", time_j[0], time_j[1], time_j[2], seconds);
									}
									else {
										b = time_j[0];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[1];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[2];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[3];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[4];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'L': /* along-track distance (km) */
									printsimplevalue(verbose, output[i], distance_total, 8, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'l': /* along-track distance (m) */
									printsimplevalue(verbose, output[i], 1000.0 * distance_total, 8, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'M': /* Decimal unix seconds since
								        1/1/70 00:00:00 */
									printsimplevalue(verbose, output[i], time_d, 0, 6, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'm': /* time in decimal seconds since
								        first record */
									if (first_m == MB_YES) {
										time_d_ref = time_d;
										first_m = MB_NO;
									}
									b = time_d - time_d_ref;
									printsimplevalue(verbose, output[i], b, 0, 6, ascii, &invert_next_value, &signflip_next_value,
									                 &error);
									break;
								case 'N': /* ping counter */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", pingnumber);
									else {
										b = pingnumber;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'P': /* pitch */
									printsimplevalue(verbose, output[i], pitch, 6, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'p': /* draft */
									printsimplevalue(verbose, output[i], draft, 7, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'q': /* bottom detection type */
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");

										fprintf(output[i], "%d", detect[k]);
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
									}
									else {
										b = detect[k];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'Q': /* bottom detection type */
									if (ascii == MB_YES) {
										if (netcdf == MB_YES) {
											fprintf(output[i], "\"");
											fprintf(output[i], "%d", detect[k]);
											fprintf(output[i], "\"");
										}
										else {
											if (detect[k] == MB_DETECT_AMPLITUDE)
												fprintf(output[i], "A");
											else if (detect[k] == MB_DETECT_PHASE)
												fprintf(output[i], "P");
											else
												fprintf(output[i], "U");
										}
									}
									else {
										b = detect[k];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'R': /* roll */
									printsimplevalue(verbose, output[i], roll, 6, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'r': /* heave */
									printsimplevalue(verbose, output[i], heave, 7, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'S': /* speed */
									printsimplevalue(verbose, output[i], speed, 6, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 's': /* speed made good */
									printsimplevalue(verbose, output[i], speed_made_good, 6, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
									seconds = time_i[5] + 1e-6 * time_i[6];
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");

										fprintf(output[i], "%.4d/%.2d/%.2d/%.2d/%.2d/%09.6f", time_i[0], time_i[1], time_i[2],
										        time_i[3], time_i[4], seconds);
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
									}
									else {
										b = time_i[0];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[1];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[2];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[3];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[4];
										fwrite(&b, sizeof(double), 1, outfile);
										b = seconds;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 't': /* yyyy mm dd hh mm ss time string */
									seconds = time_i[5] + 1e-6 * time_i[6];
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "%d, %d, %d, %d, %d, %d, %d", time_i[0], time_i[1], time_i[2],
											        time_i[3], time_i[4], time_i[5], time_i[6]);
										else
											fprintf(output[i], "%.4d %.2d %.2d %.2d %.2d %09.6f", time_i[0], time_i[1], time_i[2],
											        time_i[3], time_i[4], seconds);
									}
									else {
										b = time_i[0];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[1];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[2];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[3];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[4];
										fwrite(&b, sizeof(double), 1, outfile);
										b = seconds;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
									time_u = (int)time_d;
									if (ascii == MB_YES)
										fprintf(output[i], "%ld", time_u);
									else {
										b = time_u;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'u': /* time in seconds since first record */
									time_u = (int)time_d;
									if (first_u == MB_YES) {
										time_u_ref = time_u;
										first_u = MB_NO;
									}
									if (ascii == MB_YES)
										fprintf(output[i], "%ld", time_u - time_u_ref);
									else {
										b = time_u - time_u_ref;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'V': /* time in seconds since last ping */
								case 'v':
									if (ascii == MB_YES) {
										if (fabs(time_interval) > 100.)
											fprintf(output[i], "%g", time_interval);
										else
											fprintf(output[i], "%10.6f", time_interval);
									}
									else {
										fwrite(&time_interval, sizeof(double), 1, outfile);
									}
									break;
								case 'X': /* longitude decimal degrees */
									if (projectednav_next_value == MB_NO) {
										if (sensorrelative_next_value == MB_YES)
											dlon = 0.0;
										else
											dlon = navlon;
										if (sensornav_next_value == MB_NO && (beam_set != MBLIST_SET_OFF || k != j))
											dlon += headingy * mtodeglon * bathacrosstrack[k] +
											        headingx * mtodeglon * bathalongtrack[k];
										printsimplevalue(verbose, output[i], dlon, 15, 10, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									else {
										if (sensorrelative_next_value == MB_YES)
											deasting = 0.0;
										else
											deasting = naveasting;
										if (sensornav_next_value == MB_NO && (beam_set != MBLIST_SET_OFF || k != j))
											deasting += headingy * bathacrosstrack[k] + headingx * bathalongtrack[k];
										printsimplevalue(verbose, output[i], deasting, 15, 3, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									sensornav_next_value = MB_NO;
									sensorrelative_next_value = MB_NO;
									projectednav_next_value = MB_NO;
									break;
								case 'x': /* longitude degress + decimal minutes */
									dlon = navlon;
									if (sensornav_next_value == MB_NO && (beam_set != MBLIST_SET_OFF || k != j))
										dlon +=
										    headingy * mtodeglon * bathacrosstrack[k] + headingx * mtodeglon * bathalongtrack[k];
									if (dlon < 0.0) {
										hemi = 'W';
										dlon = -dlon;
									}
									else
										hemi = 'E';
									degrees = (int)dlon;
									minutes = 60.0 * (dlon - degrees);
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
										fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
									}
									else {
										b = degrees;
										if (hemi == 'W')
											b = -b;
										fwrite(&b, sizeof(double), 1, outfile);
										b = minutes;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									sensornav_next_value = MB_NO;
									break;
								case 'Y': /* latitude decimal degrees */
									if (projectednav_next_value == MB_NO) {
										if (sensorrelative_next_value == MB_YES)
											dlat = 0.0;
										else
											dlat = navlat;
										if (sensornav_next_value == MB_NO && (beam_set != MBLIST_SET_OFF || k != j))
											dlat += -headingx * mtodeglat * bathacrosstrack[k] +
											        headingy * mtodeglat * bathalongtrack[k];
										printsimplevalue(verbose, output[i], dlat, 15, 10, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
										sensornav_next_value = MB_NO;
									}
									else {
										if (sensorrelative_next_value == MB_YES)
											dnorthing = 0.0;
										else
											dnorthing = navnorthing;
										if (sensornav_next_value == MB_NO && (beam_set != MBLIST_SET_OFF || k != j))
											dnorthing += -headingx * bathacrosstrack[k] + headingy * bathalongtrack[k];
										printsimplevalue(verbose, output[i], dnorthing, 15, 3, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									sensornav_next_value = MB_NO;
									sensorrelative_next_value = MB_NO;
									projectednav_next_value = MB_NO;
									break;
								case 'y': /* latitude degrees + decimal minutes */
									dlat = navlat;
									if (sensornav_next_value == MB_NO && (beam_set != MBLIST_SET_OFF || k != j))
										dlat +=
										    -headingx * mtodeglat * bathacrosstrack[k] + headingy * mtodeglat * bathalongtrack[k];
									if (dlat < 0.0) {
										hemi = 'S';
										dlat = -dlat;
									}
									else
										hemi = 'N';
									degrees = (int)dlat;
									minutes = 60.0 * (dlat - degrees);
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
										fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
									}
									else {
										b = degrees;
										if (hemi == 'S')
											b = -b;
										fwrite(&b, sizeof(double), 1, outfile);
										b = minutes;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									sensornav_next_value = MB_NO;
									break;
								case 'Z': /* topography */
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										b = -bathy_scale * bath[k];
										if (sensorrelative_next_value == MB_YES)
											b -= -bathy_scale * sonardepth;
										printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									sensornav_next_value = MB_NO;
									sensorrelative_next_value = MB_NO;
									break;
								case 'z': /* depth */
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										b = bathy_scale * bath[k];
										if (sensorrelative_next_value == MB_YES)
											b -= bathy_scale * sonardepth;
										printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									sensornav_next_value = MB_NO;
									sensorrelative_next_value = MB_NO;
									break;
								case '#': /* beam number */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", k);
									else {
										b = k;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								default:
									if (ascii == MB_YES)
										fprintf(output[i], "<Invalid Option: %c>", list[i]);
									break;
								}
							}
							else /* raw_next_value */
							{
								switch (list[i]) {
								case '/': /* Inverts next simple value */
									invert_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '-': /* Flip sign on next simple value */
									signflip_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
									sensornav_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
									sensorrelative_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '^': /* Print position values in projected coordinates
								           * - easting northing rather than lon lat
								           * - applies to XY */
									projectednav_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '.': /* Raw value next field */
									raw_next_value = MB_YES;
									special_character = MB_YES;
									count = 0;
									break;
								case '=': /* Port-most value next field -ignored here */
									port_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '+': /* Starboard-most value next field - ignored here*/
									stbd_next_value = MB_YES;
									special_character = MB_YES;
									break;

								case '0':
								case '1':
								case '2':
								case '3':
								case '4':
								case '5':
								case '6':
								case '7':
								case '8':
								case '9':
									count = count * 10 + list[i] - '0';
									break;

								case 'A': /* backscatter */
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										printsimplevalue(verbose, output[i], bs[k], 5, 1, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									raw_next_value = MB_NO;
									break;
								case 'a': /* absorption */
									printsimplevalue(verbose, output[i], absorption, 5, 2, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'B': /* BSN - Normal incidence backscatter */
									printsimplevalue(verbose, output[i], bsn, 5, 2, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'b': /* BSO - Oblique backscatter */
									printsimplevalue(verbose, output[i], bso, 5, 2, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'c': /* Mean backscatter */
									mback = 0;
									nback = 0;
									for (m = 0; m < beams_amp; m++) {
										if (mb_beam_ok(beamflag[m])) {
											mback += amp[m];
											nback++;
										}
									}
									printsimplevalue(verbose, output[i], mback / nback, 5, 2, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'd': /* beam depression angle */
									if (beamflag[k] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										printsimplevalue(verbose, output[i], depression[k], 5, 2, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									raw_next_value = MB_NO;
									break;

								case 'F': /* filename */
									if (netcdf == MB_YES)
										fprintf(output[i], "\"");
									fprintf(output[i], "%s", file);

									if (netcdf == MB_YES)
										fprintf(output[i], "\"");

									if (ascii == MB_NO)
										for (k = strlen(file); k < MB_PATH_MAXLINE; k++)
											fwrite(&file[strlen(file)], sizeof(char), 1, outfile);

									raw_next_value = MB_NO;
									break;

								case 'f': /* format */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", format);
									else {
										b = format;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;

									break;
								case 'G': /* TVG start */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", tvg_start);
									else {
										b = tvg_start;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'g': /* TVG stop */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", tvg_stop);
									else {
										b = tvg_stop;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'L': /* Pulse length */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", ipulse_length);
									else {
										b = ipulse_length;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'l': /* Transmit pulse length (sec) */
									printsimplevalue(verbose, output[i], pulse_length, 9, 6, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'M': /* mode */
									if (ascii == MB_YES)
										fprintf(output[i], "%4d", mode);
									else {
										b = mode;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'N': /* ping counter */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", png_count);
									else {
										b = png_count;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'p': /* sidescan */
									invert = invert_next_value;
									flip = signflip_next_value;
									printsimplevalue(verbose, output[i], ss_pixels[start_sample[k]], 5, 1, ascii,
									                 &invert_next_value, &signflip_next_value, &error);
									if (count > 0) {
										for (m = 1; m < count && m < beam_samples[k]; m++) {
											if (netcdf == MB_YES)
												fprintf(output[i], ", ");
											if (ascii == MB_YES)
												fprintf(output[i], "%s", delimiter);
											invert_next_value = invert;
											signflip_next_value = flip;

											printsimplevalue(verbose, output[i], ss_pixels[start_sample[k] + m], 5, 1, ascii,
											                 &invert_next_value, &signflip_next_value, &error);
										}
										for (; m < count; m++) {
											if (netcdf == MB_YES)
												fprintf(output[i], ", ");
											if (ascii == MB_YES)
												fprintf(output[i], "%s", delimiter);
											printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
										}
									}

									raw_next_value = MB_NO;
									break;
								case 'R': /* range */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", range[k]);
									else {
										b = range[k];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'r': /* Sample rate */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", sample_rate);
									else {
										b = sample_rate;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'S': /* Sidescan pixels */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", npixels);
									else {
										b = npixels;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 's': /* Sidescan pixels per beam */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", beam_samples[k]);
									else {
										b = beam_samples[k];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'T': /* Transmit gain (dB) */
									printsimplevalue(verbose, output[i], transmit_gain, 5, 1, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 't': /* Receive gain (dB) */
									printsimplevalue(verbose, output[i], receive_gain, 5, 1, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;

								default:
									if (ascii == MB_YES)
										fprintf(output[i], "<Invalid Option: %c>", list[i]);
									raw_next_value = MB_NO;
									break;
								}
							}
							if (ascii == MB_YES) {
								if (i < (n_list - 1)) {
									if (special_character == MB_NO) {
										fprintf(output[i], "%s", delimiter);
									}
									else {
										special_character = MB_NO;
									}
								}
								else
									fprintf(output[lcount++ % n_list], "\n");
							}
						}
					}
				}

			/* now loop over pixels */
			if (error == MB_ERROR_NO_ERROR && (nread - 1) % decimate == 0)
				for (j = pixel_start; j <= pixel_end; j++) {
					/* check pixel status */
					pixel_status = MB_SUCCESS;
					if (check_bath == MB_YES && j != pixel_vertical)
						pixel_status = MB_FAILURE;
					else if (check_bath == MB_YES && j == pixel_vertical) {
						if (check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[beam_vertical]))
							pixel_status = MB_FAILURE;
						else if (check_values == MBLIST_CHECK_ON_NULL && beamflag[beam_vertical] == MB_FLAG_NULL)
							pixel_status = MB_FAILURE;
					}
					if (check_amp == MB_YES && j != pixel_vertical)
						pixel_status = MB_FAILURE;
					else if (check_amp == MB_YES && j == pixel_vertical) {
						if (check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[beam_vertical]))
							pixel_status = MB_FAILURE;
						else if (check_values == MBLIST_CHECK_ON_NULL && beamflag[beam_vertical] == MB_FLAG_NULL)
							pixel_status = MB_FAILURE;
					}
					if (check_ss == MB_YES && ss[j] <= MB_SIDESCAN_NULL)
						pixel_status = MB_FAILURE;
					if (use_time_interval == MB_YES && first == MB_YES)
						pixel_status = MB_FAILURE;
					if (check_nav == MB_YES && (navlon == 0.0 || navlon == 0.0))
						pixel_status = MB_FAILURE;

					/* print out good pixels */
					if (pixel_status == MB_SUCCESS) {
						signflip_next_value = MB_NO;
						invert_next_value = MB_NO;
						raw_next_value = MB_NO;
						sensornav_next_value = MB_NO;
						projectednav_next_value = MB_NO;
						special_character = MB_NO;
						for (i = 0; i < n_list; i++) {
							if (netcdf == MB_YES && lcount > 0)
								fprintf(output[i], ", ");
							if (port_next_value == MB_YES) {
								k = pixel_port;
								port_next_value = MB_NO;
							}
							else if (stbd_next_value == MB_YES) {
								k = pixel_stbd;
								stbd_next_value = MB_NO;
							}
							else
								k = j;

							if (raw_next_value == MB_NO) {
								switch (list[i]) {
								case '/': /* Inverts next simple value */
									invert_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '-': /* Flip sign on next simple value */
									signflip_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
									sensornav_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
									sensorrelative_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '^': /* Print position values in projected coordinates
								           * - easting northing rather than lon lat
								           * - applies to XY */
									projectednav_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '.': /* Raw value next field */
									raw_next_value = MB_YES;
									count = 0;
									special_character = MB_YES;
									break;
								case '=': /* Port-most value next field -ignored here */
									port_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '+': /* Starboard-most value next field - ignored here*/
									stbd_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case 'A': /* Average seafloor crosstrack slope */
									printsimplevalue(verbose, output[i], avgslope, 0, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'a': /* Per-pixel seafloor crosstrack slope */
									status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
									                        slopeacrosstrack, ssacrosstrack[k], &depth, &slope, &error);
									printsimplevalue(verbose, output[i], slope, 0, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'B': /* amplitude */
									printsimplevalue(verbose, output[i], amp[beam_vertical], 0, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'b': /* sidescan */
									printsimplevalue(verbose, output[i], ss[k], 0, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'C': /* Sonar altitude (m) */
									printsimplevalue(verbose, output[i], altitude, 0, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'c': /* Sonar transducer depth (m) */
									printsimplevalue(verbose, output[i], sonardepth, 0, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'D': /* acrosstrack dist. */
								case 'd':
									b = bathy_scale * ssacrosstrack[k];
									printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value,
									                 &error);
									break;
								case 'E': /* alongtrack dist. */
								case 'e':
									b = bathy_scale * ssalongtrack[k];
									printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value,
									                 &error);
									break;
								case 'G': /* flat bottom grazing angle */
									status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
									                        slopeacrosstrack, ssacrosstrack[k], &depth, &slope, &error);
									angle = RTD * (atan(ssacrosstrack[k] / (depth - sonardepth)));
									printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'g': /* grazing angle using slope */
									status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
									                        slopeacrosstrack, ssacrosstrack[k], &depth, &slope, &error);
									angle = RTD * (atan(bathacrosstrack[k] / (depth - sonardepth))) + slope;
									printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'H': /* heading */
									printsimplevalue(verbose, output[i], heading, 7, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'h': /* course */
									printsimplevalue(verbose, output[i], course, 7, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'J': /* time string */
									mb_get_jtime(verbose, time_i, time_j);
									seconds = time_i[5] + 1e-6 * time_i[6];
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "%d, %d, %d, %d, %d, %d", time_j[0], time_j[1], time_i[3],
											        time_i[4], time_i[5], time_i[6]);
										else
											fprintf(output[i], "%.4d %.3d %.2d %.2d %9.6f", time_j[0], time_j[1], time_i[3],
											        time_i[4], seconds);
									}
									else {
										b = time_j[0];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[1];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[3];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[4];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[5];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[6];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'j': /* time string */
									mb_get_jtime(verbose, time_i, time_j);
									seconds = time_i[5] + 1e-6 * time_i[6];
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "%d, %d, %d, %d, %d", time_j[0], time_j[1], time_j[2], time_j[3],
											        time_j[4]);
										else
											fprintf(output[i], "%.4d %.3d %.4d %9.6f", time_j[0], time_j[1], time_j[2], seconds);
									}
									else {
										b = time_j[0];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[1];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[2];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[3];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_j[4];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'L': /* along-track distance (km) */
									printsimplevalue(verbose, output[i], distance_total, 8, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'l': /* along-track distance (m) */
									printsimplevalue(verbose, output[i], 1000.0 * distance_total, 8, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'M': /* Decimal unix seconds since
								        1/1/70 00:00:00 */
									printsimplevalue(verbose, output[i], time_d, 0, 6, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'm': /* time in decimal seconds since
								        first record */
									if (first_m == MB_YES) {
										time_d_ref = time_d;
										first_m = MB_NO;
									}
									b = time_d - time_d_ref;
									printsimplevalue(verbose, output[i], b, 0, 6, ascii, &invert_next_value, &signflip_next_value,
									                 &error);
									break;
								case 'N': /* ping counter */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", pingnumber);
									else {
										b = pingnumber;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'P': /* pitch */
									printsimplevalue(verbose, output[i], pitch, 6, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'p': /* draft */
									printsimplevalue(verbose, output[i], draft, 7, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'Q': /* bottom detection type */
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");

										fprintf(output[i], "%d", MB_DETECT_UNKNOWN);
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
									}
									else {
										b = MB_DETECT_UNKNOWN;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'R': /* roll */
									printsimplevalue(verbose, output[i], roll, 6, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'r': /* heave */
									printsimplevalue(verbose, output[i], heave, 7, 4, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'S': /* speed */
									printsimplevalue(verbose, output[i], speed, 6, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 's': /* speed made good */
									printsimplevalue(verbose, output[i], speed_made_good, 6, 3, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									break;
								case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
									seconds = time_i[5] + 1e-6 * time_i[6];
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
										fprintf(output[i], "%.4d/%.2d/%.2d/%.2d/%.2d/%09.6f", time_i[0], time_i[1], time_i[2],
										        time_i[3], time_i[4], seconds);
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
									}
									else {
										b = time_i[0];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[1];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[2];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[3];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[4];
										fwrite(&b, sizeof(double), 1, outfile);
										b = seconds;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 't': /* yyyy mm dd hh mm ss time string */
									seconds = time_i[5] + 1e-6 * time_i[6];
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "%d, %d, %d, %d, %d, %d, %d", time_i[0], time_i[1], time_i[2],
											        time_i[3], time_i[4], time_i[5], time_i[6]);
										else
											fprintf(output[i], "%.4d %.2d %.2d %.2d %.2d %09.6f", time_i[0], time_i[1], time_i[2],
											        time_i[3], time_i[4], seconds);
									}
									else {
										b = time_i[0];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[1];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[2];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[3];
										fwrite(&b, sizeof(double), 1, outfile);
										b = time_i[4];
										fwrite(&b, sizeof(double), 1, outfile);
										b = seconds;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
									time_u = (int)time_d;
									if (ascii == MB_YES)
										fprintf(output[i], "%ld", time_u);
									else {
										b = time_u;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'u': /* time in seconds since first record */
									time_u = (int)time_d;
									if (first_u == MB_YES) {
										time_u_ref = time_u;
										first_u = MB_NO;
									}
									if (ascii == MB_YES)
										fprintf(output[i], "%ld", time_u - time_u_ref);
									else {
										b = time_u - time_u_ref;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								case 'V': /* time in seconds since last ping */
								case 'v':
									if (ascii == MB_YES) {
										if (fabs(time_interval) > 100.)
											fprintf(output[i], "%g", time_interval);
										else
											fprintf(output[i], "%10.6f", time_interval);
									}
									else {
										fwrite(&time_interval, sizeof(double), 1, outfile);
									}
									break;
								case 'X': /* longitude decimal degrees */
									if (projectednav_next_value == MB_NO) {
										if (sensorrelative_next_value == MB_YES)
											dlon = 0.0;
										else
											dlon = navlon;
										if (sensornav_next_value == MB_NO && (pixel_set != MBLIST_SET_OFF || k != j))
											dlon +=
											    headingy * mtodeglon * ssacrosstrack[k] + headingx * mtodeglon * ssalongtrack[k];
										printsimplevalue(verbose, output[i], dlon, 15, 10, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									else {
										if (sensorrelative_next_value == MB_YES)
											deasting = 0.0;
										else
											deasting = naveasting;
										if (sensornav_next_value == MB_NO && (pixel_set != MBLIST_SET_OFF || k != j))
											deasting += headingy * ssacrosstrack[k] + headingx * ssalongtrack[k];
										printsimplevalue(verbose, output[i], deasting, 15, 3, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									sensornav_next_value = MB_NO;
									sensorrelative_next_value = MB_NO;
									projectednav_next_value = MB_NO;
									break;
								case 'x': /* longitude degress + decimal minutes */
									dlon = navlon;
									if (sensornav_next_value == MB_NO && (pixel_set != MBLIST_SET_OFF || k != j))
										dlon += headingy * mtodeglon * ssacrosstrack[k] + headingx * mtodeglon * ssalongtrack[k];
									if (dlon < 0.0) {
										hemi = 'W';
										dlon = -dlon;
									}
									else
										hemi = 'E';
									degrees = (int)dlon;
									minutes = 60.0 * (dlon - degrees);
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
										fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
									}
									else {
										b = degrees;
										if (hemi == 'W')
											b = -b;
										fwrite(&b, sizeof(double), 1, outfile);
										b = minutes;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									sensornav_next_value = MB_NO;
									break;
								case 'Y': /* latitude decimal degrees */
									if (projectednav_next_value == MB_NO) {
										if (sensorrelative_next_value == MB_YES)
											dlat = 0.0;
										else
											dlat = navlat;
										if (sensornav_next_value == MB_NO && (pixel_set != MBLIST_SET_OFF || k != j))
											dlat +=
											    -headingx * mtodeglat * ssacrosstrack[k] + headingy * mtodeglat * ssalongtrack[k];
										printsimplevalue(verbose, output[i], dlat, 15, 10, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									else {
										if (sensorrelative_next_value == MB_YES)
											dnorthing = 0.0;
										else
											dnorthing = navnorthing;
										if (sensornav_next_value == MB_NO && (beam_set != MBLIST_SET_OFF || k != j))
											dnorthing += -headingx * ssacrosstrack[k] + headingy * ssalongtrack[k];
										printsimplevalue(verbose, output[i], dnorthing, 15, 3, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									sensornav_next_value = MB_NO;
									sensorrelative_next_value = MB_NO;
									projectednav_next_value = MB_NO;
									break;
								case 'y': /* latitude degrees + decimal minutes */
									dlat = navlat;
									if (sensornav_next_value == MB_NO && (pixel_set != MBLIST_SET_OFF || k != j))
										dlat += -headingx * mtodeglat * ssacrosstrack[k] + headingy * mtodeglat * ssalongtrack[k];
									if (dlat < 0.0) {
										hemi = 'S';
										dlat = -dlat;
									}
									else
										hemi = 'N';
									degrees = (int)dlat;
									minutes = 60.0 * (dlat - degrees);
									if (ascii == MB_YES) {
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
										fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
										if (netcdf == MB_YES)
											fprintf(output[i], "\"");
									}
									else {
										b = degrees;
										if (hemi == 'S')
											b = -b;
										fwrite(&b, sizeof(double), 1, outfile);
										b = minutes;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									sensornav_next_value = MB_NO;
									break;
								case 'Z': /* topography */
									if (beamflag[beam_vertical] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[beam_vertical]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										b = -bathy_scale * bath[beam_vertical];
										if (sensorrelative_next_value == MB_YES)
											b -= -bathy_scale * sonardepth;
										printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									sensornav_next_value = MB_NO;
									sensorrelative_next_value = MB_NO;
									break;
								case 'z': /* depth */
									if (beamflag[beam_vertical] == MB_FLAG_NULL &&
									    (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else if (!mb_beam_ok(beamflag[beam_vertical]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
									else {
										b = bathy_scale * bath[beam_vertical];
										if (sensorrelative_next_value == MB_YES)
											b -= bathy_scale * sonardepth;
										printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
										                 &signflip_next_value, &error);
									}
									sensornav_next_value = MB_NO;
									sensorrelative_next_value = MB_NO;
									break;
								case '#': /* pixel number */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", k);
									else {
										b = k;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									break;
								default:
									fprintf(output[i], "<Invalid Option: %c>", list[i]);
									break;
								}
							}
							else /* raw_next_value */
							{
								switch (list[i]) {
								case '/': /* Inverts next simple value */
									invert_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '-': /* Flip sign on next simple value */
									signflip_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
									sensornav_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
									sensorrelative_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '^': /* Print position values in projected coordinates
								           * - easting northing rather than lon lat
								           * - applies to XY */
									projectednav_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '.': /* Raw value next field */
									raw_next_value = MB_YES;
									count = 0;
									special_character = MB_YES;
									break;
								case '=': /* Port-most value next field -ignored here */
									port_next_value = MB_YES;
									special_character = MB_YES;
									break;
								case '+': /* Starboard-most value next field - ignored here*/
									stbd_next_value = MB_YES;
									special_character = MB_YES;
									break;

								case '0':
								case '1':
								case '2':
								case '3':
								case '4':
								case '5':
								case '6':
								case '7':
								case '8':
								case '9':
									count = count * 10 + list[i] - '0';
									break;

								case 'A': /* backscatter */
									printsimplevalue(verbose, output[i], bs[beam_vertical], 5, 1, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'a': /* absorption */
									printsimplevalue(verbose, output[i], absorption, 5, 2, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'B': /* BSN - Normal incidence backscatter */
									printsimplevalue(verbose, output[i], bsn, 5, 2, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'b': /* BSO - Oblique backscatter */
									printsimplevalue(verbose, output[i], bso, 5, 2, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'c': /* Mean backscatter */
									mback = 0;
									nback = 0;
									for (m = 0; m < beams_amp; m++) {
										if (mb_beam_ok(beamflag[m])) {
											mback += amp[m];
											nback++;
										}
									}
									printsimplevalue(verbose, output[i], mback / nback, 5, 2, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'd': /* beam depression angle */
									printsimplevalue(verbose, output[i], depression[beam_vertical], 5, 2, ascii,
									                 &invert_next_value, &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'F': /* filename */
									if (netcdf == MB_YES)
										fprintf(output[i], "\"");
									fprintf(output[i], "%s", file);
									if (netcdf == MB_YES)
										fprintf(output[i], "\"");

									if (ascii == MB_NO)
										for (k = strlen(file); k < MB_PATH_MAXLINE; k++)
											fwrite(&file[strlen(file)], sizeof(char), 1, outfile);

									raw_next_value = MB_NO;
									break;

								case 'f': /* format */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", format);
									else {
										b = format;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;

								case 'G': /* TVG start */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", tvg_start);
									else {
										b = tvg_start;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'g': /* TVG stop */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", tvg_stop);
									else {
										b = tvg_stop;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'L': /* Pulse length */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", ipulse_length);
									else {
										b = ipulse_length;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'l': /* Transmit pulse length (sec) */
									printsimplevalue(verbose, output[i], pulse_length, 9, 6, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 'M': /* mode */
									if (ascii == MB_YES)
										fprintf(output[i], "%4d", mode);
									else {
										b = mode;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'N': /* ping counter */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", png_count);
									else {
										b = png_count;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'p': /* sidescan */
									invert = invert_next_value;
									flip = signflip_next_value;
									printsimplevalue(verbose, output[i], ss_pixels[start_sample[beam_vertical]], 5, 1, ascii,
									                 &invert_next_value, &signflip_next_value, &error);
									if (count > 0) {
										for (m = 1; m < count && m < beam_samples[beam_vertical]; m++) {
											if (netcdf == MB_YES)
												fprintf(output[i], ", ");
											if (ascii == MB_YES)
												fprintf(output[i], "%s", delimiter);
											invert_next_value = invert;
											signflip_next_value = flip;

											printsimplevalue(verbose, output[i], ss_pixels[start_sample[beam_vertical] + m], 5, 1,
											                 ascii, &invert_next_value, &signflip_next_value, &error);
										}
										for (; m < count; m++) {
											if (netcdf == MB_YES)
												fprintf(output[i], ", ");
											if (ascii == MB_YES)
												fprintf(output[i], "%s", delimiter);
											printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
										}
									}

									raw_next_value = MB_NO;
									break;

								case 'R': /* range */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", range[beam_vertical]);
									else {
										b = range[beam_vertical];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;

								case 'r': /* Sample rate */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", sample_rate);
									else {
										b = sample_rate;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;

								case 'S': /* Sidescan pixels */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", npixels);
									else {
										b = npixels;
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 's': /* Sidescan pixels per beam */
									if (ascii == MB_YES)
										fprintf(output[i], "%6d", beam_samples[beam_vertical]);
									else {
										b = beam_samples[beam_vertical];
										fwrite(&b, sizeof(double), 1, outfile);
									}
									raw_next_value = MB_NO;
									break;
								case 'T': /* Transmit gain (dB) */
									printsimplevalue(verbose, output[i], transmit_gain, 5, 1, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;
								case 't': /* Receive gain (dB) */
									printsimplevalue(verbose, output[i], receive_gain, 5, 1, ascii, &invert_next_value,
									                 &signflip_next_value, &error);
									raw_next_value = MB_NO;
									break;

								default:
									if (ascii == MB_YES)
										fprintf(output[i], "<Invalid Option: %c>", list[i]);
									raw_next_value = MB_NO;
									break;
								}
							}
							if (ascii == MB_YES) {
								if (i < (n_list - 1)) {
									if (special_character == MB_NO) {
										fprintf(output[i], "%s", delimiter);
									}
									else {
										special_character = MB_NO;
									}
								}
								else
									fprintf(output[lcount++ % n_list], "\n");
							}
						}
					}
				}

			/* reset first flag */
			if (error == MB_ERROR_NO_ERROR && first == MB_YES) {
				first = MB_NO;
			}
		}

		/* close the swath file */
		status = mb_close(verbose, &mbio_ptr, &error);

		/* deallocate memory used for data arrays */
		if (use_raw == MB_YES) {
			mb_freed(verbose, __FILE__, __LINE__, (void **)&ss_pixels, &error);
		}

		/* figure out whether and what to read next */
		if (read_datalist == MB_YES) {
			if ((status = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
				read_data = MB_YES;
			else
				read_data = MB_NO;
		}
		else {
			read_data = MB_NO;
		}

		/* end loop over files in list */
	}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose, &datalist, &error);

	/* compile CDL file */
	if (netcdf == MB_YES) {
		for (i = 0; i < n_list; i++) {
			if (list[i] != '/' && list[i] != '-' && list[i] != '.' && !(list[i] >= '0' && list[i] <= '9')) {
				fprintf(output[i], " ;\n\n");
				rewind(output[i]);

				/* copy data to CDL file */
				for (j = fread(buffer, sizeof(char), MB_BUFFER_MAX, output[i]); j > 0;
				     j = fread(buffer, sizeof(char), MB_BUFFER_MAX, output[i])) {
					if (j != fwrite(buffer, sizeof(char), j, outfile)) {
						fprintf(stderr, "Error writing to CDL file");
					}
				}
			}
			fclose(output[i]);
		}

		fprintf(outfile, "}\n");
		fclose(outfile);

		/* convert cdl to netcdf */
		if (netcdf_cdl == MB_NO) {
			sprintf(output_file_temp, "ncgen -o %s %s.cdl", output_file, output_file);
			shellstatus = system(output_file_temp);
			if (shellstatus == 0) {
				sprintf(output_file_temp, "rm %s.cdl", output_file);
				shellstatus = system(output_file_temp);
			}
		}
	}
	else {
		fclose(outfile);
	}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
