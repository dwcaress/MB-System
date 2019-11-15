/*******************************************************************************
NAME                            POLAR STEREOGRAPHIC 

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the Polar Stereographic projection.  The
		longitude and latitude must be in radians.  The Easting
		and Northing values will be returned in meters.

PROGRAMMER              DATE
----------              ----
T. Mittan		2-26-93

ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include "cproj.h"

/* Variables common to all subroutines in this code file
  -----------------------------------------------------*/
  static double r_major;		/* major axis			*/
  static double r_minor;		/* minor axis			*/
  static double es;			/* eccentricity squared		*/
  static double e;			/* eccentricity			*/
  static double e4;			/* e4 calculated from eccentricity*/
  static double center_lon;		/* center longitude		*/
  static double center_lat;		/* center latitude		*/
  static double fac;			/* sign variable		*/
  static double ind;			/* flag variable		*/
  static double mcs;			/* small m			*/
  static double tcs;			/* small t			*/
  static double false_northing;		/* y offset in meters		*/
  static double false_easting;		/* x offset in meters		*/

/* Initialize the Polar Stereographic projection
  --------------------------------------------*/
long psforint(r_maj,r_min,c_lon,c_lat,false_east,false_north) 

double r_maj;				/* major axis			*/
double r_min;				/* minor axis			*/
double c_lon;				/* center longitude		*/
double c_lat;				/* center latitude		*/
double false_east;			/* x offset in meters		*/
double false_north;			/* y offset in meters		*/

{
double temp;				/* temporary variable		*/
double con1;				/* temporary angle		*/
double sinphi;				/* sin value			*/
double cosphi;				/* cos value			*/

r_major = r_maj;
r_minor = r_min;
false_northing = false_north;
false_easting = false_east;
temp = r_minor / r_major;
es = 1.0 - SQUARE(temp);
e = sqrt(es);
e4 = e4fn(e);
center_lon = c_lon;
center_lat = c_lat;

if (c_lat < 0)
   fac = -1.0;
else
   fac = 1.0;
ind = 0;
if (fabs(fabs(c_lat) - HALF_PI) > EPSLN)
   {
   ind = 1;
   con1 = fac * center_lat; 
   sincos(con1,&sinphi,&cosphi);
   mcs = msfnz(e,sinphi,cosphi);
   tcs = tsfnz(e,con1,sinphi);
   }
/* Report parameters to the user
  -----------------------------*/
ptitle("POLAR STEREOGRAPHIC");
radius2(r_major, r_minor);
cenlon(center_lon);
offsetp(false_east,false_north);

return(OK);
}

/* Polar Stereographic forward equations--mapping lat,long to x,y
  --------------------------------------------------------------*/
long psfor(lon, lat, x, y)
double lon;			/* (I) Longitude 		*/
double lat;			/* (I) Latitude 		*/
double *x;			/* (O) X projection coordinate 	*/
double *y;			/* (O) Y projection coordinate 	*/

{
double con1;			/* adjusted longitude		*/
double con2;			/* adjusted latitude		*/
double rh;			/* height above ellipsoid	*/
double sinphi;			/* sin value			*/
double ts;			/* value of small t		*/

con1 = fac * adjust_lon(lon - center_lon);
con2 = fac * lat;
sinphi = sin(con2);
ts = tsfnz(e,con2,sinphi);
if (ind != 0)
   rh = r_major * mcs * ts / tcs;
else
   rh = 2.0 * r_major * ts / e4;
*x = fac * rh * sin(con1) + false_easting;
*y = -fac * rh * cos(con1) + false_northing;;

return(OK);
}
