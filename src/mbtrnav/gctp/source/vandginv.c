/*******************************************************************************
NAME                    VAN DER GRINTEN 

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Van der Grinten projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

PROGRAMMER              DATE            
----------              ----           
T. Mittan		March, 1993

This function was adapted from the Van Der Grinten projection code
(FORTRAN) in the General Cartographic Transformation Package software
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
long vandginvint(r, center_long,false_east,false_north) 

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

/* Van Der Grinten inverse equations--mapping x,y to lat/long
  ---------------------------------------------------------*/
long vandginv(x, y, lon, lat)
double x;			/* (O) X projection coordinate */
double y;			/* (O) Y projection coordinate */
double *lon;			/* (I) Longitude */
double *lat;			/* (I) Latitude */

{
double dlon;
double xx,yy,xys,c1,c2,c3;
double al,asq;
double a1;
double m1;
double con;
double th1;
double d;

/* inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
con = PI * R;
xx = x / con;
yy = y / con;
xys = xx * xx + yy * yy;
c1 = -fabs(yy) * (1.0 + xys);
c2 = c1 - 2.0 * yy * yy + xx * xx;
c3 = -2.0 * c1 + 1.0 + 2.0 * yy * yy + xys * xys;
d = yy * yy / c3 + (2.0 * c2 * c2 * c2 / c3 / c3 / c3 - 9.0 * c1 * c2 / c3 /c3)
    / 27.0;
a1 = (c1 - c2 * c2 / 3.0 / c3) / c3;
m1 = 2.0 * sqrt( -a1 / 3.0);
con = ((3.0 * d) / a1) / m1;
if (fabs(con) > 1.0)
   {
   if (con >= 0.0)
      con = 1.0;
   else
      con = -1.0;
   }
th1 = acos(con) / 3.0;
if (y >= 0)
   *lat = (-m1 * cos(th1 + PI / 3.0) - c2 / 3.0 / c3) * PI;
else
   *lat = -(-m1 * cos(th1 + PI / 3.0) - c2 / 3.0 / c3) * PI;

if (fabs(xx) < EPSLN)
   {
   *lon = lon_center;
   return(OK);
   }
*lon = adjust_lon(lon_center + PI * (xys - 1.0 + sqrt(1.0 + 2.0 * 
		 (xx * xx - yy * yy) + xys * xys)) / 2.0 / xx);

return(OK);
}
