/* From http://calgo.acm.org/642.gz */

/*     ALGORITHM 642 COLLECTED ALGORITHMS FROM ACM. */
/*     ALGORITHM APPEARED IN ACM-TRANS. MATH. SOFTWARE, VOL.12, NO. 2, */
/*     JUN., 1986, P. 150. */
/*   SUBROUTINE NAME     - CUBGCV */

/* -------------------------------------------------------------------------- */

/*   AUTHOR              - M.F.HUTCHINSON */
/*                         CSIRO DIVISION OF MATHEMATICS AND STATISTICS */
/*                         P.O. BOX 1965 */
/*                         CANBERRA, ACT 2601 */
/*                         AUSTRALIA */

/*   LATEST REVISION     - 15 AUGUST 1985 */

/*   PURPOSE             - CUBIC SPLINE DATA SMOOTHER */

/*   USAGE               - CALL CUBGCV (X,F,DF,N,Y,C,IC,VAR,JOB,SE,WK,IER) */

/*   ARGUMENTS    X      - VECTOR OF LENGTH N CONTAINING THE */
/*                           ABSCISSAE OF THE N DATA POINTS */
/*                           (X(I),F(I)) I=1..N. (INPUT) X */
/*                           MUST BE ORDERED SO THAT */
/*                           X(I) .LT. X(I+1). */
/*                F      - VECTOR OF LENGTH N CONTAINING THE */
/*                           ORDINATES (OR FUNCTION VALUES) */
/*                           OF THE N DATA POINTS (INPUT). */
/*                DF     - VECTOR OF LENGTH N. (INPUT/OUTPUT) */
/*                           DF(I) IS THE RELATIVE STANDARD DEVIATION */
/*                           OF THE ERROR ASSOCIATED WITH DATA POINT I. */
/*                           EACH DF(I) MUST BE POSITIVE.  THE VALUES IN */
/*                           DF ARE SCALED BY THE SUBROUTINE SO THAT */
/*                           THEIR MEAN SQUARE VALUE IS 1, AND UNSCALED */
/*                           AGAIN ON NORMAL EXIT. */
/*                           THE MEAN SQUARE VALUE OF THE DF(I) IS RETURNED */
/*                           IN WK(7) ON NORMAL EXIT. */
/*                           IF THE ABSOLUTE STANDARD DEVIATIONS ARE KNOWN, */
/*                           THESE SHOULD BE PROVIDED IN DF AND THE ERROR */
/*                           VARIANCE PARAMETER VAR (SEE BELOW) SHOULD THEN */
/*                           BE SET TO 1. */
/*                           IF THE RELATIVE STANDARD DEVIATIONS ARE UNKNOWN, */
/*                           SET EACH DF(I)=1. */
/*                N      - NUMBER OF DATA POINTS (INPUT). */
/*                           N MUST BE .GE. 3. */
/*                Y,C    - SPLINE COEFFICIENTS. (OUTPUT) Y */
/*                           IS A VECTOR OF LENGTH N. C IS */
/*                           AN N-1 BY 3 MATRIX. THE VALUE */
/*                           OF THE SPLINE APPROXIMATION AT T IS */
/*                           S(T)=((C(I,3)*D+C(I,2))*D+C(I,1))*D+Y(I) */
/*                           WHERE X(I).LE.T.LT.X(I+1) AND */
/*                           D = T-X(I). */
/*                IC     - ROW DIMENSION OF MATRIX C EXACTLY */
/*                           AS SPECIFIED IN THE DIMENSION */
/*                           STATEMENT IN THE CALLING PROGRAM. (INPUT) */
/*                VAR    - ERROR VARIANCE. (INPUT/OUTPUT) */
/*                           IF VAR IS NEGATIVE (I.E. UNKNOWN) THEN */
/*                           THE SMOOTHING PARAMETER IS DETERMINED */
/*                           BY MINIMIZING THE GENERALIZED CROSS VALIDATION */
/*                           AND AN ESTIMATE OF THE ERROR VARIANCE IS */
/*                           RETURNED IN VAR. */
/*                           IF VAR IS NON-NEGATIVE (I.E. KNOWN) THEN THE */
/*                           SMOOTHING PARAMETER IS DETERMINED TO MINIMIZE */
/*                           AN ESTIMATE, WHICH DEPENDS ON VAR, OF THE TRUE */
/*                           MEAN SQUARE ERROR, AND VAR IS UNCHANGED. */
/*                           IN PARTICULAR, IF VAR IS ZERO, THEN AN */
/*                           INTERPOLATING NATURAL CUBIC SPLINE IS CALCULATED. */
/*                           VAR SHOULD BE SET TO 1 IF ABSOLUTE STANDARD */
/*                           DEVIATIONS HAVE BEEN PROVIDED IN DF (SEE ABOVE). */
/*                JOB    - JOB SELECTION PARAMETER. (INPUT) */
/*                         JOB = 0 SHOULD BE SELECTED IF POINT STANDARD ERROR */
/*                           ESTIMATES ARE NOT REQUIRED IN SE. */
/*                         JOB = 1 SHOULD BE SELECTED IF POINT STANDARD ERROR */
/*                           ESTIMATES ARE REQUIRED IN SE. */
/*                SE     - VECTOR OF LENGTH N CONTAINING BAYESIAN STANDARD */
/*                           ERROR ESTIMATES OF THE FITTED SPLINE VALUES IN Y. */
/*                           SE IS NOT REFERENCED IF JOB=0. (OUTPUT) */
/*                WK     - WORK VECTOR OF LENGTH 7*(N + 2). ON NORMAL EXIT THE */
/*                           FIRST 7 VALUES OF WK ARE ASSIGNED AS FOLLOWS:- */

