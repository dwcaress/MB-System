/*--------------------------------------------------------------------
 *    The MB-system:	mb_spline.c	10/11/00
 *
 *    Copyright (c) 2000-2023 by
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
 * This source file includes 1D interpolation routines.
 *
 * The 1D spline interpolation routines are modified from spline()
 * and splint() in the book:
 *   Press, W. H., S. A. Teukolsky, W. T. Vetterling, B. P. Flannery,
 *   Numerical Recipies in C: the Art of Scientific Computing,
 *   Cambridge University Press, 1988.
 * The 1D linear interpolation routine is homegrown, but mimics the
 * spline routines in usage.
 *
 * Author:	D. W. Caress
 * Date:	October 11, 2000
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "mb_define.h"
#include "mb_status.h"

/*--------------------------------------------------------------------------*/
int mb_spline_init(int verbose, const double *x, const double *y, int n, double yp1, double ypn, double *y2, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       x:                %p\n", x);
		fprintf(stderr, "dbg2       y:                %p\n", y);
		fprintf(stderr, "dbg2       n:                %d\n", n);
		fprintf(stderr, "dbg2       yp1:              %f\n", yp1);
		fprintf(stderr, "dbg2       ypn:              %f\n", ypn);
		fprintf(stderr, "dbg2       y2:               %p\n", y2);
	}

	int status = MB_SUCCESS;

	/* check for n > 2 */
	if (n < 3) {
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
	}

	/* allocate memory for working vector */
	double *u;
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose, __FILE__, __LINE__, n * sizeof(double), (void **)&u, error);


	/* set up spline interpolation coefficients */
	if (status == MB_SUCCESS) {
		if (yp1 > 0.99e30)
			y2[1] = u[1] = 0.0;
		else {
			y2[1] = -0.5;
			u[1] = (3.0 / (x[2] - x[1])) * ((y[2] - y[1]) / (x[2] - x[1]) - yp1);
		}
		for (int i = 2; i <= n - 1; i++) {
			const double sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
			const double p = sig * y2[i - 1] + 2.0;
			y2[i] = (sig - 1.0) / p;
			u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
			u[i] = (6.0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
		}
                double qn = 0.0;
                double un = 0.0;
		if (ypn > 0.99e30) {
			/* qn = un = 0.0; */
		} else {
			qn = 0.5;
			un = (3.0 / (x[n] - x[n - 1])) * (ypn - (y[n] - y[n - 1]) / (x[n] - x[n - 1]));
		}
		y2[n] = (un - qn * u[n - 1]) / (qn * y2[n - 1] + 1.0);
		for (int k = n - 1; k >= 1; k--)
			y2[k] = y2[k] * y2[k + 1] + u[k];

		/* deallocate memory for vector */
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&u, error);
	}

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
int mb_spline_interp(int verbose, const double *xa, const double *ya, double *y2a, int n, double x, double *y, int *i, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       xa:               %p\n", xa);
		fprintf(stderr, "dbg2       ya:               %p\n", ya);
		fprintf(stderr, "dbg2       y2a:              %p\n", y2a);
		fprintf(stderr, "dbg2       n:                %d\n", n);
		fprintf(stderr, "dbg2       x:                %f\n", x);
	}

	int status = MB_SUCCESS;

	/* check for n >= 1 */
	if (n < 1) {
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
	}

	/* perform interpolation */
	if (status == MB_SUCCESS) {
		int klo = 1;
		int khi = n;
		while (khi - klo > 1) {
			const int k = (khi + klo) >> 1;
			if (xa[k] > x)
				khi = k;
			else
				klo = k;
		}
		if (khi == 1)
			khi = 2;
		if (klo == n)
			klo = n - 1;
		const double h = xa[khi] - xa[klo];
		const double a = (xa[khi] - x) / h;
		const double b = (x - xa[klo]) / h;
		*y = a * ya[klo] + b * ya[khi] + ((a * a * a - a) * y2a[klo] + (b * b * b - b) * y2a[khi]) * (h * h) / 6.0;
		*i = klo;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       y:          %f\n", *y);
		fprintf(stderr, "dbg2       i:          %d\n", *i);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
// TODO(schwehr): What is the semantic meaning of the args?  i appears to be output only.
int mb_linear_interp(int verbose, const double *xa, const double *ya, int n, double x, double *y, int *i, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       xa:               %p\n", xa);
		fprintf(stderr, "dbg2       ya:               %p\n", ya);
		fprintf(stderr, "dbg2       n:                %d\n", n);
		fprintf(stderr, "dbg2       x:                %f\n", x);
	}

	int status = MB_SUCCESS;

	/* check for n >= 1 */
	if (n < 1) {
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
	} else {
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
	}
	/* perform interpolation */
	if (status == MB_SUCCESS) {
		/* do not extrapolate before or after the model, just use the
		 * first or last values */
		if (x <= xa[1]) {
			*y = ya[1];
			*i = 1;
		}
		else if (x >= xa[n]) {
			*y = ya[n];
			*i = n;
		}
		/* in range of model so linearly interpolate */
		else {
			int klo = 1;
			int khi = n;
			while (khi - klo > 1) {
				const int k = (khi + klo) >> 1;
				if (xa[k] > x)
					khi = k;
				else
					klo = k;
			}
			if (khi == 1)
				khi = 2;
			if (klo == n)
				klo = n - 1;
			const double h = xa[khi] - xa[klo];
			const double b = (ya[khi] - ya[klo]) / h;
			*y = ya[klo] + b * (x - xa[klo]);
			*i = klo;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       y:          %f\n", *y);
		fprintf(stderr, "dbg2       i:          %d\n", *i);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_linear_interp_longitude(int verbose, const double *xa, const double *ya, int n, double x, double *y, int *i, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       xa:               %p\n", xa);
		fprintf(stderr, "dbg2       ya:               %p\n", ya);
		fprintf(stderr, "dbg2       n:                %d\n", n);
		fprintf(stderr, "dbg2       x:                %f\n", x);
	}

	int status = MB_SUCCESS;

	/* check for n >= 1 */
	if (n < 1) {
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
	}

	/* perform interpolation */
	if (status == MB_SUCCESS) {
		/* do not extrapolate before or after the model, just use the
		 * first or last values */
		if (x <= xa[1]) {
			*y = ya[1];
			*i = 1;
		}
		else if (x >= xa[n]) {
			*y = ya[n];
			*i = n;
		}
		/* in range of model so linearly interpolate */
		else {
			int klo = 1;
			int khi = n;
			while (khi - klo > 1) {
				const int k = (khi + klo) >> 1;
				if (xa[k] > x)
					khi = k;
				else
					klo = k;
			}
			if (khi == 1)
				khi = 2;
			if (klo == n)
				klo = n - 1;
			const double h = xa[khi] - xa[klo];
			double yahi = ya[khi];
			const double yalo = ya[klo];
			if (yahi - yalo > 180.0)
				yahi -= 360.0;
			else if (yahi - yalo < -180.0)
				yahi += 360.0;
			const double b = (yahi - yalo) / h;
			*y = ya[klo] + b * (x - xa[klo]);
			if (*y >= 180.0)
				*y -= 360.0;
			else if (*y < -180.0)
				*y += 360.0;
			*i = klo;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       y:          %f\n", *y);
		fprintf(stderr, "dbg2       i:          %d\n", *i);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_linear_interp_latitude(int verbose, const double *xa, const double *ya, int n, double x, double *y, int *i, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       xa:               %p\n", xa);
		fprintf(stderr, "dbg2       ya:               %p\n", ya);
		fprintf(stderr, "dbg2       n:                %d\n", n);
		fprintf(stderr, "dbg2       x:                %f\n", x);
	}

	int status = MB_SUCCESS;

	/* check for n >= 1 */
	if (n < 1) {
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
	}

	/* perform interpolation */
	if (status == MB_SUCCESS) {
		/* do not extrapolate before or after the model, just use the
		 * first or last values */
		if (x <= xa[1]) {
			*y = ya[1];
			*i = 1;
		}
		else if (x >= xa[n]) {
			*y = ya[n];
			*i = n;
		}
		/* in range of model so linearly interpolate */
		else {
			int klo = 1;
			int khi = n;
			while (khi - klo > 1) {
				const int k = (khi + klo) >> 1;
				if (xa[k] > x)
					khi = k;
				else
					klo = k;
			}
			if (khi == 1)
				khi = 2;
			if (klo == n)
				klo = n - 1;
			const double h = xa[khi] - xa[klo];
			const double yahi = ya[khi];
			const double yalo = ya[klo];
			const double b = (yahi - yalo) / h;
			*y = ya[klo] + b * (x - xa[klo]);
			if (*y > 90.0)
				*y = 90.0;
			else if (*y < -90.0)
				*y = -90.0;
			*i = klo;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       y:          %f\n", *y);
		fprintf(stderr, "dbg2       i:          %d\n", *i);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_linear_interp_heading(int verbose, const double *xa, const double *ya, int n, double x, double *y, int *i, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       xa:               %p\n", xa);
		fprintf(stderr, "dbg2       ya:               %p\n", ya);
		fprintf(stderr, "dbg2       n:                %d\n", n);
		fprintf(stderr, "dbg2       x:                %f\n", x);
	}

	int status = MB_SUCCESS;

	/* check for n >= 1 */
	if (n < 1) {
		status = MB_FAILURE;
		*error = MB_ERROR_NOT_ENOUGH_DATA;
	}

	/* perform interpolation */
	if (status == MB_SUCCESS) {
		/* do not extrapolate before or after the model, just use the
		 * first or last values */
		if (x <= xa[1]) {
			*y = ya[1];
			*i = 1;
		}
		else if (x >= xa[n]) {
			*y = ya[n];
			*i = n;
		}
		/* in range of model so linearly interpolate */
		else {
			int klo = 1;
			int khi = n;
			while (khi - klo > 1) {
				const int k = (khi + klo) >> 1;
				if (xa[k] > x)
					khi = k;
				else
					klo = k;
			}
			if (khi == 1)
				khi = 2;
			if (klo == n)
				klo = n - 1;
			const double h = xa[khi] - xa[klo];
			double yahi = ya[khi];
			const double yalo = ya[klo];
			if (yahi - yalo > 180.0)
				yahi -= 360.0;
			else if (yahi - yalo < -180.0)
				yahi += 360.0;
			const double b = (yahi - yalo) / h;
			*y = ya[klo] + b * (x - xa[klo]);
			if (*y >= 360.0)
				*y -= 360.0;
			else if (*y < 0.0)
				*y += 360.0;
			*i = klo;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       y:          %f\n", *y);
		fprintf(stderr, "dbg2       i:          %d\n", *i);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
