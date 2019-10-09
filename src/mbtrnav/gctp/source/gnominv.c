/*******************************************************************************
NAME                            GNOMONIC 

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Gnomonic projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

PROGRAMMER              DATE            
----------              ----           
T. Mittan		Mar, 1993

This function was adapted from the Gnomonic projection code (FORTRAN)
in the General Cartographic Transformation Package software which is
available from the U.S. Geological Survey National Mapping Division.
 
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
static double sin_p13;		/* Sine of the center latitude 		*/
static double cos_p13;		/* Cosine of the center latitude 	*/
static double false_easting;	/* x offset in meters			*/
static double false_northing;	/* y offset in meters			*/

/* Initialize the Gnomonic projection
  ---------------------------------*/
long gnominvint(r, center_long, center_lat,false_east,false_north) 

double r; 			/* (I) Radius of the earth (sphere) 	*/
double center_long;		/* (I) Center longitude 		*/
double center_lat;		/* (I) Center latitude 			*/
double false_east;		/* x offset in meters			*/
double false_north;		/* y offset in meters			*/
{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;
lon_center = center_long;
lat_center = center_lat;
false_easting = false_east;
false_northing = false_north;
sincos(center_lat, &sin_p13, &cos_p13);

/* Report parameters to the user
  -----------------------------*/
ptitle("GNOMONIC"); 
radius(r);
cenlon(center_long);
cenlat(center_lat);
offsetp(false_easting,false_northing);
return(OK);
}

/* Gnomonic inverse equations--mapping x,y to lat/long
  --------------------------------------------------*/
long gnominv(x, y, lon, lat)
double x;			/* (O) X projection coordinate */
double y;			/* (O) Y projection coordinate */
double *lon;			/* (I) Longitude */
double *lat;			/* (I) Latitude */

{
double rh;
double z,sinz,cosz;
double con;


/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
rh = sqrt(x * x + y * y);
z = atan(rh / R);
sincos(z,&sinz,&cosz);
*lon = lon_center;

if (fabs(rh) <= EPSLN)
  {
  *lat = lat_center;
  return(OK);
  }
*lat = asinz(cosz * sin_p13 + (y * sinz * cos_p13) / rh);
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
con = cosz - sin_p13 * sin(*lat);
if ((fabs(con) < EPSLN) && (fabs(x) < EPSLN))
   return(OK);
*lon = adjust_lon(lon_center + atan2((x * sinz * cos_p13), (con * rh)));


return(OK);
}
