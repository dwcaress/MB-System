/*--------------------------------------------------------------------
 *    The MB-system:	mb_cheb.c			3/23/00
 *    $Id$
 *
 *    Copyright (c) 2000-2009 by
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
 * Author:	D. W. Caress
 * Date:	March 23, 2000
 *
 * $Log: mb_cheb.c,v $
 * Revision 5.3  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.2  2007/10/08 05:56:18  caress
 * Changed convergence criteria.
 *
 * Revision 5.1  2006/01/11 07:33:01  caress
 * Working towards 5.0.8
 *
 * Revision 5.0  2000/12/01 22:53:59  caress
 * First cut at Version 5.0.
 *
 * Revision 4.1  2000/09/30  06:54:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  2000/09/08  17:19:14  caress
 * Initial version.
 *
 *
 *
 */

/*--------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>

/* mbio include files */
#include "../../include/mb_define.h"

void lsqup(
    double  *a,
    int	    *ia,
    int	    *nia,
    int	    nnz,
    int	    nc,
    int	    nr,
    double  *x,
    double  *dx,
    double  *d,
    int	    nfix,
    int	    *ifix,
    double  *fix,
    int	    ncycle,
    double  *sigma);
void chebyu(
    double  *sigma,
    int	    ncycle,
    double  shi,
    double  slo,
    double  *work);
void splits(
    double  *x,
    double  *t,
    int	    n);
double errlim(
	double	*sigma,
	int	ncycle,
	double	shi,
	double	slo);
double errrat(
	double	x1,
	double	x2,
	double	*sigma,
	int	ncycle);
void lspeig(
	double	*a,
	int	*ia,
	int	*nia,
	int	nnz,
	int	nc,
	int	nr,
	int	ncyc,
	int	*nsig,
	double	*x,
	double	*dx,
	double	*sigma,
	double	*w,
	double	*smax,
	double	*err,
	double	*sup);

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
void lsqup(
    double  *a,
    int	    *ia,
    int	    *nia,
    int	    nnz,
    int	    nc,
    int	    nr,
    double  *x,
    double  *dx,
    double  *d,
    int	    nfix,
    int	    *ifix,
    double  *fix,
    int	    ncycle,
    double  *sigma)
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
 *
 *----------------------------------------------------------------------*/
{
    int	    i, j, k, icyc;
    double  res;
    double  s;

for (i=0;i<nr;i++)
	{
	s = 0.0;
	for (j=0;j<nia[i];j++)
		{
		k = nnz * i + j;
		s += x[ia[k]] * a[k];
		}
	}

    /* loop over all cycles */
    for (icyc=0;icyc<ncycle;icyc++)
	{
	/* initialize dx */
	for (j=0;j<nc;j++)
	    dx[j] = 0.0;
	    
	/* loop over each row */
	for (i=0;i<nr;i++)
	    {
	    res = 0.0;
	    for (j=0;j<nia[i];j++)
		{
		k = nnz * i + j;
		res += a[k] * x[ia[k]];
		}
	    res = d[i] - res;
	    for (j=0;j<nia[i];j++)
		{
		k = nnz * i + j;
		dx[ia[k]] += res * a[k];
		}
	    }
	    
	/* update x */
 	for (j=0;j<nc;j++)
	    x[j] += dx[j] / sigma[icyc];
	    
	/* apply fixed values */
	for (j=0;j<nfix;j++)
	    x[ifix[j]] = fix[j];
	    
	/* output info */
	/*fprintf(stderr, "lsqup cycle %d completed...\n", icyc);*/
	}
}
/*----------------------------------------------------------------------*/
void chebyu(
    double  *sigma,
    int	    ncycle,
    double  shi,
    double  slo,
    double  *work)
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
 {
    int	    i, len, is, i0, nsort;
    
    /* set up the chebyshev weights in increasing order */
    for (i=0;i<ncycle;i++)
	{
	sigma[i] = -cos((2 * (i + 1) - 1)
			* M_PI / 2 / ncycle);
	sigma[i] = (sigma[i] * (shi - slo) + (shi + slo)) / 2;
	}

    /* sort the weights */
    len = ncycle;
    while (len > 2)
	{
	nsort = ncycle / len;
	for (is=0;is<nsort;is++)
	    {
	    i0 = is * len;
	    splits(&sigma[i0], work, len);
	    }
	len /= 2;
	}
 }
/*----------------------------------------------------------------------*/
void splits(
    double  *x,
    double  *t,
    int	    n)
{
    int	    i, l, nb2, nb2m1;
    
    l = 0;
    for (i=0;i<n;i+=2)
	{
	t[l] = x[i];
	l++;
	}
    for (i=1;i<n;i+=2)
	{
	t[l] = x[i];
	l++;
	}
	
    nb2 = n / 2;
    nb2m1 = nb2 - 1;
    if (nb2 >= 2)
	{
	for (i=0;i<nb2;i++)
	    {
	    x[i] = t[nb2m1 - i];
	    }
	for (i=nb2;i<n;i++)
	    {
	    x[i] = t[i];
	    }
	}
    else
	{
	for (i=0;i<n;i++)
	    x[i] = t[i];
	}
}
/*----------------------------------------------------------------------*/
/* returns limit of the maximum theoretical error using chebyshev weights */
double errlim(
	double	*sigma,
	int	ncycle,
	double	shi,
	double	slo)
