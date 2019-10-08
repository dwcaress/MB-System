/*******************************************************************************
NAME                            MOLLWEIDE

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the MOllweide projection.  The
		longitude and latitude must be in radians.  The Easting
		and Northing values will be returned in meters.

PROGRAMMER              DATE
----------              ----
D. Steinwand, EROS      May, 1991;  Updated Sept, 1992; Updated Feb, 1993
S. Nelson, EDC		Jun, 2993;	Made corrections in precision and
					number of iterations.

ALGORITHM REFERENCES

1.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.

2.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.
*******************************************************************************/
#include "cproj.h"

/* Variables common to all subroutines in this code file
  -----------------------------------------------------*/
static double lon_center;	/* Center longitude (projection center) */
static double R;		/* Radius of the earth (sphere) */
static double false_easting;	/* x offset in meters			*/
static double false_northing;	/* y offset in meters			*/

/* Initialize the Mollweide projection
  ------------------------------------*/
long molwforint(r, center_long,false_east,false_north)
double r; 			/* (I) Radius of the earth (sphere) 	*/
double center_long;		/* (I) Center longitude 		*/
double false_east;		/* x offset in meters			*/
double false_north;		/* y offset in meters			*/
{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
false_easting = false_east;
false_northing = false_north;
R = r;
lon_center = center_long;

/* Report parameters to the user
  -----------------------------*/
ptitle("MOLLWEIDE"); 
radius(r);
cenlon(center_long);
offsetp(false_easting,false_northing);
return(OK);
}

/* Mollweide forward equations--mapping lat,long to x,y
  ----------------------------------------------------*/
long molwfor(lon, lat, x, y)
double lon;			/* (I) Longitude */
double lat;			/* (I) Latitude */
double *x;			/* (O) X projection coordinate */
double *y;			/* (O) Y projection coordinate */
{
double delta_lon;	/* Delta longitude (Given longitude - center */
double theta;
double delta_theta;
double con;
long i;

/* Forward equations
  -----------------*/
delta_lon = adjust_lon(lon - lon_center);
theta = lat;
con = PI * sin(lat);

/* Iterate using the Newton-Raphson method to find theta
  -----------------------------------------------------*/
for (i=0;;i++)
   {
   delta_theta = -(theta + sin(theta) - con)/ (1.0 + cos(theta));
   theta += delta_theta;
   if (fabs(delta_theta) < EPSLN) break;
   if (i >= 50) 
     {
     p_error("Iteration failed to converge","Mollweide-forward");
     return(241);
     }
   }
theta /= 2.0;

/* If the latitude is 90 deg, force the x coordinate to be "0 + false easting"
   this is done here because of precision problems with "cos(theta)"
   --------------------------------------------------------------------------*/
if (PI/2 - fabs(lat) < EPSLN)
   delta_lon =0;
*x = 0.900316316158 * R * delta_lon * cos(theta) + false_easting;
*y = 1.4142135623731 * R * sin(theta) + false_northing;
return(OK);
}
