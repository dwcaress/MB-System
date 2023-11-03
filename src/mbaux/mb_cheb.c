/*--------------------------------------------------------------------
 *    The MB-system:	mb_cheb.c			3/23/00
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
 * The mb_cheb library is a least squares matrix solver using
 * Richardson's algorithm with chebyshev acceleration. The step size is
 * varied to obtain uniform convergence over a prescribed
 * range of eigenvalues. This code is a C translation of Fortran 77
 * code distributed by Dr. Allen H. Olson in 1987 following the
 * publication of
 *   Olson, A. H., A Chebyshev condition for accelerating
 *   convergance of iterative tomographic methods - Solving
 *   large least squares problems, Phys. Earth Planet. Inter.,
 *   47, 333-345, 1987.
 * This algorithm is both time and memory efficient for large,
 * sparse least squares problems. The instructions for use are
 * given in comments below.
 *
 *
 *
 *   This C version of LSQR was first created by
 *      Michael P Friedlander <mpf@cs.ubc.ca>
 *   as part of his BCLS package:
 *      http://www.cs.ubc.ca/~mpf/bcls/index.html.
 *  The present file is maintained by
 *      Michael Saunders <saunders@stanford.edu>
 *
 *   31 Aug 2007: First version of this file lsqr.c obtained from
 *                Michael Friedlander's BCLS package, svn version number
 *                $Revision: 273 $ $Date: 2006-09-04 15:59:04 -0700 (Mon, 04 Sep 2006) $
 *
 *                The stopping rules were slightly altered in that version.
 *                They have been restored to the original rules used in the f77 LSQR.
 *
 *   13 September 2016
 *                The lsqr library was added to MB-System by David Caress
 *                after accessing the source code at
 *                    http://web.stanford.edu/group/SOL/software/lsqr/
 *
 * Author:	D. W. Caress
 * Date:	March 23, 2000
 *
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_aux.h"
#include "mb_define.h"

/*----------------------------------------------------------------------
 *
 *     least squares solution using richardson's algorithm
 *     with chebyshev acceleration.  the step size is varied to obtain
 *     uniform convergence over a prescribed range of eigenvalues.
 *
 *                             ..... allen h. olson 6-29-85.
 *                                   university of california, san diego
 *
 *     Altered to allow particular values to be fixed.
 *                             ..... David W. Caress 4-14-88
 *----------------------------------------------------------------------
 *
 *                       nc                   ~
 *          given :     sum [ a(j,i) * x(j) ] = d(i)   ;  i=1, ... nr
 *                      j=1
 *                                               t
 *          minimize :  || a*x - d || = (a*x - d) * (a*x -d)
 *
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
 * -------
 *  input
 * -------
 *     a(j,i)  -------  - packed matrix defined above dimensioned at least
 *                        (nnz,nr) note  that the first index is the column
 *                        index! i.e. the matrix is stored in row order.
 *     ia(j,i)  ------  - indices of values in packed matrix a
 *                        i.e. a(j,i) packed = a(j,ia(i,j)) unpacked
 *     nia(i)  -------  - number of nonzero values in each row
 *     nnz   ---------  - number of values in packed rows of a and ia
 *     nc,nr  --------  - number of columns and number of rows of unpacked
 *                        matrix as defined above.
 *     x(1...nc) -----  - initial guess solution; can be set to zero
 *                        or values returned form previous calls to lsquc.
 *     dx(1...nc) ----  - temporary storage array
 *     d(1...nr) -----  - data as defined above
 *     nfix  ---------  - number of solution values to be fixed
 *     ifix  ---------  - idices of fixed values
 *     fix  ----------  - fixed values:  x(ifix(j)) = fix(j)
 *     ncycle  -------  - number of iterations to perform. must be power of 2.
 *     sigma(1..ncycle) - array containing the weights for step sizes.
 *                        see subroutine 'chebyu' for computing these.
 * --------
 *  output
 * --------
 *     x(1..nc)   - solution vector as defined above
 *                  only array x(..) is over-written by lsquc.
 *----------------------------------------------------------------------*/
void lsqup(const double *a, const int *ia, const int *nia, int nnz, int nc, int nr,
           double *x, double *dx, const double *d, int nfix, const int *ifix,
           const double *fix, int ncycle, const double *sigma)
{
	/* loop over all cycles */
	for (int icyc = 0; icyc < ncycle; icyc++) {
		/* initialize dx */
		for (int j = 0; j < nc; j++)
			dx[j] = 0.0;

		/* loop over each row */
		for (int i = 0; i < nr; i++) {
			double res = 0.0;
			for (int j = 0; j < nia[i]; j++) {
				const int k = nnz * i + j;
				res += a[k] * x[ia[k]];
			}
			res = d[i] - res;
			for (int j = 0; j < nia[i]; j++) {
				const int k = nnz * i + j;
				dx[ia[k]] += res * a[k];
			}
		}

		/* update x */
		for (int j = 0; j < nc; j++)
			x[j] += dx[j] / sigma[icyc];

		/* apply fixed values */
		for (int j = 0; j < nfix; j++)
			x[ifix[j]] = fix[j];
	}
}
/*----------------------------------------------------------------------
 *
 * computes the chebyshev weights with uniform distribution.
 * weights are ordered pair-wise in such a fashion that after an even
 * number of steps they are distributed uniformly on the interval [slo,shi].
 * this ordering provides optimum numerical stability of routine lsquc.
 *
 *                             ..... allen h. olson 6-29-85.
 *                                   university of california, san diego
 *
 *----------------------------------------------------------------------
 * -------
 *  input
 * -------
 *
 *     ncycle   ------  - must be a power of two!  number of iterations.
 *
 *     shi,slo  ------  - high and low limits defining the band of eigenvalues
 *                        to retain in the solution.  shi >= largest eigenvalue
 *                        of the normal equations.
 *     work(1..ncycle)  - work array for sorting array sigma(..).
 *
 * -------
 *  output
 * -------
 *
 *     sigma(1..ncycle) - weights for the step sizes in routine lsquc.
 * -------
 * calls function splits.
 *
 *----------------------------------------------------------------------*/