/*----------------------------------------------------------------------*/
{
    double  errlim;
    double  delta;
    int	    i;
    
    errlim = 1.0;
    delta = 0.25 * (shi - slo);
    for (i=0;i<ncycle;i++)
	{
	errlim *= delta / sigma[i];
	}
    errlim = 2 * errlim;
    return(errlim);
}
/*----------------------------------------------------------------------*/
/* computes the ratio of the error at eigenvalue x1 to the error at x2. */
double errrat(
	double	x1,
	double	x2,
	double	*sigma,
	int	ncycle)
/*----------------------------------------------------------------------*/
{
    double  errrat;
    double  rat;
    int	    k;
    
    errrat = 1.0;
    rat = x1 / x2;
    for (k=0;k<ncycle;k++)
	{
	errrat = errrat * rat * (1.0 - sigma[k] / x1)
				/ (1.0 - sigma[k] / x2);
	}
    errrat = fabs(errrat);
    return(errrat);
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
void lspeig(
	double	*a,
	int	*ia,
	int	*nia,
	int	nnz,
	int	nc,
	int	nr,
	int	ncyc,
	int	*nsig,
	double	*x,
	double	*dx,
	double	*sigma,
	double	*w,
	double	*smax,
	double	*err,
	double	*sup)
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
 *    x(1..nc)  -----  - inital guess for the eigenvector.
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
 *        smax and reflects the uncertainty due to an anomolous bad choice
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
{
    int	    i, j, k, icyc;
    int	    nsig1;
    double  eps = 1.0e-6;
    double  res = 0.0;
    double  slo, smp, errsmp;
    
    if (ncyc == 0)
	{
	i = 0;
	for (j=0;j<nia[i];j++)
	    {
	    k = nnz * i + j;
	    x[ia[k]] = a[k];
	    }
	for (i=1;i<nr;i++)
	    {
	    res = 0.0;
	    for (j=0;j<nia[i];j++)
		{
		k = nnz * i + j;
		res += x[ia[k]] * a[k];
		}
	    if (fabs(res) <= 1.0e-30)
		res = 1.0;
	    else
		res = res / fabs(res);
	    for (j=0;j<nia[i];j++)
		{
		k = nnz * i + j;
		x[ia[k]] += res * a[k];
		}
	    }
	res = 0.0;
	for (j=0;j<nc;j++)
	    {
	    res += x[j] * x[j];
	    }
	res = 1.0 / sqrt(res);
	for (j=0;j<nc;j++)
	    {
	    x[j] = x[j] * res;
	    }
	}
	
    else
	{
	slo = 0.0;
	chebyu(&sigma[*nsig], ncyc, *smax, slo, w);
	}
	
    nsig1 = *nsig + 1;
    *nsig = nsig1 + ncyc;
    sigma[*nsig-1] = 0.0;
    for (icyc=nsig1-1;icyc<*nsig;icyc++)
	{
	for (j=0;j<nc;j++)
	    {
	    dx[j] = 0.0;
	    }
	for (i=0;i<nr;i++)
	    {
	    res = 0.0;
	    for (j=0;j<nia[i];j++)
		{
		k = nnz * i + j;
		res += a[k] * x[ia[k]];
		}
	    for (j=0;j<nia[i];j++)
		{
		k = nnz * i + j;
		dx[ia[k]] += res * a[k];
		}
	    }
	for (j=0;j<nc;j++)
	    {
	    dx[j] -= sigma[icyc] * x[j];
	    }
	*smax = 0.0;
	for (j=0;j<nc;j++)
	    {
	    *smax += dx[j] * dx[j];
	    }
	*smax = sqrt(*smax);
	
	if (icyc == *nsig - 1)
	    {
	    *err = 0.0;
	    for (j=0;j<nc;j++)
		{
		res = dx[j] - *smax * x[j];
		*err += res * res;
		}
	    *err = sqrt(*err);
	    }
	    
	for (j=0;j<nc;j++)
	    {
	    x[j] = dx[j] / *smax;
	    }
	}
	
    slo = *smax;
    *sup = (1.0 + eps) * (*smax)
	    * pow(eps, -1.0 / *nsig);
    res = 1.0;
    for (icyc=0;icyc<25 && res > eps;i++)
	{
	smp = 0.5 * (*sup + slo);
	errsmp = errrat(*smax, smp, sigma, *nsig);
	if (errsmp > eps)
	    slo = smp;
	else
	    *sup = smp;
	res = (*sup - slo) / slo;
	}
}
/*----------------------------------------------------------------------*/
