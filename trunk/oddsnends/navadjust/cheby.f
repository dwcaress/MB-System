
c----------------------------------------------------------------------
      subroutine lsqup(a,ia,na,nnz,nc,nr,x,dx,d,
     &                 nfix,ifix,fix,ncycle,sigma)
c----------------------------------------------------------------------
c
c     least squares solution using richardson's algorithm
c     with chebyshev acceleration.  the step size is varied to obtain
c     uniform convergence over a prescribed range of eigenvalues.
c
c                             ..... allen h. olson 6-29-85.
c                                   university of california, san diego  
c
c     Altered to allow particular values to be fixed.
c                             ..... David W. Caress 4-14-88
c----------------------------------------------------------------------
c
c                       nc                   ~
c          given :     sum [ a(j,i) * x(j) ] = d(i)   ;  i=1, ... nr
c                      j=1
c                                               t
c          minimize :  || a*x - d || = (a*x - d) * (a*x -d)
c
c----------------------------------------------------------------------
c -------
c  input
c -------
c     a(j,i)  -------  - packed matrix defined above dimensioned at least
c                        (nnz,nr) note  that the first index is the column
c                        index! i.e. the matrix is stored in row order. 
c     ia(j,i)  ------  - indices of values in packed matrix a
c                        i.e. a(i,j) packed = a(ia(i,j),j) unpacked
c     na    ---------  - leading dimension of a(.,.)      
c     nnz   ---------  - number of values in packed rows of a and ia
c     nc,nr  --------  - number of columns and number of rows of unpacked 
c                        matrix as defined above.
c     x(1...nc) -----  - initial guess solution; can be set to zero
c                        or values returned form previous calls to lsquc.
c     dx(1...nc) ----  - temporary storage array
c     d(1...nr) -----  - data as defined above    
c     nfix  ---------  - number of solution values to be fixed
c     ifix  ---------  - idices of fixed values
c     fix  ----------  - fixed values:  x(ifix(j)) = fix(j)
c     ncycle  -------  - number of iterations to perform. must be power of 2.
c     sigma(1..ncycle) - array containing the weights for step sizes.
c                        see subroutine 'chebyu' for computing these.
c --------
c  output
c --------
c     x(1..nc)   - solution vector as defined above
c                  only array x(..) is over-written by lsquc.
c
c----------------------------------------------------------------------
      real*8 a(na,*),x(nc),dx(nc),d(nr),fix(*),sigma(*),res 
      integer ia(na,*),ifix(*)
c
      do 3000 icyc=1,ncycle
c
      do 1000 j=1,nc
1000  dx(j)=0.0
c
      do 1200 i=1,nr
      res=0.0
      do 1100 j=1,nnz
1100  res=res+a(j,i)*x(ia(j,i))
      res=d(i)-res
      do 1150 j=1,nnz
1150  dx(ia(j,i))=dx(ia(j,i)) + res*a(j,i)
1200  continue
c
      do 1300 j=1,nc
1300  x(j)=x(j)+dx(j)/sigma(icyc)  
      do 1400 j=1,nfix
1400  x(ifix(j))=fix(j)
c               
c      write(*,*)'cycle ',icyc,' completed'
3000  continue
c
      return
      end
c----------------------------------------------------------------------
      subroutine chebyu(sigma,ncycle,shi,slo,work)
c----------------------------------------------------------------------
c
c computes the chebyshev weights with uniform distribution.
c weights are ordered pair-wise in such a fashion that after an even
c number of steps they are distributed uniformly on the interval [slo,shi].
c this ordering provides optimum numerical stability of routine lsquc.
c
c                             ..... allen h. olson 6-29-85.
c                                   university of california, san diego
c
c----------------------------------------------------------------------
c -------
c  input
c -------
c
c     ncycle   ------  - must be a power of two!  number of iterations.
c
c     shi,slo  ------  - high and low limits defining the band of eigenvalues
c                        to retain in the solution.  shi >= largest eigenvalue
c                        of the normal equations.
c     work(1..ncycle)  - work array for sorting array sigma(..).
c
c -------
c  output
c -------
c
c     sigma(1..ncycle) - weights for the step sizes in routine lsquc.
c -------
c calls subroutine splits.
c
c----------------------------------------------------------------------
      real*8 sigma(*),work(*),shi,slo,pi