void chebyu(double *sigma, int ncycle, double shi, double slo, double *work)
{
	/* set up the chebyshev weights in increasing order */
	for (int i = 0; i < ncycle; i++) {
		sigma[i] = -cos((2 * (i + 1) - 1) * M_PI / 2 / ncycle);
		sigma[i] = (sigma[i] * (shi - slo) + (shi + slo)) / 2;
	}

	/* sort the weights */
	int len = ncycle;
	while (len > 2) {
		const int nsort = ncycle / len;
		for (int is = 0; is < nsort; is++) {
			const int i0 = is * len;
			splits(&sigma[i0], work, len);
		}
		len /= 2;
	}
}
/*----------------------------------------------------------------------*/
void splits(double *x, double *t, int n) {
	int l = 0;
	for (int i = 0; i < n; i += 2) {
		t[l] = x[i];
		l++;
	}
	for (int i = 1; i < n; i += 2) {
		t[l] = x[i];
		l++;
	}

	const int nb2 = n / 2;
	const int nb2m1 = nb2 - 1;
	if (nb2 >= 2) {
		for (int i = 0; i < nb2; i++) {
			x[i] = t[nb2m1 - i];
		}
		for (int i = nb2; i < n; i++) {
			x[i] = t[i];
		}
	} else {
		for (int i = 0; i < n; i++)
			x[i] = t[i];
	}
}
/*----------------------------------------------------------------------*/
/* returns limit of the maximum theoretical error using chebyshev weights */
double errlim(double *sigma, int ncycle, double shi, double slo)
{
	double errlim = 1.0;
	const double delta = 0.25 * (shi - slo);
	for (int i = 0; i < ncycle; i++) {
		errlim *= delta / sigma[i];
	}
	errlim = 2 * errlim;
	return (errlim);
}
/*----------------------------------------------------------------------*/
/* computes the ratio of the error at eigenvalue x1 to the error at x2. */
double errrat(double x1, double x2, double *sigma, int ncycle)
{
	double errrat = 1.0;
	const double rat = x1 / x2;
	for (int k = 0; k < ncycle; k++) {
		errrat = errrat * rat * (1.0 - sigma[k] / x1) / (1.0 - sigma[k] / x2);
	}
	errrat = fabs(errrat);
	return (errrat);
}
/*----------------------------------------------------------------------
 *    least-squares eigenvalue
 *
 *    iteratively estimates the largest eigenvalue and eigenvectior
 *    with error bounds for the least-squares normal matrix a'a.
 *    a chebyshev criterion is used to calculate the optimum set of
 *    origin shifts in order to accelerate convergence.
 *    based upon the rayleigh quotient and error analysis presented in
 *    j. h. wilkinson's "the algebraic eigenvalue problem",
 *    (pp 170 ...), (pp 572 ...).
 *    under very pesimistic assumptions regarding the starting vector,
 *    the algorithm will initially converge to an eigenvalue less than
 *    the largest.  hence, a corresponding pessimistic estimate of an
 *    upper-bound on the largest eigenvalue is also made.
 *
 *                            ..... allen h. olson 10-4-85.
 *                                  university of california, san diego
 *----------------------------------------------------------------------*/
/*-------
 * input
 *-------
 *    a(j,i)  -------  - packed matrix defined above dimensioned at least
 *                       (nnz,nr) note  that the first index is the column
 *                       index! i.e. the matrix is stored in row order.
 *    ia(j,i)  ------  - indices of values in packed matrix a
 *                       i.e. a(j,i) packed = a(j,ia(i,j)) unpacked
 *    nia(i)  -------  - number of nonzero values in each row
 *    nnz   ---------  - number of values in packed rows of a and ia
 *    nc,nr  --------  - number of columns and number of rows of unpacked
 *                       matrix as defined above.
 *    ncyc  ---------  - number of chebyshev iterations to perform.
 *                         must be a power of two.
 *    nsig  ---------  - cumulative number of iterations performed by
 *                         previous calls to this routine.  must be set
 *                         to zero on initial call.  nsig is automatically
 *                         incremented by this routine and must not be
 *                         redefined on subsequent calls by calling program.
 *    x(1..nc)  -----  - initial guess for the eigenvector.
 *    dx(1...nc) ----  - temporary storage array for x(.).
 *    sigma(1..nsmx)-  - array for holding the chebyshev origin shifts.
 *                         each call to lseig performs ncyc+1 iterations.
 *                         nsmx must be greater than or equal to the
 *                         cumulative number of iterations to be performed.
 *    w(1..nsmx)  ---  - temporary storage array for sigma(.).
 *    smax  ---------  - initial guess for the eigenvalue.
 *--------
 * output
 *--------
 *    x(1...nc) -----  - revised estimate of largest eigenvector.
 *    smax  ---------  - reivsed estimate of largest eigenvalue of a'a.
 *    err   ---------  - error bound for smax. we are guaranteed that
 *                         at least one eigenvalue is contained in the
 *                         interval smax+-err.  in the neighborhood of
 *                         convergence, this will contain the maximum.
 *    sup   ---------  - a pessimistic upper bound for the largest
 *                         eigenvector.
 *------
 * note:(1) for ncyc=0, an initial guess for the eigenvector is formed by
 *------  summing the rows of the matrix so that the accumulated vector
 *        increases in length as each row is added.  one iteration of the
 *        power method is then performed to estimate smax.
 *      (2) caution: an anomalous bad guess for the initial eigenvector
 *        being virtually orthogonal to the largest eigenvector
 *        will cause earlier iterations to converge to the next largest
 *        eigenvector.  this is impossible to detect.  in this case, smax
 *        may be much less than the largest eigenvalue.
 *          the parameter eps set below is a pessimistic assumption about
 *        the relative size of the component of the largest eigenvector in
 *        the initial iteration.  from this, an upper-bound is calculated
 *        for the largest eigenvalue, sup.  sup will always be larger than
 *        smax and reflects the uncertainty due to an anomalous bad choice
 *        for the starting vector.
 *-------------------------
 * sample calling sequence
 *-------------------------
 *    ncyc=0
 *    nsig=0
 *    call lseig(a,nc,nr,ncyc,nsig,x,dx,sigma,w,smax,err,sup)
 *    ncyc=4
 *    call lseig(a,nc,nr,ncyc,nsig,x,dx,sigma,w,smax,err,sup)
 *    ncyc=8
 *    call lseig(a,nc,nr,ncyc,nsig,x,dx,sigma,w,smax,err,sup)
 *
 *    the first call with ncyc=0 initializes x(.) and smax.  if these
 *      are already known then ncyc can be set to a nonzero value for the
 *      first call.  nsig must always be zero for the first call however.
 *    the next two calls perform chebyshev iteration to improve x(.) and smax.
 *      upon completion, a total of nsig=1+5+9=15 iterations have actually
 *      been performed.
 *    by making repeated calls to lseig, the error in smax and the difference
 *      between smax and sup can be monitored until the desired level of
 *      certainty is obtained. */