/*                           WK(1) = SMOOTHING PARAMETER (= RHO/(RHO + 1)) */
/*                           WK(2) = ESTIMATE OF THE NUMBER OF DEGREES OF */
/*                                   FREEDOM OF THE RESIDUAL SUM OF SQUARES */
/*                           WK(3) = GENERALIZED CROSS VALIDATION */
/*                           WK(4) = MEAN SQUARE RESIDUAL */
/*                           WK(5) = ESTIMATE OF THE TRUE MEAN SQUARE ERROR */
/*                                   AT THE DATA POINTS */
/*                           WK(6) = ESTIMATE OF THE ERROR VARIANCE */
/*                           WK(7) = MEAN SQUARE VALUE OF THE DF(I) */

/*                           IF WK(1)=0 (RHO=0) AN INTERPOLATING NATURAL CUBIC */
/*                           SPLINE HAS BEEN CALCULATED. */
/*                           IF WK(1)=1 (RHO=INFINITE) A LEAST SQUARES */
/*                           REGRESSION LINE HAS BEEN CALCULATED. */
/*                           WK(2) IS AN ESTIMATE OF THE NUMBER OF DEGREES OF */
/*                           FREEDOM OF THE RESIDUAL WHICH REDUCES TO THE */
/*                           USUAL VALUE OF N-2 WHEN A LEAST SQUARES REGRESSION */
/*                           LINE IS CALCULATED. */
/*                           WK(3),WK(4),WK(5) ARE CALCULATED WITH THE DF(I) */
/*                           SCALED TO HAVE MEAN SQUARE VALUE 1.  THE */
/*                           UNSCALED VALUES OF WK(3),WK(4),WK(5) MAY BE */
/*                           CALCULATED BY DIVIDING BY WK(7). */
/*                           WK(6) COINCIDES WITH THE OUTPUT VALUE OF VAR IF */
/*                           VAR IS NEGATIVE ON INPUT.  IT IS CALCULATED WITH */
/*                           THE UNSCALED VALUES OF THE DF(I) TO FACILITATE */
/*                           COMPARISONS WITH A PRIORI VARIANCE ESTIMATES. */

/*                IER    - ERROR PARAMETER. (OUTPUT) */
/*                         TERMINAL ERROR */
/*                           IER = 129, IC IS LESS THAN N-1. */
/*                           IER = 130, N IS LESS THAN 3. */
/*                           IER = 131, INPUT ABSCISSAE ARE NOT */
/*                             ORDERED SO THAT X(I).LT.X(I+1). */
/*                           IER = 132, DF(I) IS NOT POSITIVE FOR SOME I. */
/*                           IER = 133, JOB IS NOT 0 OR 1. */

