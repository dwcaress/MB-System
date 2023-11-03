/*--------------------------------------------------------------------
 *    The MB-system:	mbfilter.c	1/16/95
 *
 *    Copyright (c) 1995-2023 by
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
 * mbfilter applies one or more simple filters to the specified
 * data (sidescan, beam amplitude, and/or bathymetry). The filters
 * include:
 *   a: boxcar mean filter for smoothing
 *   b: gaussian mean filter for smoothing
 *   c: boxcar median filter for smoothing
 *   d: inverse gradient filter for smoothing
 *   e: edge detection filter for contrast enhancement
 *   f: gradient subtraction filter for contrast enhancement
 * These filters are mostly intended for use with sidescan
 * data, and operate on 3x3 or 5x5 value windows with
 * no accommodation for differences in acrosstrack vs
 * alongtrack sampling.
 * The default input and output streams are stdin and stdout.
 *
 * Author:	D. W. Caress
 * Date:	January 16, 1995
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_ldeoih.h"

typedef enum {
    MBFILTER_BATH = 0,
    MBFILTER_AMP = 1,
    MBFILTER_SS = 2,
} filter_kind_t;

typedef enum {
    MBFILTER_HIPASS_NONE = 0,
    MBFILTER_HIPASS_MEAN = 1,
    MBFILTER_HIPASS_GAUSSIAN = 2,
    MBFILTER_HIPASS_MEDIAN = 3,
} hipass_mode_t;

typedef enum {
    MBFILTER_SMOOTH_NONE = 0,
    MBFILTER_SMOOTH_MEAN = 1,
    MBFILTER_SMOOTH_GAUSSIAN = 2,
    MBFILTER_SMOOTH_MEDIAN = 3,
    MBFILTER_SMOOTH_GRADIENT = 4,
} smooth_mode_t;

typedef enum {
    MBFILTER_CONTRAST_NONE = 0,
    MBFILTER_CONTRAST_EDGE = 1,
    MBFILTER_CONTRAST_GRADIENT = 2,
} contrast_mode_t;

typedef enum {
    MBFILTER_A_NONE = 0,
    MBFILTER_A_HIPASS_MEAN = 1,
    MBFILTER_A_HIPASS_GAUSSIAN = 2,
    MBFILTER_A_HIPASS_MEDIAN = 3,
    MBFILTER_A_SMOOTH_MEAN = 4,
    MBFILTER_A_SMOOTH_GAUSSIAN = 5,
    MBFILTER_A_SMOOTH_MEDIAN = 6,
    MBFILTER_A_SMOOTH_GRADIENT = 7,
    MBFILTER_A_CONTRAST_EDGE = 8,
    MBFILTER_A_CONTRAST_GRADIENT = 9,
} filter_a_mode_t;

/* MBIO buffer size default */
constexpr int MBFILTER_BUFFER_DEFAULT = 5000;

/* ping structure definition */
struct mbfilter_ping_struct {
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sensordepth;
	double roll;
	double pitch;
	double heave;
	int beams_bath;
	int beams_amp;
	int pixels_ss;
	char *beamflag;
	double *bath;
	double *bathacrosstrack;
	double *bathalongtrack;
	double *amp;
	char *pixelflag;
	double *ss;
	double *ssacrosstrack;
	double *ssalongtrack;
	double *dataprocess;
	double *datasave;
	int ndatapts;
	double *data_i_ptr;
	double *data_f_ptr;
	char *flag_ptr;
};

/* filter structure definition */
constexpr int MBFILTER_NFILTER_MAX = 10;
struct mbfilter_filter_struct {
	filter_a_mode_t mode;
	int xdim;
	int ldim;
	int iteration;
	bool threshold;
	double threshold_lo;
	double threshold_hi;
	double hipass_offset;
};

constexpr char program_name[] = "MBFILTER";
constexpr char help_message[] = "mbfilter applies one or more simple filters to the specified\n\t"
    "data (sidescan and/or beam amplitude). The filters\n\t"
    "include:\n\t"
    "  - boxcar mean for lo-pass filtering (-S1)\n\t"
    "  - gaussian mean for lo-pass filtering (-S2)\n\t"
    "  - boxcar median for lo-pass filtering (-S3)\n\t"
    "  - inverse gradient for lo-pass filtering (-S4)\n\t"
    "  - boxcar mean subtraction for hi-pass filtering (-D1)\n\t"
    "  - gaussian mean subtraction for hi-pass filtering (-D2)\n\t"
    "  - boxcar median subtraction for hi-pass filtering (-D3)\n\t"
    "  - edge detection for contrast enhancement (-C1)\n\t"
    "  - gradient magnitude subtraction for contrast enhancement (-C2)\n\t"
    "These filters are primarily intended for use with sidescan\n\t"
    "data. In particular, the lo-pass or smoothing filters\n\t"
    "can be used for first-order speckle reduction in sidescan\n\t"
    "data, and the hi-pass filters can be used to emphasize\n\t"
    "fine scale structure in the data.\n\t"
    "The default input and output streams are stdin and stdout.\n";
constexpr char usage_message[] =
    "mbfilter ["
    "-Akind -Byr/mo/da/hr/mn/sc\n\t"
    "-Cmode/xdim/ldim/iteration\n\t"
    "-Dmode/xdim/ldim/iteration/offset\n\t"
    "-Eyr/mo/da/hr/mn/sc -Fformat -Iinfile -Nbuffersize\n\t"
    "-Rwest/east/south/north -Smode/xdim/ldim/iteration\n\t"
    "-Tthreshold -V -H]";