/*----------------------------------------------------------------------*/
void lspeig(const double *a, const int *ia, const int *nia, int nnz, int nc,
            int nr, int ncyc, int *nsig, double *x, double *dx, double *sigma,
            double *w, double *smax, double *err, double *sup)
{
	int i;
	double eps = 1.0e-6;
	double res = 0.0;
	double slo;
	double smp;
	double errsmp;

	if (ncyc == 0) {
		i = 0;  /* TODO(schwehr): Bug? */
		for (int j = 0; j < nia[i]; j++) {
			const int k = nnz * i + j;
			x[ia[k]] = a[k];
		}
		for (i = 1; i < nr; i++) {
			res = 0.0;
			for (int j = 0; j < nia[i]; j++) {
				const int k = nnz * i + j;
				res += x[ia[k]] * a[k];
			}
			if (fabs(res) <= 1.0e-30)
				res = 1.0;
			else
				res = res / fabs(res);
			for (int j = 0; j < nia[i]; j++) {
				const int k = nnz * i + j;
				x[ia[k]] += res * a[k];
			}
		}
		res = 0.0;
		for (int j = 0; j < nc; j++) {
			res += x[j] * x[j];
		}
		res = 1.0 / sqrt(res);
		for (int j = 0; j < nc; j++) {
			x[j] = x[j] * res;
		}
	} else {
		slo = 0.0;
		chebyu(&sigma[*nsig], ncyc, *smax, slo, w);
	}

	int nsig1 = *nsig + 1;
	*nsig = nsig1 + ncyc;
	sigma[*nsig - 1] = 0.0;
	for (int icyc = nsig1 - 1; icyc < *nsig; icyc++) {
		for (int j = 0; j < nc; j++) {
			dx[j] = 0.0;
		}
		for (int i = 0; i < nr; i++) {
			res = 0.0;
			for (int j = 0; j < nia[i]; j++) {
				const int k = nnz * i + j;
				res += a[k] * x[ia[k]];
			}
			for (int j = 0; j < nia[i]; j++) {
				const int k = nnz * i + j;
				dx[ia[k]] += res * a[k];
			}
		}
		for (int j = 0; j < nc; j++) {
			dx[j] -= sigma[icyc] * x[j];
		}
		*smax = 0.0;
		for (int j = 0; j < nc; j++) {
			*smax += dx[j] * dx[j];
		}
		*smax = sqrt(*smax);

		if (icyc == *nsig - 1) {
			*err = 0.0;
			for (int j = 0; j < nc; j++) {
				res = dx[j] - *smax * x[j];
				*err += res * res;
			}
			*err = sqrt(*err);
		}

		for (int j = 0; j < nc; j++) {
			x[j] = dx[j] / *smax;
		}
	}

	slo = *smax;
	*sup = (1.0 + eps) * (*smax) * pow(eps, -1.0 / *nsig);
	res = 1.0;
	for (int icyc = 0; icyc < 25 && res > eps; icyc++) {
		smp = 0.5 * (*sup + slo);
		errsmp = errrat(*smax, smp, sigma, *nsig);
		if (errsmp > eps)
			slo = smp;
		else
			*sup = smp;
		res = (*sup - slo) / slo;
	}
}

// ---------------------------------------------------------------------
/* bccblas.c
   $Revision: 231 $ $Date: 2006-04-15 18:47:05 -0700 (Sat, 15 Apr 2006) $

   ----------------------------------------------------------------------
   This file is part of BCLS (Bound-Constrained Least Squares).

   Copyright (C) 2006 Michael P. Friedlander, Department of Computer
   Science, University of British Columbia, Canada. All rights
   reserved. E-mail: <mpf@cs.ubc.ca>.

   BCLS is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   BCLS is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
   Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with BCLS; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
   USA
   ----------------------------------------------------------------------
*/
/*!
   \file

   This file contains C-wrappers to the BLAS (Basic Linear Algebra
   Subprograms) routines that are used by BCLS.  Whenever possible,
   they should be replaced by corresponding BLAS routines that have
   been optimized to the machine being used.

   Included BLAS routines:

   - mbcblas_daxpy
   - mbcblas_dcopy
   - mbcblas_ddot
   - mbcblas_dnrm2
   - mbcblas_dscal
*/

