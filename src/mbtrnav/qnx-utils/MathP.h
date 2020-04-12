/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : Math.h                                                        */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _MATHP_H
#define _MATHP_H

/*
$Log$
Revision 1.1.2.1  2018/11/20 20:03:21  henthorn
QNX-additions is a library of routines for use outside of QNX - stripped down version of DataLog, TimeP, and NavUtils. auv-qnx dependencies removed. Link to libqnx.a.

Revision 1.4.2.2  2016/10/05 18:24:23  rob
Added both uniform and Gaussian noise generators.  Added a unit test, testGasDev.

Revision 1.4.2.1  2016/02/02 22:49:56  rob
Overloaed shellSort to also sort an array of structs.

Revision 1.4  2013/12/05 19:18:29  rob
Overloaded the shell sort routine to take a pointer to a long, and a pointer
to a double, as arguments.  testShell is a unit test.

I'll probably replace these variations with a single template version the
next time around.

Revision 1.3  2006/09/06 21:29:23  rob
Added a Shell sort routine.

Revision 1.2  2004/09/15 23:28:06  rob
Added a root-solver using the bisect algorithm from Numerical Recipes.

Revision 1.1  2001/06/02 21:33:54  hthomas
various changes; remove stale entries in root makefile, fixed file naming to allow for serving from windows, and fixed makefiles to avoid file linking/unlinking problem in makedepend when talking through samba to windows boxes

Revision 1.5  2000/08/21 20:54:20  oreilly
Added round() method

Revision 1.4  2000/05/02 21:08:57  rob
Added RpmToRadps conversion constant.

Revision 1.3  2000/02/07 20:59:22  pean
Added revised header to include Proprietary Information

Revision 1.2  2000/01/19 06:13:44  oreilly
Debug vers of matherr()

Revision 1.1  1999/12/07 23:03:07  pean
Created utils subdirectory and moved source files here to help manage sources

Revision 1.6  1999/12/04 01:06:38  rob
Added "static" to the declarations of the new functions.

Revision 1.5  1999/12/03 23:21:32  rob
Moved some functions from DynamicControl here.

Revision 1.4  1999/11/19 19:19:07  oreilly
*** empty log message ***

Revision 1.3  1999/10/12 23:17:48  oreilly
*** empty log message ***

Revision 1.2  1999/09/15 23:03:42  pean
Added kvh compass interface module

Revision 1.1  1999/09/09 18:39:37  oreilly
*** empty log message ***

Revision 1.3  1997/04/09 16:34:48  oreilly
include "trig.h"

 * Revision 1.2  96/10/28  09:00:15  09:00:15  oreilly (Thomas C. O'Reilly)
 * Math utilities
 * 
 * Revision 1.1  96/08/14  13:30:39  13:30:39  oreilly (Thomas C. O'Reilly)
 * Initial revision
 * 
*/


#include <math.h>
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#ifndef PI
#define PI          3.14159265358979323846
#endif


// Indices for vectors
//#define X 0
//#define Y 1
//#define Z 2

typedef double Radians;

//
// For shell short:
struct beams { float *range; short num; };

/*
CLASS 
Math

DESCRIPTION
Various static utility methods, including angular conversions.

AUTHOR
Tom O'Reilly
*/
class Math
{
  public:
  
  ///////////////////////////////////////////////////////////////////
  // 2 times pi
  static const double TwoPi;

  ///////////////////////////////////////////////////////////////////
  // Radians per degree
  static const double RadsPerDeg;

  ///////////////////////////////////////////////////////////////////
  // Degrees per radian
  static const double DegsPerRad;

  ///////////////////////////////////////////////////////////////////
  // Convert rpm to radians per second
  static const double RpmToRadps;
  
  ///////////////////////////////////////////////////////////////////
  // Normalize angle so it is in range 0 to 2PI
  static void zeroToTwoPi(Radians *angle);

  ///////////////////////////////////////////////////////////////////
  // Normalize angle so it is in range -PI to PI
  static void minusPiToPi(Radians *pitch);

  ///////////////////////////////////////////////////////////////////
  // Angular separation in clockwise direction
  static Radians angularSeparation(Radians start, Radians stop);

  ///////////////////////////////////////////////////////////////////
  // Convert and angle in degs to an angle in radians
  static Radians degToRad(double deg);

  ///////////////////////////////////////////////////////////////////
  // Convert and angle in radians to an angle in degrees
  static double radToDeg(Radians rads);

  ///////////////////////////////////////////////////////////////////
  // Simple limit function:
  static double limit( double value, double max, double min );

  ///////////////////////////////////////////////////////////////////
  // sgn function:
  static double sgn( double x );

  ///////////////////////////////////////////////////////////////////
  // modulo Pi function:
  static double modPi( double angle );
  
  ///////////////////////////////////////////////////////////////////
  // Round to nearest integer (QNX has no rint()!!!)
  static int round(double x);

  //
  // Bisection root solver from the old Numerical Recipes book.
  // Inputs:
  //    func(x) - function pointer to a scalar function that passes
  //              through zero in the interval [x1, x2].
  //    xtol    - this routine terminates when |x0 - xz| <= tol, 
  //              where f(xz) = 0 analytically.
  // Output:  
  //    x0      - numerical solution; f(x0) approx = 0.
  //    count   - number of iterations.
  static double bisect( double (*func)(double), double x1, double x2, 
		 double xtol, int *count);
  //
  // Simple sort routine, invented by Donald Shell 1959. It is 0(n^2), but
  // the code is simple, and appropriate for small arrays here.
  //
  // Changed int to long; rsm 5 Sept 06.
  static void shellSort(long numbers[], int array_size);
  //
  // Also do it for pointers to longs; rsm 8 Nov 13.
  static void shellSort(long *numbers[], int array_size);
  //
  // Also do it for pointers to doubles; rsm 12 Nov 13.
  static void shellSort(double *numbers[], int array_size);
  //
  // Also do it a struct; rsm 28 Jan 2016:
  static void shellSort(struct beams b[], int array_size);

};

#endif
