/*--------------------------------------------------------------------
 *    The MB-system:  mblist.c   (GMT-module rewrite of mblist.cc)
 *
 *    Original mblist.c (1993) by D. W. Caress.
 *    Re-converted from current upstream src/utilities/mblist.cc back
 *    to C and wrapped as a GMT module entry so it can be invoked from
 *    the GMT API (and therefore from Julia FFI / Matlab MEX via GMT).
 *
 *    Copyright (c) 1993-2026 by
 *    David W. Caress (caress@mbari.org) — MBARI
 *
 *    See README.md for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBLIST prints the specified contents of a swath sonar data file in
 * tab-separated columns. Output controlled by -O letter list. Full
 * parity with mblist.cc including netcdf CDL generation (-C),
 * dump modes (-D), beam/pixel ranges (-M/-N), secondary file (-Y),
 * segmentation (-Z), projection (-J).
 */

#define THIS_MODULE_NAME    "mblist"
#define THIS_MODULE_LIB		"mbsystem"
#define THIS_MODULE_PURPOSE "List contents of a swath sonar data file"
#define THIS_MODULE_KEYS    ""
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"->V"

#include "gmt_dev.h"

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_simrad2.h"
#include "mbsys_simrad3.h"

#define MAX_OPTIONS                 100
#define SECONDARY_FILE_COLUMNS_MAX  20

typedef enum {
	DUMP_MODE_LIST = 1,
	DUMP_MODE_BATH = 2,
	DUMP_MODE_TOPO = 3,
	DUMP_MODE_AMP  = 4,
	DUMP_MODE_SS   = 5
} dump_mode_t;

typedef enum {
	MBLIST_CHECK_ON          = 0,
	MBLIST_CHECK_ON_NULL     = 1,
	MBLIST_CHECK_OFF_RAW     = 2,
	MBLIST_CHECK_OFF_NAN     = 3,
	MBLIST_CHECK_OFF_FLAGNAN = 4
} check_t;

typedef enum {
	MBLIST_SET_OFF            = 0,
	MBLIST_SET_ON             = 1,
	MBLIST_SET_ALL            = 2,
	MBLIST_SET_EXCLUDE_OUTER  = 3
} beam_set_t;

typedef enum {
	MBLIST_SEGMENT_MODE_NONE      = 0,
	MBLIST_SEGMENT_MODE_TAG       = 1,
	MBLIST_SEGMENT_MODE_SWATHFILE = 2,
	MBLIST_SEGMENT_MODE_DATALIST  = 3
} segment_mode_t;

EXTERN_MSC int GMT_mblist(void *API, int mode, void *args);

/* secondary file globals */
static int     num_secondary = 0;
static int     num_secondary_columns = 0;
static int     num_secondary_alloc = 0;
static int     j_secondary_interp = 0;
static double *secondary_time_d = NULL;
static double *secondary_data = NULL;

/* --- helpers from mblist.cc ------------------------------------------ */

static int set_output(int verbose, int beams_bath, int beams_amp, int pixels_ss,
                      bool use_bath, bool use_amp, bool use_ss, dump_mode_t dump_mode,
                      beam_set_t beam_set, int pixel_set, int beam_vertical, int pixel_vertical,
                      int *beam_start, int *beam_end, int *beam_exclude_percent,
                      int *pixel_start, int *pixel_end, int *n_list, char *list, int *error) {
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
		fprintf(stderr, "dbg2       beam_set:        %d\n", beam_set);
		fprintf(stderr, "dbg2       pixel_set:       %d\n", pixel_set);
		fprintf(stderr, "dbg2       beam_vertical:   %d\n", beam_vertical);
		fprintf(stderr, "dbg2       pixel_vertical:  %d\n", pixel_vertical);
		fprintf(stderr, "dbg2       beam_start:      %d\n", *beam_start);
		fprintf(stderr, "dbg2       beam_end:        %d\n", *beam_end);
		fprintf(stderr, "dbg2       beam_exclude_percent: %d\n", *beam_exclude_percent);
		fprintf(stderr, "dbg2       pixel_start:     %d\n", *pixel_start);
		fprintf(stderr, "dbg2       pixel_end:       %d\n", *pixel_end);
		fprintf(stderr, "dbg2       n_list:          %d\n", *n_list);
		for (int i = 0; i < *n_list; i++)
			fprintf(stderr, "dbg2       list[%2d]:        %c\n", i, list[i]);
	}

	*error = MB_ERROR_NO_ERROR;
	int status = MB_SUCCESS;

	if (beam_set == MBLIST_SET_OFF && pixel_set == MBLIST_SET_OFF && beams_bath <= 0 && pixels_ss <= 0) {
		*beam_start = 0; *beam_end = 1; *pixel_start = 0; *pixel_end = -1;
	} else if (beam_set == MBLIST_SET_OFF && pixel_set != MBLIST_SET_OFF) {
		*beam_start = 0; *beam_end = -1;
	} else if (beam_set == MBLIST_SET_OFF && beams_bath <= 0) {
		*beam_start = 0; *beam_end = -1;
		*pixel_start = pixel_vertical; *pixel_end = pixel_vertical;
	} else if (beam_set == MBLIST_SET_OFF) {
		*beam_start = beam_vertical; *beam_end = beam_vertical;
	} else if (beam_set == MBLIST_SET_ALL) {
		*beam_start = 0; *beam_end = beams_bath - 1;
	} else if (beam_set == MBLIST_SET_EXCLUDE_OUTER) {
		*beam_start = (beams_bath * *beam_exclude_percent) / 100;
		*beam_end = beams_bath - (*beam_start + 1);
	}
	if (pixel_set == MBLIST_SET_OFF && beams_bath > 0) {
		*pixel_start = 0; *pixel_end = -1;
	} else if (pixel_set == MBLIST_SET_ALL) {
		*pixel_start = 0; *pixel_end = pixels_ss - 1;
	}

	if (dump_mode == DUMP_MODE_BATH) {
		*beam_start = 0; *beam_end = beams_bath - 1;
		*pixel_start = 0; *pixel_end = -1;
		strcpy(list, "XYz"); *n_list = 3;
	} else if (dump_mode == DUMP_MODE_TOPO) {
		*beam_start = 0; *beam_end = beams_bath - 1;
		*pixel_start = 0; *pixel_end = -1;
		strcpy(list, "XYZ"); *n_list = 3;
	} else if (dump_mode == DUMP_MODE_AMP) {
		*beam_start = 0; *beam_end = beams_bath - 1;
		*pixel_start = 0; *pixel_end = -1;
		strcpy(list, "XYB"); *n_list = 3;
	} else if (dump_mode == DUMP_MODE_SS) {
		*beam_start = 0; *beam_end = -1;
		*pixel_start = 0; *pixel_end = pixels_ss - 1;
		strcpy(list, "XYb"); *n_list = 3;
	}

	if ((use_bath && *beam_end >= *beam_start) && beams_bath <= 0) {
		fprintf(stderr, "\nBathymetry data not available\n");
		status = MB_FAILURE; *error = MB_ERROR_BAD_USAGE;
	} else if (use_bath && *beam_end >= *beam_start && (*beam_start < 0 || *beam_end >= beams_bath)) {
		fprintf(stderr, "\nBeam range %d to %d exceeds available beams 0 to %d\n", *beam_start, *beam_end, beams_bath - 1);
		status = MB_FAILURE; *error = MB_ERROR_BAD_USAGE;
	}
	if (*error == MB_ERROR_NO_ERROR && use_amp && beams_amp <= 0) {
		fprintf(stderr, "\nAmplitude data not available\n");
		status = MB_FAILURE; *error = MB_ERROR_BAD_USAGE;
	} else if (*error == MB_ERROR_NO_ERROR && *beam_end >= *beam_start && use_amp &&
	           (*beam_start < 0 || *beam_end >= beams_amp)) {
		fprintf(stderr, "\nAmplitude beam range %d to %d exceeds available beams 0 to %d\n", *beam_start, *beam_end, beams_amp - 1);
		status = MB_FAILURE; *error = MB_ERROR_BAD_USAGE;
	}
	if (*error == MB_ERROR_NO_ERROR && (use_ss || *pixel_end >= *pixel_start) && pixels_ss <= 0) {
		fprintf(stderr, "\nSidescan data not available\n");
		status = MB_FAILURE; *error = MB_ERROR_BAD_USAGE;
	} else if (*error == MB_ERROR_NO_ERROR && *pixel_end >= *pixel_start && (*pixel_start < 0 || *pixel_end >= pixels_ss)) {
		fprintf(stderr, "\nPixels range %d to %d exceeds available pixels 0 to %d\n", *pixel_start, *pixel_end, pixels_ss - 1);
		status = MB_FAILURE; *error = MB_ERROR_BAD_USAGE;
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
		for (int i = 0; i < *n_list; i++)
			fprintf(stderr, "dbg2       list[%2d]:      %c\n", i, list[i]);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return status;
}

static int set_bathyslope(int verbose, int nbath, char *beamflag, double *bath, double *bathacrosstrack,
                          int *ndepths, double *depths, double *depthacrosstrack,
                          int *nslopes, double *slopes, double *slopeacrosstrack, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       nbath:           %d\n", nbath);
		fprintf(stderr, "dbg2       bath:            %p\n", (void *)bath);
		fprintf(stderr, "dbg2       bathacrosstrack: %p\n", (void *)bathacrosstrack);
		fprintf(stderr, "dbg2       bath:\n");
		for (int i = 0; i < nbath; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, bath[i], bathacrosstrack[i]);
	}

	*ndepths = 0;
	for (int i = 0; i < nbath; i++)
		if (mb_beam_ok(beamflag[i])) {
			depths[*ndepths] = bath[i];
			depthacrosstrack[*ndepths] = bathacrosstrack[i];
			(*ndepths)++;
		}
	*nslopes = *ndepths + 1;
	for (int i = 0; i < *ndepths - 1; i++) {
		slopes[i + 1] = (depths[i + 1] - depths[i]) / (depthacrosstrack[i + 1] - depthacrosstrack[i]);
		slopeacrosstrack[i + 1] = 0.5 * (depthacrosstrack[i + 1] + depthacrosstrack[i]);
	}
	if (*ndepths > 1) {
		slopes[0] = 0.0; slopeacrosstrack[0] = depthacrosstrack[0];
		slopes[*ndepths] = 0.0; slopeacrosstrack[*ndepths] = depthacrosstrack[*ndepths - 1];
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ndepths:         %d\n", *ndepths);
		fprintf(stderr, "dbg2       depths:\n");
		for (int i = 0; i < *ndepths; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, depths[i], depthacrosstrack[i]);
		fprintf(stderr, "dbg2       nslopes:         %d\n", *nslopes);
		fprintf(stderr, "dbg2       slopes:\n");
		for (int i = 0; i < *nslopes; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}

static int get_bathyslope(int verbose, int ndepths, double *depths, double *depthacrosstrack,
                          int nslopes, double *slopes, double *slopeacrosstrack,
                          double acrosstrack, double *depth, double *slope, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       ndepths:         %d\n", ndepths);
		fprintf(stderr, "dbg2       depths:\n");
		for (int i = 0; i < ndepths; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, depths[i], depthacrosstrack[i]);
		fprintf(stderr, "dbg2       nslopes:         %d\n", nslopes);
		fprintf(stderr, "dbg2       slopes:\n");
		for (int i = 0; i < nslopes; i++)
			fprintf(stderr, "dbg2         %d %f %f\n", i, slopes[i], slopeacrosstrack[i]);
		fprintf(stderr, "dbg2       acrosstrack:     %f\n", acrosstrack);
	}

	bool found_depth = false, found_slope = false;
	if (ndepths > 1 && acrosstrack >= depthacrosstrack[0] && acrosstrack <= depthacrosstrack[ndepths - 1]) {
		int idepth = -1;
		while (found_depth && idepth < ndepths - 2) {
			idepth++;
			if (acrosstrack >= depthacrosstrack[idepth] && acrosstrack <= depthacrosstrack[idepth + 1]) {
				*depth = depths[idepth] + (acrosstrack - depthacrosstrack[idepth]) /
				    (depthacrosstrack[idepth + 1] - depthacrosstrack[idepth]) *
				    (depths[idepth + 1] - depths[idepth]);
				found_depth = true; *error = MB_ERROR_NO_ERROR;
			}
		}
		int islope = -1;
		while (!found_slope && islope < nslopes - 2) {
			islope++;
			if (acrosstrack >= slopeacrosstrack[islope] && acrosstrack <= slopeacrosstrack[islope + 1]) {
				*slope = slopes[islope] + (acrosstrack - slopeacrosstrack[islope]) /
				    (slopeacrosstrack[islope + 1] - slopeacrosstrack[islope]) *
				    (slopes[islope + 1] - slopes[islope]);
				found_slope = true; *error = MB_ERROR_NO_ERROR;
			}
		}
	}
	if (found_slope) *slope = RTD * atan(*slope);
	int status = MB_SUCCESS;
	if (!found_depth || !found_slope) {
		status = MB_FAILURE; *error = MB_ERROR_OTHER;
		*depth = 0.0; *slope = 0.0;
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

	return status;
}

static int printsimplevalue(int verbose, FILE *output, double value, int width, int precision,
                            bool ascii, bool *invert, bool *flipsign, int *error) {
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

	char format[24];
	format[0] = '%';
	if (*invert)       strcpy(format, "%g");
	else if (width > 0) snprintf(&format[1], 23, "%d.%df", width, precision);
	else                snprintf(&format[1], 23, ".%df", precision);
	if (*invert)   { *invert = false; if (value != 0.0) value = 1.0 / value; }
	if (*flipsign) { *flipsign = false; value = -value; }
	if (ascii) fprintf(output, format, value);
	else       fwrite(&value, sizeof(double), 1, output);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}

static int printNaN(int verbose, FILE *output, bool ascii, bool *invert, bool *flipsign, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       ascii:           %d\n", ascii);
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       flipsign:        %d\n", *flipsign);
	}

	if (*invert)   *invert = false;
	if (*flipsign) *flipsign = false;
	if (ascii) fprintf(output, "NaN");
	else {
		double NaN = 0.0; NaN = NaN / NaN;  /* produce NaN */
		fwrite(&NaN, sizeof(double), 1, output);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}

static int mb_get_raw_simrad2(int verbose, void *mbio_ptr, int *mode, int *ipulse_length,
                              int *png_count, int *sample_rate, double *absorption, int *max_range,
                              int *r_zero, int *r_zero_corr, int *tvg_start, int *tvg_stop,
                              double *bsn, double *bso, int *tx, int *tvg_crossover,
                              int *nbeams_ss, int *npixels, int *beam_samples, int *start_sample,
                              int *range, double *depression, double *bs, double *ss_pixels, int *error) {
	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
	}

	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_simrad2_struct *store_ptr = (struct mbsys_simrad2_struct *)mb_io_ptr->store_data;
	struct mbsys_simrad2_ping_struct *ping_ptr = store_ptr->ping;
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
		for (int i = 0; i < ping_ptr->png_nbeams; i++) {
			range[ping_ptr->png_beam_num[i] - 1] = ping_ptr->png_range[i];
			depression[ping_ptr->png_beam_num[i] - 1] = ping_ptr->png_depression[i] * .01;
			bs[ping_ptr->png_beam_num[i] - 1] = ping_ptr->png_amp[i] * 0.5;
		}
		for (int i = 0; i < ping_ptr->png_nbeams_ss; i++) {
			beam_samples[ping_ptr->png_beam_index[i]] = ping_ptr->png_beam_samples[i];
			start_sample[ping_ptr->png_beam_index[i]] = ping_ptr->png_start_sample[i];
		}
		for (int i = 0; i < ping_ptr->png_npixels; i++)
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
		for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d range:%d depression:%f bs:%f\n", i, range[i], depression[i], bs[i]);
		}
		for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d samples:%d start:%d\n", i, beam_samples[i], start_sample[i]);
		}
		for (int i = 0; i < *npixels; i++) {
			fprintf(stderr, "dbg2       pixel:%d ss:%f\n", i, ss_pixels[i]);
		}
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}

