/*******************************************************************************
NAME                            PAKSZ 

PURPOSE:	This function converts a packed DMS angle to seconds.  The
		standard packed DMS format is:

		degrees * 1000000 + minutes * 1000 + seconds

		Example:	ang = 120025045.25 yields
				deg = 120
				min = 25
				sec = 45.25

		The algorithm used for the conversion is as follows:

		1.  The absolute value of the angle is used.

		2.  The degrees are separated out:
		    deg = ang/1000000 	(fractional portion truncated)
	
		3.  The minutes are separated out:
		    min = (ang - deg * 1000000) / 1000 	   (fractional
							portion	truncated)

		4.  The seconds are then computed:
		    sec = ang - deg * 1000000 - min * 1000

		5.  The total angle in seconds is computed:
		    sec = deg * 3600.0 + min * 60.0 + sec

		6.  The sign of sec is set to that of the input angle.
		    

PROGRAMMER              DATE
----------              ----
T. Mittan	      MARCH, 1993


ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Proffesional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include "cproj.h"

/* Convert DMS packed angle into deg 
----------------------------------*/
double paksz(ang,iflg)

double ang;		/* angle which in DMS		*/
long *iflg;		/* error flag number		*/
{
double fac;		/* sign flag			*/
double deg;		/* degree variable		*/
double min;		/* minute variable		*/
double sec;		/* seconds variable		*/
double tmp;		/* temporary variable		*/
long i;			/* temporary variable		*/

*iflg = 0;

if (ang < 0.0)
   fac = -1;
else
   fac = 1;

/* find degrees
-------------*/
sec = fabs(ang);
tmp = 1000000.0;
i = (long) sec/tmp;
if (i > 360)
  {
  p_error("Illegal DMS field","paksz-deg");
  *iflg = 1116;
  return(ERROR);
  }
else
  deg = i;

/* find minutes
-------------*/
sec = sec - deg * tmp;
tmp = 1000;
i = (long) sec / tmp;
if (i > 60)
  {
  p_error("Illegal DMS field","paksz-min");
  *iflg = 1116;
  return(ERROR);
  }
else
  min = i;

/* find seconds
-------------*/
sec = sec - min * tmp;
if (sec > 60)
  {
  p_error("Illegal DMS field","paksz-sec");
  *iflg = 1116;
  return(ERROR);
  }
else
  sec = fac * (deg * 3600.0 + min * 60.0 + sec);
deg = sec / 3600.0;
 
return(deg);
}
