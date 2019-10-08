/*******************************************************************************
NAME                            MOLLWEIDE

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Mollweide projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

PROGRAMMER              DATE
----------              ----
D. Steinwand, EROS      May, 1991;  Updated Sept, 1992; Updated Feb, 1993
S. Nelson, EROS		Nov, 1993;  fixed infinite loop at poles

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
long molwinvint(r, center_long, false_east, false_north) 
double r; 			/* (I) Radius of the earth (sphere) */
double center_long;		/* (I) Center longitude */
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

/* Mollweide inverse equations--mapping x,y to lat,long 
  ----------------------------------------------------*/
long molwinv(x, y, lon, lat)
double x;		/* (I) X projection coordinate */
double y;		/* (I) Y projection coordinate */
double *lon;		/* (O) Longitude */
double *lat;		/* (O) Latitude */
{
double theta;
double arg;

/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
arg = y /  (1.4142135623731 * R);

/* Because of division by zero problems, 'arg' can not be 1.0.  Therefore
   a number very close to one is used instead.
   -------------------------------------------------------------------*/
if(fabs(arg) > 0.999999999999) arg=0.999999999999;
theta = asin(arg);
*lon = adjust_lon(lon_center + (x / (0.900316316158 * R * cos(theta))));
if(*lon < (-PI)) *lon= -PI;
if(*lon > PI) *lon= PI;
arg = (2.0 * theta + sin(2.0 * theta)) / PI;
if(fabs(arg) > 1.0)arg=1.0;
*lat = asin(arg);
return(OK);
}