static int mb_get_raw_simrad3(int verbose, void *mbio_ptr, int *mode, int *ipulse_length,
                              int *png_count, int *sample_rate, double *absorption, int *max_range,
                              int *r_zero, int *r_zero_corr, int *tvg_start, int *tvg_stop,
                              double *bsn, double *bso, int *tx, int *tvg_crossover,
                              int *nbeams_ss, int *npixels, int *beam_samples, int *start_sample,
                              int *range, double *depression, double *bs, double *ss_pixels, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
	}

	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	struct mbsys_simrad3_struct *store_ptr = (struct mbsys_simrad3_struct *)mb_io_ptr->store_data;
	struct mbsys_simrad3_ping_struct *ping_ptr =
	    (struct mbsys_simrad3_ping_struct *)&(store_ptr->pings[store_ptr->ping_index]);
	if (store_ptr->kind == MB_DATA_DATA) {
		*mode = store_ptr->run_mode;
		*ipulse_length = store_ptr->run_tran_pulse;
		*png_count = ping_ptr->png_count;
		*sample_rate = ping_ptr->png_sample_rate;
		*absorption = store_ptr->run_absorption * 0.01;
		*max_range = 0;
		*r_zero = ping_ptr->png_r_zero;
		*r_zero_corr = 0; *tvg_start = 0; *tvg_stop = 0;
		*bsn = ping_ptr->png_bsn * 0.1;
		*bso = ping_ptr->png_bso * 0.1;
		*tx = ping_ptr->png_tx * 0.1;
		*tvg_crossover = ping_ptr->png_tvg_crossover;
		*nbeams_ss = ping_ptr->png_nbeams_ss;
		*npixels = ping_ptr->png_npixels;
		for (int i = 0; i < ping_ptr->png_nbeams; i++) {
			range[i] = ping_ptr->png_range[i];
			depression[i] = ping_ptr->png_depression[i] * .01;
			bs[i] = ping_ptr->png_amp[i] * 0.5;
		}
		for (int i = 0; i < ping_ptr->png_nbeams_ss; i++) {
			beam_samples[i] = ping_ptr->png_beam_samples[i];
			start_sample[i] = ping_ptr->png_start_sample[i];
		}
		for (int i = 0; i < ping_ptr->png_npixels; i++)
			ss_pixels[i] = ping_ptr->png_ssraw[i] * 0.5;
	}

	const int status = MB_SUCCESS;

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
		for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d range:%d depression:%f bs:%f\n", i, range[i], depression[i], bs[i]);
		}
		for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d samples:%d start:%d\n", i, beam_samples[i], start_sample[i]);
		}
		for (int i = 0; i < *npixels; i++) {
			fprintf(stderr, "dbg2       pixel:%d ss:%f\n", i, ss_pixels[i]);
		}
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}

static int mb_get_raw(int verbose, void *mbio_ptr, int *mode, int *ipulse_length,
                      int *png_count, int *sample_rate, double *absorption, int *max_range,
                      int *r_zero, int *r_zero_corr, int *tvg_start, int *tvg_stop,
                      double *bsn, double *bso, int *tx, int *tvg_crossover,
                      int *nbeams_ss, int *npixels, int *beam_samples, int *start_sample,
                      int *range, double *depression, double *bs, double *ss_pixels, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
	}

	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	*mode = -1; *ipulse_length = 0; *png_count = 0; *sample_rate = 0;
	*absorption = 0; *max_range = 0; *r_zero = 0; *r_zero_corr = 0;
	*tvg_start = 0; *tvg_stop = 0; *bsn = 0; *bso = 0; *tx = 0;
	*tvg_crossover = 0; *nbeams_ss = 0; *npixels = 0;
	for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
		beam_samples[i] = 0; start_sample[i] = 0; range[i] = 0;
		depression[i] = 0.0; bs[i] = 0.0;
	}
	switch (mb_io_ptr->format) {
	case MBF_EM300MBA: case MBF_EM300RAW:
		mb_get_raw_simrad2(verbose, mbio_ptr, mode, ipulse_length, png_count, sample_rate, absorption,
		                   max_range, r_zero, r_zero_corr, tvg_start, tvg_stop, bsn, bso, tx, tvg_crossover,
		                   nbeams_ss, npixels, beam_samples, start_sample, range, depression, bs, ss_pixels, error);
		break;
	case MBF_EM710MBA: case MBF_EM710RAW:
		mb_get_raw_simrad3(verbose, mbio_ptr, mode, ipulse_length, png_count, sample_rate, absorption,
		                   max_range, r_zero, r_zero_corr, tvg_start, tvg_stop, bsn, bso, tx, tvg_crossover,
		                   nbeams_ss, npixels, beam_samples, start_sample, range, depression, bs, ss_pixels, error);
		break;
	}

	int status = MB_SUCCESS;

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
		for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d range:%d depression:%f bs:%f\n", i, range[i], depression[i], bs[i]);
		}
		for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
			fprintf(stderr, "dbg2       beam:%d samples:%d start:%d\n", i, beam_samples[i], start_sample[i]);
		}
		for (int i = 0; i < *npixels; i++) {
			fprintf(stderr, "dbg2       pixel:%d ss:%f\n", i, ss_pixels[i]);
		}
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}

/* --- Control structure ----------------------------------------------- */

struct MBLIST_CTRL {
	struct ml_A { bool active; } A;        /* ascii=false (binary out) */
	struct ml_B { bool active; int t[7]; } B;
	struct ml_C { bool active; } C;        /* netcdf */
	struct ml_D { bool active; int mode; } D;
	struct ml_E { bool active; int t[7]; } E;
	struct ml_F { bool active; int format; } F;
	struct ml_G { bool active; char delim[MB_PATH_MAXLINE]; } G;
	struct ml_I { bool active; char *inputfile; } I;
	struct ml_J { bool active; char proj[MB_PATH_MAXLINE]; } J;
	struct ml_K { bool active; int decimate; } K;
	struct ml_L { bool active; int lonflip; } L;
	struct ml_M { bool active; int beam_set; int beam_start, beam_end, beam_exclude_percent; } M;
	struct ml_N { bool active; int pixel_set; int pixel_start, pixel_end; } N;
	struct ml_O { bool active; char list[MAX_OPTIONS]; int n_list; bool use_projection; } O;
	struct ml_P { bool active; int pings; } P;
	struct ml_Q { bool active; } Q;        /* check_values = OFF_RAW */
	struct ml_R { bool active; double bounds[4]; } R;
	struct ml_S { bool active; double speedmin; } S;
	struct ml_T { bool active; double timegap; } T;
	struct ml_U { bool active; check_t check_values; bool check_nav; } U;
	struct ml_W { bool active; } W;        /* feet */
	struct ml_X { bool active; char outfile[MB_PATH_MAXLINE]; } X;
	struct ml_Y { bool active; char secfile[MB_PATH_MAXLINE]; } Y;
	struct ml_Z { bool active; segment_mode_t mode; char tag[MB_PATH_MAXLINE]; } Z;
};

static void *New_mblist_Ctrl(struct GMT_CTRL *GMT) {
	struct MBLIST_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBLIST_CTRL);
	strcpy(Ctrl->G.delim, "\t");
	Ctrl->K.decimate = 1;
	Ctrl->P.pings = 1;
	Ctrl->U.check_values = MBLIST_CHECK_ON;
	strcpy(Ctrl->O.list, "TXYHSLZ");
	Ctrl->O.n_list = 7;
	strcpy(Ctrl->X.outfile, "-");
	return Ctrl;
}

static void Free_mblist_Ctrl(struct GMT_CTRL *GMT, struct MBLIST_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.inputfile) free(Ctrl->I.inputfile);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE,
	    "usage: mblist [-A -Byr/mo/da/hr/mn/sc -C -Ddump -Eyr/mo/da/hr/mn/sc\n"
	    "\t-Fformat -Gdelim -Ifile -Jproj -Kdec -Llonflip\n"
	    "\t-M[beam0/beam1|A|X%%] -Npix0/pix1 -Ooptions -Ppings -Q\n"
	    "\t-Rw/e/s/n -Sspeed -Ttimegap -Ucheck -W -Xoutfile -Ysec -Zseg -V -H]\n\n");
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBLIST_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0, n_files = 0;
	int n;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case '<':
			Ctrl->I.active = true;
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
				Ctrl->I.inputfile = strdup(opt->arg); n_files = 1;
			} else n_errors++;
			break;
		case 'A': Ctrl->A.active = true; break;
		case 'B':
			Ctrl->B.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &Ctrl->B.t[0], &Ctrl->B.t[1], &Ctrl->B.t[2],
			           &Ctrl->B.t[3], &Ctrl->B.t[4], &Ctrl->B.t[5]);
			if (n == 6) Ctrl->B.active = true; else n_errors++;
			break;
		case 'C': Ctrl->C.active = true; break;
		case 'D': {
			int tmp;
			if (sscanf(opt->arg, "%d", &tmp) > 0) { Ctrl->D.mode = tmp; Ctrl->D.active = true; }
			else n_errors++;
			break;
		}
		case 'E':
			Ctrl->E.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &Ctrl->E.t[0], &Ctrl->E.t[1], &Ctrl->E.t[2],
			           &Ctrl->E.t[3], &Ctrl->E.t[4], &Ctrl->E.t[5]);
			if (n == 6) Ctrl->E.active = true; else n_errors++;
			break;
		case 'F':
			if (sscanf(opt->arg, "%d", &Ctrl->F.format) > 0) Ctrl->F.active = true; else n_errors++;
			break;
		case 'G':
			sscanf(opt->arg, "%1023s", Ctrl->G.delim);
			Ctrl->G.active = true;
			break;
		case 'I':
			if (!gmt_access(GMT, opt->arg, R_OK)) {
				Ctrl->I.inputfile = strdup(opt->arg); Ctrl->I.active = true; n_files = 1;
			} else n_errors++;
			break;
		case 'J':
			sscanf(opt->arg, "%1023s", Ctrl->J.proj);
			Ctrl->J.active = true;
			break;
		case 'K':
			if (sscanf(opt->arg, "%d", &Ctrl->K.decimate) > 0) Ctrl->K.active = true; else n_errors++;
			break;
		case 'L':
			if (sscanf(opt->arg, "%d", &Ctrl->L.lonflip) > 0) Ctrl->L.active = true; else n_errors++;
			break;
		case 'M':
			if (opt->arg[0] == 'a' || opt->arg[0] == 'A') {
				Ctrl->M.beam_set = MBLIST_SET_ALL;
			} else if (opt->arg[0] == 'x' || opt->arg[0] == 'X') {
				Ctrl->M.beam_set = MBLIST_SET_EXCLUDE_OUTER;
				sscanf(opt->arg, "%*c%d", &Ctrl->M.beam_exclude_percent);
			} else {
				sscanf(opt->arg, "%d/%d", &Ctrl->M.beam_start, &Ctrl->M.beam_end);
				Ctrl->M.beam_set = MBLIST_SET_ON;
			}
			Ctrl->M.active = true;
			break;
		case 'N':
			if (opt->arg[0] == 'a' || opt->arg[0] == 'A') {
				Ctrl->N.pixel_set = MBLIST_SET_ALL;
			} else {
				sscanf(opt->arg, "%d/%d", &Ctrl->N.pixel_start, &Ctrl->N.pixel_end);
				Ctrl->N.pixel_set = MBLIST_SET_ON;
			}
			Ctrl->N.active = true;
			break;
		case 'O':
			if (strcmp(opt->arg, "%fnv") == 0 || strcmp(opt->arg, "%FNV") == 0) {
				strncpy(Ctrl->O.list, "tMXYHScRPr=X=Y+X+Y", sizeof(Ctrl->O.list));
				Ctrl->O.n_list = (int)strlen(Ctrl->O.list);
			} else if (strlen(opt->arg) > 0) {
				int len = (int)strlen(opt->arg);
				if (len > MAX_OPTIONS) len = MAX_OPTIONS;
				Ctrl->O.n_list = len;
				for (int j = 0; j < len; j++) {
					Ctrl->O.list[j] = opt->arg[j];
					if (Ctrl->O.list[j] == '^') Ctrl->O.use_projection = true;
				}
			}
			Ctrl->O.active = true;
			break;
		case 'P':
			if (sscanf(opt->arg, "%d", &Ctrl->P.pings) > 0) Ctrl->P.active = true; else n_errors++;
			break;
		case 'Q':
			Ctrl->Q.active = true; break;
		case 'R':
			mb_get_bounds(opt->arg, Ctrl->R.bounds);
			Ctrl->R.active = true;
			break;
		case 'S':
			if (sscanf(opt->arg, "%lf", &Ctrl->S.speedmin) > 0) Ctrl->S.active = true; else n_errors++;
			break;
		case 'T':
			if (sscanf(opt->arg, "%lf", &Ctrl->T.timegap) > 0) Ctrl->T.active = true; else n_errors++;
			break;
		case 'U':
			if (opt->arg[0] == 'N') Ctrl->U.check_nav = true;
			else {
				int tmp;
				if (sscanf(opt->arg, "%d", &tmp) > 0) {
					if (tmp < MBLIST_CHECK_ON || tmp > MBLIST_CHECK_OFF_FLAGNAN) tmp = MBLIST_CHECK_ON;
					Ctrl->U.check_values = (check_t)tmp;
				}
			}
			Ctrl->U.active = true;
			break;
		case 'W':
			Ctrl->W.active = true; break;
		case 'X':
			sscanf(opt->arg, "%1023s", Ctrl->X.outfile);
			Ctrl->X.active = true;
			break;
		case 'Y':
			sscanf(opt->arg, "%1023s", Ctrl->Y.secfile);
			Ctrl->Y.active = true;
			break;
		case 'Z':
			sscanf(opt->arg, "%1023s", Ctrl->Z.tag);
			if      (strcmp(Ctrl->Z.tag, "swathfile") == 0) Ctrl->Z.mode = MBLIST_SEGMENT_MODE_SWATHFILE;
			else if (strcmp(Ctrl->Z.tag, "datalist") == 0)  Ctrl->Z.mode = MBLIST_SEGMENT_MODE_DATALIST;
			else                                            Ctrl->Z.mode = MBLIST_SEGMENT_MODE_TAG;
			Ctrl->Z.active = true;
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, n_files != 1, "Syntax: Must specify one input file\n");
	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return (code); }
