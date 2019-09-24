/*******************************************************************************
NAME                  		WAGNER VII

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the Wagner VII projection.  The
		longitude and latitude must be in radians.  The Easting
		and Northing values will be returned in meters.

PROGRAMMER              DATE            
----------              ----           
D. Steinwand, EROS      May, 1991     

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
static double false_easting;	/* x offset				*/
static double false_northing;	/* y offset				*/

/* Initialize the Wagner VII projection
  ------------------------------------*/
long wviiforint(r, center_long,false_east,false_north) 
double r; 			/* (I) Radius of the earth (sphere) */
double center_long;		/* (I) Center longitude */
double false_east;		/* x offset				*/
double false_north;		/* y offset				*/
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

/* Wagner VII forward equations--mapping lat,long to x,y
  -----------------------------------------------------*/
long wviifor(lon, lat, x, y)
double lon;			/* (I) Longitude */
double lat;			/* (I) Latitude */
double *x;			/* (O) X projection coordinate */
double *y;			/* (O) Y projection coordinate */
{
double delta_lon;	/* Delta longitude (Given longitude - center */
double sin_lon, cos_lon;
double s, c0, c1;

/* Forward equations
  -----------------*/
delta_lon = adjust_lon(lon - lon_center);
sincos((delta_lon/3.0), &sin_lon, &cos_lon);
s = 0.90631 * sin(lat);
c0 = sqrt(1-s*s);
c1 = sqrt(2.0 / (1.0 + c0 * cos_lon));
*x = 2.66723 * R * c0 * c1 * sin_lon + false_easting;
*y = 1.24104 * R * s * c1 + false_northing;
return(OK);
}
