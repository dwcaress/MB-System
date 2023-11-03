/*--------------------------------------------------------------------
 *    The MB-system:	mb_zgrid.c	    4/25/95
 *
 *    Copyright (c) 1993-2023 by
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
 * This is a function to generate thin plate spline interpolation
 * of a data field. This code originated as fortran in the
 * 1960's and was used routinely at the Institute of
 * Geophysics and Planetary Physics at the Scripps Institution
 * of Oceanography through the 1970's and 1980's. The Fortran
 * code was obtained from Professory Robert L. Parker at
 * IGPP in 1989. A version similar and possibly identical to this
 * can be found online (March 27, 2020) at:
 *    https://oceanai.mit.edu/svn/oases-aro/contour/zgrid.f
 *
 * The Fortran code was modified by David Caress and then was converted
 * to C in 1995 for use with the MB-System software package.
 *
 * The nature of the interpolation is controlled by the
 * parameters cay and nrng: cay sets the tension of the
 * interpolation such that cay=0.0 yields a pure Laplace
 * (minimum curvature) solution and cay=infinity yields
 * a pure thin plate spline solution. A cay=1e10 value
 * has commonly been used to yield spline solutions.
 * The nrng value sets the number of grid spaces from
 * data that will be interpolated; if nrng exceeds the
 * maximum dimension of the grid then the entire grid
 * will be interpolated.
 *
 * The input parameters are:
 *     nx,ny = max subscripts of z in x and y directions .
 *     x1,y1 = coordinates of z(1,1)
 *     dx,dy = x and y increments .
 *     xyz(3,*) = array giving x-y position and hgt of each data point.
 *     n = length of xyz series.
 *     zpij[n] = float work array
 *     knxt[n] = int work array
 *     imnew[MAX(nx, ny)+1] = bool work array
 *     cay = k = amount of spline eqn (between 0 and inf.)
 *     nrng...grid points more than nrng grid spaces from the nearest
 *            data point are set to undefined.
 *
 * Author:	Unknown, but "jdt", "ian crain",  and "dr t murty"
 *              obviously contributed.
 * Hacker:	D. W. Caress
 * Date:	April 25, 1995
 *
 * The following are the original comments from the Fortran code:
 *
 *     sets up square grid for contouring , given arbitrarily placed
 *     data points. laplace interpolation is used.
 *     the method used here was lifted directly from notes left by
 *     mr ian crain formerly with the comp.science div.
 *     info on relaxation soln of laplace eqn supplied by dr t murty.
 *     fortran ii   oceanography/emr   dec/68   jdt
 *
 *     z = 2-d array of hgts to be set up. points outside region to be
 *     contoured should be initialized to 10**35 . the rest should be 0.0
 *
 *     modification feb/69   to get smoother results a portion of the
 *     beam eqn  was added to the laplace eqn giving
 *     delta2x(z)+delta2y(z) - k(delta4x(z)+delta4y(z)) = 0 .
 *     k=0 gives pure laplace solution.  k=infinity gives pure spline
 *     solution.
 *
 *     nx,ny = max subscripts of z in x and y directions .
 *     x1,y1 = coordinates of z(1,1)
 *     dx,dy = x and y increments .
 *     xyz(3,*) = array giving x-y position and hgt of each data point.
 *     n = length of xyz series.
 *     cay = k = amount of spline eqn (between 0 and inf.)
 *     nrng...grid points more than nrng grid spaces from the nearest
 *            data point are set to undefined.
 *
 *     modification dec23/69   data pts no longer moved to grid pts.
 *
 *     modification feb/85  common blocks work1 and work2 replaced by
 *     dimension statement and parameters nwork, mwork introduced.
 *
 *     modification feb/90  nwork and mwork replaced by maxdat and maxdim
 *     for compatibility with command driven interface program
 *     David W. Caress
 *
 * zgrid.c -- translated by f2c (version 19950314) from zgrid.f.
 * Work arrays zpij[n], knxt[n], imnew[MAX(nx, ny)+1] are now
 * passed into the function.
 * David W. Caress
 * April 25,  1995
 *
 * Added function mb_zgrid2() with same calling parameters
 * - This function does interpolation onto a grid with a maximum dimension
 *   of 500 and then translates that to the desired grid by bilinear
 *   interpolation
 * - this approach is much faster and also yields nicer results - the
 *   zgrid algorithm does not seem to work well with small grid cell
 *   sizes.
 * David W. Caress
 * 2 October 2012
 *
 *--------------------------------------------------------------------*/

