/*******************************************************************************
NAME                            UNTFZ 

PURPOSE:	This function determines the convergence factor between the
		input unit type and the output unit type.  Valid types include:

		0 = Radians		3 = Seconds of arc
		1 = U.S. feet		4 = Degrees of arc
		2 = Meters		5 = International feet

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
#include "cproj.h"
#include "proj.h"

static double factors[6][6] = {
	{1.0, 0.0, 0.0, 206264.8062470963, 57.29577951308231, 0.0},
	{0.0, 1.0, .3048006096012192, 0.0, 0.0, 1.000002000004},
	{0.0, 3.280833333333333, 1.0, 0.0, 0.0, 3.280839895013124},
	{.484813681109536e-5, 0.0, 0.0, 1.0, .27777777777778e-3, 0.0}, 
	{.0174532925199433, 0.0, 0.0, 3600, 1.0, 0.0},
	{0.0, .999998, .3048, 0.0, 0.0, 1.0}};

/* Convert DMS packed angle into deg 
----------------------------------*/
long untfz(inunit,outunit,factor)

long inunit;
long outunit;
double *factor;
{
if ((outunit >= 0) && (outunit <= MAXUNIT) && (inunit >= 0)
						&& (inunit <= MAXUNIT))
   {
   *factor = factors[inunit][outunit];

   /* Angle units can not be converted to length units
     ------------------------------------------------*/
   if (*factor == 0.0)
       {
       p_error("Incompatable unit codes","untfz-code");
       return(1101);
       }
   }
else
  {
  p_error("Illegal source or target unit code","untfz-unit");
  return(5);
  }

return(OK);
}