/*   REQUIRED ROUTINES   - SPINT1,SPFIT1,SPCOF1,SPERR1 */

/*   REMARKS      THE NUMBER OF ARITHMETIC OPERATIONS REQUIRED BY THE */
/*                SUBROUTINE IS PROPORTIONAL TO N.  THE SUBROUTINE */
/*                USES AN ALGORITHM DEVELOPED BY M.F. HUTCHINSON AND */
/*                F.R. DE HOOG, 'SMOOTHING NOISY DATA WITH SPLINE */
/*                FUNCTIONS', NUMER. MATH. (IN PRESS) */

#include <stdio.h>
#include <math.h>

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))	/* min and max value macros */
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

int cubgcv(double *x, double *f, double *df, int n, double *y, double *c, int ic, double *var,
	int job, double *se, double *wk);
int spint1(double *x, double *avh, double *y, double *dy, double *avdy, int n, double *a,
	double *c, int ic, double *r, double *t);
int spfit1(double *x, double *avh, double *dy, int n, double *rho, double *p, double *q, double *fun,
	double var, double *stat, double *a, double *c, int ic, double *r, double *t, double *u,
	double *v);
int sperr1(double *x, double avh, double *dy, int n, double *r, double p, double var, double *se);
int spcof(double *x, double avh, double *y, double *dy, int n, double p, double q, double *a,
	double *c, int ic, double *u, double *v);

