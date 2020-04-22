/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : Math.cc                                                       */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
//static char Math_id[] = "$Header$";

/*
$Log$
Revision 1.1.2.1  2018/11/20 20:03:21  henthorn
QNX-additions is a library of routines for use outside of QNX - stripped down version of DataLog, TimeP, and NavUtils. auv-qnx dependencies removed. Link to libqnx.a.

Revision 1.9.2.2  2016/10/05 18:24:23  rob
Added both uniform and Gaussian noise generators.  Added a unit test, testGasDev.

Revision 1.9.2.1  2016/02/02 22:49:56  rob
Overloaed shellSort to also sort an array of structs.

Revision 1.9  2013/12/05 19:18:29  rob
Overloaded the shell sort routine to take a pointer to a long, and a pointer
to a double, as arguments.  testShell is a unit test.

I'll probably replace these variations with a single template version the
next time around.

Revision 1.8  2006/09/06 21:29:23  rob
Added a Shell sort routine.

Revision 1.7  2004/09/15 23:28:06  rob
Added a root-solver using the bisect algorithm from Numerical Recipes.

Revision 1.6  2001/06/02 21:33:53  hthomas
various changes; remove stale entries in root makefile, fixed file naming to allow for serving from windows, and fixed makefiles to avoid file linking/unlinking problem in makedepend when talking through samba to windows boxes

Revision 1.5  2000/08/21 20:54:20  oreilly
Added round() method

Revision 1.4  2000/05/02 21:04:10  rob
Corrected conversion constant.

Revision 1.3  2000/05/02 21:01:02  rob
Added rpm to radians per second conversion constant.

Revision 1.2  2000/02/07 20:59:22  pean
Added revised header to include Proprietary Information

Revision 1.1  1999/12/07 23:03:05  pean
Created utils subdirectory and moved source files here to help manage sources

Revision 1.4  1999/12/03 23:21:15  rob
Moved some functions from DynamicControl here.

Revision 1.3  1999/10/12 23:17:47  oreilly
*** empty log message ***

Revision 1.2  1999/09/15 23:03:41  pean
Added kvh compass interface module

Revision 1.1  1999/09/09 18:39:38  oreilly
*** empty log message ***

Revision 1.3  1997/04/29 10:15:49  oreilly
Use more efficient algorithms for angle normalization

Revision 1.2  96/10/28  08:53:24  08:53:24  oreilly (Thomas C. O'Reilly)
Math utilities

Revision 1.1  96/08/14  13:30:38  13:30:38  oreilly (Thomas C. O'Reilly)
Initial revision

*/

#include <stdio.h>
#include "MathP.h"

double const Math::TwoPi      = M_PI  * 2.0;
double const Math::RadsPerDeg = M_PI  / 180.0;
double const Math::DegsPerRad = 180.0 / M_PI;
double const Math::RpmToRadps = M_PI  / 30.0;

void Math::zeroToTwoPi(Radians *angle)
{
  float m = 1.;
  if (*angle < 0.)
    m = -1.;
  
  double ratio = fabs(*angle) / TwoPi;

  *angle = m * TwoPi * (ratio - floor(ratio));

  while (*angle < 0.0)
    (*angle) += TwoPi;
  
  while (*angle > TwoPi)
    (*angle) -= TwoPi; 
}


void Math::minusPiToPi(Radians *angle)
{
  Math::zeroToTwoPi(angle);
  
  if (*angle < 0.0)
  {
    while (fabs(*angle) > M_PI)
      (*angle) += (TwoPi);
  }
  else
  {
    while (fabs(*angle) > M_PI)
      (*angle) -= (TwoPi);
  }
}


Radians Math::angularSeparation(Radians start, Radians stop)
{
  Radians delta = stop - start;
  while (delta > TwoPi)
    delta -= TwoPi;
  
  while (delta < 0.0)
    delta += TwoPi;

  return delta;
}

Radians Math::degToRad(double degs)
{
  return degs * RadsPerDeg;
}

double Math::radToDeg(Radians rads)
{
  return rads * DegsPerRad;
}


/* ==================================================================
|    LIMIT:
|      Makes sure a variable falls within defined limits...
| ================================================================== */

double Math::limit( double value, double max, double min )
{
    double dummy;

    if ( max < min )
    {
        dummy = max;
        max = min;
        min = dummy;
    }

    if ( value > max )
        value = max;
    if ( value < min )
        value = min;
    return value;
}

/*===================================================================*/

double Math::sgn( double x )
{
    if ( x < 0 )
        return -1;
    else
        return  1;
}

/* --------------------------------------------------------------------------
| modPi:
|
|   J. G. Bellingham, 3/1/94
  -------------------------------------------------------------------------- */

