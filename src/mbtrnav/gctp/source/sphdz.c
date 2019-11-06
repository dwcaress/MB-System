/*******************************************************************************
NAME                           SPHDZ 

PURPOSE:	This function assigns values to the semimajor axis, semiminor
		axis, and radius of sphere.  If the spheroid code is negative,
		the first two values in the parameter array (parm) are used
		to define the values as follows:

		--If parm[0] is a non-zero value and parm[1] is greater than
		  one, the semimajor axis and radius are set to parm[0] and
		  the semiminor axis is set to parm[1]. 

		--If parm[0] is nonzero and parm[1] is greater than zero but
		  less than or equal to one, the semimajor axis and radius
		  are set to parm[0] and the semiminor axis is computed
		  from the eccentricity squared value parm[1].  This
		  algorithm is given below.

		--If parm[0] is nonzero and parm[1] is equal to zero, the
		  semimajor axis, radius, and semiminor axis are set to
		  parm[0].

		--If parm[0] equals zero and parm[1] is greater than zero,
		  the default Clarke 1866 is used to assign values to the
		  semimajor axis, radius and semiminor axis.

		--If parm[0] and parm[1] equals zero, the semimajor axis
		  and radius are set to 6370997.0 (This value is represented
	 	  as the last value in the spheroid code array) and the
		  semiminor axis is set to zero.

		if a spheroid code is zero or greater, the semimajor and
		semiminor axis are defined by the spheroid code, listed below
		in the array assignment, and the radius is set to 6370997.0.
		If the spheroid code is greater than SPHDCT the default
		spheroid, Clarke 1866, is used to define the semimajor
		and semiminor axis and radius is set to 6370997.0.

		The algorithm to define the semiminor axis using the
		eccentricity squared value is as follows:

		      semiminor = sqrt(1.0 - ES) * semimajor   where
		      ES = eccentricity squared

		

PROGRAMMER              DATE
----------              ----
T. Mittan	      MARCH, 1993

ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/

/* Assign the radius value to the radius argument if a spheroid is assigned */
#define RADVAL 19

#include "cproj.h"
#include "proj.h"

	/* Semi-Major axis of supported Spheroids */
static double major[SPHDCT] = {
		6378206.4,		/* 0: Clarke 1866 (default) */
		6378249.145,		/* 1: Clarke 1880 */
		6377397.155,		/* 2: Bessel */
		6378157.5,		/* 3: International 1967 */
		6378388.0,		/* 4: International 1909 */
		6378135.0,		/* 5: WGS 72 */
		6377276.3452,		/* 6: Everest */
		6378145.0,		/* 7: WGS 66 */
                6378137.0,		/* 8: GRS 1980 */
		6377563.396,		/* 9: Airy */
		6377304.063,		/* 10: Modified Everest */
		6377340.189,		/* 11: Modified Airy */
                6378137.0,		/* 12: WGS 84 */
		6378155.0,		/* 13: Southeast Asia */
		6378160.0,		/* 14: Australian National */
		6378245.0,		/* 15: Krassovsky */
                6378270.0,		/* 16: Hough */
		6378166.0,		/* 17: Mercury 1960 */
		6378150.0,		/* 18: Modified Mercury 1968 */
		6370997.0,		/* 19: Sphere of Radius 6370997 meters*/
		6377483.865,		/* 20: Bessel 1841(Namibia) */
		6377298.556,		/* 21: Everest (Sabah & Sarawak) */
		6377301.243,		/* 22: Everest (India 1956) */
		6377295.664,		/* 23: Everest (Malaysia 1969) */
		6377304.063,		/* 24: Everest (Malay & Singapr 1948)*/
		6377309.613,		/* 25: Everest (Pakistan) */
		6378388.0,		/* 26: Hayford */
		6378200.0,		/* 27: Helmert 1906 */
		6378160.000,		/* 28: Indonesian 1974 */
		6378160.0,		/* 29: South American 1969 */
		6378165.0};		/* 30: WGS 60 */

	/* Semi-Minor axis of supported Spheroids */