/*--------------------------------------------------------------------*/
int hipass_mean(int verbose, int n, const double *val, double *wgt, double *hipass) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       n:               %d\n", n);
		fprintf(stderr, "dbg2       val:             %p\n", (void *)val);
		fprintf(stderr, "dbg2       wgt:             %p\n", (void *)wgt);
		for (int i = 0; i < n; i++)
			fprintf(stderr, "dbg2       val[%d]: %f\n", i, val[i]);
	}

	/* get mean */
	*hipass = 0.0;
	int nn = 0;  // TODO(schwehr): Why not just use the value of n?
	for (int i = 0; i < n; i++) {
		*hipass += val[i];
		nn++;
	}
	if (nn > 0)
		*hipass = val[0] - *hipass / nn;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       hipass:          %f\n", *hipass);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int hipass_gaussian(int verbose, int n, double *val, double *wgt, double *dis, double *hipass) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       n:               %d\n", n);
		fprintf(stderr, "dbg2       val:             %p\n", (void *)val);
		fprintf(stderr, "dbg2       wgt:             %p\n", (void *)wgt);
		fprintf(stderr, "dbg2       dis:             %p\n", (void *)dis);
		for (int i = 0; i < n; i++)
			fprintf(stderr, "dbg2       val[%d]: %f  dis[%d]: %f\n", i, val[i], i, dis[i]);
	}

	/* get weights */
	*hipass = 0.0;
	double wgtsum = 0.0;
	for (int i = 0; i < n; i++) {
		wgt[i] = exp(-dis[i] * dis[i]);
		wgtsum += wgt[i];
	}

	if (wgtsum > 0.0) {
		/* get value */
		*hipass = 0.0;
		for (int i = 0; i < n; i++) {
			*hipass += wgt[i] * val[i];
		}
		*hipass = val[0] - *hipass / wgtsum;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       hipass:          %f\n", *hipass);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int hipass_median(int verbose, int n, double *val, double *wgt, double *hipass) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       n:               %d\n", n);
		fprintf(stderr, "dbg2       val:             %p\n", (void *)val);
		fprintf(stderr, "dbg2       wgt:             %p\n", (void *)wgt);
		for (int i = 0; i < n; i++)
			fprintf(stderr, "dbg2       val[%d]: %f\n", i, val[i]);
	}

	/* start */
	*hipass = 0.0;

	/* sort values and get median value */
	if (n > 0) {
		qsort((char *)val, n, sizeof(double), mb_double_compare);
		*hipass = val[0] - val[n / 2];
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       hipass:          %f\n", *hipass);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int smooth_mean(int verbose, int n, double *val, double *wgt, double *smooth) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       n:               %d\n", n);
		fprintf(stderr, "dbg2       val:             %p\n", (void *)val);
		fprintf(stderr, "dbg2       wgt:             %p\n", (void *)wgt);
		for (int i = 0; i < n; i++)
			fprintf(stderr, "dbg2       val[%d]: %f\n", i, val[i]);
	}

	/* get mean */
	*smooth = 0.0;
	int nn = 0;
	for (int i = 0; i < n; i++) {
		*smooth += val[i];
		nn++;
	}
	if (nn > 0)
		*smooth = *smooth / nn;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       smooth:          %f\n", *smooth);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int smooth_gaussian(int verbose, int n, double *val, double *wgt, double *dis, double *smooth) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       n:               %d\n", n);
		fprintf(stderr, "dbg2       val:             %p\n", (void *)val);
		fprintf(stderr, "dbg2       wgt:             %p\n", (void *)wgt);
		fprintf(stderr, "dbg2       dis:             %p\n", (void *)dis);
		for (int i = 0; i < n; i++)
			fprintf(stderr, "dbg2       val[%d]: %f  dis[%d]: %f\n", i, val[i], i, dis[i]);
	}

	/* get weights */
	*smooth = 0.0;
	double wgtsum = 0.0;
	for (int i = 0; i < n; i++) {
		wgt[i] = exp(-dis[i] * dis[i]);
		wgtsum += wgt[i];
	}

	if (wgtsum > 0.0) {
		/* get value */
		*smooth = 0.0;
		for (int i = 0; i < n; i++) {
			*smooth += wgt[i] * val[i];
		}
		*smooth = *smooth / wgtsum;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       smooth:          %f\n", *smooth);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int smooth_median(int verbose, double original, bool apply_threshold, double threshold_lo, double threshold_hi, int n, double *val,
                  double *wgt, double *smooth) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       original:        %f\n", original);
		fprintf(stderr, "dbg2       apply_threshold: %d\n", apply_threshold);
		fprintf(stderr, "dbg2       n:               %d\n", n);
		fprintf(stderr, "dbg2       val:             %p\n", (void *)val);
		fprintf(stderr, "dbg2       wgt:             %p\n", (void *)wgt);
		for (int i = 0; i < n; i++)
			fprintf(stderr, "dbg2       val[%d]: %f\n", i, val[i]);
	}

	*smooth = 0.0;

	/* sort values and get median value */
	if (n > 0) {
		qsort((char *)val, n, sizeof(double), mb_double_compare);
		*smooth = val[n / 2];
	}

	/* apply thresholding */
	if (apply_threshold) {
		const double ratio = original / (*smooth);
		if (ratio < threshold_hi && ratio > threshold_lo) {
			*smooth = original;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       smooth:          %f\n", *smooth);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int smooth_gradient(int verbose, int n, double *val, double *wgt, double *smooth) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       n:               %d\n", n);
		fprintf(stderr, "dbg2       val:             %p\n", (void *)val);
		fprintf(stderr, "dbg2       wgt:             %p\n", (void *)wgt);
		for (int i = 0; i < n; i++)
			fprintf(stderr, "dbg2       val[%d]: %f\n", i, val[i]);
	}

	/* get weights */
	*smooth = 0.0;
	double wgtsum = 0.0;
	int nn = 0;  // TODO(schwehr): Why not just use n?
	wgt[0] = 0.5;
	for (int i = 1; i < n; i++) {
		double diff = fabs(val[i] - val[0]);
		if (diff < 0.01)
			diff = 0.01;
		wgt[i] = 1.0 / diff;
		wgtsum += wgt[i];
		nn++;
	}
	if (nn > 0) {
		*smooth = wgt[0] * val[0];
		for (int i = 1; i < n; i++) {
			*smooth += 0.5 * wgt[i] * val[i] / wgtsum;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       smooth:          %f\n", *smooth);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int contrast_edge(int verbose, int n, double *val, double *grad, double *result) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       n:               %d\n", n);
		fprintf(stderr, "dbg2       val:             %p\n", (void *)val);
		fprintf(stderr, "dbg2       grad:            %p\n", (void *)grad);
		for (int i = 0; i < n; i++)
			fprintf(stderr, "dbg2       val[%d]: %f\n", i, val[i]);
	}

	/* get gradients */
	double gradsum = 0.0;
	double edge = 0.0;
	for (int i = 0; i < n; i++) {
		grad[i] = 0.0;
		for (int ii = 0; ii < n; ii++) {
			if (val[ii] > 0.0 && i != ii) {
				grad[i] += (val[ii] - val[i]) * (val[ii] - val[i]);
			}
		}
		gradsum += grad[i];
		edge += val[i] * grad[i];
	}
	edge = edge / gradsum;
	const double contrast = pow((fabs(val[0] - edge) / fabs(val[0] + edge)), 0.75);
	if (val[0] >= edge)
		*result = edge * (1.0 + contrast) / (1.0 - contrast);
	else
		*result = edge * (1.0 - contrast) / (1.0 + contrast);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       result:          %f\n", *result);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int contrast_gradient(int verbose, int n, double *val, double *wgt, double *result) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       n:               %d\n", n);
		fprintf(stderr, "dbg2       val:             %p\n", (void *)val);
		fprintf(stderr, "dbg2       wgt:             %p\n", (void *)wgt);
		for (int i = 0; i < n; i++)
			fprintf(stderr, "dbg2       val[%d]: %f\n", i, val[i]);
	}

	/* get weights */
	*result = 0.0;
	double gradient = 0.0;
	int nn = 0;
	for (int i = 1; i < n; i++) {
		gradient += (val[i] - val[0]) * (val[i] - val[0]);
		nn++;
	}
	gradient = sqrt(gradient);
	*result = val[0] - 2 * gradient;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBFILTER function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       result:          %f\n", *result);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbcopy_any_to_mbldeoih(int verbose, int system, int kind, int *time_i, double time_d, double navlon, double navlat,
                           double speed, double heading, double draft, double altitude, double roll, double pitch, double heave,
                           double beamwidth_xtrack, double beamwidth_ltrack, int nbath, int namp, int nss, char *beamflag,
                           double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                           double *ssacrosstrack, double *ssalongtrack, char *comment, char *ombio_ptr, char *ostore_ptr,
                           int *error) {
	/* get data structure pointer */
	struct mbsys_ldeoih_struct *ostore = (struct mbsys_ldeoih_struct *)ostore_ptr;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBcopy function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       ombio_ptr:  %p\n", (void *)ombio_ptr);
		fprintf(stderr, "dbg2       ostore_ptr: %p\n", (void *)ostore_ptr);
		fprintf(stderr, "dbg2       system:     %d\n", system);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
	}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV)) {
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", speed);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
		fprintf(stderr, "dbg2       draft:      %f\n", draft);
		fprintf(stderr, "dbg2       altitude:   %f\n", altitude);
		fprintf(stderr, "dbg2       roll:       %f\n", roll);
		fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
		fprintf(stderr, "dbg2       heave:      %f\n", heave);
		fprintf(stderr, "dbg2       beamwidth_xtrack: %f\n", beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack: %f\n", beamwidth_ltrack);
	}
	if (verbose >= 2 && kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
		if (verbose >= 3)
			for (int i = 0; i < nbath; i++)
				fprintf(stderr, "dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2       namp:       %d\n", namp);
		if (verbose >= 3)
			for (int i = 0; i < namp; i++)
				fprintf(stderr, "dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:       %d\n", nss);
		if (verbose >= 3)
			for (int i = 0; i < nss; i++)
				fprintf(stderr, "dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
				        ssalongtrack[i]);
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	int status = MB_SUCCESS;

	/* copy the data  */
	if (ostore != nullptr) {
		/* set beam widths */
		ostore->beam_xwidth = beamwidth_xtrack;
		ostore->beam_lwidth = beamwidth_ltrack;
		if (system == MB_SYS_SB2100)
			ostore->ss_type = MB_SIDESCAN_LINEAR;
		else
			ostore->ss_type = MB_SIDESCAN_LOGARITHMIC;
		ostore->kind = kind;

		/* insert data */
		if (kind == MB_DATA_DATA) {
			mb_insert_altitude(verbose, ombio_ptr, (void *)ostore, draft, altitude, error);
			mb_insert_nav(verbose, ombio_ptr, (void *)ostore, time_i, time_d, navlon, navlat, speed, heading, draft, roll, pitch,
			              heave, error);
		}
		status =
		    mb_insert(verbose, ombio_ptr, (void *)ostore, kind, time_i, time_d, navlon, navlat, speed, heading, nbath, namp, nss,
		              beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, error);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBcopy function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* reset all defaults but the format and lonflip */
	pings = 1;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";
	int smooth_iter = 1;
	bool apply_threshold = false;
	double threshold_lo = 0.0;
	double threshold_hi = 0.0;
	double hipass_offset = 1000.0;
	smooth_mode_t smooth_mode = MBFILTER_SMOOTH_NONE;
	int smooth_xdim = 3;
	int smooth_ldim = 3;
	int num_filters = 0;
	struct mbfilter_filter_struct filters[MBFILTER_NFILTER_MAX];
	hipass_mode_t hipass_mode = MBFILTER_HIPASS_NONE;
	int hipass_xdim = 10;
	int hipass_ldim = 3;
	int hipass_iter = 1;
	filter_kind_t datakind = MBFILTER_SS;
	int contrast_mode = MBFILTER_CONTRAST_NONE;
	int contrast_xdim = 5;
	int contrast_ldim = 5;
	int contrast_iter = 1;
	int n_buffer_max = MBFILTER_BUFFER_DEFAULT;

	{
		bool errflg = 0;
		int c;
		bool help = 0;
		while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:HhI:i:N:n:R:r:S:s:T:t:Vv")) != -1)
		{
			switch (c) {
			case 'A':
			case 'a':
			{
				int tmp;
				sscanf(optarg, "%d", &tmp);
				datakind = (filter_kind_t)tmp;
				if (datakind != MBFILTER_SS && datakind != MBFILTER_AMP)
					datakind = MBFILTER_SS;
				break;
			}
			case 'B':
			case 'b':
				sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
				btime_i[6] = 0;
				break;
			case 'C':
			case 'c':
			{
				int tmp;
				const int n = sscanf(optarg, "%d/%d/%d/%d", &tmp, &contrast_xdim, &contrast_ldim, &contrast_iter);
				contrast_mode = (contrast_mode_t)tmp;
				if (n >= 3) {
					filters[num_filters].mode = static_cast<filter_a_mode_t>(contrast_mode + 7);
					filters[num_filters].xdim = contrast_xdim;
					filters[num_filters].ldim = contrast_ldim;
					filters[num_filters].threshold = false;
				}
				if (n >= 4)
					filters[num_filters].iteration = contrast_iter;
				else
					filters[num_filters].iteration = 1;
				if (n >= 3)
					num_filters++;
				break;
			}
			case 'D':
			case 'd':
			{
				int tmp;
				const int n = sscanf(optarg, "%d/%d/%d/%d/%lf", &tmp, &hipass_xdim, &hipass_ldim, &hipass_iter, &hipass_offset);
				hipass_mode = (hipass_mode_t)tmp;  // TODO(schwehr): Range check.
				if (n >= 3) {
					filters[num_filters].mode = static_cast<filter_a_mode_t>(hipass_mode + 0);
					filters[num_filters].xdim = hipass_xdim;
					filters[num_filters].ldim = hipass_ldim;
					filters[num_filters].threshold = false;
				}
				if (n >= 4)
					filters[num_filters].iteration = hipass_iter;
				else
					filters[num_filters].iteration = 1;
				if (n >= 5)
					filters[num_filters].hipass_offset = hipass_offset;
				else
					filters[num_filters].hipass_offset = 1000.0;
				if (n >= 3)
					num_filters++;
				break;
			}
			case 'E':
			case 'e':
				sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
				etime_i[6] = 0;
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				break;
			case 'H':
			case 'h':
				help = true;
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);
				break;
			case 'N':
			case 'n':
				sscanf(optarg, "%d", &n_buffer_max);
				if (n_buffer_max > MBFILTER_BUFFER_DEFAULT || n_buffer_max < 10)
					n_buffer_max = MBFILTER_BUFFER_DEFAULT;
				break;
			case 'R':
			case 'r':
				mb_get_bounds(optarg, bounds);
				break;
			case 'S':
			case 's':
			{
				int tmp;
				const int n = sscanf(optarg, "%d/%d/%d/%d/%lf/%lf", &tmp, &smooth_xdim, &smooth_ldim, &smooth_iter, &threshold_lo,
				           &threshold_hi);
				smooth_mode = (smooth_mode_t)tmp;  // TODO(schwehr): Range check.
				if (n >= 3) {
					filters[num_filters].mode = static_cast<filter_a_mode_t>(smooth_mode + 3);
					filters[num_filters].xdim = smooth_xdim;
					filters[num_filters].ldim = smooth_ldim;
				}
				if (n >= 4)
					filters[num_filters].iteration = smooth_iter;
				else
					filters[num_filters].iteration = 1;
				if (n >= 6) {
					filters[num_filters].threshold = true;
					filters[num_filters].threshold_lo = threshold_lo;
					filters[num_filters].threshold_hi = threshold_hi;
				}
				else if (apply_threshold) {
					filters[num_filters].threshold = true;
					filters[num_filters].threshold_lo = threshold_lo;
					filters[num_filters].threshold_hi = threshold_hi;
				}
				else
					filters[num_filters].threshold = false;
				if (n >= 3)
					num_filters++;
				break;
			}
			case 'T':
			case 't':
				sscanf(optarg, "%lf/%lf", &threshold_lo, &threshold_hi);
				apply_threshold = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case '?':
				errflg = true;
			}
		}

		if (errflg) {
			fprintf(stderr, "usage: %s\n", usage_message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (verbose == 1 || help) {
			fprintf(stderr, "\nProgram %s\n", program_name);
			fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		}

		/* set data type if not set properly */
		if (datakind != MBFILTER_BATH && datakind != MBFILTER_AMP)
			datakind = MBFILTER_SS;

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
			fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "dbg2  Control Parameters:\n");
			fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
			fprintf(stderr, "dbg2       help:           %d\n", help);
			fprintf(stderr, "dbg2       pings:          %d\n", pings);
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
			fprintf(stderr, "dbg2       data format:    %d\n", format);
			fprintf(stderr, "dbg2       read_file:      %s\n", read_file);
			fprintf(stderr, "dbg2       datakind:       %d\n", datakind);
			fprintf(stderr, "dbg2       n_buffer_max:   %d\n", n_buffer_max);
			fprintf(stderr, "dbg2       num_filters:    %d\n", num_filters);
			for (int i = 0; i < num_filters; i++) {
				fprintf(stderr, "dbg2       filters[%d].mode:          %d\n", i, filters[i].mode);
				fprintf(stderr, "dbg2       filters[%d].xdim:          %d\n", i, filters[i].xdim);
				fprintf(stderr, "dbg2       filters[%d].ldim:          %d\n", i, filters[i].ldim);
				fprintf(stderr, "dbg2       filters[%d].iteration:     %d\n", i, filters[i].iteration);
				fprintf(stderr, "dbg2       filters[%d].threshold:     %d\n", i, filters[i].threshold);
				fprintf(stderr, "dbg2       filters[%d].threshold_lo:  %f\n", i, filters[i].threshold_lo);
				fprintf(stderr, "dbg2       filters[%d].threshold_hi:  %f\n", i, filters[i].threshold_hi);
				fprintf(stderr, "dbg2       filters[%d].hipass_offset: %f\n", i, filters[i].hipass_offset);
			}
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;

	if (format == 0)
		mb_get_format(verbose, read_file, nullptr, &format, &error);

	/* output some information */
	if (verbose > 0) {
		if (datakind == MBFILTER_BATH)
			fprintf(stderr, "\nProcessing bathymetry data...\n");
		else if (datakind == MBFILTER_AMP)
			fprintf(stderr, "\nProcessing beam amplitude data...\n");
		else if (datakind == MBFILTER_SS)
			fprintf(stderr, "\nProcessing sidescan data...\n");
		fprintf(stderr, "Number of filters to be applied: %d\n\n", num_filters);
		for (int i = 0; i < num_filters; i++) {
			if (filters[i].mode == MBFILTER_A_HIPASS_MEAN)
				fprintf(stderr, "Filter %d: High pass mean subtraction\n", i);
			else if (filters[i].mode == MBFILTER_A_HIPASS_GAUSSIAN)
				fprintf(stderr, "Filter %d: High pass Gaussian subtraction\n", i);
			else if (filters[i].mode == MBFILTER_A_HIPASS_MEDIAN)
				fprintf(stderr, "Filter %d: High pass median subtraction\n", i);
			else if (filters[i].mode == MBFILTER_A_SMOOTH_MEAN)
				fprintf(stderr, "Filter %d: Low pass mean\n", i);
			else if (filters[i].mode == MBFILTER_A_SMOOTH_GAUSSIAN)
				fprintf(stderr, "Filter %d: Low pass Gaussian\n", i);
			else if (filters[i].mode == MBFILTER_A_SMOOTH_MEDIAN)
				fprintf(stderr, "Filter %d: Low pass median\n", i);
			else if (filters[i].mode == MBFILTER_A_SMOOTH_GRADIENT)
				fprintf(stderr, "Filter %d: Low pass gradient\n", i);
			else if (filters[i].mode == MBFILTER_A_CONTRAST_EDGE)
				fprintf(stderr, "Filter %d: Contrast edge\n", i);
			else if (filters[i].mode == MBFILTER_A_CONTRAST_GRADIENT)
				fprintf(stderr, "Filter %d: Contrast gradient\n", i);
			fprintf(stderr, "          Acrosstrack dimension: %d\n", filters[i].xdim);
			fprintf(stderr, "          Alongtrack dimension:  %d\n", filters[i].ldim);
			fprintf(stderr, "          Iterations:            %d\n", filters[i].iteration);
			if (filters[i].mode == MBFILTER_A_SMOOTH_MEDIAN) {
				if (filters[i].threshold) {
					fprintf(stderr, "          Threshold applied\n");
					fprintf(stderr, "          Threshold_lo:          %f\n", filters[i].threshold_lo);
					fprintf(stderr, "          Threshold_hi:          %f\n", filters[i].threshold_hi);
				}
				else {
					fprintf(stderr, "          Threshold not applied\n");
				}
			}
			if (filters[i].mode >= MBFILTER_A_HIPASS_MEAN && filters[i].mode <= MBFILTER_A_HIPASS_MEDIAN) {
				fprintf(stderr, "          Hipass_offset:         %f\n", filters[i].hipass_offset);
			}
			fprintf(stderr, "\n");
		}
	}

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data;
	void *datalist;
	char file[MB_PATH_MAXLINE];
	char dfile[MB_PATH_MAXLINE];
	double file_weight;

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
		read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		// else copy single filename to be read
		strcpy(file, read_file);
		read_data = true;
	}

	/* MBIO read control parameters */
	int system;
	double btime_d;
	double etime_d;
	int beams_bath;
	int beams_amp;
	int pixels_ss;
	int obeams_bath;
	int obeams_amp;
	int opixels_ss;
	void *imbio_ptr = nullptr;

	/* MBIO write control parameters */
	char ofile[MB_PATH_MAXLINE+10];
	void *ombio_ptr = nullptr;

	/* mbio read and write values */
	void *store_ptr;
	int kind;
	char comment[MB_COMMENT_MAXLINE];

	/* buffer handling parameters */
	int nreadtot = 0;
	int nwritetot = 0;
	struct mbfilter_ping_struct ping[MBFILTER_BUFFER_DEFAULT];

	double *weights;
	double *values;
	double *distances;

	/* loop over all files to be read */
	while (read_data) {

		/* check for format with amplitude or sidescan data */
		/* status = */ mb_format_system(verbose, &format, &system, &error);
		/* status = */ mb_format_dimensions(verbose, &format, &beams_bath, &beams_amp, &pixels_ss, &error);

		/* initialize reading the input swath sonar file */
		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &imbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		struct mb_io_struct *imb_io_ptr = (struct mb_io_struct *)imbio_ptr;

		/* initialize writing the output swath sonar file */
		if (datakind == MBFILTER_BATH)
			snprintf(ofile, sizeof(ofile), "%s.ffb", file);
		else if (datakind == MBFILTER_AMP)
			snprintf(ofile, sizeof(ofile), "%s.ffa", file);
		else if (datakind == MBFILTER_SS)
			snprintf(ofile, sizeof(ofile), "%s.ffs", file);
		if (mb_write_init(verbose, ofile, 71, &ombio_ptr, &obeams_bath, &obeams_amp, &opixels_ss, &error) !=
		    MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for writing\n", ofile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		struct mb_io_struct *omb_io_ptr = (struct mb_io_struct *)ombio_ptr;

		/* allocate memory for data arrays */
		for (int i = 0; i < n_buffer_max; i++) {
			ping[i].beamflag = nullptr;
			ping[i].bath = nullptr;
			ping[i].amp = nullptr;
			ping[i].bathacrosstrack = nullptr;
			ping[i].bathalongtrack = nullptr;
			ping[i].pixelflag = nullptr;
			ping[i].ss = nullptr;
			ping[i].ssacrosstrack = nullptr;
			ping[i].ssalongtrack = nullptr;
			ping[i].dataprocess = nullptr;
			ping[i].datasave = nullptr;
			ping[i].data_i_ptr = nullptr;
			ping[i].data_f_ptr = nullptr;
			ping[i].flag_ptr = nullptr;
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&ping[i].beamflag,
				                           &error);
			if (error == MB_ERROR_NO_ERROR)
				status =
				    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].bath, &error);
			if (error == MB_ERROR_NO_ERROR)
				status =
				    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&ping[i].amp, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
				                           (void **)&ping[i].bathacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
				                           (void **)&ping[i].bathalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(char), (void **)&ping[i].pixelflag,
				                           &error);
			if (error == MB_ERROR_NO_ERROR)
				status =
				    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ping[i].ss, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
				                           (void **)&ping[i].ssacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
				                           (void **)&ping[i].ssalongtrack, &error);
			if (datakind == MBFILTER_BATH) {
				// ndatapts = beams_bath;
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
					                           (void **)&ping[i].dataprocess, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
					                           (void **)&ping[i].datasave, &error);
			}
			else if (datakind == MBFILTER_AMP) {
				// ndatapts = beams_amp;
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
					                           (void **)&ping[i].dataprocess, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
					                           (void **)&ping[i].datasave, &error);
			}
			else if (datakind == MBFILTER_SS) {
				// ndatapts = pixels_ss;
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
					                           (void **)&ping[i].dataprocess, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
					                           (void **)&ping[i].datasave, &error);
			}
		}

		/* get ideal number of ping records to hold */
		int nhold_ping = 1;
		int nweightmax = 1;
		for (int i = 0; i < num_filters; i++) {
			nhold_ping = std::max(nhold_ping, filters[i].ldim);
			nweightmax = std::max(nweightmax, filters[i].xdim * filters[i].ldim);
		}

		/* allocate memory for weights */
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nweightmax * sizeof(double), (void **)&weights, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nweightmax * sizeof(double), (void **)&values, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nweightmax * sizeof(double), (void **)&distances, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* write comments to beginning of output file */
		kind = MB_DATA_COMMENT;
		snprintf(comment, sizeof(comment), "Data filtered by program %s", program_name);
		status = mb_put_comment(verbose, ombio_ptr, comment, &error);
		snprintf(comment, sizeof(comment), "MB-system Version %s", MB_VERSION);
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, &error);
		snprintf(comment, sizeof(comment), "Run by user <%s> on cpu <%s> at <%s>", user, host, date);
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		if (datakind == MBFILTER_BATH) {
			snprintf(comment, sizeof(comment), "Processing bathymetry data...");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		else if (datakind == MBFILTER_AMP) {
			snprintf(comment, sizeof(comment), "Processing beam amplitude data...");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		else if (datakind == MBFILTER_SS) {
			snprintf(comment, sizeof(comment), "Processing sidescan data...");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (hipass_mode == MBFILTER_HIPASS_MEAN) {
			snprintf(comment, sizeof(comment), "applying mean subtraction filter for hipass");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		else if (hipass_mode == MBFILTER_HIPASS_GAUSSIAN) {
			snprintf(comment, sizeof(comment), "applying gaussian mean subtraction filter for hipass");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		else if (hipass_mode == MBFILTER_HIPASS_MEDIAN) {
			snprintf(comment, sizeof(comment), "applying median subtraction filter for hipass");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (hipass_mode != MBFILTER_HIPASS_NONE) {
			snprintf(comment, sizeof(comment), "  filter acrosstrack dimension: %d", hipass_xdim);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter alongtrack dimension:  %d", hipass_ldim);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter iterations:            %d", hipass_iter);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter offset:                %f", hipass_offset);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (smooth_mode == MBFILTER_SMOOTH_MEAN) {
			snprintf(comment, sizeof(comment), "applying mean filter for smoothing");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		else if (smooth_mode == MBFILTER_SMOOTH_GAUSSIAN) {
			snprintf(comment, sizeof(comment), "applying gaussian mean filter for smoothing");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		else if (smooth_mode == MBFILTER_SMOOTH_MEDIAN) {
			snprintf(comment, sizeof(comment), "applying median filter for smoothing");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		else if (smooth_mode == MBFILTER_SMOOTH_GRADIENT) {
			snprintf(comment, sizeof(comment), "applying inverse gradient filter for smoothing");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (smooth_mode == MBFILTER_SMOOTH_MEDIAN && apply_threshold) {
			snprintf(comment, sizeof(comment), "  filter low ratio threshold:   %f", threshold_lo);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter high ratio threshold:  %f", threshold_hi);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (smooth_mode != MBFILTER_SMOOTH_NONE) {
			snprintf(comment, sizeof(comment), "  filter acrosstrack dimension: %d", smooth_xdim);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter alongtrack dimension:  %d", smooth_ldim);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter iterations:            %d", smooth_iter);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (contrast_mode == MBFILTER_CONTRAST_EDGE) {
			snprintf(comment, sizeof(comment), "applying edge detection filter for contrast enhancement");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		else if (contrast_mode == MBFILTER_CONTRAST_GRADIENT) {
			snprintf(comment, sizeof(comment), "applying gradient subtraction filter for contrast enhancement");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (contrast_mode != MBFILTER_CONTRAST_NONE) {
			snprintf(comment, sizeof(comment), "  filter acrosstrack dimension: %d", contrast_xdim);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter alongtrack dimension:  %d", contrast_ldim);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter iterations:            %d", contrast_iter);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		strncpy(comment, "", 256);
		snprintf(comment, sizeof(comment), "Control Parameters:");
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		strncpy(comment, "", 256);
		snprintf(comment, sizeof(comment), "  MBIO data format:   %d", format);
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		strncpy(comment, "", 256);
		snprintf(comment, sizeof(comment), "  Input file:         %s", file);
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		strncpy(comment, "", 256);
		snprintf(comment, sizeof(comment), "  Output file:        %s", ofile);
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		strncpy(comment, "", 256);
		snprintf(comment, sizeof(comment), "  Longitude flip:     %d", lonflip);
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		strncpy(comment, "", 256);
		snprintf(comment, sizeof(comment), "  Data kind:         %d", datakind);
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		strncpy(comment, "", 256);
		snprintf(comment, sizeof(comment), " ");
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);

		/* read and write */
		bool first = true;
		int ndata = 0;
		int nhold = 0;
		int nread = 0;
		int nwrite = 0;
		bool done = false;
    if (status != MB_SUCCESS)
      done = true;
		while (!done) {
			/* load some data into the buffer */
			error = MB_ERROR_NO_ERROR;
			int nload = 0;
			int nunload = 0;
			// const int nexpect = n_buffer_max - ndata;
			while (status == MB_SUCCESS && ndata < n_buffer_max) {
				status = mb_get_all(verbose, imbio_ptr, &store_ptr, &kind, ping[ndata].time_i, &ping[ndata].time_d,
				                    &ping[ndata].navlon, &ping[ndata].navlat, &ping[ndata].speed, &ping[ndata].heading,
				                    &ping[ndata].distance, &ping[ndata].altitude, &ping[ndata].sensordepth,
				                    &ping[ndata].beams_bath, &ping[ndata].beams_amp, &ping[ndata].pixels_ss, ping[ndata].beamflag,
				                    ping[ndata].bath, ping[ndata].amp, ping[ndata].bathacrosstrack, ping[ndata].bathalongtrack,
				                    ping[ndata].ss, ping[ndata].ssacrosstrack, ping[ndata].ssalongtrack, comment, &error);
				if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
					if (datakind == MBFILTER_SS) {
						for (int i = 0; i < ping[ndata].pixels_ss; i++) {
							if (ping[ndata].ss[i] > MB_SIDESCAN_NULL) {
								ping[ndata].pixelflag[i] = MB_FLAG_NONE;
              }
							else {
								ping[ndata].pixelflag[i] = MB_FLAG_NULL;
              }
						}
					}
					status = mb_extract_nav(verbose, imbio_ptr, store_ptr, &kind, ping[ndata].time_i, &ping[ndata].time_d,
					                        &ping[ndata].navlon, &ping[ndata].navlat, &ping[ndata].speed, &ping[ndata].heading,
					                        &ping[ndata].sensordepth, &ping[ndata].roll, &ping[ndata].pitch, &ping[ndata].heave,
					                        &error);
					status &= mb_extract_altitude(verbose, imbio_ptr, store_ptr, &kind, &ping[ndata].sensordepth,
					                             &ping[ndata].altitude, &error);
				}
				if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
					ndata++;
					nread++;
					nreadtot++;
					nload++;
				}
				if (status == MB_FAILURE && error < 0) {
					status = MB_SUCCESS;
					error = MB_ERROR_NO_ERROR;
				}
			}
			if (status == MB_FAILURE && error > 0) {
				status = MB_SUCCESS;
				error = MB_ERROR_NO_ERROR;
				done = true;
			}

			/* give the statistics */
			if (verbose >= 1) {
				fprintf(stderr, "%d records loaded into buffer\n", nload);
				fprintf(stderr, "%d records held in buffer\n", ndata);
			}

			/* get start of ping output range */
			const int jbeg = first ? 0 : std::min(nhold / 2 + 1, ndata);
			if (first) { first = false; }

			/* find number of pings to hold */
			if (done)
				nhold = 0;
			else if (ndata > nhold_ping)
				nhold = nhold_ping;
			else
				nhold = 0;

			/* get end of ping output range */
			int jend = done ? ndata - 1 : ndata - 1 - nhold / 2;

			if (jend < jbeg)
				jend = jbeg;
			if (verbose >= 1) {
				fprintf(stderr, "%d survey records being processed\n\n", (jend - jbeg + 1));
			}

			/* loop over all filters */
			for (int ifilter = 0; ifilter < num_filters; ifilter++) {
				int iteration = 0;
				const int ndx = filters[ifilter].xdim / 2;
				const int ndl = filters[ifilter].ldim / 2;

				while (iteration < filters[ifilter].iteration) {
					if (verbose > 0)
						fprintf(stderr, "Applying filter %d iteration %d of %d...\n", ifilter + 1, iteration + 1,
						        filters[ifilter].iteration);

					/* set in and out data arrays */
					for (int j = 0; j < ndata; j++) {
						if (datakind == MBFILTER_BATH) {
							ping[j].ndatapts = ping[j].beams_bath;
							ping[j].data_i_ptr = ping[j].bath;
							ping[j].flag_ptr = ping[j].beamflag;
						}
						else if (datakind == MBFILTER_AMP) {
							ping[j].ndatapts = ping[j].beams_amp;
							ping[j].data_i_ptr = ping[j].amp;
							ping[j].flag_ptr = ping[j].beamflag;
						}
						else if (datakind == MBFILTER_SS) {
							ping[j].ndatapts = ping[j].pixels_ss;
							ping[j].data_i_ptr = ping[j].ss;
							ping[j].flag_ptr = ping[j].pixelflag;
						}
						ping[j].data_f_ptr = ping[j].dataprocess;
					}

					/* loop over all the data */
					for (int j = 0; j < ndata; j++) {

						/* get beginning and end pings */
						int ja = j - ndl;
						int jb = j + ndl;
						if (ja < 0)
							ja = 0;
						if (jb >= ndata)
							jb = ndata - 1;

						/* get data arrays and sizes to be used */
						double *dataptr0 = ping[j].data_i_ptr;
						char *flagptr0 = ping[j].flag_ptr;
						const int ndatapts = ping[j].ndatapts;

						/* loop over each value */
						for (int i = 0; i < ndatapts; i++) {
							/* get beginning and end values */
							int ia = i - ndx;
							int ib = i + ndx;
							if (ia < 0)
								ia = 0;
							if (ib >= ndatapts)
								ib = ndatapts - 1;
							int nweight = 0;

							/* construct arrays of values and weights */
							if (mb_beam_ok(flagptr0[i])) {
								/* use primary value if valid */
								nweight = 1;
								values[0] = dataptr0[i];
								distances[0] = 0.0;

								/* loop over surrounding pings and values */
								for (int jj = ja; jj <= jb; jj++) {
									for (int ii = ia; ii <= ib; ii++) {
                    if (ii < ping[jj].ndatapts) {
  										double *dataptr1 = ping[jj].data_i_ptr;
  										char *flagptr1 = ping[jj].flag_ptr;
  										if ((jj != j || ii != i) && mb_beam_ok(flagptr1[ii])) {
  											values[nweight] = dataptr1[ii];
  											double ddis = 0.0;
                        if (ndx > 0) {
                          double di = ((double) (ii - i)) / ((double)ndx);
                          ddis += di * di;
                        }
                        if (ndl > 0) {
                          double dj = ((double) (jj - j)) / ((double)ndl);
                          ddis += dj * dj;
                        }
  											distances[nweight] = sqrt(ddis);
  											nweight++;
  										}
                    }
									}
								}
							}

							/* get filtered value */
							if (nweight > 0) {
								if (filters[ifilter].mode == MBFILTER_A_HIPASS_MEAN)
									hipass_mean(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]);
								else if (filters[ifilter].mode == MBFILTER_A_HIPASS_GAUSSIAN)
									hipass_gaussian(verbose, nweight, values, weights, distances, &ping[j].data_f_ptr[i]);
								else if (filters[ifilter].mode == MBFILTER_A_HIPASS_MEDIAN)
									hipass_median(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]);
								else if (filters[ifilter].mode == MBFILTER_A_SMOOTH_MEAN)
									smooth_mean(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]);
								else if (filters[ifilter].mode == MBFILTER_A_SMOOTH_GAUSSIAN)
									smooth_gaussian(verbose, nweight, values, weights, distances, &ping[j].data_f_ptr[i]);
								else if (filters[ifilter].mode == MBFILTER_A_SMOOTH_MEDIAN)
									smooth_median(verbose, dataptr0[i], filters[ifilter].threshold, filters[ifilter].threshold_lo,
									              filters[ifilter].threshold_hi, nweight, values, weights, &ping[j].data_f_ptr[i]);
								else if (filters[ifilter].mode == MBFILTER_A_SMOOTH_GRADIENT)
									smooth_gradient(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]);
								else if (filters[ifilter].mode == MBFILTER_A_CONTRAST_EDGE)
									contrast_edge(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]);
								else if (filters[ifilter].mode == MBFILTER_A_CONTRAST_GRADIENT)
									contrast_gradient(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]);
							}
							else {
								ping[j].data_f_ptr[i] = MB_SIDESCAN_NULL;
							}
						}
					}

					/* reset initial array and add offset
					    if done with final iteration */
					if (iteration == filters[ifilter].iteration - 1) {
						for (int j = 0; j < ndata; j++) {
							for (int i = 0; i < ping[j].ndatapts; i++) {
								ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i] + filters[ifilter].hipass_offset;
              }
            }
					} else {
						for (int j = 0; j < ndata; j++) {
							for (int i = 0; i < ping[j].ndatapts; i++) {
								ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i];
              }
            }
          }

					/* save results if done with final iteration */
					if (ndata > 0 && iteration == filters[ifilter].iteration - 1) {
						for (int j = jbeg; j <= jend; j++) {
							for (int i = 0; i < ping[j].ndatapts; i++) {
								ping[j].datasave[i] = ping[j].data_i_ptr[i];
              }
            }
					}

					iteration++;
				}
			}

			/* output pings to be cleared from buffer */
			if (ndata > 0)
				for (int j = jbeg; j <= jend; j++) {
					if (datakind == MBFILTER_BATH) {
						status &= mbcopy_any_to_mbldeoih(
						    verbose, system, MB_DATA_DATA, ping[j].time_i, ping[j].time_d, ping[j].navlon, ping[j].navlat,
						    ping[j].speed, ping[j].heading, ping[j].sensordepth, ping[j].altitude, ping[j].roll, ping[j].pitch,
						    ping[j].heave, imb_io_ptr->beamwidth_xtrack, imb_io_ptr->beamwidth_ltrack, ping[j].beams_bath, 0, 0,
						    ping[j].beamflag, ping[j].datasave, ping[j].amp, ping[j].bathacrosstrack, ping[j].bathalongtrack,
						    ping[j].ss, ping[j].ssacrosstrack, ping[j].ssalongtrack, comment, static_cast<char *>(ombio_ptr), static_cast<char *>(omb_io_ptr->store_data),
						    &error);
					}
					else if (datakind == MBFILTER_AMP) {
						status &= mbcopy_any_to_mbldeoih(
						    verbose, system, MB_DATA_DATA, ping[j].time_i, ping[j].time_d, ping[j].navlon, ping[j].navlat,
						    ping[j].speed, ping[j].heading, ping[j].sensordepth, ping[j].altitude, ping[j].roll, ping[j].pitch,
						    ping[j].heave, imb_io_ptr->beamwidth_xtrack, imb_io_ptr->beamwidth_ltrack, ping[j].beams_bath,
						    ping[j].beams_amp, 0, ping[j].beamflag, ping[j].bath, ping[j].datasave, ping[j].bathacrosstrack,
						    ping[j].bathalongtrack, ping[j].ss, ping[j].ssacrosstrack, ping[j].ssalongtrack, comment, static_cast<char *>(ombio_ptr),
						    static_cast<char *>(omb_io_ptr->store_data), &error);
					}

					else if (datakind == MBFILTER_SS) {
						status &= mbcopy_any_to_mbldeoih(
						    verbose, system, MB_DATA_DATA, ping[j].time_i, ping[j].time_d, ping[j].navlon, ping[j].navlat,
						    ping[j].speed, ping[j].heading, ping[j].sensordepth, ping[j].altitude, ping[j].roll, ping[j].pitch,
						    ping[j].heave, imb_io_ptr->beamwidth_xtrack, imb_io_ptr->beamwidth_ltrack, ping[j].beams_bath, 0,
						    ping[j].pixels_ss, ping[j].beamflag, ping[j].bath, ping[j].amp, ping[j].bathacrosstrack,
						    ping[j].bathalongtrack, ping[j].datasave, ping[j].ssacrosstrack, ping[j].ssalongtrack, comment,
						    static_cast<char *>(ombio_ptr), static_cast<char *>(omb_io_ptr->store_data), &error);
					}

					/* write the data */
					status &= mb_write_ping(verbose, ombio_ptr, omb_io_ptr->store_data, &error);
					if (status == MB_SUCCESS) {
						nunload++;
						nwrite++;
						nwritetot++;
					}
				}

			/* save processed data in buffer */
			if (ndata > nhold) {
				for (int j = 0; j < nhold; j++) {
					const int jj = ndata - nhold + j;
					for (int i = 0; i < 7; i++)
						ping[j].time_i[i] = ping[jj].time_i[i];
					ping[j].time_d = ping[jj].time_d;
					ping[j].navlon = ping[jj].navlon;
					ping[j].navlat = ping[jj].navlat;
					ping[j].speed = ping[jj].speed;
					ping[j].heading = ping[jj].heading;
					ping[j].distance = ping[jj].distance;
					ping[j].altitude = ping[jj].altitude;
					ping[j].sensordepth = ping[jj].sensordepth;
					ping[j].roll = ping[jj].roll;
					ping[j].pitch = ping[jj].pitch;
					ping[j].heave = ping[jj].heave;
					ping[j].beams_bath = ping[jj].beams_bath;
					ping[j].beams_amp = ping[jj].beams_amp;
					ping[j].pixels_ss = ping[jj].pixels_ss;
					for (int i = 0; i < ping[j].beams_bath; i++) {
						ping[j].beamflag[i] = ping[jj].beamflag[i];
						ping[j].bath[i] = ping[jj].bath[i];
						ping[j].bathacrosstrack[i] = ping[jj].bathacrosstrack[i];
						ping[j].bathalongtrack[i] = ping[jj].bathalongtrack[i];
					}
					for (int i = 0; i < ping[j].beams_amp; i++) {
						ping[j].amp[i] = ping[jj].amp[i];
					}
					for (int i = 0; i < ping[j].pixels_ss; i++) {
						ping[j].pixelflag[i] = ping[jj].pixelflag[i];
						ping[j].ss[i] = ping[jj].ss[i];
						ping[j].ssacrosstrack[i] = ping[jj].ssacrosstrack[i];
						ping[j].ssalongtrack[i] = ping[jj].ssalongtrack[i];
					}
					for (int i = 0; i < ping[jj].ndatapts; i++)
						ping[j].datasave[i] = ping[jj].datasave[i];
				}
				ndata = nhold;
			}

			/* give the statistics */
			if (verbose >= 1) {
				fprintf(stderr, "\n%d records written from buffer\n", nunload);
				fprintf(stderr, "%d records saved in buffer\n\n", ndata);
			}
		}

    status = MB_SUCCESS;
    error = MB_ERROR_NO_ERROR;

		status = mb_close(verbose, &imbio_ptr, &error);
		status = mb_close(verbose, &ombio_ptr, &error);

		/* give the statistics */
		if (verbose >= 1) {
			fprintf(stderr, "%d data records read from:  %s\n", nread, file);
			fprintf(stderr, "%d data records written to: %s\n\n", nwrite, ofile);
		}

		/* figure out whether and what to read next */
		if (read_datalist) {
			read_data = (mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS);
		}
		else {
			read_data = false;
		}
	} /* end loop over files in list */

	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	if (verbose >= 1) {
		fprintf(stderr, "%d total data records read\n", nreadtot);
		fprintf(stderr, "%d total data records written\n", nwritetot);
	}

  /* check memory */
  if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
    fprintf(stderr, "Program %s completed but failed to deallocate all allocated memory - the code has a memory leak somewhere!\n", program_name);
  }

	exit(error);
}
/*--------------------------------------------------------------------*/
