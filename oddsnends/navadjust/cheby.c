/*	CHEBY row action matrix inversion package
 */
#define PI 3.14159265358979 
/*----------------------------------------------------------------------*/
void lsqup(a,ia,na,nnz,nc,nr,x,dx,d,nfix,ifix,fix,ncycle,sigma)
double	a[na][nr];
int	ia[na][nr];
int	na,nnz,nc,nr;
double	x[nc];
double	dx[nc];
double	d[nr];
int	nfix;
int	ifix[nfix];
double	fix[nfix];
int	ncycle;
double	sigma[ncycle];
{
/*	Least squares solution using richardson's algorithm
 *	with chebyshev acceleration.  The step size is varied to obtain
 *	uniform convergence over a prescribed range of eigenvalues.
 *
 *	Duplicated in C from fortran code by Allen H. Olson (SIO)
 *	The original code dates from 6/29/85 with alterations by
 *	David W. Caress on 4/14/88.
 *
 *                        ..... David W. Caress 10/1/90
 *----------------------------------------------------------------------
 *
 *			     nc                   ~
 *		given :     sum ( a[i][j] * x[j] ) = d[i]   ;  i=1, ... nr
 *			    j=1
 *						     t
 *		minimize :  || a*x - d || = (a*x - d) * (a*x -d)
 *
 *----------------------------------------------------------------------
 * -------
 *  input
 * -------
 *	a[i][j]  -------  - packed matrix defined above dimensioned at least
 * 	                   (nr,nnz). 
 *	ia[i][j]  ------  - indices of values in packed matrix a
 *	                   i.e. a[i][j] packed = a[i][ia[i][j]] unpacked
 *	na    ---------  - leading dimension of a(.,.)      
 *	nnz   ---------  - number of values in packed rows of a and ia
 *	nc,nr  --------  - number of columns and number of rows of unpacked 
 *                         matrix as defined above.
 *	x[1...nc) -----  - initial guess solution; can be set to zero
 *                         or values returned form previous calls to lsquc.
 *	dx[1...nc] ----  - temporary storage array
 *	d[1...nr] -----  - data as defined above    
 *	nfix  ---------  - number of solution values to be fixed
 *	ifix  ---------  - idices of fixed values
 *	fix  ----------  - fixed values:  x[ifix[j]] = fix[j]
 *	ncycle  -------  - number of iterations to perform. must be power of 2.
 *	sigma[1..ncycle] - array containing the weights for step sizes.
 *                         see subroutine 'chebyu' for computing these.
 * --------
 *  output
 * --------
 *	x[1..nc]   - solution vector as defined above
 * 	            only array x[..] is over-written by lsquc.
 *
 *----------------------------------------------------------------------
	double	res;
	int	i,j,icyc;

	res = 0.0

	for (icyc=0;icyc<ncycle;icyc++)
		{
		for (j=0;j<nc;j++)
			dx[j] = 0.0;
		for (i=0;i<nr;i++)
			{
			for (j=0;j<nnz;j++)
				res = res + a[i][j]*x[ia[i][j]];
			res = d[i] - res;
			for (j=0;j<nnz;j++)
				dx[ia[i][j]] = dx[ia[i][j]] + res*a[i][j];
			}
		for (j=0;j<nc;j++)
			x[j] = x[j] + dx[j]/sigma[icyc];
		for (j=0;j<nfix;j++)
			x[ifix[j]] = fix[j];
		printf("cycled %d completed\n",icyc);
		}
	return;
}
/*----------------------------------------------------------------------*/
void chebyu(sigma,ncycle,shi,slo,work)
double	sigma[ncycle];
int	ncycle;
double	shi;
double	slo;
double	work[ncycle];
/*----------------------------------------------------------------------
 *
 * Computes the chebyshev weights with uniform distribution.
 * weights are ordered pair-wise in such a fashion that after an even
 * number of steps they are distributed uniformly on the interval [slo,shi].
 * this ordering provides optimum numerical stability of routine lsquc.
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
 *     work[1..ncycle]  - work array for sorting array sigma(..).
 *
 * -------
 *  output
 * -------
 *
 *     sigma[1..ncycle] - weights for the step sizes in routine lsquc.
 * -------
 * calls void splits.
 *
 *----------------------------------------------------------------------
 */
	int	i,i0,is,len;
	/* set up the chebyshev weights in increasing order */
	for (i=0;i<ncycle;i++)
		{
		sigma[i] = - cos((PI*0.5*(2.0*i+1.0)/ncycle));
		sigma[i] = 0.5*(sigma[i]*(shi - slo) + (shi + slo));
		}

	/* sort the weights */
	len = ncycle;
	while (len > 2)
		{
		nsort = ncycle/len;
		for (is=0;is<nsort;is++)
			{
			i0 = 1 + is*len;
			splits(sigma[i0],work,len);
			}
		len = len/2;
		}

	return
}
/*----------------------------------------------------------------------*/
void splits(x,t,n)
double	x[n];
double	t[n];
int	n;
{
	int	i,l;
	l = 0;
	for (i=0;i<n;i=i+2)
		{
		t[l] = x[i];
		l++;
		}
	for (i=1;i<n;i=i+2)
		{
		t[l] = x[i];
		l++;
		}
	nb2 = n/2;
	nb2p1 = nb2 + 1;

c---------------
      subroutine splits(x,t,n)
c---------  called by chebyu
      real*8 x(*),t(*)  
      l=0
      do 20 i=1,n,2
      l=l+1 
   20 t(l)=x(i)
      do 30 i=2,n,2
      l=l+1
   30 t(l)=x(i)
c
      nb2=n/2
      nb2p1=nb2+1
      if(nb2.ge.2) then
        do 40 i=1,nb2
   40   x(i)=t(nb2-i+1)
        do 50 i=nb2p1,n
   50   x(i)=t(i)
      else
        do 60 i=1,n
   60   x(i)=t(i)
      endif
      return
      end
c----------------------------------------------------------------------
      function errlim(sigma,ncycle,shi,slo)
c  returns limit of the maximum theoretical error using chebyshev weights
c----------------------------------------------------------------------
      real*8 sigma(*),shi,slo,delta,errlim
      errlim=1.0
      delta=0.25*(shi-slo)
      do 10 i=1,ncycle
   10 errlim=errlim*delta/sigma(i)
      errlim=2*errlim
      return
      end
c----------------------------------------------------------------------
      function errrat(x1,x2,sigma,ncycle)
c  computes the ratio of the error at eigenvalue x1 to the error at x2.
c----------------------------------------------------------------------
      real*8 sigma(*),rat,x1,x2
c
      errrat=1.0
      rat=x1/x2
      do 50 k=1,ncycle 
   50 errrat=errrat*rat*(1.0-sigma(k)/x1)/(1.0-sigma(k)/x2)
      errrat=abs(errrat)
      return
      end