/* ----------------------------------------------------------------------- */
int cubgcv(double *x, double *f, double *df, int n, double *y, double *c, int ic, double *var,
	int job, double *se, double *wk) {

	static double ratio = 2., tau = 1.618033989;

	int i, ier, c_dim1, c_offset, wk_dim1, wk_offset;
	static double p, q, r1, r2, r3, r4, gf1, gf2, gf3, gf4, avh, err, avdf, avar, stat[6], delta;

	/* Parameter adjustments */
	wk_dim1 = n + 1 + 1;
	wk_offset = wk_dim1;
	wk -= wk_offset;
	--se;
	--y;
	--df;
	--f;
	--x;
	c_dim1 = ic;
	c_offset = 1 + c_dim1;
	c -= c_offset;

	/* ---INITIALIZE--- */
	if (job < 0 || job > 1)
		return(133);

	ier = spint1(&x[1], &avh, &f[1], &df[1], &avdf, n, &y[1], &c[c_offset], ic, &wk[wk_offset], &wk[wk_dim1 * 4]);
	if (ier != 0)
		return(ier);

	avar = *var;
	if (*var > 0.)
		avar = *var * avdf * avdf;

	/* ---CHECK FOR ZERO VARIANCE--- */
	if (*var != 0.)
		goto L10;

	r1 = 0.;
	goto L90;

	/* ---FIND LOCAL MINIMUM OF GCV OR THE EXPECTED MEAN SQUARE ERROR--- */
L10:
	r1 = 1.;
	r2 = ratio * r1;
	spfit1(&x[1], &avh, &df[1], n, &r2, &p, &q, &gf2, avar, stat, &y[1], &c[c_offset],
		ic, &wk[wk_offset], &wk[wk_dim1 * 4], &wk[wk_dim1 * 6], &wk[wk_dim1 * 7]);
L20:
	spfit1(&x[1], &avh, &df[1], n, &r1, &p, &q, &gf1, avar, stat, &y[1], &c[c_offset],
		ic, &wk[wk_offset], &wk[wk_dim1 * 4], &wk[wk_dim1 * 6], &wk[wk_dim1 * 7]);
	if (gf1 > gf2)
		goto L30;


	/* ---EXIT IF P ZERO--- */
	if (p <= 0.)
		goto L100;

	r2 = r1;
	gf2 = gf1;
	r1 /= ratio;
	goto L20;
L30:
	r3 = ratio * r2;
L40:
	spfit1(&x[1], &avh, &df[1], n, &r3, &p, &q, &gf3, avar, stat, &y[1], &c[c_offset],
		ic, &wk[wk_offset], &wk[wk_dim1 * 4], &wk[wk_dim1 * 6], &wk[wk_dim1 * 7]);
	if (gf3 > gf2)
		goto L50;

	/* ---EXIT IF Q ZERO--- */
	if (q <= 0.)
		goto L100;

	r2 = r3;
	gf2 = gf3;
	r3 = ratio * r3;
	goto L40;
L50:
	r2 = r3;
	gf2 = gf3;
	delta = (r2 - r1) / tau;
	r4 = r1 + delta;
	r3 = r2 - delta;
	spfit1(&x[1], &avh, &df[1], n, &r3, &p, &q, &gf3, avar, stat, &y[1], &c[c_offset],
		ic, &wk[wk_offset], &wk[wk_dim1 * 4], &wk[wk_dim1 * 6], &wk[wk_dim1 * 7]);
	spfit1(&x[1], &avh, &df[1], n, &r4, &p, &q, &gf4, avar, stat, &y[1], &c[c_offset],
		ic, &wk[wk_offset], &wk[wk_dim1 * 4], &wk[wk_dim1 * 6], &wk[wk_dim1 * 7]);

	/* ---GOLDEN SECTION SEARCH FOR LOCAL MINIMUM--- */
	err = 1;
	while (err > 1e-6) {
		if (gf3 <= gf4) {
			r2 = r4;
			gf2 = gf4;
			r4 = r3;
			gf4 = gf3;
			delta /= tau;
			r3 = r2 - delta;
			spfit1(&x[1], &avh, &df[1], n, &r3, &p, &q, &gf3, avar, stat, &y[1], &c[c_offset],
				ic, &wk[wk_offset], &wk[wk_dim1 * 4], &wk[wk_dim1 * 6], &wk[wk_dim1 * 7]);
		}
		else {
			r1 = r3;
			gf1 = gf3;
			r3 = r4;
			gf3 = gf4;
			delta /= tau;
			r4 = r1 + delta;
			spfit1(&x[1], &avh, &df[1], n, &r4, &p, &q, &gf4, avar, stat, &y[1], &c[c_offset],
				ic, &wk[wk_offset], &wk[wk_dim1 * 4], &wk[wk_dim1 * 6], &wk[wk_dim1 * 7]);
		}
		err = (r2 - r1) / (r1 + r2);
	}

	r1 = (r1 + r2) * .5;

	/* ---CALCULATE SPLINE COEFFICIENTS--- */
L90:
	spfit1(&x[1], &avh, &df[1], n, &r1, &p, &q, &gf1, avar, stat, &y[1], &
		c[c_offset], ic, &wk[wk_offset], &wk[wk_dim1 * 4], &wk[wk_dim1 * 6], &wk[wk_dim1 * 7]);
L100:
	spcof(&x[1], avh, &f[1], &df[1], n, p, q, &y[1], &c[c_offset], ic, &wk[wk_dim1 * 6], &wk[wk_dim1 * 7]);

	/* ---OPTIONALLY CALCULATE STANDARD ERROR ESTIMATES--- */
	if (*var < 0.) {
		avar = stat[5];
		*var = avar / (avdf * avdf);
	}

	if (job == 1)
		sperr1(&x[1], avh, &df[1], n, &wk[wk_offset], p, avar, &se[1]);

	/* ---UNSCALE DF--- */
	for (i = 1; i <= n; ++i)
		df[i] *= avdf;

	/* --PUT STATISTICS IN WK--- */
	for (i = 0; i <= 5; ++i)
		wk[i + wk_dim1] = stat[i];

	wk[wk_dim1 + 5] = stat[5] / (avdf * avdf);
	wk[wk_dim1 + 6] = avdf * avdf;

	return 0;
}

