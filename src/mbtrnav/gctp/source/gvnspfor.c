/*******************************************************************************
NAME                 GENERAL VERTICAL NEAR-SIDE PERSPECTIVE 

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the General Vertical Near-Side Perspective
		projection.  The longitude and latitude must be in
		radians.  The Easting and Northing values will be
		returned in meters.

PROGRAMMER              DATE            
----------              ----           
T. Mittan		Mar, 1993

This function was adapted from the General Vertical Near-Side Perspective
projection code (FORTRAN) in the General Cartographic Transformation Package
software which is available from the U.S. Geological Survey National Mapping
Division.
 
ALGORITHM REFERENCES

1.  "New Equal-Area Map Projections for Noncircular Regions", John P. Snyder,
    The American Cartographer, Vol 15, No. 4, October 1988, pp. 341-355.

2.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

3.  "Software Documentation for GCTP General Cartographic Transformation
    Package", U.S. Geological Survey National Mapping Division, May 1982.
*******************************************************************************/
#include "cproj.h"

/* Variables common to all subroutines in this code file
  -----------------------------------------------------*/
static double lon_center;	/* Center longitude (projection center) */
static double lat_center;	/* Center latitude (projection center) 	*/
static double R;		/* Radius of the earth (sphere)	 	*/
static double p;		/* Height above sphere			*/
static double sin_p15;		/* Sine of the center latitude 		*/
static double cos_p15;		/* Cosine of the center latitude 	*/
static double false_easting;	/* x offset in meters			*/
static double false_northing;	/* y offset in meters			*/

/* Initialize the General Vertical Near-Side Perspective projection
  ---------------------------------------------------------------*/
long gvnspforint(r, h,center_long, center_lat,false_east,false_north) 

double r; 			/* (I) Radius of the earth (sphere) 	*/
double h;			/* height above sphere			*/
double center_long;		/* (I) Center longitude 		*/
double center_lat;		/* (I) Center latitude 			*/
double false_east;		/* x offset in meters			*/
double false_north;		/* y offset in meters			*/
{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;
p = 1.0 + h / R;
lon_center = center_long;
lat_center = center_lat;
false_easting = false_east;
false_northing = false_north;
sincos(center_lat, &sin_p15, &cos_p15);

/* Report parameters to the user
  -----------------------------*/
ptitle("GENERAL VERTICAL NEAR-SIDE PERSPECTIVE"); 
radius(r);
genrpt(h,"Height of Point Above Surface of Sphere:   ");
cenlon(center_long);
cenlat(center_lat);
offsetp(false_easting,false_northing);
return(OK);
}

/* General Vertical Near-Side Perspective forward equations--mapping 
   lat,long to x,y
  ----------------------------------------------------------------*/
long gvnspfor(lon, lat, x, y)
double lon;			/* (I) Longitude */
double lat;			/* (I) Latitude */
double *x;			/* (O) X projection coordinate */
double *y;			/* (O) Y projection coordinate */

{
double dlon;
double sinphi,cosphi;
double coslon;
double g;
double ksp;

/* Forward equations
  -----------------*/
dlon = adjust_lon(lon - lon_center);
sincos(lat,&sinphi,&cosphi);
coslon = cos(dlon);
g = sin_p15 * sinphi + cos_p15 * cosphi * coslon;
if (g < (1.0/ p))
   {
   p_error("Point cannot be projected","gvnsp-for");
   return(153);
   }
ksp = (p - 1.0)/(p - g);
*x = false_easting + R * ksp * cosphi * sin(dlon);
*y = false_northing + R * ksp * (cos_p15 * sinphi - sin_p15 * cosphi * coslon);

return(OK);
}
