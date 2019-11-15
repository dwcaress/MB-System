/*******************************************************************************
NAME                  		WAGNER VII

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Wagner VII projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

PROGRAMMER              DATE            
----------              ----           
D. Steinwand, EROS      May, 1991     
S. Nelson, EROS		Dec, 1993	Added function call to "asinz" to
					fix errors at 90 and -90 deg lat.
					This replaced calls to "asin".

This function was implemented with formulas supplied by John P. Snyder.
 
ALGORITHM REFERENCES

1.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections", 
    U.S. Geological Survey Professional Paper 1453 , United State Government 
    Printing Office, Washington D.C., 1989.

2.  Snyder, John P., Personal correspondence, January 1991.
*******************************************************************************/
#include "cproj.h"

/* Variables common to all subroutines in this code file
  -----------------------------------------------------*/
static double lon_center;	/* Center longitude (projection center) */
static double R;		/* Radius of the earth (sphere) */
static double false_easting;    /* x offset                             */
static double false_northing;   /* y offset                             */

/* Initialize the Wagner VII projection
  ------------------------------------*/
long wviiinvint(r, center_long,false_east,false_north) 
double r; 			/* (I) Radius of the earth (sphere) */
double center_long;		/* (I) Center longitude */
double false_east;    		/* x offset                             */
double false_north;   		/* y offset                             */
{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;
lon_center = center_long;
false_easting = false_east;
false_northing = false_north;

/* Report parameters to the user
  -----------------------------*/
ptitle("WAGNER VII"); 
radius(r);
cenlon(center_long);
offsetp(false_easting,false_northing);
return(OK);
}

/* Wagner VII inverse equations--mapping x,y to lat,long 
  -----------------------------------------------------*/
long wviiinv(x, y, lon, lat)

double x;		/* (I) X projection coordinate */
double y;		/* (I) Y projection coordinate */
double *lon;		/* (O) Longitude */
double *lat;		/* (O) Latitude */
{
double temp;		/* Re-used temporary variable */
double t1, t2, p, c;

/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
t1 = x / 2.66723;
t2 = y / 1.24104;
t1 *= t1;
t2 *= t2;
p = sqrt(t1 + t2);
c = 2.0 * asinz(p / (2.0 * R));
*lat = asinz(y * sin(c) / (1.24104 * 0.90631 * p));
*lon = adjust_lon(lon_center + 3.0 * atan2(x * tan(c), 2.66723 * p));
return(OK);
}

