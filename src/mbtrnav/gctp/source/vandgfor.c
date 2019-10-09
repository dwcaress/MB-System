/*******************************************************************************
NAME                    VAN DER GRINTEN 

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the Van der Grinten projection.  The
		longitude and latitude must be in radians.  The Easting
		and Northing values will be returned in meters.

PROGRAMMER              DATE            
----------              ----           
T. Mittan		March, 1993

This function was adapted from the Lambert Azimuthal Equal Area projection
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
static double R;		/* Radius of the earth (sphere)	 	*/
static double false_easting;	/* x offset in meters			*/
static double false_northing;	/* y offset in meters			*/

/* Initialize the Van Der Grinten projection
  ----------------------------------------*/
long vandgforint(r, center_long,false_east,false_north) 

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
ptitle("VAN DER GRINTEN"); 
radius(r);
cenlon(center_long);
offsetp(false_easting,false_northing);
return(OK);
}

/* Van Der Grinten forward equations--mapping lat,long to x,y
  ---------------------------------------------------------*/
long vandgfor(lon, lat, x, y)
double lon;			/* (I) Longitude */
double lat;			/* (I) Latitude */
double *x;			/* (O) X projection coordinate */
double *y;			/* (O) Y projection coordinate */

{
double dlon;
double theta;
double al,asq;
double g,gsq;
double m,msq;
double con;
double costh,sinth;
double temp;

/* Forward equations
  -----------------*/
dlon = adjust_lon(lon  - lon_center);

if (fabs(lat) <= EPSLN)
   {
   *x = false_easting  + R * dlon;
   *y = false_northing;
   return (OK);
   }
theta = asinz(2.0 * fabs(lat / PI));
if ((fabs(dlon) <= EPSLN) || (fabs(fabs(lat) - HALF_PI) <= EPSLN))
   {
   *x = false_easting;
   if (lat >= 0)
      *y = false_northing + PI * R * tan(.5 * theta);
   else
      *y = false_northing + PI * R * -tan(.5 * theta);
   return(OK);
   }
al = .5 * fabs((PI / dlon) - (dlon / PI));
asq = al * al;
sincos(theta,&sinth,&costh);
g = costh / (sinth + costh - 1.0);
gsq = g * g;
m = g * (2.0 / sinth - 1.0);
msq = m * m;
con = PI * R * (al * (g - msq) + sqrt(asq * (g - msq) * (g - msq) - (msq + asq)
      * (gsq - msq))) / (msq + asq);
if (dlon < 0)
   con = -con;
*x = false_easting + con;
con = fabs(con / (PI * R));
if (lat >= 0)
   *y = false_northing + PI * R * sqrt(1.0 - con * con - 2.0 * al * con);
else
   *y = false_northing - PI * R * sqrt(1.0 - con * con - 2.0 * al * con);

return(OK);
}
