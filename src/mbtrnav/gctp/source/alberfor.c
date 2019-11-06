/*******************************************************************************
NAME                     ALBERS CONICAL EQUAL AREA 

PURPOSE:	Transforms input longitude and latitude to Easting and Northing
		for the Albers Conical Equal Area projection.  The longitude
		and latitude must be in radians.  The Easting and Northing
		values will be returned in meters.

PROGRAMMER              DATE
----------              ----
T. Mittan,       	Feb, 1992

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
  static double r_major;	/* major axis				*/
  static double r_minor;	/* minor axis				*/
  static double c;		/* constant c				*/
  static double e3;		/* eccentricity 			*/
  static double rh;		/* heigth above elipsoid		*/
  static double ns0;		/* ratio between meridians		*/
  static double lon_center;	/* center longitude			*/
  static double false_easting;	/* x offset in meters			*/
  static double false_northing;	/* y offset in meters			*/

/* Initialize the Albers projection
  -------------------------------*/
long alberforint(
double r_maj,			/* major axis				*/
double r_min,			/* minor axis				*/
double lat1,			/* first standard parallel		*/
double lat2,			/* second standard parallel		*/
double lon0,			/* center longitude			*/
double lat0,			/* center lattitude			*/
double false_east,		/* x offset in meters			*/
double false_north)		/* y offset in meters			*/
{
double sin_po,cos_po;		/* sin and cos values			*/
double con;			/* temporary variable			*/
double es,temp;			/* eccentricity squared and temp var	*/
double ms1;			/* small m 1				*/
double ms2;			/* small m 2				*/
double qs0;			/* small q 0				*/
double qs1;			/* small q 1				*/
double qs2;			/* small q 2				*/

false_easting = false_east;
false_northing = false_north;
lon_center = lon0;
if (fabs(lat1 + lat2) < EPSLN)
   {
   p_error("Equal latitudes for St. Parallels on opposite sides of equator",
	  "alber-forinit");
   return(31);
   }
r_major = r_maj;
r_minor = r_min;
temp = r_minor / r_major;
es = 1.0 - SQUARE(temp);
e3 = sqrt(es);

sincos(lat1, &sin_po, &cos_po);
con = sin_po;

ms1 = msfnz(e3,sin_po,cos_po);
qs1 = qsfnz(e3,sin_po,cos_po);

sincos(lat2,&sin_po,&cos_po);

ms2 = msfnz(e3,sin_po,cos_po);
qs2 = qsfnz(e3,sin_po,cos_po);

sincos(lat0,&sin_po,&cos_po);

qs0 = qsfnz(e3,sin_po,cos_po);

if (fabs(lat1 - lat2) > EPSLN)
   ns0 = (ms1 * ms1 - ms2 *ms2)/ (qs2 - qs1);
else
   ns0 = con;
c = ms1 * ms1 + ns0 * qs1;
rh = r_major * sqrt(c - ns0 * qs0)/ns0;

/* Report parameters to the user
  -----------------------------*/
ptitle("ALBERS CONICAL EQUAL-AREA");
radius2(r_major, r_minor);
stanparl(lat1,lat2);
cenlonmer(lon_center);
origin(lat0);
offsetp(false_easting,false_northing);

return(OK);
}

/* Albers Conical Equal Area forward equations--mapping lat,long to x,y
  -------------------------------------------------------------------*/
long alberfor(lon, lat, x, y)
double lon;			/* (I) Longitude 		*/
double lat;			/* (I) Latitude 		*/
double *x;			/* (O) X projection coordinate 	*/
double *y;			/* (O) Y projection coordinate 	*/
{
double sin_phi,cos_phi;		/* sine and cos values		*/
double qs;			/* small q			*/
double theta;			/* angle			*/ 
double rh1;			/* height above ellipsoid	*/

sincos(lat,&sin_phi,&cos_phi);
qs = qsfnz(e3,sin_phi,cos_phi);
rh1 = r_major * sqrt(c - ns0 * qs)/ns0;
theta = ns0 * adjust_lon(lon - lon_center); 
*x = rh1 * sin(theta) + false_easting;
*y = rh - rh1 * cos(theta) + false_northing;

return(OK);
}
