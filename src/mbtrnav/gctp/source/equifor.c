/*******************************************************************************
NAME                             EQUIRECTANGULAR 

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the Equirectangular projection.  The
		longitude and latitude must be in radians.  The Easting
		and Northing values will be returned in meters.

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
static double lon_center;	/* Center longitude (projection center) */
static double lat_origin;	/* center latitude			*/
static double false_northing;	/* y offset in meters			*/
static double false_easting;	/* x offset in meters			*/

/* Initialize the Equirectangular projection
  ----------------------------------------*/
long equiforint(r_maj,center_lon,lat1,false_east,false_north) 

double r_maj;			/* major axis			*/
double center_lon;		/* center longitude		*/
double lat1;			/* latitude of true scale	*/
double false_east;		/* x offset in meters		*/
double false_north;		/* y offset in meters		*/
{

/* Place parameters in static storage for common use
  -------------------------------------------------*/
r_major = r_maj;
lon_center = center_lon;
lat_origin = lat1;
false_northing = false_north;
false_easting = false_east;

/* Report parameters to the user
  -----------------------------*/
ptitle("EQUIRECTANGULAR"); 
radius(r_major);
cenlonmer(lon_center);
origin(lat_origin);
offsetp(false_easting,false_northing);
return(OK);
}


/* Equirectangular forward equations--mapping lat,long to x,y
  ---------------------------------------------------------*/
long equifor(lon, lat, x, y)
double lon;			/* (I) Longitude 		*/
double lat;			/* (I) Latitude 		*/
double *x;			/* (O) X projection coordinate 	*/
double *y;			/* (O) Y projection coordinate 	*/
{
double sinphi, cosphi;	/* sin and cos value				*/
double dlon;		/* delta longitude value			*/
double coslon;		/* cos of longitude				*/
double ksp;		/* scale factor					*/
double g;		

/* Forward equations
  -----------------*/
dlon = adjust_lon(lon - lon_center);
*x = false_easting + r_major * dlon * cos(lat_origin);
*y = false_northing + r_major * lat;
return(OK);
}
