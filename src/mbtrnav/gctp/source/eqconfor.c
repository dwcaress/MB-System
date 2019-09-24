/*******************************************************************************
NAME                            EQUIDISTANT CONIC 

PURPOSE:	Transforms input longitude and latitude to Easting and Northing
		for the Equidistant Conic projection.  The longitude and
		latitude must be in radians.  The Easting and Northing values
		will be returned in meters.

PROGRAMMER              DATE
----------              ----
T. Mittan		Mar, 1993

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
static double r_major;		/* major axis 				*/
static double r_minor;		/* minor axis 				*/
static double lon_center;	/* Center longitude (projection center) */
static double lat_origin;	/* center latitude			*/
static double e0,e1,e2,e3;	/* eccentricity constants		*/
static double e,es,esp;		/* eccentricity constants		*/
static double ml0;		/* small value m			*/
static double false_northing;	/* y offset in meters			*/
static double false_easting;	/* x offset in meters			*/
static double ns;
static double g;
static double rh;


/* Initialize the Equidistant Conic projection
  ------------------------------------------*/
long eqconforint(r_maj,r_min,lat1,lat2,center_lon,center_lat,false_east,
	false_north, mode)

double r_maj;			/* major axis			*/
double r_min;			/* minor axis			*/
double lat1;			/* latitude of standard parallel*/
double lat2;			/* latitude of standard parallel*/
double center_lon;		/* center longitude		*/
double center_lat;		/* center latitude		*/
double false_east;		/* x offset in meters		*/
double false_north;		/* y offset in meters		*/
long   mode;			/* which format is present A B	*/
{
double temp;			/* temporary variable		*/
double sinphi,cosphi;		/* sin and cos values		*/
double ms1,ms2;
double ml1,ml2;

/* Place parameters in static storage for common use
  -------------------------------------------------*/
r_major = r_maj;
r_minor = r_min;
lon_center = center_lon;
lat_origin = center_lat;
false_northing = false_north;
false_easting = false_east;

temp = r_minor / r_major;
es = 1.0 - SQUARE(temp);
e = sqrt(es);
e0 = e0fn(es);
e1 = e1fn(es);
e2 = e2fn(es);
e3 = e3fn(es);

sincos(lat1,&sinphi,&cosphi);
ms1 = msfnz(e,sinphi,cosphi);
ml1 = mlfn(e0, e1, e2, e3, lat1);

/* format B
---------*/
if (mode != 0)
   {
   if (fabs(lat1 + lat2) < EPSLN)
      {
      p_error("Standard Parallels on opposite sides of equator","eqcon_for");
      return(81);
      }
   sincos(lat2,&sinphi,&cosphi);
   ms2 = msfnz(e,sinphi,cosphi);
   ml2 = mlfn(e0, e1, e2, e3, lat2);
   if (fabs(lat1 - lat2) >= EPSLN)
      ns = (ms1 - ms2) / (ml2 - ml1);
   else
      ns = sinphi;
   }
else
   ns = sinphi;
g = ml1 + ms1/ns;
ml0 = mlfn(e0, e1, e2, e3, center_lat);
rh = r_major * (g - ml0);
   

/* Report parameters to the user
  -----------------------------*/
if (mode != 0)
   {
   ptitle("EQUIDISTANT CONIC"); 
   radius2(r_major, r_minor);
   stanparl(lat1,lat2);
   cenlonmer(lon_center);
   origin(center_lat);
   offsetp(false_easting,false_northing);
   }
else 
   {
   ptitle("EQUIDISTANT CONIC"); 
   radius2(r_major, r_minor);
   stparl1(lat1);
   cenlonmer(lon_center);
   origin(center_lat);
   offsetp(false_easting,false_northing);
   }

return(OK);
}


/* Equidistant Conic forward equations--mapping lat,long to x,y
  -----------------------------------------------------------*/
long eqconfor(lon, lat, x, y)
double lon;			/* (I) Longitude 		*/
double lat;			/* (I) Latitude 		*/
double *x;			/* (O) X projection coordinate 	*/
double *y;			/* (O) Y projection coordinate 	*/
{
double ml;
double theta;
double rh1;

/* Forward equations
  -----------------*/
ml = mlfn(e0, e1, e2, e3, lat);
rh1 = r_major * (g - ml);
theta = ns * adjust_lon(lon - lon_center);
*x = false_easting  + rh1 * sin(theta);
*y = false_northing + rh - rh1 * cos(theta);

return(OK);
}