/*!
  \param[in]     N
  \param[in]     alpha
  \param[in]     X
  \param[in]     incX
  \param[in,out] Y
  \param[in]     incY
*/
void mbcblas_daxpy(const int N, const double alpha, const double *X, const int incX, double *Y, const int incY) {
	if (N <= 0)
		return;
	if (alpha == 0.0)
		return;

	if (incX == 1 && incY == 1) {
		const int m = N % 4;

		for (int i = 0; i < m; i++)
			Y[i] += alpha * X[i];

		for (int i = m; i + 3 < N; i += 4) {
			Y[i] += alpha * X[i];
			Y[i + 1] += alpha * X[i + 1];
			Y[i + 2] += alpha * X[i + 2];
			Y[i + 3] += alpha * X[i + 3];
		}
	}
	else {
		int ix = MBCBLAS_OFFSET(N, incX);
		int iy = MBCBLAS_OFFSET(N, incY);

		for (int i = 0; i < N; i++) {
			Y[iy] += alpha * X[ix];
			ix += incX;
			iy += incY;
		}
	}
}

/*!
  \param[in]     N
  \param[in]     X
  \param[in]     incX
  \param[out]    Y
  \param[in]     incY
*/
void mbcblas_dcopy(const int N, const double *X, const int incX, double *Y, const int incY) {
	int ix = MBCBLAS_OFFSET(N, incX);
	int iy = MBCBLAS_OFFSET(N, incY);

	for (int i = 0; i < N; i++) {
		Y[iy] = X[ix];
		ix += incX;
		iy += incY;
	}
}

/*!
  \param[in]     N
  \param[in]     X
  \param[in]     incX
  \param[in]     Y
  \param[in]     incY

  \return  Dot product of X and Y.

*/
double mbcblas_ddot(const int N, const double *X, const int incX, const double *Y, const int incY) {
	double r = 0.0;
	int ix = MBCBLAS_OFFSET(N, incX);
	int iy = MBCBLAS_OFFSET(N, incY);

	for (int i = 0; i < N; i++) {
		r += X[ix] * Y[iy];
		ix += incX;
		iy += incY;
	}

	return r;
}

/*!
  \param[in]     N
  \param[in]     X
  \param[in]     incX

  \return Two-norm of X.
*/
double mbcblas_dnrm2(const int N, const double *X, const int incX) {
	if (N <= 0 || incX <= 0)
		return 0;
	else if (N == 1)
		return fabs(X[0]);

	double scale = 0.0;
	double ssq = 1.0;
	int ix = 0;
	for (int i = 0; i < N; i++) {
		const double x = X[ix];

		if (x != 0.0) {
			const double ax = fabs(x);

			if (scale < ax) {
				ssq = 1.0 + ssq * (scale / ax) * (scale / ax);
				scale = ax;
			} else {
				ssq += (ax / scale) * (ax / scale);
			}
		}

		ix += incX;
	}

	return scale * sqrt(ssq);
}

/*!
  \param[in]     N
  \param[in]     alpha
  \param[in,out] X
  \param[in]     incX
*/
void mbcblas_dscal(const int N, const double alpha, double *X, const int incX) {
	if (incX <= 0)
		return;

	int ix = MBCBLAS_OFFSET(N, incX);

	for (int i = 0; i < N; i++) {
		X[ix] *= alpha;
		ix += incX;
	}
}

/*----------------------------------------------------------------------*/
// START LSQR SOURCE CODE
//
// ---------------------------------------------------------------------
// d2norm  returns  sqrt( a**2 + b**2 )  with precautions
// to avoid overflow.
//
// 21 Mar 1990: First version.
// ---------------------------------------------------------------------
static double mblsqr_d2norm(const double a, const double b) {
	const double zero = 0.0;

	double scale = fabs(a) + fabs(b);
	if (scale == zero)
		return zero;
	else
		return scale * sqrt((a / scale) * (a / scale) + (b / scale) * (b / scale));
}

static void mblsqr_dload(const int n, const double alpha, double x[]) {
	for (int i = 0; i < n; i++)
		x[i] = alpha;
	return;
}

