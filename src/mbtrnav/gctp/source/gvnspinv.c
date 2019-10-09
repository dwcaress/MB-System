/*******************************************************************************
NAME                 GENERAL VERTICAL NEAR-SIDE PERSPECTIVE 

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the General Vertical Near-Side Perspective
		projection.  The Easting and Northing must be in meters.
		The longitude and latitude values will be returned in radians.

PROGRAMMER              DATE            
----------              ----           
T. Mittan		Mar, 1993

This function was adapted from the General Vertical Near-side projection
code (FORTRAN) in the General Cartographic Transformation Package software
which is available from the U.S. Geological Survey National Mapping Division.
 
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
long gvnspinvint(r, h,center_long, center_lat,false_east,false_north) 

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
genrpt(h,"Height of Point Above Surface of Sphere:    ");
cenlon(center_long);
cenlat(center_lat);
offsetp(false_easting,false_northing);
return(OK);
}

/* General Vertical Near-Side Perspective inverse equations--mapping 
   x,y to lat/long
  ----------------------------------------------------------------*/
long gvnspinv(x, y, lon, lat)

double x;			/* (O) X projection coordinate */
double y;			/* (O) Y projection coordinate */
double *lon;			/* (I) Longitude */
double *lat;			/* (I) Latitude */

{
double rh;
double r;
double con;
double com;
double z,sinz,cosz;


/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
rh = sqrt(x * x + y * y);
r  = rh / R;
con = p - 1.0;
com = p + 1.0;
if (r > sqrt(con/com))
   {
   p_error("Input data error","gvnsp-for");
   return(155);
   }
sinz = (p - sqrt(1.0 - (r * r * com) / con)) / (con / r + r/con);
z = asinz(sinz);
sincos(z,&sinz,&cosz);
*lon = lon_center;
if (fabs(rh) <= EPSLN)
   {
   *lat = lat_center;
   return(OK);
   }
*lat = asinz(cosz * sin_p15 + ( y * sinz * cos_p15)/rh);
con = fabs(lat_center) - HALF_PI;
if (fabs(con) <= EPSLN)
   {
   if (lat_center >= 0.0)
      {
      *lon = adjust_lon(lon_center + atan2(x, -y));
      return(OK);
      }
   else
      {
      *lon = adjust_lon(lon_center - atan2(-x, y));
      return(OK);
      }
   }
con = cosz - sin_p15 * sin(*lat);
if ((fabs(con) < EPSLN) && (fabs(x) < EPSLN))
   return(OK);
*lon  = adjust_lon(lon_center + atan2((x * sinz * cos_p15), (con * rh)));

return(OK);
}