/* TODO(schwehr): Remove gotos */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"

const int ITERMIN = 50;
const int ITERMAX = 1000;
const int ITERTRANSITION = 100;

const int ZGRID_DIMENSION_MAX = 500;

/*----------------------------------------------------------------------- */
int mb_zgrid2(float *z, int *nx, int *ny, float *x1, float *y1, float *dx, float *dy, float *xyz, int *n, float *zpij, int *knxt,
              bool *imnew, float *cay, int *nrng) {
	int status = MB_SUCCESS;

	/* if nx and ny < ZGRID_DIMENSION_MAX just call zgrid() */
	if (*nx < ZGRID_DIMENSION_MAX && *ny < ZGRID_DIMENSION_MAX) {
		fprintf(stderr, "Zgrid2 calling zgrid with unchanged grid dimensions %d %d\n", *nx, *ny);
		mb_zgrid(z, nx, ny, x1, y1, dx, dy, xyz, n, zpij, knxt, imnew, cay, nrng);
	}

	/* else set up to call zgrid() to generate a smaller grid and then
	   use bilinear interpolation to map that onto the desired grid */
	else {
		/* get scale reduction factor and new dimensions and cell sizes */
		const double dx_d = (double)*dx;
		const double dy_d = (double)*dy;
		const double sfactor = ((double)(ZGRID_DIMENSION_MAX)) / MAX(*nx, *ny);
		int snx = (int)(sfactor * (*nx)) + 1;
		int sny = (int)(sfactor * (*ny)) + 1;
		const double sdx_d = (dx_d * (*nx - 1)) / snx;
		const double sdy_d = (dy_d * (*ny - 1)) / sny;
		int snrng = (int)(sfactor * (*nrng)) + 1;
		float sdx = (float)sdx_d;
		float sdy = (float)sdy_d;

		/* allocate array for intermediate grid */
		const int verbose = 0;
		float *sz = NULL;
		int error = MB_ERROR_NO_ERROR;
		status = mb_mallocd(verbose, __FILE__, __LINE__, snx * sny * sizeof(float), (void **)&sz, &error);
		memset((void *)sz, (int)0, (size_t)(snx * sny * sizeof(float)));

		/* call zgrid() */
		fprintf(stderr, "Smooth surface being calculated for grid with dimensions reduced from %d %d to %d %d\n", *nx, *ny, snx,
		        sny);
		mb_zgrid(sz, &snx, &sny, x1, y1, &sdx, &sdy, xyz, n, zpij, knxt, imnew, cay, &snrng);

		/* now fill in the full resolution grid by bilinear interpolation */
		fprintf(stderr, "Smooth surface mapped onto full resolution grid using bilinear interpolation\n");
		for (int i = 0; i < *nx; i++)
			for (int j = 0; j < *ny; j++) {
				const int k = i + j * (*nx);

				const double xi = i * dx_d;
				const double yj = j * dy_d;
				int si = xi / sdx_d;
				int sj = yj / sdy_d;
				if (si >= snx - 1)
					si = snx - 2;
				if (si < 0)
					si = 0;
				if (sj >= sny - 1)
					sj = sny - 2;
				if (sj < 0)
					sj = 0;

				const int sk00 = si + sj * snx;
				const int sk10 = (si + 1) + sj * snx;
				const int sk01 = si + (sj + 1) * snx;
				const int sk11 = (si + 1) + (sj + 1) * snx;

				if (sz[sk00] < 5.0e34f && sz[sk10] < 5.0e34f && sz[sk01] < 5.0e34f && sz[sk11] < 5.0e34f) {
					const double sx0 = si * sdx_d;
					const double sx1 = (si + 1) * sdx_d;
					const double sy0 = sj * sdy_d;
					const double sy1 = (sj + 1) * sdy_d;

					z[k] = (float)((((double)sz[sk00]) * (sx1 - xi) * (sy1 - yj) + ((double)sz[sk10]) * (xi - sx0) * (sy1 - yj) +
					                ((double)sz[sk01]) * (sx1 - xi) * (yj - sy0) + ((double)sz[sk11]) * (xi - sx0) * (yj - sy0)) /
					               (sdx_d * sdy_d));
				}
				else
					z[k] = 1.0e35f;
			}

		/* free array for intermediate grid */
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&sz, &error);
	}

	/* return */
	return (status);
}