// LSQR  finds a solution x to the following problems:
//
// 1. Unsymmetric equations --    solve  A*x = b
//
// 2. Linear least squares  --    solve  A*x = b
//                                in the least-squares sense
//
// 3. Damped least squares  --    solve  (   A    )*x = ( b )
//                                       ( damp*I )     ( 0 )
//                                in the least-squares sense
//
// where A is a matrix with m rows and n columns, b is an
// m-vector, and damp is a scalar.  (All quantities are real.)
// The matrix A is intended to be large and sparse.  It is accessed
// by means of subroutine calls of the form
//
//            aprod ( mode, m, n, x, y, UsrWrk )
//
// which must perform the following functions:
//
//            If mode = 1, compute  y = y + A*x.
//            If mode = 2, compute  x = x + A(transpose)*y.
//
// The vectors x and y are input parameters in both cases.
// If  mode = 1,  y should be altered without changing x.
// If  mode = 2,  x should be altered without changing y.
// The parameter UsrWrk may be used for workspace as described
// below.
//
// The rhs vector b is input via u, and subsequently overwritten.
//
//
// Note:  LSQR uses an iterative method to approximate the solution.
// The number of iterations required to reach a certain accuracy
// depends strongly on the scaling of the problem.  Poor scaling of
// the rows or columns of A should therefore be avoided where
// possible.
//
// For example, in problem 1 the solution is unaltered by
// row-scaling.  If a row of A is very small or large compared to
// the other rows of A, the corresponding row of ( A  b ) should be
// scaled up or down.
//
// In problems 1 and 2, the solution x is easily recovered
// following column-scaling.  Unless better information is known,
// the nonzero columns of A should be scaled so that they all have
// the same Euclidean norm (e.g., 1.0).
//
// In problem 3, there is no freedom to re-scale if damp is
// nonzero.  However, the value of damp should be assigned only
// after attention has been paid to the scaling of A.
//
// The parameter damp is intended to help regularize
// ill-conditioned systems, by preventing the true solution from
// being very large.  Another aid to regularization is provided by
// the parameter acond, which may be used to terminate iterations
// before the computed solution becomes very large.
//
// Note that x is not an input parameter.
// If some initial estimate x0 is known and if damp = 0,
// one could proceed as follows:
//
//   1. Compute a residual vector     r0 = b - A*x0.
//   2. Use LSQR to solve the system  A*dx = r0.
//   3. Add the correction dx to obtain a final solution x = x0 + dx.
//
// This requires that x0 be available before and after the call
// to LSQR.  To judge the benefits, suppose LSQR takes k1 iterations
// to solve A*x = b and k2 iterations to solve A*dx = r0.
// If x0 is "good", norm(r0) will be smaller than norm(b).
// If the same stopping tolerances atol and btol are used for each
// system, k1 and k2 will be similar, but the final solution x0 + dx
// should be more accurate.  The only way to reduce the total work
// is to use a larger stopping tolerance for the second system.
// If some value btol is suitable for A*x = b, the larger value
// btol*norm(b)/norm(r0)  should be suitable for A*dx = r0.
//
// Preconditioning is another way to reduce the number of iterations.
// If it is possible to solve a related system M*x = b efficiently,
// where M approximates A in some helpful way
// (e.g. M - A has low rank or its elements are small relative to
// those of A), LSQR may converge more rapidly on the system
//       A*M(inverse)*z = b,
// after which x can be recovered by solving M*x = z.
//
// NOTE: If A is symmetric, LSQR should not be used!
// Alternatives are the symmetric conjugate-gradient method (cg)
// and/or SYMMLQ.
// SYMMLQ is an implementation of symmetric cg that applies to
// any symmetric A and will converge more rapidly than LSQR.
// If A is positive definite, there are other implementations of
// symmetric cg that require slightly less work per iteration
// than SYMMLQ (but will take the same number of iterations).
//
//
// Notation
// --------
//
// The following quantities are used in discussing the subroutine
// parameters:
//
// Abar   =  (   A    ),          bbar  =  ( b )
//           ( damp*I )                    ( 0 )
//
// r      =  b  -  A*x,           rbar  =  bbar  -  Abar*x
//
// rnorm  =  sqrt( norm(r)**2  +  damp**2 * norm(x)**2 )
//        =  norm( rbar )
//
// relpr  =  the relative precision of floating-point arithmetic
//           on the machine being used.  On most machines,
//           relpr is about 1.0e-7 and 1.0d-16 in single and double
//           precision respectively.
//
// LSQR  minimizes the function rnorm with respect to x.
//
//
// Parameters
// ----------
//
// m       input      m, the number of rows in A.
//
// n       input      n, the number of columns in A.
//
// aprod   external   See above.
//
// damp    input      The damping parameter for problem 3 above.
//                    (damp should be 0.0 for problems 1 and 2.)
//                    If the system A*x = b is incompatible, values
//                    of damp in the range 0 to sqrt(relpr)*norm(A)
//                    will probably have a negligible effect.
//                    Larger values of damp will tend to decrease
//                    the norm of x and reduce the number of
//                    iterations required by LSQR.
//
//                    The work per iteration and the storage needed
//                    by LSQR are the same for all values of damp.
//
// rw      workspace  Transit pointer to user's workspace.
//                    Note:  LSQR  does not explicitly use this
//                    parameter, but passes it to subroutine aprod for
//                    possible use as workspace.
//
// u(m)    input      The rhs vector b.  Beware that u is
//                    over-written by LSQR.
//
// v(n)    workspace
//
// w(n)    workspace
//
// x(n)    output     Returns the computed solution x.
//
// se(*)   output     If m .gt. n  or  damp .gt. 0,  the system is
//         (maybe)    overdetermined and the standard errors may be
//                    useful.  (See the first LSQR reference.)
//                    Otherwise (m .le. n  and  damp = 0) they do not
//                    mean much.  Some time and storage can be saved
//                    by setting  se = NULL.  In that case, se will
//                    not be touched.
//
//                    If se is not NULL, then the dimension of se must
//                    be n or more.  se(1:n) then returns standard error
//                    estimates for the components of x.
//                    For each i, se(i) is set to the value
//                       rnorm * sqrt( sigma(i,i) / t ),
//                    where sigma(i,i) is an estimate of the i-th
//                    diagonal of the inverse of Abar(transpose)*Abar
//                    and  t = 1      if  m .le. n,
//                         t = m - n  if  m .gt. n  and  damp = 0,
//                         t = m      if  damp .ne. 0.
//
// atol    input      An estimate of the relative error in the data
//                    defining the matrix A.  For example,
//                    if A is accurate to about 6 digits, set
//                    atol = 1.0e-6 .
//
// btol    input      An estimate of the relative error in the data
//                    defining the rhs vector b.  For example,
//                    if b is accurate to about 6 digits, set
//                    btol = 1.0e-6 .
//
// conlim  input      An upper limit on cond(Abar), the apparent
//                    condition number of the matrix Abar.
//                    Iterations will be terminated if a computed
//                    estimate of cond(Abar) exceeds conlim.
//                    This is intended to prevent certain small or
//                    zero singular values of A or Abar from
//                    coming into effect and causing unwanted growth
//                    in the computed solution.
//
//                    conlim and damp may be used separately or
//                    together to regularize ill-conditioned systems.
//
//                    Normally, conlim should be in the range
//                    1000 to 1/relpr.
//                    Suggested value:
//                    conlim = 1/(100*relpr)  for compatible systems,
//                    conlim = 1/(10*sqrt(relpr)) for least squares.
//
//         Note:  If the user is not concerned about the parameters
//         atol, btol and conlim, any or all of them may be set
//         to zero.  The effect will be the same as the values
//         relpr, relpr and 1/relpr respectively.
//
// itnlim  input      An upper limit on the number of iterations.
//                    Suggested value:
//                    itnlim = n/2   for well-conditioned systems
//                                   with clustered singular values,
//                    itnlim = 4*n   otherwise.
//
// nout    input      File number for printed output.  If positive,
//                    a summary will be printed on file nout.
//
// istop   output     An integer giving the reason for termination:
//
//            0       x = 0  is the exact solution.
//                    No iterations were performed.
//
//            1       The equations A*x = b are probably
//                    compatible.  Norm(A*x - b) is sufficiently
//                    small, given the values of atol and btol.
//
//            2       damp is zero.  The system A*x = b is probably
//                    not compatible.  A least-squares solution has
//                    been obtained that is sufficiently accurate,
//                    given the value of atol.
//
//            3       damp is nonzero.  A damped least-squares
//                    solution has been obtained that is sufficiently
//                    accurate, given the value of atol.
//
//            4       An estimate of cond(Abar) has exceeded
//                    conlim.  The system A*x = b appears to be
//                    ill-conditioned.  Otherwise, there could be an
//                    error in subroutine aprod.
//
//            5       The iteration limit itnlim was reached.
//
// itn     output     The number of iterations performed.
//
// anorm   output     An estimate of the Frobenius norm of  Abar.
//                    This is the square-root of the sum of squares
//                    of the elements of Abar.
//                    If damp is small and if the columns of A
//                    have all been scaled to have length 1.0,
//                    anorm should increase to roughly sqrt(n).
//                    A radically different value for anorm may
//                    indicate an error in subroutine aprod (there
//                    may be an inconsistency between modes 1 and 2).
//
// acond   output     An estimate of cond(Abar), the condition
//                    number of Abar.  A very high value of acond
//                    may again indicate an error in aprod.
//
// rnorm   output     An estimate of the final value of norm(rbar),
//                    the function being minimized (see notation
//                    above).  This will be small if A*x = b has
//                    a solution.
//
// arnorm  output     An estimate of the final value of
//                    norm( Abar(transpose)*rbar ), the norm of
//                    the residual for the usual normal equations.
//                    This should be small in all cases.  (arnorm
//                    will often be smaller than the true value
//                    computed from the output vector x.)
//
// xnorm   output     An estimate of the norm of the final
//                    solution vector x.
//
// References:
//
// C.C. Paige and M.A. Saunders,  LSQR: An algorithm for sparse
//      linear equations and sparse least squares,
//      ACM Transactions on Mathematical Software 8, 1 (March 1982),
//      pp. 43-71.
//
// C.C. Paige and M.A. Saunders,  Algorithm 583, LSQR: Sparse
//      linear equations and least-squares problems,
//      ACM Transactions on Mathematical Software 8, 2 (June 1982),
//      pp. 195-209.
//
// C.L. Lawson, R.J. Hanson, D.R. Kincaid and F.T. Krogh,
//      Basic linear algebra subprograms for Fortran usage,
//      ACM Transactions on Mathematical Software 5, 3 (Sept 1979),
//      pp. 308-323 and 324-325.
//
// Michael A. Saunders                  mike@sol-michael.stanford.edu
// Dept of Operations Research          na.Msaunders@na-net.ornl.gov
// Stanford University
// Stanford, CA 94305-4022              (415) 723-1875
void mblsqr_lsqr(int m, int n, void (*aprod)(int mode, int m, int n, double x[], double y[], void *UsrWrk), double damp,
                 void *UsrWrk,
                 double u[],  // len = m
                 double v[],  // len = n
                 double w[],  // len = n
                 double x[],  // len = n
                 double se[], // len at least n.  May be NULL.
                 double atol, double btol, double conlim, int itnlim, FILE *nout,
                 // The remaining variables are output only.
                 int *istop_out, int *itn_out, double *anorm_out, double *acond_out, double *rnorm_out, double *arnorm_out,
                 double *xnorm_out) {
	const bool extra = false; // true for extra printing below.
	const bool damped = damp > ZERO;
	const bool wantse = se != NULL;

	const char *enter = "Enter LSQR.  ";
	const char *exit = "Exit  LSQR.  ";
	const char msg[6][100] = {{"The exact solution is  x = 0"},
	                    {"A solution to Ax = b was found, given atol, btol"},
	                    {"A least-squares solution was found, given atol"},
	                    {"A damped least-squares solution was found, given atol"},
	                    {"Cond(Abar) seems to be too large, given conlim"},
	                    {"The iteration limit was reached"}};
	//-----------------------------------------------------------------------

	//  Initialize.

	if (nout != NULL)
		fprintf(nout,
		        " %s        Least-squares solution of  Ax = b\n"
		        " The matrix  A  has %7d rows  and %7d columns\n"
		        " damp   = %-22.2e    wantse = %10i\n"
		        " atol   = %-22.2e    conlim = %10.2e\n"
		        " btol   = %-22.2e    itnlim = %10d\n\n",
		        enter, m, n, damp, wantse, atol, conlim, btol, itnlim);

	int itn = 0;
	int istop = 0;
	int nstop = 0;
	int maxdx = 0;
	const double ctol = conlim > ZERO ? ONE / conlim : ZERO;
	double anorm = ZERO;
	double acond = ZERO;
	double xnorm = ZERO;

	double dnorm = ZERO;
	double dxmax = ZERO;
	double res2 = ZERO;
	double psi = ZERO;
	xnorm = ZERO;
	double xnorm1 = ZERO;
	double cs2 = -ONE;
	double sn2 = ZERO;
	double z = ZERO;

	double test1 = 0.0;
	double test2 = 0.0;
	double test3 = 0.0;

	//  ------------------------------------------------------------------
	//  Set up the first vectors u and v for the bidiagonalization.
	//  These satisfy  beta*u = b,  alpha*v = A(transpose)*u.
	//  ------------------------------------------------------------------
	mblsqr_dload(n, 0.0, v);
	mblsqr_dload(n, 0.0, x);

	if (wantse)
		mblsqr_dload(n, 0.0, se);

	double alpha = ZERO;
	double beta = mbcblas_dnrm2(m, u, 1);

	if (beta > ZERO) {
		mbcblas_dscal(m, (ONE / beta), u, 1);
		aprod(2, m, n, v, u, UsrWrk);
		alpha = mbcblas_dnrm2(n, v, 1);
	}

	if (alpha > ZERO) {
		mbcblas_dscal(n, (ONE / alpha), v, 1);
		mbcblas_dcopy(n, v, 1, w, 1);
	}

	// TODO(schwehr): Localize
	int nconv;
	double bnorm = 0.0;
	double rnorm = 0.0;
	double alfopt, cs, cs1, delta, dknorm, dxk, gamma, gambar, phi;
	double rho, rhbar1, rhs, rtol, sn, sn1, t, tau, temp, theta, t1, t2, t3;
	double zbar;

	double arnorm0 = alpha * beta;
	double arnorm = arnorm0;
	// if (arnorm == ZERO)
	//	goto goto_800;
	if (arnorm != ZERO) {
		double rhobar = alpha;
		double phibar = beta;
		bnorm = beta;
		rnorm = beta;

	if (nout != NULL) {
		if (damped)
			fprintf(nout, "    Itn       x(1)           Function"
			              "     Compatible    LS      Norm Abar   Cond Abar\n");
		else
			fprintf(nout, "    Itn       x(1)           Function"
			              "     Compatible    LS      Norm A   Cond A\n");

		test1 = ONE;
		test2 = alpha / beta;

		if (extra)
			fprintf(nout, "     phi    dknorm  dxk  alfa_opt\n");

		fprintf(nout, " %6d %16.9e %16.9e %9.2e %9.2e\n", itn, x[0], rnorm, test1, test2);
		fprintf(nout, "\n");
	}

	//  ==================================================================
	//  Main iteration loop.
	//  ==================================================================
	while (1) {
		itn = itn + 1;

		//      ------------------------------------------------------------------
		//      Perform the next step of the bidiagonalization to obtain the
		//      next  beta, u, alpha, v.  These satisfy the relations
		//                 beta*u  =  A*v  -  alpha*u,
		//                alpha*v  =  A(transpose)*u  -  beta*v.
		//      ------------------------------------------------------------------
		mbcblas_dscal(m, (-alpha), u, 1);
		aprod(1, m, n, v, u, UsrWrk);
		beta = mbcblas_dnrm2(m, u, 1);

		//      Accumulate  anorm = || Bk ||
		//                        =  sqrt( sum of  alpha**2 + beta**2 + damp**2 ).

		temp = mblsqr_d2norm(alpha, beta);
		temp = mblsqr_d2norm(temp, damp);
		anorm = mblsqr_d2norm(anorm, temp);

		if (beta > ZERO) {
			mbcblas_dscal(m, (ONE / beta), u, 1);
			mbcblas_dscal(n, (-beta), v, 1);
			aprod(2, m, n, v, u, UsrWrk);
			alpha = mbcblas_dnrm2(n, v, 1);
			if (alpha > ZERO) {
				mbcblas_dscal(n, (ONE / alpha), v, 1);
			}
		}

		//      ------------------------------------------------------------------
		//      Use a plane rotation to eliminate the damping parameter.
		//      This alters the diagonal (rhobar) of the lower-bidiagonal matrix.
		//      ------------------------------------------------------------------
		rhbar1 = rhobar;
		if (damped) {
			rhbar1 = mblsqr_d2norm(rhobar, damp);
			cs1 = rhobar / rhbar1;
			sn1 = damp / rhbar1;
			psi = sn1 * phibar;
			phibar = cs1 * phibar;
		}

		//      ------------------------------------------------------------------
		//      Use a plane rotation to eliminate the subdiagonal element (beta)
		//      of the lower-bidiagonal matrix, giving an upper-bidiagonal matrix.
		//      ------------------------------------------------------------------
		rho = mblsqr_d2norm(rhbar1, beta);
		cs = rhbar1 / rho;
		sn = beta / rho;
		theta = sn * alpha;
		rhobar = -cs * alpha;
		phi = cs * phibar;
		phibar = sn * phibar;
		tau = sn * phi;

		//      ------------------------------------------------------------------
		//      Update  x, w  and (perhaps) the standard error estimates.
		//      ------------------------------------------------------------------
		t1 = phi / rho;
		t2 = -theta / rho;
		t3 = ONE / rho;
		dknorm = ZERO;

		if (wantse) {
			for (int i = 0; i < n; i++) {
				t = w[i];
				x[i] = t1 * t + x[i];
				w[i] = t2 * t + v[i];
				t = (t3 * t) * (t3 * t);
				se[i] = t + se[i];
				dknorm = t + dknorm;
			}
		}
		else {
			for (int i = 0; i < n; i++) {
				t = w[i];
				x[i] = t1 * t + x[i];
				w[i] = t2 * t + v[i];
				dknorm = (t3 * t) * (t3 * t) + dknorm;
			}
		}

		//      ------------------------------------------------------------------
		//      Monitor the norm of d_k, the update to x.
		//      dknorm = norm( d_k )
		//      dnorm  = norm( D_k ),        where   D_k = (d_1, d_2, ..., d_k )
		//      dxk    = norm( phi_k d_k ),  where new x = x_k + phi_k d_k.
		//      ------------------------------------------------------------------
		dknorm = sqrt(dknorm);
		dnorm = mblsqr_d2norm(dnorm, dknorm);
		dxk = fabs(phi * dknorm);
		if (dxmax < dxk) {
			dxmax = dxk;
			maxdx = itn;
		}

		//      ------------------------------------------------------------------
		//      Use a plane rotation on the right to eliminate the
		//      super-diagonal element (theta) of the upper-bidiagonal matrix.
		//      Then use the result to estimate  norm(x).
		//      ------------------------------------------------------------------
		delta = sn2 * rho;
		gambar = -cs2 * rho;
		rhs = phi - delta * z;
		zbar = rhs / gambar;
		xnorm = mblsqr_d2norm(xnorm1, zbar);
		gamma = mblsqr_d2norm(gambar, theta);
		cs2 = gambar / gamma;
		sn2 = theta / gamma;
		z = rhs / gamma;
		xnorm1 = mblsqr_d2norm(xnorm1, z);

		//      ------------------------------------------------------------------
		//      Test for convergence.
		//      First, estimate the norm and condition of the matrix  Abar,
		//      and the norms of  rbar  and  Abar(transpose)*rbar.
		//      ------------------------------------------------------------------
		acond = anorm * dnorm;
		res2 = mblsqr_d2norm(res2, psi);
		rnorm = mblsqr_d2norm(res2, phibar);
		arnorm = alpha * fabs(tau);

		//      Now use these norms to estimate certain other quantities,
		//      some of which will be small near a solution.

		alfopt = sqrt(rnorm / (dnorm * xnorm));
		test1 = rnorm / bnorm;
		test2 = ZERO;
		if (rnorm > ZERO)
			test2 = arnorm / (anorm * rnorm);
		//      if (arnorm0 > ZERO) test2 = arnorm / arnorm0;  //(Michael Friedlander's modification)
		test3 = ONE / acond;
		t1 = test1 / (ONE + anorm * xnorm / bnorm);
		rtol = btol + atol * anorm * xnorm / bnorm;

		//      The following tests guard against extremely small values of
		//      atol, btol  or  ctol.  (The user may have set any or all of
		//      the parameters  atol, btol, conlim  to zero.)
		//      The effect is equivalent to the normal tests using
		//      atol = relpr,  btol = relpr,  conlim = 1/relpr.

		t3 = ONE + test3;
		t2 = ONE + test2;
		t1 = ONE + t1;
		if (itn >= itnlim)
			istop = 5;
		if (t3 <= ONE)
			istop = 4;
		if (t2 <= ONE)
			istop = 2;
		if (t1 <= ONE)
			istop = 1;

		//      Allow for tolerances set by the user.

		if (test3 <= ctol)
			istop = 4;
		if (test2 <= atol)
			istop = 2;
		if (test1 <= rtol)
			istop = 1; //(Michael Friedlander had this commented out)

		//      ------------------------------------------------------------------
		//      See if it is time to print something.
		//      ------------------------------------------------------------------
		if (nout == NULL)
			goto goto_600;
		if (n <= 40)
			goto goto_400;
		if (itn <= 10)
			goto goto_400;
		if (itn >= itnlim - 10)
			goto goto_400;
		if (itn % 100 == 0)
			goto goto_400;
		// if (test3 <=  2.0*ctol) goto goto_400;
		// if (test2 <= 2.0*atol) goto goto_400;
		// if (test1 <= 2.0*rtol) goto goto_400;
		if (istop != 0)
			goto goto_400;
		goto goto_600;

	//      Print a line for this iteration.
	//      "extra" is for experimental purposes.

	goto_400:
		if (extra) {
			fprintf(nout, " %6d %16.9e %16.9e %9.2e %9.2e %8.1e %8.1e %8.1e %7.1e %7.1e %7.1e\n", itn, x[0], rnorm, test1, test2,
			        anorm, acond, phi, dknorm, dxk, alfopt);
		}
		else {
			fprintf(nout, " %6d %16.9e %16.9e %9.2e %9.2e %8.1e %8.1e  Convergence: %9.2e %9.2e\n", itn, x[0], rnorm, test1,
			        test2, anorm, acond, test1 / rtol, test2 / atol);
		}
	// if (itn % 10 == 0) fprintf(nout, "\n");

	//      ------------------------------------------------------------------
	//      Stop if appropriate.
	//      The convergence criteria are required to be met on  nconv
	//      consecutive iterations, where  nconv  is set below.
	//      Suggested value:  nconv = 1, 2  or  3.
	//      ------------------------------------------------------------------
	goto_600:
		if (istop == 0) {
			nstop = 0;
		}
		else {
			nconv = 1;
			nstop = nstop + 1;
			if (nstop < nconv && itn < itnlim)
				istop = 0;
		}

		if (istop != 0)
			break;
	}
	//  ==================================================================
	//  End of iteration loop.
	//  ==================================================================

	//  Finish off the standard error estimates.

	if (wantse) {
		t = ONE;
		if (m > n)
			t = m - n;
		if (damped)
			t = m;
		t = rnorm / sqrt(t);

		for (int i = 0; i < n; i++)
			se[i] = t * sqrt(se[i]);
	}

//  Decide if istop = 2 or 3.
//  Print the stopping condition.
	} // goto_800:

	if (damped && istop == 2)
		istop = 3;
	if (nout != NULL) {
		fprintf(nout,
		        "\n"
		        " %s       istop  = %-10d      itn    = %-10d\n"
		        " %s       anorm  = %11.5e     acond  = %11.5e\n"
		        " %s       vnorm  = %11.5e     xnorm  = %11.5e\n"
		        " %s       rnorm  = %11.5e     arnorm = %11.5e\n",
		        exit, istop, itn, exit, anorm, acond, exit, bnorm, xnorm, exit, rnorm, arnorm);
		fprintf(nout,
		        " %s       max dx = %7.1e occurred at itn %-9d\n"
		        " %s              = %7.1e*xnorm\n",
		        exit, dxmax, maxdx, exit, dxmax / (xnorm + 1.0e-20));
		fprintf(nout, " %s       %s\n", exit, msg[istop]);
	}

	//  Assign output variables from local copies.
	*istop_out = istop;
	*itn_out = itn;
	*anorm_out = anorm;
	*acond_out = acond;
	*rnorm_out = rnorm;
	*arnorm_out = test2;
	*xnorm_out = xnorm;

	return;
}
// ---------------------------------------------------------------------