#define Return(code)   { Free_mblist_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/* netcdf CDL header emit helper: standard "float/long/etc var(data);" pattern.
 * Matches the per-letter pattern from mblist.cc that emits:
 *   output[i]: "\t<var> = "
 *   outfile  : "\t<type> <var>(data);\n"
 *              "\t\t<var>:long_name = \"<long>\";\n"
 *              "\t\t<var>:units = \"[-][1/]<units>\";\n"
 * with optional signflip/invert decorations on variable name and units. */
static void cdl_emit(FILE *out_i, FILE *outfile, const char *type, const char *base_var,
                     const char *long_name, const char *units_base,
                     bool *signflip, bool *invert, bool flip_decor_var, bool inv_decor_var,
                     bool flip_decor_units, bool inv_decor_units) {
	char variable[MB_PATH_MAXLINE];
	strcpy(variable, base_var);
	if (flip_decor_var && *signflip) strcat(variable, "-");
	if (inv_decor_var && *invert)    strcat(variable, "_");
	fprintf(out_i, "\t%s = ", variable);
	fprintf(outfile, "\t%s %s(data);\n", type, variable);
	fprintf(outfile, "\t\t%s:long_name = \"%s\";\n", variable, long_name);
	fprintf(outfile, "\t\t%s:units = \"", variable);
	if (flip_decor_units && *signflip) fprintf(outfile, "-");
	if (inv_decor_units && *invert)    fprintf(outfile, "1/");
	fprintf(outfile, "%s\";\n", units_base);
	*signflip = false; *invert = false;
}

/* end of chunk 1 - GMT_mblist entry function follows in chunk 2 */