/* ---------------------------------------------------------------------------- */
int spint1(double *x, double *avh, double *y, double *dy, double *avdy, int n, double *a,
	double *c, int ic, double *r, double *t) {

	int i, c_dim1, c_offset, r_dim1, r_offset, t_dim1, t_offset, ier;
	static double e, f, g, h;

/* INITIALIZES THE ARRAYS C, R AND T FOR ONE DIMENSIONAL CUBIC */
/* SMOOTHING SPLINE FITTING BY SUBROUTINE SPFIT1.  THE VALUES */
/* DF(I) ARE SCALED SO THAT THE SUM OF THEIR SQUARES IS N */
/* AND THE AVERAGE OF THE DIFFERENCES X(I+1) - X(I) IS CALCULATED */
/* IN AVH IN ORDER TO AVOID UNDERFLOW AND OVERFLOW PROBLEMS IN */
/* SPFIT1. */

/* SUBROUTINE SETS IER IF ELEMENTS OF X ARE NON-INCREASING, */
/* IF N IS LESS THAN 3, IF IC IS LESS THAN N-1 OR IF DY(I) IS */
/* NOT POSITIVE FOR SOME I. */

	/* Parameter adjustments */
	t_dim1 = n + 1 + 1;
	t_offset = 0 + t_dim1;
	t -= t_offset;
	r_dim1 = n + 1 + 1;
	r_offset = 0 + r_dim1;
	r -= r_offset;
	--a;
	--dy;
	--y;
	--x;
	c_dim1 = ic;
	c_offset = 1 + c_dim1;
	c -= c_offset;

	/* Function Body */

	/* ---INITIALIZATION AND INPUT CHECKING--- */
	if (n < 3) return(130);
	if (ic < n - 1) return(129);

	/* ---GET AVERAGE X SPACING IN AVH--- */
	g = 0.;
	for (i = 1; i < n; ++i) {
		h = x[i + 1] - x[i];
		if (h <= 0.)
			return(131);
		g += h;
	}
	*avh = g / (n - 1);

	/* ---SCALE RELATIVE WEIGHTS--- */
	g = 0.;
	for (i = 1; i <= n; ++i) {
		if (dy[i] <= 0.)
			return(132);
		g += dy[i] * dy[i];
	}
	*avdy = sqrt(g / n);

	for (i = 1; i <= n; ++i)
		dy[i] /= *avdy;

	/* ---INITIALIZE H,F--- */
	h = (x[2] - x[1]) / *avh;
	f = (y[2] - y[1]) / h;

	for (i = 2; i < n; ++i) {	 /* ---CALCULATE A,T,R--- */
		g = h;
		h = (x[i + 1] - x[i]) / *avh;
		e = f;
		f = (y[i + 1] - y[i]) / h;
		a[i] = f - e;
		t[i + t_dim1] = (g + h) * 2. / 3.;
		t[i + (t_dim1 << 1)] = h / 3.;
		r[i + r_dim1 * 3] = dy[i - 1] / g;
		r[i + r_dim1] = dy[i + 1] / h;
		r[i + (r_dim1 << 1)] = -dy[i] / g - dy[i] / h;
	}

	/* ---CALCULATE C = R'*R--- */
	r[n + (r_dim1 << 1)] = 0.;
	r[n + r_dim1 * 3] = 0.;
	r[n + 1 + r_dim1 * 3] = 0.;
	for (i = 2; i < n; ++i) {
		c[i + c_dim1] = r[i + r_dim1] * r[i + r_dim1] + r[i + (r_dim1 << 1)] * r[i + (r_dim1 << 1)] + r[i + r_dim1 * 3] * r[i + r_dim1 * 3];
		c[i + (c_dim1 << 1)] = r[i + r_dim1] * r[i + 1 + (r_dim1 << 1)] + r[i + (r_dim1 << 1)] * r[i + 1 + r_dim1 * 3];
		c[i + c_dim1 * 3] = r[i + r_dim1] * r[i + 2 + r_dim1 * 3];
	}
	return 0;
}

