/*******************************************************************************
NAME                            WAGNER IV

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Wagner IV projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

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

/* Initialize the Wagner IV projection
  ------------------------------------*/
long wivinvint(r, center_long,false_east,false_north) 
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
ptitle("WAGNER IV"); 
radius(r);
cenlon(center_long);
offsetp(false_east,false_north);
return(OK);
}

/* Wagner IV inverse equations--mapping x,y to lat,long 
  ----------------------------------------------------*/
long wivinv(x, y, lon, lat)
double x;		/* (I) X projection coordinate */
double y;		/* (I) Y projection coordinate */
double *lon;		/* (O) Longitude */
double *lat;		/* (O) Latitude */
{
double theta;

/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
theta = asin(y /  (1.56548 * R));
*lon = adjust_lon(lon_center + (x / (0.86310 * R * cos(theta))));
*lat = asin((2.0 * theta + sin(2.0 * theta)) / 2.9604205062);
return(OK);
}