int GMT_mblist(void *V_API, int mode, void *args) {
	int    error = MB_ERROR_NO_ERROR;

	struct MBLIST_CTRL *Ctrl = NULL;
	struct GMT_CTRL    *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION  *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr(V_API);

	if (API == NULL) return GMT_NOT_A_SESSION;
	if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);
	options = GMT_Create_Options(API, mode, args);
	if (API->error) return API->error;
	if (!options || options->option == GMT_OPT_USAGE)    bailout(usage(API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS)             bailout(usage(API, GMT_SYNOPSIS));

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_mblist_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int verbose = GMT->common.V.active;
	int format, pings, pings_read, lonflip;
	double bounds[4];
	int btime_i[7], etime_i[7];
	double speedmin, timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds,
	                         btime_i, etime_i, &speedmin, &timegap);

	if (Ctrl->F.active) format = Ctrl->F.format;
	if (Ctrl->L.active) lonflip = Ctrl->L.lonflip;
	if (Ctrl->S.active) speedmin = Ctrl->S.speedmin;
	if (Ctrl->T.active) timegap = Ctrl->T.timegap;
	if (Ctrl->R.active) for (int i = 0; i < 4; i++) bounds[i] = Ctrl->R.bounds[i];
	if (Ctrl->B.active) for (int i = 0; i < 7; i++) btime_i[i] = Ctrl->B.t[i];
	if (Ctrl->E.active) for (int i = 0; i < 7; i++) etime_i[i] = Ctrl->E.t[i];
	if (Ctrl->P.active) pings = Ctrl->P.pings;

	mb_path read_file;
	strcpy(read_file, Ctrl->I.active ? Ctrl->I.inputfile : "datalist.mb-1");

	bool   bathy_in_feet = Ctrl->W.active;
	bool   ascii = !Ctrl->A.active;
	bool   netcdf = Ctrl->C.active;
	bool   netcdf_cdl = !Ctrl->A.active;
	dump_mode_t dump_mode = Ctrl->D.active ? (dump_mode_t)Ctrl->D.mode : DUMP_MODE_LIST;
	beam_set_t  beam_set = (beam_set_t)Ctrl->M.beam_set;
	int    pixel_set = Ctrl->N.pixel_set;
	if (Ctrl->D.active) {
		if (dump_mode == DUMP_MODE_BATH || dump_mode == DUMP_MODE_TOPO || dump_mode == DUMP_MODE_AMP)
			beam_set = MBLIST_SET_ALL;
		else if (dump_mode == DUMP_MODE_SS)
			pixel_set = MBLIST_SET_ALL;
	}
	char  *delimiter = Ctrl->G.delim;
	char   projection_pars[MB_PATH_MAXLINE];
	strcpy(projection_pars, Ctrl->J.active ? Ctrl->J.proj : "");
	bool   use_projection = Ctrl->J.active || Ctrl->O.use_projection;
	int    decimate = Ctrl->K.decimate;
	int    beam_exclude_percent = Ctrl->M.beam_exclude_percent;
	int    beam_start = Ctrl->M.beam_start, beam_end = Ctrl->M.beam_end;
	int    pixel_start = Ctrl->N.pixel_start, pixel_end = Ctrl->N.pixel_end;
	check_t check_values = Ctrl->Q.active ? MBLIST_CHECK_OFF_RAW : Ctrl->U.check_values;
	bool   check_nav = Ctrl->U.check_nav;
	char   output_file[MB_PATH_MAXLINE];
	strcpy(output_file, Ctrl->X.outfile);
	bool   segment = Ctrl->Z.active;
	segment_mode_t segment_mode = Ctrl->Z.active ? Ctrl->Z.mode : MBLIST_SEGMENT_MODE_NONE;
	char  *segment_tag = Ctrl->Z.tag;
	mb_path secondary_file;
	strcpy(secondary_file, Ctrl->Y.active ? Ctrl->Y.secfile : "");
	bool   secondary_file_set = Ctrl->Y.active;

	char   list[MAX_OPTIONS];
	memcpy(list, Ctrl->O.list, MAX_OPTIONS);
	int    n_list = Ctrl->O.n_list;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
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
		fprintf(stderr, "dbg2       secondary_file: %s\n", secondary_file);
		fprintf(stderr, "dbg2       secondary_file_set:%d\n", secondary_file_set);
		fprintf(stderr, "dbg2       n_list:         %d\n", n_list);
		for (int i = 0; i < n_list; i++)
			fprintf(stderr, "dbg2         list[%d]:      %c\n", i, list[i]);
	}

	if (format == 0) mb_get_format(verbose, read_file, NULL, &format, &error);
	double bathy_scale = bathy_in_feet ? (1.0 / 0.3048) : 1.0;

	/* read secondary file if requested */
	if (secondary_file_set) {
		FILE *sfp = fopen(secondary_file, "r");
		if (sfp != NULL) {
			double dd[SECONDARY_FILE_COLUMNS_MAX];
			mb_path buffer;
			while (fgets(buffer, sizeof(mb_path), sfp) != NULL) {
				if (buffer[0] != '#') {
					int nr = sscanf(buffer,
					    "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
					    &dd[0],&dd[1],&dd[2],&dd[3],&dd[4],&dd[5],&dd[6],&dd[7],&dd[8],&dd[9],
					    &dd[10],&dd[11],&dd[12],&dd[13],&dd[14],&dd[15],&dd[16],&dd[17],&dd[18],&dd[19]);
					if (nr > num_secondary_columns) num_secondary_columns = nr;
				}
			}
			rewind(sfp);
			while (fgets(buffer, sizeof(mb_path), sfp) != NULL) {
				if (buffer[0] != '#') {
					int nr = sscanf(buffer,
					    "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
					    &dd[0],&dd[1],&dd[2],&dd[3],&dd[4],&dd[5],&dd[6],&dd[7],&dd[8],&dd[9],
					    &dd[10],&dd[11],&dd[12],&dd[13],&dd[14],&dd[15],&dd[16],&dd[17],&dd[18],&dd[19]);
					if (nr == num_secondary_columns) num_secondary_alloc++;
				}
			}
			rewind(sfp);
			status = mb_mallocd(verbose, __FILE__, __LINE__, num_secondary_alloc * sizeof(double),
			                    (void **)&secondary_time_d, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__,
			                    num_secondary_alloc * (num_secondary_columns - 1) * sizeof(double),
			                    (void **)&secondary_data, &error);
			while (fgets(buffer, sizeof(mb_path), sfp) != NULL) {
				if (buffer[0] != '#') {
					int nr = sscanf(buffer,
					    "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
					    &dd[0],&dd[1],&dd[2],&dd[3],&dd[4],&dd[5],&dd[6],&dd[7],&dd[8],&dd[9],
					    &dd[10],&dd[11],&dd[12],&dd[13],&dd[14],&dd[15],&dd[16],&dd[17],&dd[18],&dd[19]);
					if (nr == num_secondary_columns) {
						secondary_time_d[num_secondary] = dd[0];
						for (int i = 1; i < num_secondary_columns; i++) {
							int j = (i - 1) * num_secondary_alloc + num_secondary;
							secondary_data[j] = dd[i];
						}
						num_secondary++;
					}
				}
			}
			fclose(sfp);
		} else {
			fprintf(stderr, "\nUnable to open secondary file: %s\n", secondary_file);
			Return(MB_ERROR_OPEN_FAIL);
		}
	}

	const bool read_datalist = (format < 0);
	bool   read_data;
	void  *datalist = NULL;
	char   file[MB_PATH_MAXLINE] = "";
	char   path[MB_PATH_MAXLINE] = "";
	char   ppath[MB_PATH_MAXLINE] = "";
	char   apath[MB_PATH_MAXLINE] = "";
	char   dpath[MB_PATH_MAXLINE] = "";
	double file_weight;
	int    pstatus, astatus = 0;

	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			Return(MB_ERROR_OPEN_FAIL);
		}
		read_data = (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath,
		                               &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS);
		if (pstatus == MB_PROCESSED_USE) strcpy(file, ppath);
		else                              strcpy(file, path);
	} else {
		strcpy(file, read_file);
		read_data = true;
		astatus = 0;
	}

	double btime_d, etime_d;
	int beams_bath, beams_amp, pixels_ss;
	int beam_vertical = 0, pixel_vertical = 0;
	int beam_status = MB_SUCCESS, pixel_status = MB_SUCCESS;
	int time_j[5];
	bool use_bath = false, use_amp = false, use_ss = false;
	bool use_slope = false, use_attitude = false, use_gains = false;
	bool use_detects = true, use_pingnumber = false, use_linenumber = false;
	bool use_ttimes = false, use_raw = false;
	bool check_bath = false, check_amp = false, check_ss = false;
	bool signflip_next_value = false, ttimes_next_value = false, raw_next_value = false;
	bool port_next_value = false, stbd_next_value = false;
	bool sensornav_next_value = false, sensorrelative_next_value = false;
	bool projectednav_next_value = false, special_character = false;

	void *mbio_ptr = NULL, *store_ptr = NULL;
	int kind, time_i[7];
	double time_d, navlon, navlat, speed, heading, distance, altitude, sensordepth;
	double draft, roll, pitch, heave;
	char *beamflag = NULL;
	double *bath = NULL, *bathacrosstrack = NULL, *bathalongtrack = NULL;
	int    *detect = NULL;
	double *amp = NULL, *ss = NULL, *ssacrosstrack = NULL, *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];
	int icomment = 0;
	unsigned int pingnumber = 0, linenumber = 0;

	bool first_m = true, first_u = true;
	time_t time_u, time_u_ref;
	double seconds;

	double avgslope = 0.0;
	double sx, sy, sxx, sxy;
	int ns;
	double angle, depth, slope;
	int ndepths, nslopes;
	double *depths = NULL, *depthacrosstrack = NULL;
	double *slopes = NULL, *slopeacrosstrack = NULL;

	double course = 0.0, course_old = 0.0;
	double time_d_old = 0.0, dt;
	double time_interval = 0.0;
	double speed_made_good = 0.0, speed_made_good_old = 0.0;
	double navlon_old = 0.0, navlat_old = 0.0;
	double dx, dy, dist;
	double delta, b;
	double dlon, dlat, minutes;
	int degrees;
	char hemi;
	double headingx, headingy, mtodeglon, mtodeglat;

	int beam_port = 0, beam_stbd = 0, pixel_port = 0, pixel_stbd = 0;

	char projection_id[MB_PATH_MAXLINE] = "";
	int proj_status;
	void *pjptr = NULL;
	double reference_lon, reference_lat;
	int utm_zone;
	double naveasting = 0.0, navnorthing = 0.0, deasting, dnorthing;

	int tt_nbeams = 0, tt_kind;
	double *tt_ttimes = NULL, *tt_angles = NULL, *tt_angles_forward = NULL;
	double *tt_angles_null = NULL, *tt_heave = NULL, *tt_alongtrack_offset = NULL;
	double tt_sensordepth = 0.0, tt_ssv = 0.0;

	int count = 0, invert = 0, flip = 0;
	int mode_v = 0, ipulse_length = 0, png_count = 0, sample_rate = 0;
	double absorption = 0.0;
	int max_range = 0, r_zero = 0, r_zero_corr = 0, tvg_start = 0, tvg_stop = 0;
	double bsn_v = 0.0, bso_v = 0.0, mback = 0.0;
	int nback = 0, tx = 0, tvg_crossover = 0, nbeams_ss = 0, npixels = 0;
	int *beam_samples = NULL, *range = NULL, *start_sample = NULL;
	double *depression = NULL, *bs = NULL, *ss_pixels = NULL;
	double transmit_gain = 0.0, pulse_length = 0.0, receive_gain = 0.0;
	double dsecondary = 0.0;
	int nbeams;
	char output_file_temp[2*MB_PATH_MAXLINE+20] = "";
	int lcount = 0;
	double distance_total = 0.0;

	FILE **output;
	status = mb_mallocd(verbose, __FILE__, __LINE__, n_list * sizeof(FILE *), (void **)&output, &error);
	bool invert_next_value = false;

	FILE *outfile;
	if (!netcdf) {
		if (0 == strncmp("-", output_file, 2)) outfile = stdout;
		else outfile = fopen(output_file, "w");
		if (outfile == NULL) {
			fprintf(stderr, "Could not open file: %s\n", output_file);
			Return(MB_ERROR_OPEN_FAIL);
		}
		for (int i = 0; i < n_list; i++) output[i] = outfile;
	} else {
		ascii = true; segment = false;
		if (0 == strncmp("-", output_file, 2) && !netcdf_cdl) strcpy(output_file, "mblist.nc");
		if (0 == strncmp("-", output_file, 2)) outfile = stdout;
		else {
			strncpy(output_file_temp, output_file, MB_PATH_MAXLINE - 5);
			if (!netcdf_cdl) strcat(output_file_temp, ".cdl");
			outfile = fopen(output_file_temp, "w+");
			if (outfile == NULL) {
				fprintf(stderr, "Unable to open file: %s\n", output_file_temp);
				Return(MB_ERROR_OPEN_FAIL);
			}
		}

		fprintf(outfile, "netcdf mlist {\n\n\t// gmt mblist\n");
		fprintf(outfile, "dimensions:\n\ttimestring = 26, timestring_J = 24, timestring_j = 23, \n\t");
		fprintf(outfile, "timefields_J = 6,  timefields_j = 5, timefields_t = 7, latm = 13, \n\t");

		raw_next_value = false;
		for (int i = 0; i < n_list; i++)
			if (list[i] == '/' || list[i] == '-' || list[i] == '=' || list[i] == '+') {
			} else if (!raw_next_value) {
				if (list[i] == '.') raw_next_value = true;
			} else if (list[i] >= '0' && list[i] <= '9')
				count = count * 10 + list[i] - '0';
			else {
				raw_next_value = false;
				if (count > 0) { fprintf(outfile, "%c = %d,  ", list[i], count); count = 0; }
			}

		fprintf(outfile, "\n\tdata = unlimited ;\n\nvariables:\n\t");
		fprintf(outfile, ":command_line = \"gmt mblist\";\n");
		fprintf(outfile, "\t:mbsystem_version = \"%s\";\n", MB_VERSION);
		{
			char user[256], host[256], date[32];
			mb_user_host_date(verbose, user, host, date, &error);
			fprintf(outfile, "\t:run = \"by <%s> on cpu <%s> at <%s>\";\n\n", user, host, date);
		}

		for (int i = 0; i < n_list; i++) {
			output[i] = tmpfile();
			if (output[i] == NULL) { fprintf(stderr, "Unable to open temp files\n"); Return(MB_ERROR_OPEN_FAIL); }
			char variable[MB_PATH_MAXLINE] = "";
			if (!raw_next_value && !ttimes_next_value) {
				switch (list[i]) {
				case '/': invert_next_value = true; break;
				case '-': signflip_next_value = true; break;
				case '_': sensornav_next_value = true; break;
				case '@': sensorrelative_next_value = true; break;
				case '^': projectednav_next_value = true; break;
				case ',': ttimes_next_value = true; break;
				case '.': raw_next_value = true; break;
				case '=': case '+': break;
				case 'A':
					cdl_emit(output[i], outfile, "float", "aslope", "Average seafloor crosstrack slope",
					(invert_next_value ? "tangent of angle from seafloor to vertical" : "tangent of angle from seafloor to horizontal"),
					&signflip_next_value, &invert_next_value, true, true, true, false); break;
				case 'a':
					cdl_emit(output[i], outfile, "float", "bslope", "Per-beam seafloor crosstrack slope",
					(invert_next_value ? "tangent of angle from seafloor to vertical" : "tangent of angle from seafloor to horizontal"),
					&signflip_next_value, &invert_next_value, true, true, true, false); break;
				case 'B':
					cdl_emit(output[i], outfile, "float", "amplitude", "Amplitude",
					((format == MBF_EM300RAW || format == MBF_EM300MBA) ? "dB + 64" : "backscatter"),
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'b':
					cdl_emit(output[i], outfile, "float", "sidescan", "sidescan",
					((format == MBF_EM300RAW || format == MBF_EM300MBA) ? "dB + 64" : "backscatter"),
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'C':
					cdl_emit(output[i], outfile, "float", "altitude", "Sonar altitude", "m",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'c':
					cdl_emit(output[i], outfile, "float", "transducer", "Sonar transducer depth", "m",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'D':
					case 'd': cdl_emit(output[i], outfile, "float", "acrosstrack", "Acrosstrack distance",
					(bathy_in_feet ? "f" : "m"), &signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'E': case 'e':
					cdl_emit(output[i], outfile, "float", "alongtrack", "Alongtrack distance",
					(bathy_in_feet ? "f" : "m"), &signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'F': case 'f':
					cdl_emit(output[i], outfile, "float", "beamflag", "Beamflag",
					(bathy_in_feet ? "f" : "m"), &signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'G':
					cdl_emit(output[i], outfile, "float", "flatgrazing", "Flat bottom grazing angle",
					(invert_next_value ? "tangent of angle from beam to vertical" : "tangent of angle from beam to horizontal"),
					&signflip_next_value, &invert_next_value, true, true, true, false); break;
				case 'g':
					cdl_emit(output[i], outfile, "float", "grazing", "Grazing angle using slope",
					(invert_next_value ? "tangent of angle from beam to perpendicular to seafloor" : "tangent of angle from beam to seafloor"),
					&signflip_next_value, &invert_next_value, true, true, true, false); break;
				case 'H':
					cdl_emit(output[i], outfile, "float", "heading", "Heading", "degrees true",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'h':
					cdl_emit(output[i], outfile, "float", "course", "Course", "degrees true",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'J':
					strcpy(variable, "time_J"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data,timefields_J);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Time - year julian_day hour minute seconds\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"year, julian day, hour, minute, second, nanosecond\";\n", variable); break;
				case 'j':
					strcpy(variable, "time_j"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data,timefields_j);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Time - year julian_day minute seconds\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"year, julian day, minute, second, nanosecond\";\n", variable); break;
				case 'K': strcpy(variable, "goodbeamfraction"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Good beam fraction of non-null beams\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"number of good beams divided by number of non-null beams\";\n", variable);
					signflip_next_value = false; invert_next_value = false; break;
				case 'k':
					strcpy(variable, "goodbeamfractionall"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Good beam fraction of all possible beams\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"number of good beams divided by number of possible beams\";\n", variable);
					signflip_next_value = false; invert_next_value = false; break;
				case 'L':
					cdl_emit(output[i], outfile, "float", "along_track", "Alongtrack distance", "km",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'l':
					cdl_emit(output[i], outfile, "float", "along_track_m", "Alongtrack distance", "m",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'M':
					cdl_emit(output[i], outfile, "double", "unix_time", "Seconds since 1/1/70 00:00:00", "s",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'm':
					cdl_emit(output[i], outfile, "double", "survey_time", "Seconds since first record", "s",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'N':
					strcpy(variable, "ping");
					fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data);\n\t\t%s:long_name = \"Ping counter\";\n\t\t%s:units = \"pings\";\n", variable, variable, variable); break;
				case 'P':
					cdl_emit(output[i], outfile, "float", "pitch", "Pitch", "degrees from horizontal",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'p':
					cdl_emit(output[i], outfile, "float", "draft", "Draft", "m",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'q': case 'Q': strcpy(variable, "bottom_detect_type"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data);\n\t\t%s:long_name = \"Bottom detect type\";\n\t\t%s:units = \"0=unknown,1=amplitude,2=phase\";\n", variable, variable, variable); break;
				case 'R': cdl_emit(output[i], outfile, "float", "roll", "Roll", "degrees from horizontal",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'r':
					cdl_emit(output[i], outfile, "float", "heave", "Heave", "m",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'S':
					cdl_emit(output[i], outfile, "float", "speed", "Speed", "km/hr",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 's':
					cdl_emit(output[i], outfile, "float", "speed_made_good", "Speed made good", "km/hr",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'T':
					strcpy(variable, "time_T"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tchar %s(data,timestring);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Time string - year/month/day/hour/minute/seconds\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"yyyy/MM/dd/hh/mm/ss.ssssss\";\n", variable); break;
				case 't':
					strcpy(variable, "time_t");
					fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data,timefields_t);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Time - year month day hour minute seconds\";\n", variable);
					fprintf(outfile, "\t\t%s:units = \"year, month, day, hour, minute, second, nanosecond\";\n", variable); break;
				case 'U':
					strcpy(variable, "unix_time_s");
					fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Integer seconds since 1/1/70 00:00:00\";\n\t\t%s:units = \"s\";\n", variable, variable); break;
				case 'u':
					strcpy(variable, "survey_time_s");
					fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Integer seconds since first record\";\n\t\t%s:units = \"s\";\n", variable, variable); break;
				case 'V': case 'v':
					strcpy(variable, "ping_time");
					fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tfloat %s(data);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Seconds since last ping\";\n\t\t%s:units = \"s\";\n", variable, variable); break;
				case 'X':
					cdl_emit(output[i], outfile, "double", "longitude", "Longitude", "degrees",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'x':
					strcpy(variable, "longitude_minutes");
					fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tchar %s(data,latm);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Longitude - decimal minutes\";\n\t\t%s:units = \"ddd mm.mmmmmH\";\n", variable, variable); break;
				case 'Y':
					cdl_emit(output[i], outfile, "double", "latitude", "Latitude", "degrees",
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'y':
					strcpy(variable, "latitude_minutes");
					fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tchar %s(data,latm);\n", variable);
					fprintf(outfile, "\t\t%s:long_name = \"Latitude - decimal minutes\";\n\t\t%s:units = \"ddd mm.mmmmmH\";\n", variable, variable); break;
				case 'Z':
					cdl_emit(output[i], outfile, "float", "topography", "Topography", (bathy_in_feet ? "f" : "m"),
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case 'z': 
					cdl_emit(output[i], outfile, "float", "depth", "Depth", (bathy_in_feet ? "f" : "m"),
					&signflip_next_value, &invert_next_value, true, true, true, true); break;
				case '#':
					strcpy(variable, "beam");
					fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data);\n\t\t%s:long_name = \"Beam number\";\n\t\t%s:units = \"number\";\n", variable, variable, variable); break;
				}
			}
			else if (ttimes_next_value) {
				switch (list[i]) {
				case '/': invert_next_value = true; break;
				case '-': signflip_next_value = true; break;
				case '_': sensornav_next_value = true; break;
				case '@': sensorrelative_next_value = true; break;
				case '^': projectednav_next_value = true; break;
				case ',': ttimes_next_value = true; count = 0; break;
				case '.': raw_next_value = true; count = 0; break;
				case '=': case '+': break;
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
					count = count * 10 + list[i] - '0'; break;
				case 'A': cdl_emit(output[i], outfile, "float", "tt_angles", "Beam Angle", "degrees",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'a': cdl_emit(output[i], outfile, "float", "tt_angles_forward", "Beam angle forward", "degrees",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'D': cdl_emit(output[i], outfile, "float", "tt_sensordepth", "Beam sensor depth", "meters",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'H': cdl_emit(output[i], outfile, "float", "tt_heave", "Beam heave", "meters",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'N': cdl_emit(output[i], outfile, "float", "tt_angles_null", "Beam angle null", "degrees",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'O': cdl_emit(output[i], outfile, "float", "tt_alongtrack_offset", "Beam alongtrack offset", "meters",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'R': cdl_emit(output[i], outfile, "float", "tt_range", "Beam range", "meters",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'S': cdl_emit(output[i], outfile, "float", "tt_ssv", "Survey sound velocity", "meters/second",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'T': cdl_emit(output[i], outfile, "float", "tt_ttimes", "Beam travel time", "seconds",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				default: ttimes_next_value = false; raw_next_value = false; break;
				}
			}
			else /* raw_next_value */ {
				switch (list[i]) {
				case '/': invert_next_value = true; break;
				case '-': signflip_next_value = true; break;
				case '_': sensornav_next_value = true; break;
				case '@': sensorrelative_next_value = true; break;
				case '^': projectednav_next_value = true; break;
				case ',': ttimes_next_value = true; count = 0; break;
				case '.': raw_next_value = true; count = 0; break;
				case '=': case '+': break;
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
					count = count * 10 + list[i] - '0'; break;
				case 'A': cdl_emit(output[i], outfile, "float", "backscatter", "Backscatter", "dB",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'a': cdl_emit(output[i], outfile, "float", "absorption", "Mean absorption", "dB/km",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'B': cdl_emit(output[i], outfile, "float", "bsn", "Normal incidence backscatter", "dB",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'b': cdl_emit(output[i], outfile, "float", "bso", "Oblique backscatter", "dB",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'c': cdl_emit(output[i], outfile, "float", "mback", "Mean backscatter",
					((format == MBF_EM300RAW || format == MBF_EM300MBA) ? "dB + 64" : "backscatter"),
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'd': cdl_emit(output[i], outfile, "float", "depression", "Beam depression angle", "degrees",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'F': strcpy(variable, "filename"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tchar %s(data,pathsize);\n\t\t%s:long_name = \"Name of swath data file\";\n\t\t%s:units = \"file name\";\n", variable, variable, variable);
					signflip_next_value = false; invert_next_value = false;
					ttimes_next_value = false; raw_next_value = false; break;
				case 'f': strcpy(variable, "format"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tshort %s(data);\n\t\t%s:long_name = \"MBsystem file format number\";\n\t\t%s:units = \"see mbformat\";\n", variable, variable, variable);
					signflip_next_value = false; invert_next_value = false;
					ttimes_next_value = false; raw_next_value = false; break;
				case 'G': cdl_emit(output[i], outfile, "float", "tvg_start", "Start range of TVG ramp", "samples",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'g': cdl_emit(output[i], outfile, "float", "tvg_stop", "Stop range of TVG ramp", "samples",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'L': strcpy(variable, "pulse_length"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data);\n\t\t%s:long_name = \"Pulse Length\";\n\t\t%s:units = \"us", variable, variable, variable);
					signflip_next_value = false; invert_next_value = false;
					ttimes_next_value = false; raw_next_value = false; break;
				case 'l': cdl_emit(output[i], outfile, "float", "pulse_length", "Pulse length", "seconds",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'M': strcpy(variable, "mode"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data);\n\t\t%s:long_name = \"Sounder mode\";\n\t\t%s:units = \"0=very shallow,1=shallow,2=medium,3=deep,4=very deep,5=extra deep\";\n", variable, variable, variable);
					signflip_next_value = false; invert_next_value = false;
					ttimes_next_value = false; raw_next_value = false; break;
				case 'N': strcpy(variable, "ping_no"); fprintf(output[i], "\t%s = ", variable);
					fprintf(outfile, "\tlong %s(data);\n\t\t%s:long_name = \"Sounder ping counter\";\n\t\t%s:units = \"pings\";\n", variable, variable, variable);
					signflip_next_value = false; invert_next_value = false;
					ttimes_next_value = false; raw_next_value = false; break;
				case 'p': strcpy(variable, "sidescan");
					if (signflip_next_value) strcat(variable, "-");
					if (invert_next_value) strcat(variable, "_");
					fprintf(output[i], "\t%s = ", variable);
					if (count == 0) fprintf(outfile, "\tfloat %s(data);\n", variable);
					else fprintf(outfile, "\tfloat %s(data, %c);\n", variable, list[i]);
					fprintf(outfile, "\t\t%s:long_name = \"Raw sidescan pixels\";\n\t\t%s:units = \"", variable, variable);
					if (signflip_next_value) fprintf(outfile, "-");
					if (invert_next_value) fprintf(outfile, "1/");
					fprintf(outfile, "dB\";\n");
					signflip_next_value = false; invert_next_value = false;
					ttimes_next_value = false; raw_next_value = false; break;
				case 'R': cdl_emit(output[i], outfile, "float", "range", "Range ", "samples",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'r': cdl_emit(output[i], outfile, "float", "sample_rate", "Sample Rate", "Hertz",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'S': cdl_emit(output[i], outfile, "float", "pixels", "Total sidescan pixels ", "pixels",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 's': cdl_emit(output[i], outfile, "float", "beam_pixels", "Sidescan pixels per beam", "pixels",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 'T': cdl_emit(output[i], outfile, "float", "transmit_gain", "Transmit gain", "dB",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				case 't': cdl_emit(output[i], outfile, "float", "receive_gain", "Receive gain", "dB",
					&signflip_next_value, &invert_next_value, true, true, true, true);
					ttimes_next_value = false; raw_next_value = false; break;
				default: ttimes_next_value = false; raw_next_value = false; break;
				}
			}
		}
		fprintf(outfile, "\n\ndata:\n");
	}

	bool use_course = false, use_time_interval = false, use_swathbounds = false;
	double time_d_ref = 0;

	while (read_data) {
		if (mb_read_init_altnav(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin,
		                        timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d,
		                        &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nmb_read_init_altnav failed: %s\nFile: %s\n", message, file);
			Return(error);
		}

		if (dump_mode == DUMP_MODE_BATH || dump_mode == DUMP_MODE_TOPO) use_bath = true;
		else if (dump_mode == DUMP_MODE_AMP) use_amp = true;
		else if (dump_mode == DUMP_MODE_SS)  use_ss = true;
		else {
			for (int i = 0; i < n_list; i++) {
				if (!raw_next_value && !ttimes_next_value) {
					if (list[i] == 'Z' || list[i] == 'z' || list[i] == 'A' || list[i] == 'a' ||
					    list[i] == 'Q' || list[i] == 'q') use_bath = true;
					if (list[i] == 'B') use_amp = true;
					if (list[i] == 'b') use_ss = true;
					if (list[i] == 'h') use_course = true;
					if (list[i] == 's') use_course = true;
					if (list[i] == 'V' || list[i] == 'v') use_time_interval = true;
					if (list[i] == 'A' || list[i] == 'a' || list[i] == 'G' || list[i] == 'g') use_slope = true;
					if (list[i] == 'P' || list[i] == 'p' || list[i] == 'R' || list[i] == 'r') use_attitude = true;
					if (list[i] == 'Q' || list[i] == 'q') use_detects = true;
					if (list[i] == 'N') use_pingnumber = true;
					if (list[i] == 'n') use_linenumber = true;
					if (list[i] == ',') ttimes_next_value = true;
					if (list[i] == '.') raw_next_value = true;
					if (list[i] == '=' || list[i] == '+') use_swathbounds = true;
				}
				else if (raw_next_value) {
					if (list[i] == 'T' || list[i] == 't' || list[i] == 'U' || list[i] == 'l') use_gains = true;
					else if (list[i] == 'F' || list[i] == 'f') { /* skip */ }
					else {
						use_raw = true;
						if (list[i] == 'R' || list[i] == 'd') use_bath = true;
						if (list[i] == 'B' || list[i] == 'b' || list[i] == 'c') use_amp = true;
					}
					if (list[i] != '/' && list[i] != '-' && list[i] != '.') raw_next_value = false;
				}
				else if (ttimes_next_value) {
					if (list[i] == 'A' || list[i] == 'a' || list[i] == 'D' || list[i] == 'H' ||
					    list[i] == 'N' || list[i] == 'O' || list[i] == 'R' || list[i] == 'S' ||
					    list[i] == 'T' || list[i] == 'V' || list[i] == 'v') use_ttimes = true;
					if (list[i] != '/' && list[i] != '-' && list[i] != ',') ttimes_next_value = false;
				}
			}
		}
		if (check_values == MBLIST_CHECK_ON || check_values == MBLIST_CHECK_ON_NULL) {
			if (use_bath) check_bath = true;
			if (use_amp)  check_amp = true;
			if (use_ss)   check_ss = true;
		}

		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),   (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,  sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,   sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,   sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,   sizeof(double), (void **)&ssalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depths, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depthacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2*sizeof(double), (void **)&slopes, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2*sizeof(double), (void **)&slopeacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2*sizeof(int),    (void **)&detect, &error);
		if (use_ttimes) {
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&tt_ttimes, &error);
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&tt_angles, &error);
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&tt_angles_forward, &error);
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&tt_angles_null, &error);
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&tt_heave, &error);
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&tt_alongtrack_offset, &error);
		}
		if (use_raw) {
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int),    (void **)&beam_samples, &error);
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int),    (void **)&start_sample, &error);
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int),    (void **)&range, &error);
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depression, &error);
			if (error == MB_ERROR_NO_ERROR) mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bs, &error);
			mb_mallocd(verbose, __FILE__, __LINE__, MBSYS_SIMRAD2_MAXRAWPIXELS * sizeof(double), (void **)&ss_pixels, &error);
		}

		if (error != MB_ERROR_NO_ERROR) {
			char *message; mb_error(verbose, error, &message);
			fprintf(stderr, "\nAlloc failed: %s\n", message);
			Return(error);
		}

		if (segment && ascii && !netcdf) {
			if (segment_mode == MBLIST_SEGMENT_MODE_TAG)        fprintf(output[0], "%s\n", segment_tag);
			else if (segment_mode == MBLIST_SEGMENT_MODE_SWATHFILE) fprintf(output[0], "# %s\n", file);
			else if (segment_mode == MBLIST_SEGMENT_MODE_DATALIST)  fprintf(output[0], "# %s\n", dpath);
		}

		int nread = 0;
		bool first = true;
		while (error <= MB_ERROR_NO_ERROR) {
			error = MB_ERROR_NO_ERROR;
			if (pings == 1 || use_attitude || use_detects || use_pingnumber || use_linenumber || use_raw || use_ttimes) {
				status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
				                    &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
				                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);
				if (error == MB_ERROR_TIME_GAP) { error = MB_ERROR_NO_ERROR; status = MB_SUCCESS; }
				if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
					double tnavlon, tnavlat, tspeed, theading;
					status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &tnavlon, &tnavlat,
					                        &tspeed, &theading, &draft, &roll, &pitch, &heave, &error);
				}
				if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_detects) {
					nbeams = beams_bath;
					status = mb_detects(verbose, mbio_ptr, store_ptr, &kind, &nbeams, detect, &error);
				}
				if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_pingnumber)
					status = mb_pingnumber(verbose, mbio_ptr, &pingnumber, &error);
				if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_linenumber) {
					unsigned int cdpnumber;
					status = mb_segynumber(verbose, mbio_ptr, &linenumber, &pingnumber, &cdpnumber, &error);
				}
				if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_ttimes) {
					status = mb_ttimes(verbose, mbio_ptr, store_ptr, &tt_kind, &tt_nbeams, tt_ttimes, tt_angles, tt_angles_forward,
					                   tt_angles_null, tt_heave, tt_alongtrack_offset, &tt_sensordepth, &tt_ssv, &error);
				}
				if (error == MB_ERROR_NO_ERROR && use_raw) {
					status = mb_get_raw(verbose, mbio_ptr, &mode_v, &ipulse_length, &png_count, &sample_rate, &absorption, &max_range,
					                    &r_zero, &r_zero_corr, &tvg_start, &tvg_stop, &bsn_v, &bso_v, &tx, &tvg_crossover,
					                    &nbeams_ss, &npixels, beam_samples, start_sample, range, depression, bs, ss_pixels, &error);
				}
				if (error == MB_ERROR_NO_ERROR && use_gains) {
					status = mb_gains(verbose, mbio_ptr, store_ptr, &kind, &transmit_gain, &pulse_length, &receive_gain, &error);
				}
			} else {
				status = mb_get(verbose, mbio_ptr, &kind, &pings_read, time_i, &time_d, &navlon, &navlat, &speed, &heading,
				                &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
				                bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);
				if (error == MB_ERROR_TIME_GAP) { error = MB_ERROR_NO_ERROR; status = MB_SUCCESS; }
			}

			if (error == MB_ERROR_NO_ERROR && kind != MB_DATA_DATA) error = MB_ERROR_OTHER;
			if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
				nread++;
				if (!use_pingnumber) pingnumber = nread;
				distance_total += distance;
			}

			if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_projection) {
				if (pjptr == NULL) {
					if (strlen(projection_pars) == 0) strcpy(projection_pars, "U");
					if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 ||
					    strcmp(projection_pars, "utm") == 0 || strcmp(projection_pars, "u") == 0) {
						reference_lon = navlon;
						if (reference_lon < 180.0)  reference_lon += 360.0;
						if (reference_lon >= 180.0) reference_lon -= 360.0;
						utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
						reference_lat = navlat;
						if (reference_lat >= 0.0) snprintf(projection_id, sizeof(projection_id), "UTM%2.2dN", utm_zone);
						else                       snprintf(projection_id, sizeof(projection_id), "UTM%2.2dS", utm_zone);
					} else strcpy(projection_id, projection_pars);
					proj_status = mb_proj_init(verbose, projection_id, &(pjptr), &error);
					if (proj_status != MB_SUCCESS) {
						fprintf(stderr, "\nOutput projection %s not found in database\n", projection_id);
						error = MB_ERROR_BAD_PARAMETER;
						mb_memory_clear(verbose, &error);
						Return(MB_ERROR_BAD_PARAMETER);
					}
				}
				mb_proj_forward(verbose, pjptr, navlon, navlat, &naveasting, &navnorthing, &error);
			}

			if (verbose >= 2) {
				fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", THIS_MODULE_NAME);
				fprintf(stderr, "dbg2       kind:           %d\n", kind);
				fprintf(stderr, "dbg2       error:          %d\n", error);
				fprintf(stderr, "dbg2       status:         %d\n", status);
			}

			if (verbose >= 2 && kind == MB_DATA_COMMENT) {
				if (icomment == 0) {
					fprintf(stderr, "\nComments:\n");
					icomment++;
				}
				fprintf(stderr, "%s\n", comment);
			}

			int beams_null = 0, beams_flagged = 0, beams_unflagged = 0;
			double goodbeamfraction = 0.0;
			for (int kk = 0; kk < beams_bath; kk++) {
				if (mb_beam_ok(beamflag[kk])) beams_unflagged++;
				else if (beamflag[kk] == MB_FLAG_NULL) beams_null++;
				else beams_flagged++;
			}

			if (error == MB_ERROR_NO_ERROR) {
				status = mb_swathbounds(verbose, true, beams_bath, pixels_ss,
				                        beamflag, bathacrosstrack, ss, ssacrosstrack,
				                        &beam_port, &beam_vertical, &beam_stbd,
				                        &pixel_port, &pixel_vertical, &pixel_stbd, &error);
				status &= set_output(verbose, beams_bath, beams_amp, pixels_ss, use_bath, use_amp, use_ss, dump_mode, beam_set,
				                     pixel_set, beam_vertical, pixel_vertical, &beam_start, &beam_end, &beam_exclude_percent,
				                     &pixel_start, &pixel_end, &n_list, list, &error);
				if (status == MB_FAILURE) Return(error);

				if (verbose >= 2) {
					fprintf(stderr, "\ndbg2  Beams set for output in <%s>\n", THIS_MODULE_NAME);
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
					for (int i = 0; i < n_list; i++)
						fprintf(stderr, "dbg2       list[%d]:      %c\n", i, list[i]);
				}
			}

			if (error == MB_ERROR_NO_ERROR) {
				mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
				headingx = sin(DTR * heading);
				headingy = cos(DTR * heading);
			}

			if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && first) time_interval = 0.0;
			else if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) time_interval = time_d - time_d_old;

			if (error == MB_ERROR_NO_ERROR && use_course) {
				if (first) {
					course = heading; speed_made_good = speed;
					course_old = heading; speed_made_good_old = speed;
				} else {
					dx = (navlon - navlon_old) / mtodeglon;
					dy = (navlat - navlat_old) / mtodeglat;
					dist = sqrt(dx * dx + dy * dy);
					if (dist > 0.0) course = RTD * atan2(dx / dist, dy / dist);
					else course = course_old;
					if (course < 0.0) course += 360.0;
					dt = (time_d - time_d_old);
					if (dt > 0.0) speed_made_good = 3.6 * dist / dt;
					else speed_made_good = speed_made_good_old;
				}
			}

			if (error == MB_ERROR_NO_ERROR && use_slope) {
				ns = 0; sx = 0.0; sy = 0.0; sxx = 0.0; sxy = 0.0;
				for (int kk = 0; kk < beams_bath; kk++)
					if (mb_beam_ok(beamflag[kk])) {
						sx += bathacrosstrack[kk]; sy += bath[kk];
						sxx += bathacrosstrack[kk] * bathacrosstrack[kk];
						sxy += bathacrosstrack[kk] * bath[kk];
						ns++;
					}
				if (ns > 0) {
					delta = ns * sxx - sx * sx;
					b = (ns * sxy - sx * sy) / delta;
					avgslope = RTD * atan(b);
				} else avgslope = 0.0;
				set_bathyslope(verbose, beams_bath, beamflag, bath, bathacrosstrack, &ndepths, depths, depthacrosstrack,
				               &nslopes, slopes, slopeacrosstrack, &error);
			}

			if (error == MB_ERROR_NO_ERROR) {
				navlon_old = navlon; navlat_old = navlat;
				course_old = course; speed_made_good_old = speed_made_good;
				time_d_old = time_d;
			}

			if (error == MB_ERROR_NO_ERROR && (nread - 1) % decimate == 0) {
				for (int j = beam_start; j <= beam_end; j++) {
					beam_status = MB_SUCCESS;
					if (check_bath && check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[j])) beam_status = MB_FAILURE;
					else if (check_bath && check_values == MBLIST_CHECK_ON_NULL && beamflag[j] == MB_FLAG_NULL) beam_status = MB_FAILURE;
					if (check_amp && check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[j])) beam_status = MB_FAILURE;
					else if (check_amp && check_values == MBLIST_CHECK_ON_NULL && beamflag[j] == MB_FLAG_NULL) beam_status = MB_FAILURE;
					if (check_ss && j != beam_vertical) beam_status = MB_FAILURE;
					else if (check_ss && j == beam_vertical && ss[pixel_vertical] <= MB_SIDESCAN_NULL) beam_status = MB_FAILURE;
					if (use_time_interval && first) beam_status = MB_FAILURE;
					if (check_nav && (navlon == 0.0 || navlat == 0.0)) beam_status = MB_FAILURE;

					if (beam_status != MB_SUCCESS) continue;

					signflip_next_value = false; invert_next_value = false;
					ttimes_next_value = false; raw_next_value = false;
					sensornav_next_value = false; sensorrelative_next_value = false;
					projectednav_next_value = false; special_character = false;

					for (int i = 0; i < n_list; i++) {
						if (netcdf && lcount > 0) fprintf(output[i], ", ");
						int k;
						if (port_next_value) { k = beam_port; port_next_value = false; }
						else if (stbd_next_value) { k = beam_stbd; stbd_next_value = false; }
						else k = j;

						if (!ttimes_next_value && !raw_next_value) {
							switch (list[i]) {
							case '/': invert_next_value = true; special_character = true; break;
							case '-': signflip_next_value = true; special_character = true; break;
							case '_': sensornav_next_value = true; special_character = true; break;
							case '@': sensorrelative_next_value = true; special_character = true; break;
							case '^': projectednav_next_value = true; special_character = true; break;
							case ',': ttimes_next_value = true; special_character = true; count = 0; break;
							case '.': raw_next_value = true; special_character = true; count = 0; break;
							case '=': port_next_value = true; special_character = true; break;
							case '+': stbd_next_value = true; special_character = true; break;
							case 'A': printsimplevalue(verbose, output[i], avgslope, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'a':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else {
									status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
									                        slopeacrosstrack, bathacrosstrack[k], &depth, &slope, &error);
									printsimplevalue(verbose, output[i], slope, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								break;
							case 'B':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], amp[k], 0, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'b': printsimplevalue(verbose, output[i], ss[pixel_vertical], 0, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'C': printsimplevalue(verbose, output[i], altitude, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'c': printsimplevalue(verbose, output[i], sensordepth, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'D': case 'd':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else { b = bathy_scale * bathacrosstrack[k]; printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); }
								break;
							case 'E': case 'e':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else { b = bathy_scale * bathalongtrack[k]; printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); }
								break;
							case 'F':
								if (ascii) fprintf(output[i], "%d", beamflag[k]);
								else { b = beamflag[k]; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							case 'f':
								if (ascii) {
									if (netcdf) fprintf(output[i], "%d", beamflag[k]);
									else {
										if (mb_beam_check_flag_unusable(beamflag[k])) fprintf(output[i], "-");
										else if (mb_beam_ok(beamflag[k])) fprintf(output[i], "G");
										else if (mb_beam_check_flag_manual(beamflag[k])) fprintf(output[i], "M");
										else if (mb_beam_check_flag_filter(beamflag[k])) fprintf(output[i], "F");
										else if (mb_beam_check_flag_filter2(beamflag[k])) fprintf(output[i], "F");
										else if (mb_beam_check_flag_multipick(beamflag[k])) fprintf(output[i], "N");
										else if (mb_beam_check_flag_interpolate(beamflag[k])) fprintf(output[i], "I");
										else if (mb_beam_check_flag_sonar(beamflag[k])) fprintf(output[i], "S");
									}
								} else { b = beamflag[k]; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							case 'G':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else { angle = RTD * atan(bathacrosstrack[k] / (bath[k] - sensordepth));
									printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value, &signflip_next_value, &error); }
								break;
							case 'g':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else {
									status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes, slopeacrosstrack, bathacrosstrack[k], &depth, &slope, &error);
									angle = RTD * atan(bathacrosstrack[k] / (bath[k] - sensordepth)) + slope;
									printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								break;
							case 'H': printsimplevalue(verbose, output[i], heading, 7, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'h': printsimplevalue(verbose, output[i], course, 7, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'J':
								mb_get_jtime(verbose, time_i, time_j);
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									if (netcdf) fprintf(output[i], "%d, %d, %d, %d, %d, %d", time_j[0], time_j[1], time_i[3], time_i[4], time_i[5], time_i[6]);
									else        fprintf(output[i], "%.4d %.3d %.2d %.2d %9.6f", time_j[0], time_j[1], time_i[3], time_i[4], seconds);
								} else {
									b = time_j[0]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[1]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[3]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[4]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[5]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[6]; fwrite(&b, sizeof(double), 1, outfile);
								}
								break;
							case 'j':
								mb_get_jtime(verbose, time_i, time_j);
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									if (netcdf) fprintf(output[i], "%d, %d, %d, %d, %d", time_j[0], time_j[1], time_j[2], time_j[3], time_j[4]);
									else        fprintf(output[i], "%.4d %.3d %.4d %9.6f", time_j[0], time_j[1], time_j[2], seconds);
								} else {
									b = time_j[0]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[1]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[2]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[3]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[4]; fwrite(&b, sizeof(double), 1, outfile);
								}
								break;
							case 'K':
								goodbeamfraction = (beams_bath - beams_null > 0) ? ((double)beams_unflagged) / ((double)(beams_bath - beams_null)) : 0.0;
								printsimplevalue(verbose, output[i], goodbeamfraction, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'k':
								goodbeamfraction = (beams_bath > 0) ? ((double)beams_unflagged) / ((double)beams_bath) : 0.0;
								printsimplevalue(verbose, output[i], goodbeamfraction, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'L': printsimplevalue(verbose, output[i], distance_total, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'l': printsimplevalue(verbose, output[i], 1000.0 * distance_total, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'M': printsimplevalue(verbose, output[i], time_d, 0, 6, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'm':
								if (first_m) { time_d_ref = time_d; first_m = false; }
								b = time_d - time_d_ref;
								printsimplevalue(verbose, output[i], b, 0, 6, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'N': if (ascii) fprintf(output[i], "%6u", pingnumber); else { b = pingnumber; fwrite(&b, sizeof(double), 1, outfile); } break;
							case 'n': if (ascii) fprintf(output[i], "%6u", linenumber); else { b = linenumber; fwrite(&b, sizeof(double), 1, outfile); } break;
							case 'P': printsimplevalue(verbose, output[i], pitch, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'p': printsimplevalue(verbose, output[i], draft, 7, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'q':
								if (ascii) {
									if (netcdf) fprintf(output[i], "\"");
									fprintf(output[i], "%d", detect[k]);
									if (netcdf) fprintf(output[i], "\"");
								} else { b = detect[k]; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							case 'Q':
								if (ascii) {
									if (netcdf) { fprintf(output[i], "\"%d\"", detect[k]); }
									else {
										if (detect[k] == MB_DETECT_AMPLITUDE) fprintf(output[i], "A");
										else if (detect[k] == MB_DETECT_PHASE) fprintf(output[i], "P");
										else fprintf(output[i], "U");
									}
								} else { b = detect[k]; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							case 'R': printsimplevalue(verbose, output[i], roll, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'r': printsimplevalue(verbose, output[i], heave, 7, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'S': printsimplevalue(verbose, output[i], speed, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 's': printsimplevalue(verbose, output[i], speed_made_good, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'T':
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									if (netcdf) fprintf(output[i], "\"");
									fprintf(output[i], "%.4d/%.2d/%.2d/%.2d/%.2d/%09.6f", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], seconds);
									if (netcdf) fprintf(output[i], "\"");
								} else {
									b = time_i[0]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[1]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[2]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[3]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[4]; fwrite(&b, sizeof(double), 1, outfile);
									b = seconds; fwrite(&b, sizeof(double), 1, outfile);
								}
								break;
							case 't':
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									if (netcdf) fprintf(output[i], "%d, %d, %d, %d, %d, %d, %d", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
									else fprintf(output[i], "%.4d %.2d %.2d %.2d %.2d %09.6f", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], seconds);
								} else {
									b = time_i[0]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[1]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[2]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[3]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[4]; fwrite(&b, sizeof(double), 1, outfile);
									b = seconds; fwrite(&b, sizeof(double), 1, outfile);
								}
								break;
							case 'U':
								time_u = (int)time_d;
								if (ascii) fprintf(output[i], "%lld", (long long)time_u);
								else { b = time_u; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							case 'u':
								time_u = (int)time_d;
								if (first_u) { time_u_ref = time_u; first_u = false; }
								if (ascii) fprintf(output[i], "%lld", (long long)(time_u - time_u_ref));
								else { b = time_u - time_u_ref; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							case 'V': case 'v':
								if (ascii) {
									if (fabs(time_interval) > 100.) fprintf(output[i], "%g", time_interval);
									else fprintf(output[i], "%10.6f", time_interval);
								} else fwrite(&time_interval, sizeof(double), 1, outfile);
								break;
							case 'X':
								if (!projectednav_next_value) {
									if (sensorrelative_next_value) dlon = 0.0; else dlon = navlon;
									if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
										dlon += headingy * mtodeglon * bathacrosstrack[k] + headingx * mtodeglon * bathalongtrack[k];
									printsimplevalue(verbose, output[i], dlon, 15, 10, ascii, &invert_next_value, &signflip_next_value, &error);
								} else {
									if (sensorrelative_next_value) deasting = 0.0; else deasting = naveasting;
									if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
										deasting += headingy * bathacrosstrack[k] + headingx * bathalongtrack[k];
									printsimplevalue(verbose, output[i], deasting, 15, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								sensornav_next_value = false; sensorrelative_next_value = false; projectednav_next_value = false;
								break;
							case 'x':
								dlon = navlon;
								if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
									dlon += headingy * mtodeglon * bathacrosstrack[k] + headingx * mtodeglon * bathalongtrack[k];
								if (dlon < 0.0) { hemi = 'W'; dlon = -dlon; } else hemi = 'E';
								degrees = (int)dlon; minutes = 60.0 * (dlon - degrees);
								if (ascii) {
									if (netcdf) fprintf(output[i], "\"");
									fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
									if (netcdf) fprintf(output[i], "\"");
								} else {
									b = degrees; if (hemi == 'W') b = -b; fwrite(&b, sizeof(double), 1, outfile);
									b = minutes; fwrite(&b, sizeof(double), 1, outfile);
								}
								sensornav_next_value = false;
								break;
							case 'Y':
								if (!projectednav_next_value) {
									if (sensorrelative_next_value) dlat = 0.0; else dlat = navlat;
									if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
										dlat += -headingx * mtodeglat * bathacrosstrack[k] + headingy * mtodeglat * bathalongtrack[k];
									printsimplevalue(verbose, output[i], dlat, 15, 10, ascii, &invert_next_value, &signflip_next_value, &error);
								} else {
									if (sensorrelative_next_value) dnorthing = 0.0; else dnorthing = navnorthing;
									if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
										dnorthing += -headingx * bathacrosstrack[k] + headingy * bathalongtrack[k];
									printsimplevalue(verbose, output[i], dnorthing, 15, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								sensornav_next_value = false; sensorrelative_next_value = false; projectednav_next_value = false;
								break;
							case 'y':
								dlat = navlat;
								if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
									dlat += -headingx * mtodeglat * bathacrosstrack[k] + headingy * mtodeglat * bathalongtrack[k];
								if (dlat < 0.0) { hemi = 'S'; dlat = -dlat; } else hemi = 'N';
								degrees = (int)dlat; minutes = 60.0 * (dlat - degrees);
								if (ascii) {
									if (netcdf) fprintf(output[i], "\"");
									fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
									if (netcdf) fprintf(output[i], "\"");
								} else {
									b = degrees; if (hemi == 'S') b = -b; fwrite(&b, sizeof(double), 1, outfile);
									b = minutes; fwrite(&b, sizeof(double), 1, outfile);
								}
								sensornav_next_value = false;
								break;
							case 'Z':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else {
									b = -bathy_scale * bath[k];
									if (sensorrelative_next_value) b -= -bathy_scale * sensordepth;
									printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								sensornav_next_value = false; sensorrelative_next_value = false;
								break;
							case 'z':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else {
									b = bathy_scale * bath[k];
									if (sensorrelative_next_value) b -= bathy_scale * sensordepth;
									printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								sensornav_next_value = false; sensorrelative_next_value = false;
								break;
							case '#':
								if (ascii) fprintf(output[i], "%6d", k);
								else { b = k; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							default: if (ascii) fprintf(output[i], "<Invalid Option: %c>", list[i]); break;
							}
						}
						else if (ttimes_next_value) {
							switch (list[i]) {
							case '/': invert_next_value = true; special_character = true; break;
							case '-': signflip_next_value = true; special_character = true; break;
							case '_': sensornav_next_value = true; special_character = true; break;
							case '@': sensorrelative_next_value = true; special_character = true; break;
							case '^': projectednav_next_value = true; special_character = true; break;
							case ',': ttimes_next_value = true; special_character = true; count = 0; break;
							case '.': raw_next_value = true; special_character = true; count = 0; break;
							case '=': port_next_value = true; special_character = true; break;
							case '+': stbd_next_value = true; special_character = true; break;
							case '0': case '1': case '2': case '3': case '4':
							case '5': case '6': case '7': case '8': case '9':
								count = count * 10 + list[i] - '0'; break;
							case 'A':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_angles[k], 5, 2, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							case 'a':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_angles_forward[k], 5, 2, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							case 'D':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_sensordepth, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							case 'H':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_heave[k], 7, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							case 'N':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_angles_null[k], 5, 2, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							case 'O':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_alongtrack_offset[k], 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							case 'R':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], 0.5 * tt_ttimes[k] * tt_ssv, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							case 'S':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_ssv, 9, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							case 'T':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_ttimes[k], 0, 6, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							default: if (ascii) fprintf(output[i], "<Invalid Option: %c>", list[i]); ttimes_next_value = false; break;
							}
						}
						else /* raw_next_value */ {
							switch (list[i]) {
							case '/': invert_next_value = true; special_character = true; break;
							case '-': signflip_next_value = true; special_character = true; break;
							case '_': sensornav_next_value = true; special_character = true; break;
							case '@': sensorrelative_next_value = true; special_character = true; break;
							case '^': projectednav_next_value = true; special_character = true; break;
							case ',': ttimes_next_value = true; special_character = true; count = 0; break;
							case '.': raw_next_value = true; special_character = true; count = 0; break;
							case '=': port_next_value = true; special_character = true; break;
							case '+': stbd_next_value = true; special_character = true; break;
							case '0': case '1': case '2': case '3': case '4':
							case '5': case '6': case '7': case '8': case '9':
								count = count * 10 + list[i] - '0'; break;
							case 'A':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], bs[k], 5, 1, ascii, &invert_next_value, &signflip_next_value, &error);
								raw_next_value = false; break;
							case 'a': printsimplevalue(verbose, output[i], absorption, 5, 2, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 'B': printsimplevalue(verbose, output[i], bsn_v, 5, 2, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 'b': printsimplevalue(verbose, output[i], bso_v, 5, 2, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 'c':
								mback = 0; nback = 0;
								for (int m = 0; m < beams_amp; m++) if (mb_beam_ok(beamflag[m])) { mback += amp[m]; nback++; }
								printsimplevalue(verbose, output[i], (nback > 0 ? mback / nback : 0.0), 5, 2, ascii, &invert_next_value, &signflip_next_value, &error);
								raw_next_value = false; break;
							case 'C':
								mb_linear_interp(verbose, secondary_time_d - 1, (&secondary_data[num_secondary * (count - 1)]) - 1,
								                 num_secondary, time_d, &dsecondary, &j_secondary_interp, &error);
								printsimplevalue(verbose, output[i], dsecondary, 16, 8, ascii, &invert_next_value, &signflip_next_value, &error);
								raw_next_value = false; break;
							case 'd':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], depression[k], 5, 2, ascii, &invert_next_value, &signflip_next_value, &error);
								raw_next_value = false; break;
							case 'F':
								if (netcdf) fprintf(output[i], "\"");
								fprintf(output[i], "%s", file);
								if (netcdf) fprintf(output[i], "\"");
								if (!ascii) for (k = (int)strlen(file); k < MB_PATH_MAXLINE; k++) fwrite(&file[strlen(file)], sizeof(char), 1, outfile);
								raw_next_value = false; break;
							case 'f':
								if (ascii) fprintf(output[i], "%6d", format);
								else { b = format; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'G':
								if (ascii) fprintf(output[i], "%6d", tvg_start);
								else { b = tvg_start; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'g':
								if (ascii) fprintf(output[i], "%6d", tvg_stop);
								else { b = tvg_stop; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'L':
								if (ascii) fprintf(output[i], "%6d", ipulse_length);
								else { b = ipulse_length; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'l': printsimplevalue(verbose, output[i], pulse_length, 9, 6, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 'M':
								if (ascii) fprintf(output[i], "%4d", mode_v);
								else { b = mode_v; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'N':
								if (ascii) fprintf(output[i], "%6d", png_count);
								else { b = png_count; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'p':
								invert = invert_next_value; flip = signflip_next_value;
								printsimplevalue(verbose, output[i], ss_pixels[start_sample[k]], 5, 1, ascii, &invert_next_value, &signflip_next_value, &error);
								if (count > 0) {
									int m = 1;
									for (; m < count && m < beam_samples[k]; m++) {
										if (netcdf) fprintf(output[i], ", ");
										if (ascii) fprintf(output[i], "%s", delimiter);
										invert_next_value = invert; signflip_next_value = flip;
										printsimplevalue(verbose, output[i], ss_pixels[start_sample[k] + m], 5, 1, ascii, &invert_next_value, &signflip_next_value, &error);
									}
									for (; m < count; m++) {
										if (netcdf) fprintf(output[i], ", ");
										if (ascii) fprintf(output[i], "%s", delimiter);
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
								}
								raw_next_value = false; break;
							case 'R':
								if (ascii) fprintf(output[i], "%6d", range[k]);
								else { b = range[k]; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'r':
								if (ascii) fprintf(output[i], "%6d", sample_rate);
								else { b = sample_rate; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'S':
								if (ascii) fprintf(output[i], "%6d", npixels);
								else { b = npixels; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 's':
								if (ascii) fprintf(output[i], "%6d", beam_samples[k]);
								else { b = beam_samples[k]; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'T': printsimplevalue(verbose, output[i], transmit_gain, 5, 1, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 't': printsimplevalue(verbose, output[i], receive_gain, 5, 1, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							default: if (ascii) fprintf(output[i], "<Invalid Option: %c>", list[i]); raw_next_value = false; break;
							}
						}
						if (ascii) {
							if (i < (n_list - 1)) {
								if (!special_character) fprintf(output[i], "%s", delimiter);
								else special_character = false;
							} else fprintf(output[lcount++ % n_list], "\n");
						}
					}
				}
			}

			if (error == MB_ERROR_NO_ERROR && (nread - 1) % decimate == 0) {
				for (int j = pixel_start; j <= pixel_end; j++) {
					pixel_status = MB_SUCCESS;
					if (check_bath && j != pixel_vertical) pixel_status = MB_FAILURE;
					else if (check_bath && j == pixel_vertical) {
						if (check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[beam_vertical])) pixel_status = MB_FAILURE;
						else if (check_values == MBLIST_CHECK_ON_NULL && beamflag[beam_vertical] == MB_FLAG_NULL) pixel_status = MB_FAILURE;
					}
					if (check_amp && j != pixel_vertical) pixel_status = MB_FAILURE;
					else if (check_amp && j == pixel_vertical) {
						if (check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[beam_vertical])) pixel_status = MB_FAILURE;
						else if (check_values == MBLIST_CHECK_ON_NULL && beamflag[beam_vertical] == MB_FLAG_NULL) pixel_status = MB_FAILURE;
					}
					if (check_ss && ss[j] <= MB_SIDESCAN_NULL) pixel_status = MB_FAILURE;
					if (use_time_interval && first) pixel_status = MB_FAILURE;
					if (check_nav && navlon == 0.0) pixel_status = MB_FAILURE;

					if (pixel_status != MB_SUCCESS) continue;

					signflip_next_value = false; invert_next_value = false; raw_next_value = false;
					sensornav_next_value = false; projectednav_next_value = false; special_character = false;

					for (int i = 0; i < n_list; i++) {
						if (netcdf && lcount > 0) fprintf(output[i], ", ");
						int k;
						if (port_next_value) { k = pixel_port; port_next_value = false; }
						else if (stbd_next_value) { k = pixel_stbd; stbd_next_value = false; }
						else k = j;

						if (!raw_next_value && !ttimes_next_value) {
							switch (list[i]) {
							case '/': invert_next_value = true; special_character = true; break;
							case '-': signflip_next_value = true; special_character = true; break;
							case '_': sensornav_next_value = true; special_character = true; break;
							case '@': sensorrelative_next_value = true; special_character = true; break;
							case '^': projectednav_next_value = true; special_character = true; break;
							case ',': ttimes_next_value = true; count = 0; special_character = true; break;
							case '.': raw_next_value = true; count = 0; special_character = true; break;
							case '=': port_next_value = true; special_character = true; break;
							case '+': stbd_next_value = true; special_character = true; break;
							case 'A': printsimplevalue(verbose, output[i], avgslope, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'a':
								status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes, slopeacrosstrack, ssacrosstrack[k], &depth, &slope, &error);
								printsimplevalue(verbose, output[i], slope, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'B': printsimplevalue(verbose, output[i], amp[beam_vertical], 0, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'b': printsimplevalue(verbose, output[i], ss[k], 0, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'C': printsimplevalue(verbose, output[i], altitude, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'c': printsimplevalue(verbose, output[i], sensordepth, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'D': case 'd': b = bathy_scale * ssacrosstrack[k]; printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'E': case 'e': b = bathy_scale * ssalongtrack[k]; printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'G':
								status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes, slopeacrosstrack, ssacrosstrack[k], &depth, &slope, &error);
								angle = RTD * atan(ssacrosstrack[k] / (depth - sensordepth));
								printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'g':
								status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes, slopeacrosstrack, ssacrosstrack[k], &depth, &slope, &error);
								angle = RTD * atan(bathacrosstrack[k] / (depth - sensordepth)) + slope;
								printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'H': printsimplevalue(verbose, output[i], heading, 7, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'h': printsimplevalue(verbose, output[i], course, 7, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'J':
								mb_get_jtime(verbose, time_i, time_j);
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									if (netcdf) fprintf(output[i], "%d, %d, %d, %d, %d, %d", time_j[0], time_j[1], time_i[3], time_i[4], time_i[5], time_i[6]);
									else        fprintf(output[i], "%.4d %.3d %.2d %.2d %9.6f", time_j[0], time_j[1], time_i[3], time_i[4], seconds);
								} else {
									b = time_j[0]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[1]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[3]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[4]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[5]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[6]; fwrite(&b, sizeof(double), 1, outfile);
								}
								break;
							case 'j':
								mb_get_jtime(verbose, time_i, time_j);
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									if (netcdf) fprintf(output[i], "%d, %d, %d, %d, %d", time_j[0], time_j[1], time_j[2], time_j[3], time_j[4]);
									else        fprintf(output[i], "%.4d %.3d %.4d %9.6f", time_j[0], time_j[1], time_j[2], seconds);
								} else {
									b = time_j[0]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[1]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[2]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[3]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_j[4]; fwrite(&b, sizeof(double), 1, outfile);
								}
								break;
							case 'K':
								goodbeamfraction = (beams_bath - beams_null > 0) ? ((double)beams_unflagged) / ((double)(beams_bath - beams_null)) : 0.0;
								printsimplevalue(verbose, output[i], goodbeamfraction, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'k':
								goodbeamfraction = (beams_bath > 0) ? ((double)beams_unflagged) / ((double)beams_bath) : 0.0;
								printsimplevalue(verbose, output[i], goodbeamfraction, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'L': printsimplevalue(verbose, output[i], distance_total, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'l': printsimplevalue(verbose, output[i], 1000.0 * distance_total, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'M': printsimplevalue(verbose, output[i], time_d, 0, 6, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'm':
								if (first_m) { time_d_ref = time_d; first_m = false; }
								b = time_d - time_d_ref;
								printsimplevalue(verbose, output[i], b, 0, 6, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'N': if (ascii) fprintf(output[i], "%6u", pingnumber); else { b = pingnumber; fwrite(&b, sizeof(double), 1, outfile); } break;
							case 'n': if (ascii) fprintf(output[i], "%6u", linenumber); else { b = linenumber; fwrite(&b, sizeof(double), 1, outfile); } break;
							case 'P': printsimplevalue(verbose, output[i], pitch, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'p': printsimplevalue(verbose, output[i], draft, 7, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'Q':
								if (ascii) {
									if (netcdf) fprintf(output[i], "\"");
									fprintf(output[i], "%d", MB_DETECT_UNKNOWN);
									if (netcdf) fprintf(output[i], "\"");
								} else { b = MB_DETECT_UNKNOWN; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							case 'R': printsimplevalue(verbose, output[i], roll, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'r': printsimplevalue(verbose, output[i], heave, 7, 4, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'S': printsimplevalue(verbose, output[i], speed, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 's': printsimplevalue(verbose, output[i], speed_made_good, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error); break;
							case 'T':
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									if (netcdf) fprintf(output[i], "\"");
									fprintf(output[i], "%.4d/%.2d/%.2d/%.2d/%.2d/%09.6f", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], seconds);
									if (netcdf) fprintf(output[i], "\"");
								} else {
									b = time_i[0]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[1]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[2]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[3]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[4]; fwrite(&b, sizeof(double), 1, outfile);
									b = seconds; fwrite(&b, sizeof(double), 1, outfile);
								}
								break;
							case 't':
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									if (netcdf) fprintf(output[i], "%d, %d, %d, %d, %d, %d, %d", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
									else fprintf(output[i], "%.4d %.2d %.2d %.2d %.2d %09.6f", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], seconds);
								} else {
									b = time_i[0]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[1]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[2]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[3]; fwrite(&b, sizeof(double), 1, outfile);
									b = time_i[4]; fwrite(&b, sizeof(double), 1, outfile);
									b = seconds; fwrite(&b, sizeof(double), 1, outfile);
								}
								break;
							case 'U':
								time_u = (int)time_d;
								if (ascii) fprintf(output[i], "%lld", (long long)time_u);
								else { b = time_u; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							case 'u':
								time_u = (int)time_d;
								if (first_u) { time_u_ref = time_u; first_u = false; }
								if (ascii) fprintf(output[i], "%lld", (long long)(time_u - time_u_ref));
								else { b = time_u - time_u_ref; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							case 'V': case 'v':
								if (ascii) {
									if (fabs(time_interval) > 100.) fprintf(output[i], "%g", time_interval);
									else fprintf(output[i], "%10.6f", time_interval);
								} else fwrite(&time_interval, sizeof(double), 1, outfile);
								break;
							case 'X':
								if (!projectednav_next_value) {
									if (sensorrelative_next_value) dlon = 0.0; else dlon = navlon;
									if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
										dlon += headingy * mtodeglon * ssacrosstrack[k] + headingx * mtodeglon * ssalongtrack[k];
									printsimplevalue(verbose, output[i], dlon, 15, 10, ascii, &invert_next_value, &signflip_next_value, &error);
								} else {
									if (sensorrelative_next_value) deasting = 0.0; else deasting = naveasting;
									if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
										deasting += headingy * ssacrosstrack[k] + headingx * ssalongtrack[k];
									printsimplevalue(verbose, output[i], deasting, 15, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								sensornav_next_value = false; sensorrelative_next_value = false; projectednav_next_value = false;
								break;
							case 'x':
								dlon = navlon;
								if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
									dlon += headingy * mtodeglon * ssacrosstrack[k] + headingx * mtodeglon * ssalongtrack[k];
								if (dlon < 0.0) { hemi = 'W'; dlon = -dlon; } else hemi = 'E';
								degrees = (int)dlon; minutes = 60.0 * (dlon - degrees);
								if (ascii) {
									if (netcdf) fprintf(output[i], "\"");
									fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
									if (netcdf) fprintf(output[i], "\"");
								} else {
									b = degrees; if (hemi == 'W') b = -b; fwrite(&b, sizeof(double), 1, outfile);
									b = minutes; fwrite(&b, sizeof(double), 1, outfile);
								}
								sensornav_next_value = false;
								break;
							case 'Y':
								if (!projectednav_next_value) {
									if (sensorrelative_next_value) dlat = 0.0; else dlat = navlat;
									if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
										dlat += -headingx * mtodeglat * ssacrosstrack[k] + headingy * mtodeglat * ssalongtrack[k];
									printsimplevalue(verbose, output[i], dlat, 15, 10, ascii, &invert_next_value, &signflip_next_value, &error);
								} else {
									if (sensorrelative_next_value) dnorthing = 0.0; else dnorthing = navnorthing;
									if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
										dnorthing += -headingx * ssacrosstrack[k] + headingy * ssalongtrack[k];
									printsimplevalue(verbose, output[i], dnorthing, 15, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								sensornav_next_value = false; sensorrelative_next_value = false; projectednav_next_value = false;
								break;
							case 'y':
								dlat = navlat;
								if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
									dlat += -headingx * mtodeglat * ssacrosstrack[k] + headingy * mtodeglat * ssalongtrack[k];
								if (dlat < 0.0) { hemi = 'S'; dlat = -dlat; } else hemi = 'N';
								degrees = (int)dlat; minutes = 60.0 * (dlat - degrees);
								if (ascii) {
									if (netcdf) fprintf(output[i], "\"");
									fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
									if (netcdf) fprintf(output[i], "\"");
								} else {
									b = degrees; if (hemi == 'S') b = -b; fwrite(&b, sizeof(double), 1, outfile);
									b = minutes; fwrite(&b, sizeof(double), 1, outfile);
								}
								sensornav_next_value = false;
								break;
							case 'Z':
								if (beamflag[beam_vertical] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[beam_vertical]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else {
									b = -bathy_scale * bath[beam_vertical];
									if (sensorrelative_next_value) b -= -bathy_scale * sensordepth;
									printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								sensornav_next_value = false; sensorrelative_next_value = false;
								break;
							case 'z':
								if (beamflag[beam_vertical] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[beam_vertical]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else {
									b = bathy_scale * bath[beam_vertical];
									if (sensorrelative_next_value) b -= bathy_scale * sensordepth;
									printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								sensornav_next_value = false; sensorrelative_next_value = false;
								break;
							case '#':
								if (ascii) fprintf(output[i], "%6d", k);
								else { b = k; fwrite(&b, sizeof(double), 1, outfile); }
								break;
							default: fprintf(output[i], "<Invalid Option: %c>", list[i]); break;
							}
						}
						else if (ttimes_next_value) {
							switch (list[i]) {
							case '/': invert_next_value = true; special_character = true; break;
							case '-': signflip_next_value = true; special_character = true; break;
							case '_': sensornav_next_value = true; special_character = true; break;
							case '@': sensorrelative_next_value = true; special_character = true; break;
							case '^': projectednav_next_value = true; special_character = true; break;
							case ',': ttimes_next_value = true; special_character = true; count = 0; break;
							case '.': raw_next_value = true; special_character = true; count = 0; break;
							case '=': port_next_value = true; special_character = true; break;
							case '+': stbd_next_value = true; special_character = true; break;
							case '0': case '1': case '2': case '3': case '4':
							case '5': case '6': case '7': case '8': case '9':
								count = count * 10 + list[i] - '0'; break;
							case 'D':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_sensordepth, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							case 'S':
								if (beamflag[k] == MB_FLAG_NULL && (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN))
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN)
									printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
								else printsimplevalue(verbose, output[i], tt_ssv, 9, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								ttimes_next_value = false; break;
							default: if (ascii) fprintf(output[i], "<Invalid Option: %c>", list[i]); ttimes_next_value = false; break;
							}
						}
						else /* raw_next_value */ {
							switch (list[i]) {
							case '/': invert_next_value = true; special_character = true; break;
							case '-': signflip_next_value = true; special_character = true; break;
							case '_': sensornav_next_value = true; special_character = true; break;
							case '@': sensorrelative_next_value = true; special_character = true; break;
							case '^': projectednav_next_value = true; special_character = true; break;
							case ',': ttimes_next_value = true; count = 0; special_character = true; break;
							case '.': raw_next_value = true; count = 0; special_character = true; break;
							case '=': port_next_value = true; special_character = true; break;
							case '+': stbd_next_value = true; special_character = true; break;
							case '0': case '1': case '2': case '3': case '4':
							case '5': case '6': case '7': case '8': case '9':
								count = count * 10 + list[i] - '0'; break;
							case 'A': printsimplevalue(verbose, output[i], bs[beam_vertical], 5, 1, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 'a': printsimplevalue(verbose, output[i], absorption, 5, 2, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 'B': printsimplevalue(verbose, output[i], bsn_v, 5, 2, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 'b': printsimplevalue(verbose, output[i], bso_v, 5, 2, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 'c':
								mback = 0; nback = 0;
								for (int m = 0; m < beams_amp; m++) if (mb_beam_ok(beamflag[m])) { mback += amp[m]; nback++; }
								printsimplevalue(verbose, output[i], (nback > 0 ? mback / nback : 0.0), 5, 2, ascii, &invert_next_value, &signflip_next_value, &error);
								raw_next_value = false; break;
							case 'C':
								mb_linear_interp(verbose, secondary_time_d - 1, (&secondary_data[num_secondary * (count - 1)]) - 1,
								                 num_secondary, time_d, &dsecondary, &j_secondary_interp, &error);
								printsimplevalue(verbose, output[i], dsecondary, 16, 8, ascii, &invert_next_value, &signflip_next_value, &error);
								raw_next_value = false; break;
							case 'd':
								printsimplevalue(verbose, output[i], depression[beam_vertical], 5, 2, ascii, &invert_next_value, &signflip_next_value, &error);
								raw_next_value = false; break;
							case 'F':
								if (netcdf) fprintf(output[i], "\"");
								fprintf(output[i], "%s", file);
								if (netcdf) fprintf(output[i], "\"");
								if (!ascii) for (k = (int)strlen(file); k < MB_PATH_MAXLINE; k++) fwrite(&file[strlen(file)], sizeof(char), 1, outfile);
								raw_next_value = false; break;
							case 'f':
								if (ascii) fprintf(output[i], "%6d", format);
								else { b = format; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'G':
								if (ascii) fprintf(output[i], "%6d", tvg_start);
								else { b = tvg_start; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'g':
								if (ascii) fprintf(output[i], "%6d", tvg_stop);
								else { b = tvg_stop; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'L':
								if (ascii) fprintf(output[i], "%6d", ipulse_length);
								else { b = ipulse_length; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'l': printsimplevalue(verbose, output[i], pulse_length, 9, 6, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 'M':
								if (ascii) fprintf(output[i], "%4d", mode_v);
								else { b = mode_v; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'N':
								if (ascii) fprintf(output[i], "%6d", png_count);
								else { b = png_count; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'p':
								invert = invert_next_value; flip = signflip_next_value;
								printsimplevalue(verbose, output[i], ss_pixels[start_sample[beam_vertical]], 5, 1, ascii, &invert_next_value, &signflip_next_value, &error);
								if (count > 0) {
									int m = 1;
									for (; m < count && m < beam_samples[beam_vertical]; m++) {
										if (netcdf) fprintf(output[i], ", ");
										if (ascii) fprintf(output[i], "%s", delimiter);
										invert_next_value = invert; signflip_next_value = flip;
										printsimplevalue(verbose, output[i], ss_pixels[start_sample[beam_vertical] + m], 5, 1, ascii, &invert_next_value, &signflip_next_value, &error);
									}
									for (; m < count; m++) {
										if (netcdf) fprintf(output[i], ", ");
										if (ascii) fprintf(output[i], "%s", delimiter);
										printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
									}
								}
								raw_next_value = false; break;
							case 'R':
								if (ascii) fprintf(output[i], "%6d", range[beam_vertical]);
								else { b = range[beam_vertical]; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'r':
								if (ascii) fprintf(output[i], "%6d", sample_rate);
								else { b = sample_rate; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'S':
								if (ascii) fprintf(output[i], "%6d", npixels);
								else { b = npixels; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 's':
								if (ascii) fprintf(output[i], "%6d", beam_samples[beam_vertical]);
								else { b = beam_samples[beam_vertical]; fwrite(&b, sizeof(double), 1, outfile); }
								raw_next_value = false; break;
							case 'T': printsimplevalue(verbose, output[i], transmit_gain, 5, 1, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							case 't': printsimplevalue(verbose, output[i], receive_gain, 5, 1, ascii, &invert_next_value, &signflip_next_value, &error); raw_next_value = false; break;
							default: if (ascii) fprintf(output[i], "<Invalid Option: %c>", list[i]); raw_next_value = false; break;
							}
						}
						if (ascii) {
							if (i < (n_list - 1)) {
								if (!special_character) fprintf(output[i], "%s", delimiter);
								else special_character = false;
							} else fprintf(output[lcount++ % n_list], "\n");
						}
					}
				}
			}

			if (error == MB_ERROR_NO_ERROR && first) first = false;
		}

		status &= mb_close(verbose, &mbio_ptr, &error);
		if (use_raw) mb_freed(verbose, __FILE__, __LINE__, (void **)&ss_pixels, &error);

		if (read_datalist) {
			read_data = (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath,
			                               &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS);
			if (read_data) {
				if (pstatus == MB_PROCESSED_USE) strcpy(file, ppath);
				else                              strcpy(file, path);
			}
		} else read_data = false;
	}
	if (read_datalist) mb_datalist_close(verbose, &datalist, &error);

	if (netcdf) {
		for (int i = 0; i < n_list; i++) {
			if (list[i] != '/' && list[i] != '-' && list[i] != '.' && !(list[i] >= '0' && list[i] <= '9')) {
				fprintf(output[i], " ;\n\n");
				rewind(output[i]);
				char buffer[MB_BUFFER_MAX];
				size_t read_len = 0;
				while ((read_len = fread(buffer, sizeof(char), MB_BUFFER_MAX, output[i])) > 0) {
					size_t write_len = fwrite(buffer, sizeof(char), read_len, outfile);
					if (write_len != read_len) fprintf(stderr, "Error writing to CDL file");
				}
			}
			fclose(output[i]);
		}
		fprintf(outfile, "}\n");
		fclose(outfile);
		if (!netcdf_cdl) {
			snprintf(output_file_temp, sizeof(output_file_temp), "ncgen -o %s %s.cdl", output_file, output_file);
			int shellstatus = system(output_file_temp);
			if (shellstatus == 0) {
				snprintf(output_file_temp, sizeof(output_file_temp), "rm %s.cdl", output_file);
				system(output_file_temp);
			}
		}
	} else fclose(outfile);

	if (num_secondary_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&secondary_time_d, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&secondary_data, &error);
		num_secondary_alloc = 0;
	}
	if (use_projection && pjptr != NULL) mb_proj_free(verbose, &(pjptr), &error);
	if (verbose >= 4) status &= mb_memory_list(verbose, &error);

	/* reset module-static state */
	num_secondary = 0; num_secondary_columns = 0; num_secondary_alloc = 0;
	j_secondary_interp = 0; secondary_time_d = NULL; secondary_data = NULL;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", THIS_MODULE_NAME);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	Return(GMT_NOERROR);
}
/* end mblist.c */