/* ---------------------------------------------------------------------------- */
int spfit1(double *x, double *avh, double *dy, int n, double *rho, double *p, double *q, double *fun,
	double var, double *stat, double *a, double * c, int ic, double *r, double *t, double *u, double *v) {

	int i, c_dim1, c_offset, r_dim1, r_offset, t_dim1, t_offset;
	double d1, e, f, g, h, rho1;

/* FITS A CUBIC SMOOTHING SPLINE TO DATA WITH RELATIVE */
/* WEIGHTING DY FOR A GIVEN VALUE OF THE SMOOTHING PARAMETER */
/* RHO USING AN ALGORITHM BASED ON THAT OF C.H. REINSCH (1967), */
/* NUMER. MATH. 10, 177-183. */

/* THE TRACE OF THE INFLUENCE MATRIX IS CALCULATED USING AN */
/* ALGORITHM DEVELOPED BY M.F.HUTCHINSON AND F.R.DE HOOG (NUMER. */
/* MATH., IN PRESS), ENABLING THE GENERALIZED CROSS VALIDATION */
/* AND RELATED STATISTICS TO BE CALCULATED IN ORDER N OPERATIONS. */

/* THE ARRAYS A, C, R AND T ARE ASSUMED TO HAVE BEEN INITIALIZED */
/* BY THE SUBROUTINE SPINT1.  OVERFLOW AND UNDERFLOW PROBLEMS ARE */
/* AVOIDED BY USING P=RHO/(1 + RHO) AND Q=1/(1 + RHO) INSTEAD OF */
/* RHO AND BY SCALING THE DIFFERENCES X(I+1) - X(I) BY AVH. */

/* THE VALUES IN DF ARE ASSUMED TO HAVE BEEN SCALED SO THAT THE */
/* SUM OF THEIR SQUARED VALUES IS N.  THE VALUE IN VAR, WHEN IT IS */
/* NON-NEGATIVE, IS ASSUMED TO HAVE BEEN SCALED TO COMPENSATE FOR */
/* THE SCALING OF THE VALUES IN DF. */

/* THE VALUE RETURNED IN FUN IS AN ESTIMATE OF THE TRUE MEAN SQUARE */
/* WHEN VAR IS NON-NEGATIVE, AND IS THE GENERALIZED CROSS VALIDATION */
/* WHEN VAR IS NEGATIVE. */

	/* Parameter adjustments */
	t_dim1 = n + 1 - 0 + 1;
	t_offset = 0 + t_dim1;
	t -= t_offset;
	r_dim1 = n + 1 - 0 + 1;
	r_offset = 0 + r_dim1;
	r -= r_offset;
	--a;
	--dy;
	--x;
	--stat;
	c_dim1 = ic;
	c_offset = 1 + c_dim1;
	c -= c_offset;

	/* ---USE P AND Q INSTEAD OF RHO TO PREVENT OVERFLOW OR UNDERFLOW--- */
	rho1 = 1. + *rho;
	*p = *rho / rho1;
	*q = 1. / rho1;
	if (rho1 == 1.)
		*p = 0.;

	if (rho1 == *rho)
		*q = 0.;


	/* ---RATIONAL CHOLESKY DECOMPOSITION OF P*C + Q*T--- */
	f = g = h = 0.;
	for (i = 0; i < 2; ++i)
		r[i + r_dim1] = 0;

	for (i = 2; i < n; ++i) {
		r[i - 2 + r_dim1 * 3] = g * r[i - 2 + r_dim1];
		r[i - 1 + (r_dim1 << 1)] = f * r[i - 1 + r_dim1];
		r[i + r_dim1] = 1. / (*p * c[i + c_dim1] + *q * t[i + t_dim1] - f * r[i - 1 + (r_dim1 << 1)] - g * r[i - 2 + r_dim1 * 3]);
		f = *p * c[i + (c_dim1 << 1)] + *q * t[i + (t_dim1 << 1)] - h * r[i - 1 + (r_dim1 << 1)];
		g = h;
		h = *p * c[i + c_dim1 * 3];
	}

	/* ---SOLVE FOR U--- */
	u[0] = 0.;
	u[1] = 0.;
	for (i = 2; i < n; ++i)
		u[i] = a[i] - r[i - 1 + (r_dim1 << 1)] * u[i - 1] - r[i - 2 + r_dim1 * 3] * u[i - 2];

	u[n] = 0.;
	u[n + 1] = 0.;
	for (i = n - 1; i >= 2; --i)
		u[i] = r[i + r_dim1] * u[i] - r[i + (r_dim1 << 1)] * u[i + 1] - r[i + r_dim1 * 3] * u[i + 2];


	/* ---CALCULATE RESIDUAL VECTOR V--- */
	e = h = 0.;
	for (i = 1; i < n; ++i) {
		g = h;
		h = (u[i + 1] - u[i]) / ((x[i + 1] - x[i]) / *avh);
		v[i] = dy[i] * (h - g);
		e += v[i] * v[i];
	}
	v[n] = dy[n] * (-h);
	e += v[n] * v[n];

	/* ---CALCULATE UPPER THREE BANDS OF INVERSE MATRIX--- */
	r[n + r_dim1] = 0.;
	r[n + (r_dim1 << 1)] = 0.;
	r[n + 1 + r_dim1] = 0.;
	for (i = n - 1; i >= 2; i--) {
		g = r[i + (r_dim1 << 1)];
		h = r[i + r_dim1 * 3];
		r[i + (r_dim1 << 1)] = -g * r[i + 1 + r_dim1] - h * r[i + 1 + (r_dim1 << 1)];
		r[i + r_dim1 * 3] = -g * r[i + 1 + (r_dim1 << 1)] - h * r[i + 2 + r_dim1];
		r[i + r_dim1] = r[i + r_dim1] - g * r[i + (r_dim1 << 1)] - h * r[i + r_dim1 * 3];
	}

	/* ---CALCULATE TRACE--- */
	f = g = h = 0.;
	for (i = 2; i < n; i++) {
		f += r[i + r_dim1] * c[i + c_dim1];
		g += r[i + (r_dim1 << 1)] * c[i + (c_dim1 << 1)];
		h += r[i + r_dim1 * 3] * c[i + c_dim1 * 3];
	}
	f += 2 * (g + h);

	/* ---CALCULATE STATISTICS--- */
	stat[1] = *p;
	stat[2] = f * *p;
	stat[3] = n * e / (f * f);
	stat[4] = e * *p * *p / n;
	stat[6] = e * *p / f;
	if (var < 0.) {
		stat[5] = stat[6] - stat[4];
		*fun = stat[3];
	}
	else {
		/* Computing MAX */
		d1 = stat[4] - 2 * var * stat[2] / n + var;
		stat[5] = MAX(d1,0.);
		*fun = stat[5];
	}

	return 0;
}