/*----------------------------------------------------------------------- */
int mb_zgrid(float *z, int *nx, int *ny, float *x1, float *y1, float *dx, float *dy, float *xyz, int *n, float *zpij, int *knxt,
             bool *imnew, float *cay, int *nrng) {
	/* Parameter adjustments */
	int z_dim1 = *nx;
	int z_offset = z_dim1 + 1;
	z -= z_offset;
	xyz -= 4;

	/* Function Body */
	int nmax;
	if (*nx > *ny)
		nmax = *nx;
	else
		nmax = *ny;
	/* if (*nrng < nmax)
	    nmax = *nrng; */
	float eps = ((float)nmax) * 0.000016;
	if (eps < 0.02)
		eps = 0.02;

	/* trying dzmax > eps as a simple convergence criterea */
	float dzcriteria = 0.001;
	float convtestlast = 0.0;

	float big = 9.0e29f;
	int nconvtestincrease = 0;

	// get zbase which will make all zp values positive by 20*(zmax-zmin)

	float zmin = xyz[6];
	float zmax = xyz[6];
	int i__1 = *n;
	for (int k = 2; k <= i__1; ++k) {
		if (xyz[k * 3 + 3] - zmax <= 0.0f) {
		} else {
			zmax = xyz[k * 3 + 3];
		}
		if (xyz[k * 3 + 3] - zmin >= 0.0f) {
		} else {
			zmin = xyz[k * 3 + 3];
		}
	}
	const float zrange = zmax - zmin;
	const float zbase = zrange * 20.0f - zmin;
	const float hrange = MIN(*dx * (*nx - 1), *dy * (*ny - 1));
	const float derzm = zrange * 2.0f / hrange;

	int i__3;
	float r__1;
	float delz;
	int nnew;
	float zijn, zimm, zjmm, zipp, zjpp;
	float root, zsum, zpxy, a, b, c, d;
	int j;
	float x, y, delzm;
	float dzmax, dzrms;
	int im, jm;
	float dzrms8 = 0.0f;  // TODO(schwehr): -Wmaybe-uninitialized
	float z00;
	float dz;
	float ze;
	float zn, zs, zw;
	float dzmaxf, convtest, relaxn, rootgs, dzrmsp, abz;
	int npg;
	float zim, zjm;
	int npt;
	float wgt, zip, zjp, tpy, zxy;
	int ii, jj, kkk;

	// set pointer array knxt

	i__1 = *n;
	int kk;
	for (kk = 1; kk <= i__1; ++kk) {
		const int k = *n + 1 - kk;
		knxt[k - 1] = 0;
		const int i = (xyz[k * 3 + 1] - *x1) / *dx + 1.5f;
		if (i * (*nx + 1 - i) <= 0) {
			continue;
		}

		j = (xyz[k * 3 + 2] - *y1) / *dy + 1.5f;
		if (j * (*ny + 1 - j) <= 0) {
			continue;
		}

		if (z[i + j * z_dim1] - big >= 0.0f) {
			continue;
		}
		knxt[k - 1] = *n + 1;
		if (z[i + j * z_dim1] <= 0.0f) {
		} else {
			knxt[k - 1] = z[i + j * z_dim1] + 0.5f;
		}

		z[i + j * z_dim1] = (float)k;
	}

	// affix each data point zp to its nearby grid point.  take avg zp if
	// more than one zp nearby the grid point. add zbase and complement.

	i__1 = *n;
	for (int k = 1; k <= i__1; ++k) {
		if (knxt[k - 1] <= 0) {
			continue;
		}
		npt = 0;
		zsum = 0.0f;
		const int i = (xyz[k * 3 + 1] - *x1) / *dx + 1.5f;
		j = (xyz[k * 3 + 2] - *y1) / *dy + 1.5f;
		kk = k;
	L70:
		++npt;
		zsum += xyz[kk * 3 + 3];
		knxt[kk - 1] = -knxt[kk - 1];
		kk = -knxt[kk - 1];
		if (kk <= 0) {
		} else if (kk - *n <= 0) {
			goto L70;
		}

		z[i + j * z_dim1] = -(double)zsum / npt - zbase;
	}

	for (ii = 1; ii <= *nx; ++ii) {
		for (jj = 1; jj <= *ny; ++jj) {
			if (z[ii + jj * z_dim1] < big && z[ii + jj * z_dim1] > 0.0) {
				kkk = (int)z[ii + jj * z_dim1];
				/* fprintf(stderr,"ERROR: i:%d j:%d k:%d z:%f     iii:%d jjj:%d xyz:%f %f %f\n",
				ii,jj,ii+jj*z_dim1,z[ii + jj * z_dim1],iii,jjj,xyz[kkk * 3 + 1],xyz[kkk * 3 + 2],xyz[kkk * 3 + 3]); */
				z[ii + jj * z_dim1] = -xyz[kkk * 3 + 3] - zbase;
			}
		}
	}

	// initially set each unset grid point to value of nearest known pt.

	i__1 = *nx;
	for (int i = 1; i <= i__1; ++i) {
		const int i__2 = *ny;
		for (j = 1; j <= i__2; ++j) {
			if (z[i + j * z_dim1] != 0.0f) {
				continue;
			}
			z[i + j * z_dim1] = (float)-1e35;
		}
	}
	int i__2 = *nrng;
	bool jmnew = false;
	for (int iter = 1; iter <= i__2; ++iter) {
		nnew = 0;
		i__1 = *nx;
		for (int i = 1; i <= i__1; ++i) {
			i__3 = *ny;
			for (j = 1; j <= i__3; ++j) {
				if (z[i + j * z_dim1] + big >= 0.0f) {
					goto L192;
				}
				if (j - 1 <= 0) {
					goto L162;
				}
				if (jmnew) {
					goto L162;
				}
				zijn = (r__1 = z[i + (j - 1) * z_dim1], (float)fabs((double)r__1));
				if (zijn - big >= 0.0f) {
				}
				else {
					goto L195;
				}
			L162:
				if (i - 1 <= 0) {
					goto L172;
				}
				if (imnew[j - 1]) {
					goto L172;
				}
				zijn = (r__1 = z[i - 1 + j * z_dim1], (float)fabs((double)r__1));
				if (zijn - big >= 0.0f) {
				}
				else {
					goto L195;
				}
			L172:
				if (j - *ny >= 0) {
					goto L182;
				}
				zijn = (r__1 = z[i + (j + 1) * z_dim1], (float)fabs((double)r__1));
				if (zijn - big >= 0.0f) {
				} else {
					goto L195;
				}
			L182:
				if (i - *nx >= 0) {
					goto L192;
				}
				zijn = (r__1 = z[i + 1 + j * z_dim1], (float)fabs((double)r__1));
				if (zijn - big >= 0.0f) {
				} else {
					goto L195;
				}
			L192:
				imnew[j - 1] = false;
				jmnew = false;
				goto L197;
			L195:
				imnew[j - 1] = true;
				jmnew = true;
				z[i + j * z_dim1] = zijn;
				++nnew;
			L197:;
			}
		}
		if (nnew <= 0) {
			goto L200;
		}
	}
L200:
	i__2 = *nx;
	for (int i = 1; i <= i__2; ++i) {
		i__3 = *ny;
		for (j = 1; j <= i__3; ++j) {
			abz = (r__1 = z[i + j * z_dim1], (float)fabs((double)r__1));
			if (abz - big >= 0.0f) {
				goto L201;
			}
			else {
				goto L202;
			}
		L201:
			z[i + j * z_dim1] = abz;
		L202:;
		}
	}

	/*     improve the non-data points by applying point over-relaxation */
	/*     using the laplace-spline equation  (carres method is used) */
	/* **********************************************************************
	 */
	fprintf(stderr, "Zgrid starting iterations\n");
	dzrmsp = zrange;
	float relax = 1.0f;
	for (int iter = 1; iter <= ITERMAX; ++iter) {
		dzrms = 0.0f;
		dzmax = 0.0f;
		npg = 0;
		i__2 = *nx;
		for (int i = 1; i <= i__2; ++i) {
			i__1 = *ny;
			for (j = 1; j <= i__1; ++j) {
				z00 = z[i + j * z_dim1];
				if (z00 - big >= 0.0f) {
					goto L2000;
				}
				else {
					goto L205;
				}
			L205:
				if (z00 >= 0.0f) {
					goto L208;
				}
				else {
					goto L2000;
				}
			L208:
				wgt = 0.0f;
				zsum = 0.0f;

				im = 0;
				if (i - 1 <= 0) {
					goto L570;
				}
				else {
					goto L510;
				}
			L510:
				zim = (r__1 = z[i - 1 + j * z_dim1], (float)fabs((double)r__1));
				if (zim - big >= 0.0f) {
					goto L570;
				}
				else {
					goto L530;
				}
			L530:
				im = 1;
				wgt += 1.0f;
				zsum += zim;
				if (i - 2 <= 0) {
					goto L570;
				}
				else {
					goto L540;
				}
			L540:
				zimm = (r__1 = z[i - 2 + j * z_dim1], (float)fabs((double)r__1));
				if (zimm - big >= 0.0f) {
					goto L570;
				}
				else {
					goto L560;
				}
			L560:
				wgt += *cay;
				zsum -= *cay * (zimm - zim * 2.0f);
			L570:
				if (*nx - i <= 0) {
					goto L700;
				}
				else {
					goto L580;
				}
			L580:
				zip = (r__1 = z[i + 1 + j * z_dim1], (float)fabs((double)r__1));
				if (zip - big >= 0.0f) {
					goto L700;
				}
				else {
					goto L600;
				}
			L600:
				wgt += 1.0f;
				zsum += zip;
				if (im <= 0) {
					goto L620;
				}
				else {
					goto L610;
				}
			L610:
				wgt += *cay * 4.0f;
				zsum += *cay * 2.0f * (zim + zip);
			L620:
				if (*nx - 1 - i <= 0) {
					goto L700;
				}
				else {
					goto L630;
				}
			L630:
				zipp = (r__1 = z[i + 2 + j * z_dim1], (float)fabs((double)r__1));
				if (zipp - big >= 0.0f) {
					goto L700;
				}
				else {
					goto L650;
				}
			L650:
				wgt += *cay;
				zsum -= *cay * (zipp - zip * 2.0f);
			L700:

				jm = 0;
				if (j - 1 <= 0) {
					goto L1570;
				}
				else {
					goto L1510;
				}
			L1510:
				zjm = (r__1 = z[i + (j - 1) * z_dim1], (float)fabs((double)r__1));
				if (zjm - big >= 0.0f) {
					goto L1570;
				}
				else {
					goto L1530;
				}
			L1530:
				jm = 1;
				wgt += 1.0f;
				zsum += zjm;
				if (j - 2 <= 0) {
					goto L1570;
				}
				else {
					goto L1540;
				}
			L1540:
				zjmm = (r__1 = z[i + (j - 2) * z_dim1], (float)fabs((double)r__1));
				if (zjmm - big >= 0.0f) {
					goto L1570;
				}
				else {
					goto L1560;
				}
			L1560:
				wgt += *cay;
				zsum -= *cay * (zjmm - zjm * 2.0f);
			L1570:
				if (*ny - j <= 0) {
					goto L1700;
				}
				else {
					goto L1580;
				}
			L1580:
				zjp = (r__1 = z[i + (j + 1) * z_dim1], (float)fabs((double)r__1));
				if (zjp - big >= 0.0f) {
					goto L1700;
				}
				else {
					goto L1600;
				}
			L1600:
				wgt += 1.0f;
				zsum += zjp;
				if (jm <= 0) {
					goto L1620;
				}
				else {
					goto L1610;
				}
			L1610:
				wgt += *cay * 4.0f;
				zsum += *cay * 2.0f * (zjm + zjp);
			L1620:
				if (*ny - 1 - j <= 0) {
					goto L1700;
				}
				else {
					goto L1630;
				}
			L1630:
				zjpp = (r__1 = z[i + (j + 2) * z_dim1], (float)fabs((double)r__1));
				if (zjpp - big >= 0.0f) {
					goto L1700;
				}
				else {
					goto L1650;
				}
			L1650:
				wgt += *cay;
				zsum -= *cay * (zjpp - zjp * 2.0f);
			L1700:

				dz = zsum / wgt - z00;
				++npg;
				dzrms += dz * dz;
				dzmax = MAX((float)fabs((double)dz), dzmax);
				z[i + j * z_dim1] = z00 + dz * relax;
			L2000:;
			}
		}

		/*     shift data points zp progressively back to their proper places
		as */
		/*     the shape of surface z becomes evident. */
		/* ******************************************************************
		**** */

		if (iter - iter / 10 * 10 != 0) {
			goto L3600;
		}
		else {
			goto L3020;
		}
	L3020:
		i__1 = *n;
		for (int k = 1; k <= i__1; ++k) {
			knxt[k - 1] = (i__2 = knxt[k - 1], abs(i__2));
			if (knxt[k - 1] <= 0) {
				goto L3400;
			}
			else {
				goto L3030;
			}
		L3030:
			x = (xyz[k * 3 + 1] - *x1) / *dx;
			const int i = x + 1.5f;
			x = x + 1.0f - i;
			y = (xyz[k * 3 + 2] - *y1) / *dy;
			j = y + 1.5f;
			y = y + 1.0f - j;
			zpxy = xyz[k * 3 + 3] + zbase;
			z00 = (r__1 = z[i + j * z_dim1], (float)fabs((double)r__1));

			zw = 1.0e35f;
			if (i - 1 <= 0) {
				goto L3120;
			}
			else {
				goto L3110;
			}
		L3110:
			zw = (r__1 = z[i - 1 + j * z_dim1], (float)fabs((double)r__1));
		L3120:
			ze = 1.0e35f;
			if (i - *nx >= 0) {
				goto L3140;
			}
			else {
				goto L3130;
			}
		L3130:
			ze = (r__1 = z[i + 1 + j * z_dim1], (float)fabs((double)r__1));
		L3140:
			if (ze - big >= 0.0f) {
				goto L3150;
			}
			else {
				goto L3160;
			}
		L3150:
			if (zw - big >= 0.0f) {
				goto L3170;
			}
			else {
				goto L3180;
			}
		L3160:
			if (zw - big >= 0.0f) {
				goto L3190;
			}
			else {
				goto L3200;
			}
		L3170:
			ze = z00;
			zw = z00;
			goto L3200;
		L3180:
			ze = z00 * 2.0f - zw;
			goto L3200;
		L3190:
			zw = z00 * 2.0f - ze;

		L3200:
			zs = 1.0e35f;
			if (j - 1 <= 0) {
				goto L3220;
			}
			else {
				goto L3210;
			}
		L3210:
			zs = (r__1 = z[i + (j - 1) * z_dim1], (float)fabs((double)r__1));
		L3220:
			zn = 1.0e35f;
			if (j - *ny >= 0) {
				goto L3240;
			}
			else {
				goto L3230;
			}
		L3230:
			zn = (r__1 = z[i + (j + 1) * z_dim1], (float)fabs((double)r__1));
		L3240:
			if (zn - big >= 0.0f) {
				goto L3250;
			}
			else {
				goto L3260;
			}
		L3250:
			if (zs - big >= 0.0f) {
				goto L3270;
			}
			else {
				goto L3280;
			}
		L3260:
			if (zs - big >= 0.0f) {
				goto L3290;
			}
			else {
				goto L3300;
			}
		L3270:
			zn = z00;
			zs = z00;
			goto L3300;
		L3280:
			zn = z00 * 2.0f - zs;
			goto L3300;
		L3290:
			zs = z00 * 2.0f - zn;

		L3300:
			a = (ze - zw) * 0.5f;
			b = (zn - zs) * 0.5f;
			c = (ze + zw) * 0.5f - z00;
			d = (zn + zs) * 0.5f - z00;
			zxy = z00 + a * x + b * y + c * x * x + d * y * y;
			delz = z00 - zxy;
			delzm = derzm * ((float)fabs((double)x) * *dx + (float)fabs((double)y) * *dy) * 0.8f;
			if (delz - delzm <= 0.0f) {
				goto L3355;
			}
			else {
				goto L3350;
			}
		L3350:
			delz = delzm;
		L3355:
			if (delz + delzm >= 0.0f) {
				goto L3365;
			}
			else {
				goto L3360;
			}
		L3360:
			delz = -(double)delzm;
		L3365:
			zpij[k - 1] = zpxy + delz;
		L3400:;
		}

		i__1 = *n;
		for (int k = 1; k <= i__1; ++k) {
			if (knxt[k - 1] <= 0) {
				goto L3500;
			}
			else {
				goto L3410;
			}
		L3410:
			npt = 0;
			zsum = 0.0f;
			const int i = (xyz[k * 3 + 1] - *x1) / *dx + 1.5f;
			j = (xyz[k * 3 + 2] - *y1) / *dy + 1.5f;
			kk = k;
		L3420:
			++npt;
			zsum += zpij[kk - 1];
			knxt[kk - 1] = -knxt[kk - 1];
			kk = -knxt[kk - 1];
			if (kk <= 0) {
				goto L3430;
			}
			else if (kk - *n <= 0) {
				goto L3420;
			}
			else {
				goto L3430;
			}
		L3430:
			z[i + j * z_dim1] = -(double)zsum / npt;
		L3500:;
		}
	L3600:

		/*     test for convergence */
		/* ********************************************************************** */
		/* all grid points assigned */
		if (npg <= 1) {
			goto L4010;
		}
		dzrms = sqrt(dzrms / npg);
		if (dzrms > 0.0 && dzrmsp > 0.0)
			root = dzrms / dzrmsp;
		else
			root = 0.0;
		dzmaxf = dzmax / zrange;
		dzrmsp = dzrms;
		if (iter - iter / 10 * 10 - 2 != 0) {
			goto L3715;
		}
		else {
			goto L3710;
		}
	L3710:
		dzrms8 = dzrms;
	L3715:
		if (iter - iter / 10 * 10 != 0) {
			goto L4000;
		}
		else {
			goto L3720;
		}
	L3720:
		if (dzrms > 0.0 && dzrms8 > 0.0)
			root = sqrt(sqrt(sqrt(dzrms / dzrms8)));
		else
			root = 0.0;
		if (root - 0.9999f >= 0.0f) {
			fprintf(stderr, "Zgrid iteration %d convergence test skipped root: %f\n", iter, root);
			if (iter >= ITERTRANSITION)
				nconvtestincrease++;
			if (iter >= ITERMIN || (iter >= ITERTRANSITION && nconvtestincrease >= 4)) {
				goto L4010;
			}
			else {
				goto L4000;
			}
		}
		else {
			goto L3730;
		}
	L3730:
		/* convtest = dzmaxf / (1.0f - root) - eps; */
		convtest = dzmaxf - dzcriteria;
		if (iter >= ITERTRANSITION && convtest > convtestlast)
			nconvtestincrease++;
		fprintf(stderr, "Zgrid iteration %d convergence test: %f last:%f\n", iter, convtest, convtestlast);
		if ((convtest <= 0.0f && iter >= ITERMIN) || (iter >= ITERTRANSITION && nconvtestincrease >= 4)) {
			goto L4010;
		}
		else {
			convtestlast = convtest;
			goto L3740;
		}

	/*     improve the relaxation factor. */
	/* ********************************************************************** */

	L3740:
		if ((iter - 20) * (iter - 40) * (iter - 60) != 0) {
			goto L4000;
		}
		else {
			goto L3750;
		}
	L3750:
		if (relax - 1.0f - root >= 0.0f) {
			goto L4000;
		}
		else {
			goto L3760;
		}
	L3760:
		tpy = (root + relax - 1.0f) / relax;
		rootgs = tpy * tpy / root;
		relaxn = 2.0f / (sqrt(1.0f - rootgs) + 1.0f);
		if (iter - 60 != 0) {
			goto L3780;
		}
		else {
			goto L3785;
		}
	L3780:
		relaxn -= (2.0f - relaxn) * 0.25f;
	L3785:
		relax = MAX(relax, relaxn);
	L4000:;
	}
L4010:

	/*     remove zbase from array z and return. */
	/* ***********************************************************************/

	i__3 = *nx;
	for (int i = 1; i <= i__3; ++i) {
		i__1 = *ny;
		for (j = 1; j <= i__1; ++j) {
			if (z[i + j * z_dim1] - big >= 0.0f) {
				goto L4500;
			}
			else {
				goto L4400;
			}
		L4400:
			z[i + j * z_dim1] = (r__1 = z[i + j * z_dim1], (float)fabs((double)r__1)) - zbase;
		L4500:;
		}
	}
	return 0;
} /* mb_zgrid */