static double minor[SPHDCT] = {
		6356583.8,		/* 0: Clarke 1866 (default) */
		6356514.86955,		/* 1: Clarke 1880 */
		6356078.96284,		/* 2: Bessel */
		6356772.2,		/* 3: International 1967 */
                6356911.94613,		/* 4: International 1909 */
		6356750.519915,		/* 5: WGS 72 */
		6356075.4133,		/* 6: Everest */
                6356759.769356,		/* 7: WGS 66 */
		6356752.31414,		/* 8: GRS 1980 */
		6356256.91,		/* 9: Airy */
                6356103.039,		/* 10: Modified Everest */
		6356034.448,		/* 11: Modified Airy */
		6356752.314245,		/* 12: WGS 84 */
                6356773.3205,		/* 13: Southeast Asia */
		6356774.719,		/* 14: Australian National */
		6356863.0188,		/* 15: Krassovsky */
                6356794.343479,		/* 16: Hough */
		6356784.283666,		/* 17: Mercury 1960 */
		6356768.337303,		/* 18: Modified Mercury 1968 */
                6370997.0,		/* 19: Sphere of Radius 6370997 meters*/
		6356165.382966,		/* 20: Bessel 1841(Namibia) */
		6356097.571445,		/* 21: Everest (Sabah & Sarawak) */
		6356100.228368,		/* 22: Everest (India 1956) */
		6356094.667915,		/* 23: Everest (Malaysia 1969) */
		6356103.038993,		/* 24: Everest (Malay & Singapr 1948)*/
		6356108.570542,		/* 25: Everest (Pakistan) */
		6356911.946128,		/* 26: Hayford */
		6356818.169,		/* 27: Helmert 1906 */
		6356774.504086,		/* 28: Indonesian 1974 */
		6356774.719,		/* 29: South American 1969 */
		6356783.287};		/* 30: WGS 60 */


/* Finds the correct ellipsoid axis
---------------------------------*/

void sphdz(isph,parm,r_major,r_minor,radius)


long isph;		/* spheroid code number				*/
double *parm;		/* projection parameters			*/
double *r_major;	/* major axis					*/
double *r_minor;	/* minor axis					*/
double *radius;		/* radius					*/
{

double t_major;		/* temporary major axis				*/
double t_minor;		/* temporary minor axis				*/
long jsph;		/* spheroid code number				*/

/* if the spheroid code is a negative number, get the semi-major and semi-minor
   axis from the projection array
  --------------------------------------------------------------------------*/
if (isph < 0)
   {
   t_major = fabs(parm[0]);
   t_minor = fabs(parm[1]);
   
   if (t_major  > 0.0) 
     {
     /* The semimajor axis and the semiminor axis are in the array, assign
	them directly
      --------------------------------------------------------------------*/
     if (t_minor > 1.0)
        {
        *r_major = t_major;
  	*r_minor = t_minor;
	*radius = t_major;
        } 
     /* The semimajor axis and the eccentricity squared values are in the array,
	therefore, the semiminor axis is computed from the eccentricity
	squared value parm[1]
      ----------------------------------------------------------------------*/
     else
     if (t_minor > 0.0)
        {
        *r_major = t_major;
	*radius = t_major;
        *r_minor = (sqrt(1.0 - t_minor)) * t_major; 
        }
     /* The semiminor axis is zero or less, assign the semimajor axis to
	the semiminor axis.
       -----------------------------------------------------------------*/
     else
        {
        *r_major = t_major;
	*radius = t_major;
        *r_minor = t_major;
        }
     }
   /* The sphroid code is to be used to assign the axis
    -------------------------------------------------*/
   else
   if (t_minor > 0.0)	/* t_major = 0 */

     /* Assign Clarke 1866 semi-major and semi-minor axis
     ---------------------------------------------------*/
     {
     *r_major = major[0];
     *radius = major[0];
     *r_minor = minor[0];
     }
   else
     /* Assign Spheroid radius to semi-major and semi-minor axis
     ---------------------------------------------------------*/
     {
     *r_major = major[RADVAL];
     *radius = major[RADVAL];
     *r_minor = major[RADVAL];
     }
  }
/* Use the spheroid code to assign the semi-major and semi-minor axis
   -----------------------------------------------------------------*/
else		/* isph >= 0 */
  {
  jsph = isph;

  /* The spheroid code is out of range, assign Clarke 1866
   ------------------------------------------------------*/
  if (jsph > (SPHDCT - 1))
     {
     p_error("Invalid spheroid selection","INFORMATIONAL");
     p_error("Reset to 0","INFORMATIONAL");
     jsph = 0;
     }
  /* Assign the radius argument to the standard radius value
    -------------------------------------------------------*/
  *r_major = major[jsph];
  *r_minor = minor[jsph];
  *radius = major[RADVAL];
  }
return;
}