/* ---------------------------------------------------------------------------- */
int sperr1(double *x, double avh, double *dy, int n, double *r, double p, double var, double *se) {

	/* CALCULATES BAYESIAN ESTIMATES OF THE STANDARD ERRORS OF THE FITTED */
	/* VALUES OF A CUBIC SMOOTHING SPLINE BY CALCULATING THE DIAGONAL ELEMENTS */
	/* OF THE INFLUENCE MATRIX. */

	int i, r_dim1, r_offset;
	static double f, g, h, f1, g1, h1;

	/* Parameter adjustments */
	--se;
	r_dim1 = n + 1 + 1;
	r_offset = 0 + r_dim1;
	r -= r_offset;
	--dy;
	--x;

	/* ---INITIALIZE--- */
	h = avh / (x[2] - x[1]);
	se[1] = 1. - p * dy[1] * dy[1] * h * h * r[r_dim1 + 2];
	r[r_dim1 + 1] = 0.;
	r[(r_dim1 << 1) + 1] = 0.;
	r[r_dim1 * 3 + 1] = 0.;

	for (i = 2; i < n; i++) { 	/* ---CALCULATE DIAGONAL ELEMENTS--- */
		f = h;
		h = avh / (x[i + 1] - x[i]);
		g = -f - h;
		f1 = f * r[i - 1 + r_dim1] + g * r[i - 1 + (r_dim1 << 1)] + h * r[i - 1 + r_dim1 * 3];
		g1 = f * r[i - 1 + (r_dim1 << 1)] + g * r[i + r_dim1] + h * r[i + (r_dim1 << 1)];
		h1 = f * r[i - 1 + r_dim1 * 3] + g * r[i + (r_dim1 << 1)] + h * r[i + 1 + r_dim1];
		se[i] = 1. - p * dy[i] * dy[i] * (f * f1 + g * g1 + h * h1);
	}
	se[n] = 1. - p * dy[n] * dy[n] * h * h * r[n - 1 + r_dim1];

	/* ---CALCULATE STANDARD ERROR ESTIMATES--- */
	for (i = 1; i <= n; ++i)  /* Computing MAX */
		se[i] = sqrt((MAX((se[i] * var),0.))) * dy[i];

	return 0;
}

