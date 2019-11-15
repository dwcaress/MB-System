/*******************************************************************************
NAME                  		SINUSOIDAL

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Sinusoidal projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

PROGRAMMER              DATE            
----------              ----           
D. Steinwand, EROS      May, 1991     

This function was adapted from the Sinusoidal projection code (FORTRAN) in the 
General Cartographic Transformation Package software which is available from 
the U.S. Geological Survey National Mapping Division.
 
ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  "Software Documentation for GCTP General Cartographic Transformation
    Package", U.S. Geological Survey National Mapping Division, May 1982.
*******************************************************************************/
#include "cproj.h"

/* Variables common to all subroutines in this code file
  -----------------------------------------------------*/
static double lon_center;	/* Center longitude (projection center) */
static double R;		/* Radius of the earth (sphere) 	*/
static double false_easting;	/* x offset in meters			*/
static double false_northing;	/* y offset in meters			*/

/* Initialize the Sinusoidal projection
  ------------------------------------*/
long sininvint(r, center_long,false_east,false_north) 
double r; 			/* (I) Radius of the earth (sphere) 	*/
double center_long;		/* (I) Center longitude 		*/
double false_east;		/* x offset in meters			*/
double false_north;		/* y offset in meters			*/
{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;
lon_center = center_long;
false_easting = false_east;
false_northing = false_north;

/* Report parameters to the user
  -----------------------------*/
ptitle("SINUSOIDAL"); 
radius(r);
cenlon(center_long);
offsetp(false_easting,false_northing);
return(OK);
}

/* Sinusoidal inverse equations--mapping x,y to lat,long 
  -----------------------------------------------------*/
long sininv(x, y, lon, lat)
double x;		/* (I) X projection coordinate */
double y;		/* (I) Y projection coordinate */
double *lon;		/* (O) Longitude */
double *lat;		/* (O) Latitude */
{
double temp;		/* Re-used temporary variable */

/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
*lat = y / R;
if (fabs(*lat) > HALF_PI) 
   {
   p_error("Input data error","sinusoidal-inverse");
   return(164);
   }
temp = fabs(*lat) - HALF_PI;
if (fabs(temp) > EPSLN)
   {
   temp = lon_center + x / (R * cos(*lat));
   *lon = adjust_lon(temp);
   }
else *lon = lon_center;
return(OK);
}