c
      pi=3.14159265358979 
c
c  set up the chebyshev weights in increasing order
c
      do 100 i=1,ncycle
      sigma(i)=-cos( (2*i-1)*pi/2/ncycle )
      sigma(i)=( sigma(i)*(shi-slo)+(shi+slo) )/2
  100 continue
c
c  sort the weights
c
      len=ncycle
  200 nsort=ncycle/len
      do 300 is=1,nsort
      i0=1+(is-1)*len  
      call splits(sigma(i0),work,len)
  300 continue
      len=len/2
      if(len.gt.2) goto 200
c
      return
      end
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
c----------------------------------------------------------------------
      subroutine lspeig(a,ia,na,nnz,nc,nr,ncyc,nsig,x,dx,sigma,w,
     &                  smax,err,sup)
c----------------------------------------------------------------------
c     least-squares eigenvalue
c
c     iteratively estimates the largest eigenvalue and eigenvectior
c     with error bounds for the least-squares normal matrix a'a.
c     a chebyshev criterion is used to calculate the optimum set of
c     origin shifts in order to accelerate convergence.
c     based upon the rayleigh quotient and error analysis presented in
c     j. h. wilkinson's "the algebraic eigenvalue problem",
c     (pp 170 ...), (pp 572 ...).
c     under very pesimistic assumptions regarding the starting vector,
c     the algorithm will initially converge to an eigenvalue less than
c     the largest.  hence, a corresponding pessimistic estimate of an
c     upper-bound on the largest eigenvalue is also made.
c
c                             ..... allen h. olson 10-4-85.
c                                   university of california, san diego
c----------------------------------------------------------------------
c -------
c  input
c -------
c     a(j,i)  -------  - packed matrix defined above dimensioned at least
c                        (nnz,nr) note  that the first index is the column
c                        index! i.e. the matrix is stored in row order. 
c     ia(j,i)  ------  - indices of values in packed matrix a
c                        i.e. a(i,j) packed = a(ia(i,j),j) unpacked
c     na    ---------  - leading dimension of a(.,.)      
c     nnz   ---------  - number of values in packed rows of a and ia
c     nc,nr  --------  - number of columns and number of rows of unpacked 
c                        matrix as defined above.
c     ncyc  ---------  - number of chebyshev iterations to perform.
c                          must be a power of two.
c     nsig  ---------  - cumulative number of iterations performed by
c                          previous calls to this routine.  must be set
c                          to zero on initial call.  nsig is automatically
c                          incremented by this routine and must not be
c                          redefined on subsequent calls by calling program.
c     x(1..nc)  -----  - inital guess for the eigenvector.
c     dx(1...nc) ----  - temporary storage array for x(.).
c     sigma(1..nsmx)-  - array for holding the chebyshev origin shifts.
c                          each call to lseig performs ncyc+1 iterations.
c                          nsmx must be greater than or equal to the
c                          cumulative number of iterations to be performed.
c     w(1..nsmx)  ---  - temporary storage array for sigma(.).
c     smax  ---------  - initial guess for the eigenvalue.
c --------
c  output
c --------
c     x(1...nc) -----  - revised estimate of largest eigenvector.
c     smax  ---------  - reivsed estimate of largest eigenvalue of a'a.
c     err   ---------  - error bound for smax. we are guaranteed that
c                          at least one eigenvalue is contained in the
c                          interval smax+-err.  in the neighborhood of
c                          convergence, this will contain the maximum.
c     sup   ---------  - a pessimistic upper bound for the largest
c                          eigenvector.
c ------
c  note:(1) for ncyc=0, an initial guess for the eigenvector is formed by
c ------  summing the rows of the matrix so that the accumulated vector
c         increases in length as each row is added.  one iteration of the
c         power method is then performed to estimate smax.
c       (2) caution: an anomalous bad guess for the initial eigenvector
c         being virtually orthogonal to the largest eigenvector
c         will cause earlier iterations to converge to the next largest
c         eigenvector.  this is impossible to detect.  in this case, smax
c         may be much less than the largest eigenvalue.
c           the parameter eps set below is a pessimistic assumption about
c         the relative size of the component of the largest eigenvector in
c         the initial iteration.  from this, an upper-bound is calculated
c         for the largest eigenvalue, sup.  sup will always be larger than
c         smax and reflects the uncertainty due to an anomolous bad choice
c         for the starting vector.
c -------------------------
c  sample calling sequence
c -------------------------
c     ncyc=0
c     nsig=0
c     call lseig(a,na,nc,nr,ncyc,nsig,x,dx,sigma,w,smax,err,sup)
c     ncyc=4
c     call lseig(a,na,nc,nr,ncyc,nsig,x,dx,sigma,w,smax,err,sup)
c     ncyc=8
c     call lseig(a,na,nc,nr,ncyc,nsig,x,dx,sigma,w,smax,err,sup)
c
c     the first call with ncyc=0 initializes x(.) and smax.  if these
c       are already known then ncyc can be set to a nonzero value for the
c       first call.  nsig must always be zero for the first call however.
c     the next two calls perform chebyshev iteration to improve x(.) and smax.
c       upon completion, a total of nsig=1+5+9=15 iterations have actually
c       been performed.
c     by making repeated calls to lseig, the error in smax and the difference
c       between smax and sup can be monitored until the desired level of
c       certainty is obtained.
c----------------------------------------------------------------------
      real*8 a(na,*),x(*),dx(*),sigma(*),w(*)  
      real*8 res,sup,slo,smp,err,errsmp,smax,errrat
      integer ia(na,*)
