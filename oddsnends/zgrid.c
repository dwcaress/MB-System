/*--------------------------------------------------------------------
 *    The MB-system:	zgrid.c	    4/25/95
 *    $Id: zgrid.c,v 4.4 2000-10-11 01:06:15 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 1995, 2000 by
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
 * This is a function to generate thin plate spline interpolation
 * of a data field. This code originated as fortran in the
 * 1960's and was used routinely at the Institute of 
 * Geophysics and Planetary Physics at the Scripps Institution
 * of Oceanography through the 1970's and 1980's. The Fortran
 * code was obtained from Professory Robert L. Parker at
 * IGPP in 1989.
 * It was converted to C in 1995 for use with the MB-System
 * software package.
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
 *     imnew[MAX(nx, ny)+1] = int work array
 *     cay = k = amount of spline eqn (between 0 and inf.) 
 *     nrng...grid points more than nrng grid spaces from the nearest 
 *            data point are set to undefined. 
 *
 * Author:	Unknown, but "jdt", "ian crain",  and "dr t murty"
 *              obviously contributed.
 * Hacker:	D. W. Caress
 * Date:	April 25, 1995
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.3  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.1  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.0  1995/04/25  19:07:29  caress
 * First cut at C version of zgrid.
 *
 * 
 *     The following are the original comments from the Fortran code:
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
 *     zgrid.c -- translated by f2c (version 19950314) from zgrid.f.
 *     Work arrays zpij[n], knxt[n], imnew[MAX(nx, ny)+1] are now
 *     passed into the function.
 *     David W. Caress
 *     April 25,  1995
 *--------------------------------------------------------------------*/

/* standard include files */
#include <stdio.h>
#include <math.h>

/* MBIO include files */
#include "../../include/mb_define.h"