/* -------------------------------------------------------------------------- */
int spcof(double *x, double avh, double *y, double *dy, int n, double p, double q, double *a,
	double *c, int ic, double *u, double *v) {

	/* CALCULATES COEFFICIENTS OF A CUBIC SMOOTHING SPLINE FROM */
	/* PARAMETERS CALCULATED BY SUBROUTINE SPFIT1. */

	int i, c_dim1, c_offset;
	static double qh, h;

	/* ---CALCULATE A--- */
	/* Parameter adjustments */
	--a;
	--dy;
	--y;
	--x;
	c_dim1 = ic;
	c_offset = 1 + c_dim1;
	c -= c_offset;

	qh = q / (avh * avh);
	for (i = 1; i <= n; i++) {
		a[i] = y[i] - p * dy[i] * v[i];
		u[i] = qh * u[i];
	}

	/* ---CALCULATE C--- */
	for (i = 1; i < n; i++) {
		h = x[i + 1] - x[i];
		c[i + c_dim1 * 3] = (u[i + 1] - u[i]) / (h * 3.);
		c[i + c_dim1] = (a[i + 1] - a[i]) / h - (h * c[i + c_dim1 * 3] + u[i]) * h;
		c[i + (c_dim1 << 1)] = u[i];
	}
	return 0;
}

#if 0
int main (int argc, char **argv) {

	double X[50] = {
		0.0046, 0.0360, 0.0435, 0.0735, 0.0955, 0.1078, 0.1269, 0.1565, 0.1679, 0.1869, 0.2149,
		0.2356, 0.2557, 0.2674, 0.2902, 0.3155, 0.3364, 0.3557, 0.3756, 0.3881, 0.4126, 0.4266,
		0.4566, 0.4704, 0.4914, 0.5084, 0.5277, 0.5450, 0.5641, 0.5857, 0.6159, 0.6317, 0.6446,
		0.6707, 0.6853, 0.7064, 0.7310, 0.7531, 0.7686, 0.7952, 0.8087, 0.8352, 0.8501, 0.8726,
		0.8874, 0.9139, 0.9271, 0.9473, 0.9652, 0.9930};

	double F[50] = {
 		0.2222, -0.1098, -0.0658, 0.3906, 0.6054, 0.3034, 0.7386, 0.4616, 0.4315, 0.5716, 0.6736,
 		0.7388, 1.1953, 1.0299, 0.7981, 0.8973, 1.2695, 0.7253, 1.2127, 0.7304, 0.9810, 0.7117,
 		0.7203, 0.9242, 0.7345, 0.7378, 0.7441, 0.5612, 0.5049, 0.4725, 0.1380, 0.1412, -0.1110,
		-0.2605, -0.1284, -0.3452, -0.5527, -0.3459, -0.5902, -0.7644, -0.5392, -0.4247, -0.6327,
		-0.9983, -0.9082, -0.8930, -1.0233, -0.8839, -1.0172, -1.2715};

	double DF[50], Y[50], C[49*3], SE[50], WK[7*(50+2)], VAR;
	int i, n, err, IC;

	n = 50;
	IC = 49;
	VAR = -1;
	for (i = 0; i < 50; i++) DF[i] = 1.;

	err = cubgcv_(X, F, DF, n, Y, C, IC, &VAR, 1, SE, WK);

	if (err) {
		fprintf(stderr, "Deu merda. err = %d\n", err);
		return(0);
	}

	fprintf(stdout, "X(I)    F(I)    Y(I)    SE(I)\n");
	for (i = 0; i < 50; i++)
		fprintf(stdout, "%.4f\t%.4f\t%.4f\t%.4f\n", X[i], F[i], Y[i], SE[i]);

	return(0);
}
#endif