c
      eps=1.e-6  
      if(ncyc.eq.0) then
        do 50 j=1,nnz
  50    x(ia(j,1))=a(j,1)
        do 150 i=2,nr
        res=0.0
        do 75 j=1,nnz
  75    res=res+x(ia(j,i))*a(j,i)
        if(abs(res).le. 1.e-30) then
           res=1.0
        else
           res=res/abs(res)
        endif
        do 100 j=1,nnz
  100   x(ia(j,i))=x(ia(j,i))+res*a(j,i)
  150   continue
        res=0.0
        do 200 j=1,nc
  200   res=res+x(j)*x(j)
        res=1.0/sqrt(res)
        do 300 j=1,nc
  300   x(j)=x(j)*res
      else
        slo=0.0
        call chebyu(sigma(nsig+1),ncyc,smax,slo,w)
      endif
c
      nsig1=nsig+1
      nsig =nsig1+ncyc
      sigma(nsig)=0.0
      do 3000 icyc=nsig1,nsig
c
      do 1000 j=1,nc
 1000 dx(j)=0.0
c
      do 1200 i=1,nr
      res=0.0
      do 1100 j=1,nnz
 1100 res=res+a(j,i)*x(ia(j,i))
      do 1150 j=1,nnz
 1150 dx(ia(j,i))=dx(ia(j,i)) + res*a(j,i)
 1200 continue
      do 1250 j=1,nc
 1250 dx(j)=dx(j) - sigma(icyc)*x(j)
c
      smax=0.0
      do 1300 j=1,nc
 1300 smax=smax+ dx(j)*dx(j)
      smax=sqrt(smax)
c
      if(icyc.eq.nsig) then
      err=0.0
      do 1350 j=1,nc
      res=dx(j)-smax*x(j)
 1350 err=err+ res*res
      err=sqrt(err)
      endif
c
      do 1400 j=1,nc
 1400 x(j)=dx(j)/smax
c
 3000 continue
c
      slo=smax
      sup=(1.+eps)*smax/(eps**(1.0/nsig))
      do 4000 icyc=1,25
      smp=(sup+slo)/2
      errsmp=errrat(smax,smp,sigma,nsig)
      if(errsmp.gt.eps) then
        slo=smp
      else
        sup=smp
      endif
      res=(sup-slo)/slo
      if(res.le.eps) go to 4500
 4000 continue
 4500 continue
c
      return
      end