/*----------------------------------------------------------------------- */
int zgrid(float *z, int *nx, int *ny, 
		float *x1, float *y1, float *dx, float *dy, float *xyz, 
		int *n, float *zpij, int *knxt, int *imnew, 
		float *cay, int *nrng)
{
    /* System generated locals */
    int z_dim1, z_offset, i__1, i__2, i__3;
    float r__1, r__2;

    /* Local variables */
    float delz;
    int iter, nnew;
    float zijn, zmin, zimm, zmax, zjmm, zipp, zjpp;
    float root, zsum, zpxy, a, b, c, d;
    int i, j, k;
    float x, y, zbase, relax, delzm;
    float derzm;
    int itmax, jmnew;
    float dzmax, dzrms;
    int kk, im, jm;
    float dzrms8, z00, dz, ze, hrange, zn, zs, zw, zrange, dzmaxf, 
	    relaxn, rootgs, dzrmsp, big, abz;
    int npg;
    float eps, zim, zjm;
    int npt;
    float wgt, zip, zjp, tpy, zxy;

    /* Parameter adjustments */
    z_dim1 = *nx;
    z_offset = z_dim1 + 1;
    z -= z_offset;
    xyz -= 4;
    

    /* Function Body */
    itmax = 100;
    eps = (float).002;
    big = (float)9e29;

/*     get zbase which will make all zp values positive by 20*(zmax-zmin) 
*/
/* ********************************************************************** 
*/

    zmin = xyz[6];
    zmax = xyz[6];
    i__1 = *n;
    for (k = 2; k <= i__1; ++k) {
	if (xyz[k * 3 + 3] - zmax <= (float)0.) {
	    goto L14;
	} else {
	    goto L12;
	}
L12:
	zmax = xyz[k * 3 + 3];
L14:
	if (xyz[k * 3 + 3] - zmin >= (float)0.) {
	    goto L20;
	} else {
	    goto L16;
	}
L16:
	zmin = xyz[k * 3 + 3];
L20:
	;
    }
    zrange = zmax - zmin;
    zbase = zrange * (float)20. - zmin;
    hrange = MIN(*dx * (*nx - 1), *dy * (*ny - 1));
    derzm = zrange * (float)2. / hrange;

/*     set pointer array knxt */
/* ********************************************************************** 
*/

    i__1 = *n;
    for (kk = 1; kk <= i__1; ++kk) {
	k = *n + 1 - kk;
	knxt[k - 1] = 0;
	i = (xyz[k * 3 + 1] - *x1) / *dx + (float)1.5;
	if (i * (*nx + 1 - i) <= 0) {
	    goto L60;
	} else {
	    goto L35;
	}
L35:
	j = (xyz[k * 3 + 2] - *y1) / *dy + (float)1.5;
	if (j * (*ny + 1 - j) <= 0) {
	    goto L60;
	} else {
	    goto L40;
	}
L40:
	if (z[i + j * z_dim1] - big >= (float)0.) {
	    goto L60;
	} else {
	    goto L45;
	}
L45:
	knxt[k - 1] = *n + 1;
	if (z[i + j * z_dim1] <= (float)0.) {
	    goto L55;
	} else {
	    goto L50;
	}
L50:
	knxt[k - 1] = z[i + j * z_dim1] + (float).5;
L55:
	z[i + j * z_dim1] = (float) k;
L60:
	;
    }

/*     affix each data point zp to its nearby grid point.  take avg zp if 
*/
/*     more than one zp nearby the grid point. add zbase and complement. 
*/
/* ********************************************************************** 
*/

    i__1 = *n;
    for (k = 1; k <= i__1; ++k) {
	if (knxt[k - 1] <= 0) {
	    goto L80;
	} else {
	    goto L65;
	}
L65:
	npt = 0;
	zsum = (float)0.;
	i = (xyz[k * 3 + 1] - *x1) / *dx + (float)1.5;
	j = (xyz[k * 3 + 2] - *y1) / *dy + (float)1.5;
	kk = k;
L70:
	++npt;
	zsum += xyz[kk * 3 + 3];
	knxt[kk - 1] = -knxt[kk - 1];
	kk = -knxt[kk - 1];
	if (kk - *n <= 0) {
	    goto L70;
	} else {
	    goto L75;
	}
L75:
	z[i + j * z_dim1] = -(double)zsum / npt - zbase;
L80:
	;
    }

/*     initially set each unset grid point to value of nearest known pt. 
*/
/* ********************************************************************** 
*/

    i__1 = *nx;
    for (i = 1; i <= i__1; ++i) {
	i__2 = *ny;
	for (j = 1; j <= i__2; ++j) {
	    if (z[i + j * z_dim1] != (float)0.) {
		goto L110;
	    } else {
		goto L100;
	    }
L100:
	    z[i + j * z_dim1] = (float)-1e35;
L110:
	    ;
	}
    }
    i__2 = *nrng;
    for (iter = 1; iter <= i__2; ++iter) {
	nnew = 0;
	i__1 = *nx;
	for (i = 1; i <= i__1; ++i) {
	    i__3 = *ny;
	    for (j = 1; j <= i__3; ++j) {
		if (z[i + j * z_dim1] + big >= (float)0.) {
		    goto L192;
		} else {
		    goto L152;
		}
L152:
		if (j - 1 <= 0) {
		    goto L162;
		} else {
		    goto L153;
		}
L153:
		if (jmnew <= 0) {
		    goto L154;
		} else {
		    goto L162;
		}
L154:
		zijn = (r__1 = z[i + (j - 1) * z_dim1], (float)fabs((double)r__1));
		if (zijn - big >= (float)0.) {
		    goto L162;
		} else {
		    goto L195;
		}
L162:
		if (i - 1 <= 0) {
		    goto L172;
		} else {
		    goto L163;
		}
L163:
		if (imnew[j - 1] <= 0) {
		    goto L164;
		} else {
		    goto L172;
		}
L164:
		zijn = (r__1 = z[i - 1 + j * z_dim1], (float)fabs((double)r__1));
		if (zijn - big >= (float)0.) {
		    goto L172;
		} else {
		    goto L195;
		}
L172:
		if (j - *ny >= 0) {
		    goto L182;
		} else {
		    goto L173;
		}
L173:
		zijn = (r__1 = z[i + (j + 1) * z_dim1], (float)fabs((double)r__1));
		if (zijn - big >= (float)0.) {
		    goto L182;
		} else {
		    goto L195;
		}
L182:
		if (i - *nx >= 0) {
		    goto L192;
		} else {
		    goto L183;
		}
L183:
		zijn = (r__1 = z[i + 1 + j * z_dim1], (float)fabs((double)r__1));
		if (zijn - big >= (float)0.) {
		    goto L192;
		} else {
		    goto L195;
		}
L192:
		imnew[j - 1] = 0;
		jmnew = 0;
		goto L197;
L195:
		imnew[j - 1] = 1;
		jmnew = 1;
		z[i + j * z_dim1] = zijn;
		++nnew;
L197:
		;
	    }
	}
	if (nnew <= 0) {
	    goto L200;
	} else {
	    goto L199;
	}
L199:
	;
    }
L200:
    i__2 = *nx;
    for (i = 1; i <= i__2; ++i) {
	i__3 = *ny;
	for (j = 1; j <= i__3; ++j) {
	    abz = (r__1 = z[i + j * z_dim1], (float)fabs((double)r__1));
	    if (abz - big >= (float)0.) {
		goto L201;
	    } else {
		goto L202;
	    }
L201:
	    z[i + j * z_dim1] = abz;
L202:
	    ;
	}
    }

/*     improve the non-data points by applying point over-relaxation */
/*     using the laplace-spline equation  (carres method is used) */
/* ********************************************************************** 
*/

    dzrmsp = zrange;
    relax = (float)1.;
    i__3 = itmax;
    for (iter = 1; iter <= i__3; ++iter) {
	dzrms = (float)0.;
	dzmax = (float)0.;
	npg = 0;
	i__2 = *nx;
	for (i = 1; i <= i__2; ++i) {
	    i__1 = *ny;
	    for (j = 1; j <= i__1; ++j) {
		z00 = z[i + j * z_dim1];
		if (z00 - big >= (float)0.) {
		    goto L2000;
		} else {
		    goto L205;
		}
L205:
		if (z00 >= (float)0.) {
		    goto L208;
		} else {
		    goto L2000;
		}
L208:
		wgt = (float)0.;
		zsum = (float)0.;

		im = 0;
		if (i - 1 <= 0) {
		    goto L570;
		} else {
		    goto L510;
		}
L510:
		zim = (r__1 = z[i - 1 + j * z_dim1], (float)fabs((double)r__1));
		if (zim - big >= (float)0.) {
		    goto L570;
		} else {
		    goto L530;
		}
L530:
		im = 1;
		wgt += (float)1.;
		zsum += zim;
		if (i - 2 <= 0) {
		    goto L570;
		} else {
		    goto L540;
		}
L540:
		zimm = (r__1 = z[i - 2 + j * z_dim1], (float)fabs((double)r__1));
		if (zimm - big >= (float)0.) {
		    goto L570;
		} else {
		    goto L560;
		}
L560:
		wgt += *cay;
		zsum -= *cay * (zimm - zim * (float)2.);
L570:
		if (*nx - i <= 0) {
		    goto L700;
		} else {
		    goto L580;
		}
L580:
		zip = (r__1 = z[i + 1 + j * z_dim1], (float)fabs((double)r__1));
		if (zip - big >= (float)0.) {
		    goto L700;
		} else {
		    goto L600;
		}
L600:
		wgt += (float)1.;
		zsum += zip;
		if (im <= 0) {
		    goto L620;
		} else {
		    goto L610;
		}
L610:
		wgt += *cay * (float)4.;
		zsum += *cay * (float)2. * (zim + zip);
L620:
		if (*nx - 1 - i <= 0) {
		    goto L700;
		} else {
		    goto L630;
		}
L630:
		zipp = (r__1 = z[i + 2 + j * z_dim1], (float)fabs((double)r__1));
		if (zipp - big >= (float)0.) {
		    goto L700;
		} else {
		    goto L650;
		}
L650:
		wgt += *cay;
		zsum -= *cay * (zipp - zip * (float)2.);
L700:

		jm = 0;
		if (j - 1 <= 0) {
		    goto L1570;
		} else {
		    goto L1510;
		}
L1510:
		zjm = (r__1 = z[i + (j - 1) * z_dim1], (float)fabs((double)r__1));
		if (zjm - big >= (float)0.) {
		    goto L1570;
		} else {
		    goto L1530;
		}
L1530:
		jm = 1;
		wgt += (float)1.;
		zsum += zjm;
		if (j - 2 <= 0) {
		    goto L1570;
		} else {
		    goto L1540;
		}
L1540:
		zjmm = (r__1 = z[i + (j - 2) * z_dim1], (float)fabs((double)r__1));
		if (zjmm - big >= (float)0.) {
		    goto L1570;
		} else {
		    goto L1560;
		}
L1560:
		wgt += *cay;
		zsum -= *cay * (zjmm - zjm * (float)2.);
L1570:
		if (*ny - j <= 0) {
		    goto L1700;
		} else {
		    goto L1580;
		}
L1580:
		zjp = (r__1 = z[i + (j + 1) * z_dim1], (float)fabs((double)r__1));
		if (zjp - big >= (float)0.) {
		    goto L1700;
		} else {
		    goto L1600;
		}
L1600:
		wgt += (float)1.;
		zsum += zjp;
		if (jm <= 0) {
		    goto L1620;
		} else {
		    goto L1610;
		}
L1610:
		wgt += *cay * (float)4.;
		zsum += *cay * (float)2. * (zjm + zjp);
L1620:
		if (*ny - 1 - j <= 0) {
		    goto L1700;
		} else {
		    goto L1630;
		}
L1630:
		zjpp = (r__1 = z[i + (j + 2) * z_dim1], (float)fabs((double)r__1));
		if (zjpp - big >= (float)0.) {
		    goto L1700;
		} else {
		    goto L1650;
		}
L1650:
		wgt += *cay;
		zsum -= *cay * (zjpp - zjp * (float)2.);
L1700:

		dz = zsum / wgt - z00;
		++npg;
		dzrms += dz * dz;
		dzmax = MAX((float)fabs((double)dz), dzmax);
		z[i + j * z_dim1] = z00 + dz * relax;
L2000:
		;
	    }
	}


/*     shift data points zp progressively back to their proper places 
as */
/*     the shape of surface z becomes evident. */
/* ******************************************************************
**** */

	if (iter - iter / 10 * 10 != 0) {
	    goto L3600;
	} else {
	    goto L3020;
	}
L3020:
	i__1 = *n;
	for (k = 1; k <= i__1; ++k) {
	    knxt[k - 1] = (i__2 = knxt[k - 1], abs(i__2));
	    if (knxt[k - 1] <= 0) {
		goto L3400;
	    } else {
		goto L3030;
	    }
L3030:
	    x = (xyz[k * 3 + 1] - *x1) / *dx;
	    i = x + (float)1.5;
	    x = x + (float)1. - i;
	    y = (xyz[k * 3 + 2] - *y1) / *dy;
	    j = y + (float)1.5;
	    y = y + (float)1. - j;
	    zpxy = xyz[k * 3 + 3] + zbase;
	    z00 = (r__1 = z[i + j * z_dim1], (float)fabs((double)r__1));

	    zw = (float)1e35;
	    if (i - 1 <= 0) {
		goto L3120;
	    } else {
		goto L3110;
	    }
L3110:
	    zw = (r__1 = z[i - 1 + j * z_dim1], (float)fabs((double)r__1));
L3120:
	    ze = (float)1e35;
	    if (i - *nx >= 0) {
		goto L3140;
	    } else {
		goto L3130;
	    }
L3130:
	    ze = (r__1 = z[i + 1 + j * z_dim1], (float)fabs((double)r__1));
L3140:
	    if (ze - big >= (float)0.) {
		goto L3150;
	    } else {
		goto L3160;
	    }
L3150:
	    if (zw - big >= (float)0.) {
		goto L3170;
	    } else {
		goto L3180;
	    }
L3160:
	    if (zw - big >= (float)0.) {
		goto L3190;
	    } else {
		goto L3200;
	    }
L3170:
	    ze = z00;
	    zw = z00;
	    goto L3200;
L3180:
	    ze = z00 * (float)2. - zw;
	    goto L3200;
L3190:
	    zw = z00 * (float)2. - ze;

L3200:
	    zs = (float)1e35;
	    if (j - 1 <= 0) {
		goto L3220;
	    } else {
		goto L3210;
	    }
L3210:
	    zs = (r__1 = z[i + (j - 1) * z_dim1], (float)fabs((double)r__1));
L3220:
	    zn = (float)1e35;
	    if (j - *ny >= 0) {
		goto L3240;
	    } else {
		goto L3230;
	    }
L3230:
	    zn = (r__1 = z[i + (j + 1) * z_dim1], (float)fabs((double)r__1));
L3240:
	    if (zn - big >= (float)0.) {
		goto L3250;
	    } else {
		goto L3260;
	    }
L3250:
	    if (zs - big >= (float)0.) {
		goto L3270;
	    } else {
		goto L3280;
	    }
L3260:
	    if (zs - big >= (float)0.) {
		goto L3290;
	    } else {
		goto L3300;
	    }
L3270:
	    zn = z00;
	    zs = z00;
	    goto L3300;
L3280:
	    zn = z00 * (float)2. - zs;
	    goto L3300;
L3290:
	    zs = z00 * (float)2. - zn;

L3300:
	    a = (ze - zw) * (float).5;
	    b = (zn - zs) * (float).5;
	    c = (ze + zw) * (float).5 - z00;
	    d = (zn + zs) * (float).5 - z00;
	    zxy = z00 + a * x + b * y + c * x * x + d * y * y;
	    delz = z00 - zxy;
	    delzm = derzm * ((float)fabs((double)x) * *dx + (float)fabs((double)y) * *dy) * (float).8;
	    if (delz - delzm <= (float)0.) {
		goto L3355;
	    } else {
		goto L3350;
	    }
L3350:
	    delz = delzm;
L3355:
	    if (delz + delzm >= (float)0.) {
		goto L3365;
	    } else {
		goto L3360;
	    }
L3360:
	    delz = -(double)delzm;
L3365:
	    zpij[k - 1] = zpxy + delz;
L3400:
	    ;
	}

	i__1 = *n;
	for (k = 1; k <= i__1; ++k) {
	    if (knxt[k - 1] <= 0) {
		goto L3500;
	    } else {
		goto L3410;
	    }
L3410:
	    npt = 0;
	    zsum = (float)0.;
	    i = (xyz[k * 3 + 1] - *x1) / *dx + (float)1.5;
	    j = (xyz[k * 3 + 2] - *y1) / *dy + (float)1.5;
	    kk = k;
L3420:
	    ++npt;
	    zsum += zpij[kk - 1];
	    knxt[kk - 1] = -knxt[kk - 1];
	    kk = -knxt[kk - 1];
	    if (kk - *n <= 0) {
		goto L3420;
	    } else {
		goto L3430;
	    }
L3430:
	    z[i + j * z_dim1] = -(double)zsum / npt;
L3500:
	    ;
	}
L3600:

/*     test for convergence */
/* ******************************************************************
**** */
/* all grid points assigned */
	if (npg <= 1) {
	    goto L4010;
	}
	dzrms = sqrt(dzrms / npg);
	root = dzrms / dzrmsp;
	dzrmsp = dzrms;
	dzmaxf = dzmax / zrange;
	if (iter - iter / 10 * 10 - 2 != 0) {
	    goto L3715;
	} else {
	    goto L3710;
	}
L3710:
	dzrms8 = dzrms;
L3715:
	if (iter - iter / 10 * 10 != 0) {
	    goto L4000;
	} else {
	    goto L3720;
	}
L3720:
	root = sqrt(sqrt(sqrt(dzrms / dzrms8)));
	if (root - (float).9999 >= (float)0.) {
	    goto L4000;
	} else {
	    goto L3730;
	}
L3730:
	if (dzmaxf / ((float)1. - root) - eps <= (float)0.) {
	    goto L4010;
	} else {
	    goto L3740;
	}

/*     improve the relaxation factor. */
/* ******************************************************************
**** */

L3740:
	if ((iter - 20) * (iter - 40) * (iter - 60) != 0) {
	    goto L4000;
	} else {
	    goto L3750;
	}
L3750:
	if (relax - (float)1. - root >= (float)0.) {
	    goto L4000;
	} else {
	    goto L3760;
	}
L3760:
	tpy = (root + relax - (float)1.) / relax;
	rootgs = tpy * tpy / root;
	relaxn = (float)2. / (sqrt((float)1. - rootgs) + (float)1.);
	if (iter - 60 != 0) {
	    goto L3780;
	} else {
	    goto L3785;
	}
L3780:
	relaxn -= ((float)2. - relaxn) * (float).25;
L3785:
	relax = MAX(relax,relaxn);
L4000:
	;
    }
L4010:

/*     remove zbase from array z and return. */
/* ********************************************************************** 
*/

    i__3 = *nx;
    for (i = 1; i <= i__3; ++i) {
	i__1 = *ny;
	for (j = 1; j <= i__1; ++j) {
	    if (z[i + j * z_dim1] - big >= (float)0.) {
		goto L4500;
	    } else {
		goto L4400;
	    }
L4400:
	    z[i + j * z_dim1] = (r__1 = z[i + j * z_dim1], (float)fabs((double)r__1)) - 
		    zbase;
L4500:
	    ;
	}
    }
    return 0;
} /* zgrid */