double Math::modPi( double angle )
{
    double ipart, result;

    result = 2*PI*modf( angle/(2*PI), &ipart );

    if ( result >= PI )
        result -= 2*PI;

    if ( result < -PI )
        result += 2*PI;

    return result;
}



int Math::round(double x)
{
  double incr;
  if (x > 0.) {
    incr = 0.5;
  }
  else {
    incr = -0.5;
  }

  return (int )(x + incr);
}
#define CMAX 40
//
// See MathP.h
//
// (From Numerical Recipes) Using bisection, find the root of a function
// func() known to lie between x1 and x2.  The root, returned as bisect, 
// will be refined  until its accuracy is +/- xtol.
double Math::bisect( double (*func)(double), double x1, double x2, 
	       double xtol, int *count)
{
   //int count;
   double dx, f, fmid, xmid, rtb;

   f    = (*func)(x1);
   fmid = (*func)(x2);
   if( f*fmid >= 0.0 )
   {
      printf("bisect:: Error - x1 and x2 do not bracket the root.\n");
      return 0.0;
   }
   //
   // Orient the search so that f>0 lies at x+dx.
   rtb = f < 0.0 ? (dx = x2-x1, x1) : (dx = x1-x2, x2);
   for( *count=1; *count<=CMAX; (*count)++ )
   {
      fmid = (*func)(xmid = rtb+(dx *= 0.5));        //Bisection loop.
      if( fmid <= 0.0 ) rtb = xmid;
      if( fabs(dx) < xtol || fmid == 0.0 ) return rtb;
   }
   printf("bisect:: Error - Too many bisections.\n");
   return 0.0;                                      //Never get here.
}

//
// Simple sort routine, invented by Donald Shell 1959. It is 0(n^2), but
// the code is simple, and appropriate for small arrays here.
//
// Changed int to long; rsm 5 Sept 06.
void Math::shellSort(long numbers[], int array_size)
{
  int i, j, increment; 
  long temp;

  increment = 3;
  while (increment > 0)
  {
    for (i=0; i < array_size; i++)
    {
      j = i;
      temp = numbers[i];
      while ((j >= increment) && (numbers[j-increment] > temp))
      {
        numbers[j] = numbers[j - increment];
        j = j - increment;
      }
      numbers[j] = temp;
    }
    if (increment/2 != 0)
      increment = increment/2;
    else if (increment == 1)
      increment = 0;
    else
      increment = 1;
  }
}

//
// Simple sort routine, invented by Donald Shell 1959. It is 0(n^2), but
// the code is simple, and appropriate for small arrays here.
//
// Changed to an array of pointers to longs; rsm 8 Nov 13.
void Math::shellSort(long *numbers[], int array_size)
{
  int i, j, increment; 
  long *temp;

  increment = 3;
  while (increment > 0)
  {
    for (i=0; i < array_size; i++)
    {
      j = i;
      temp = numbers[i];
      while ((j >= increment) && (*numbers[j-increment] > *temp))
      {
        numbers[j] = numbers[j - increment];
        j = j - increment;
      }
      numbers[j] = temp;
    }
    if (increment/2 != 0)
      increment = increment/2;
    else if (increment == 1)
      increment = 0;
    else
      increment = 1;
  }
}

//
// Simple sort routine, invented by Donald Shell 1959. It is 0(n^2), but
// the code is simple, and appropriate for small arrays here.
//
// Overloaded with an array of pointers to doubles; rsm 12 Nov 13.
void Math::shellSort(double *numbers[], int array_size)
{
  int i, j, increment; 
  double *temp;

  increment = 3;
  while (increment > 0)
  {
    for (i=0; i < array_size; i++)
    {
      j = i;
      temp = numbers[i];
      while ((j >= increment) && (*numbers[j-increment] > *temp))
      {
        numbers[j] = numbers[j - increment];
        j = j - increment;
      }
      numbers[j] = temp;
    }
    if (increment/2 != 0)
      increment = increment/2;
    else if (increment == 1)
      increment = 0;
    else
      increment = 1;
  }
}
//
// 
void Math::shellSort(beams b[], int array_size)
{
  int i, j, increment; 
  struct beams temp;

  increment = 3;
  while (increment > 0)
  {
    for (i=0; i < array_size; i++)
    {
      j = i;

      temp = b[i];
      
      while ((j >= increment) && (*b[j-increment].range > *temp.range))
      {
        b[j] = b[j - increment];
        j = j - increment;
      }
      b[j] = temp;
    }
    if (increment/2 != 0)
      increment = increment/2;
    else if (increment == 1)
      increment = 0;
    else
      increment = 1;
  }
}

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX
/* (C) Copr. 1986-92 Numerical Recipes Software 7MZ9%"W5:!+). */
