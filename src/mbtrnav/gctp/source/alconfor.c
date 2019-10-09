/*******************************************************************************
NAME                      ALASKA CONFORMAL 

PURPOSE:	Transforms input longitude and latitude to Easting and Northing
		for the Alaska Conformal projection.  The longitude and latitude
		must be in radians.  The Easting and Northing values will
		be returned in meters.

PROGRAMMER              DATE            
----------              ----           
T. Mittan		March, 1993

This function was adapted from the Alaska Conformal projection code
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
static double r_major;		/* major axis			 	*/
static double r_minor;		/* minor axis			 	*/
static double lon_center;	/* Center longitude (projection center) */
static double lat_center;	/* center latitude			*/
static double false_easting;	/* x offset in meters			*/
static double false_northing;	/* y offset in meters			*/
static double acoef[7];
static double bcoef[7];
static double sin_p26;
static double cos_p26;
static double e;
static long n;

/* Initialize the ALASKA CONFORMAL projection
  -----------------------------------------*/
long alconforint(r_maj,r_min,false_east,false_north) 

double r_maj; 			/* Major axis			 	*/
double r_min; 			/* Minor axis			 	*/
double false_east;		/* x offset in meters			*/
double false_north;		/* y offset in meters			*/
{
long i;
double temp;
double es;
double chi;
double esphi;

/* Place parameters in static storage for common use
  -------------------------------------------------*/
r_major = r_maj;
r_minor = r_min;
false_easting = false_east;
false_northing = false_north;
lon_center = -152.0 * D2R;
lat_center = 64.0 * D2R;
n = 6;

es = .006768657997291094;
e = sqrt(es);

         acoef[1]= 0.9945303;  
         acoef[2]= 0.0052083;   
         acoef[3]= 0.0072721;    
         acoef[4]= -0.0151089;    
         acoef[5]= 0.0642675;      
         acoef[6]= 0.3582802;       
         bcoef[1]= 0.0;      
         bcoef[2]= -.0027404; 
         bcoef[3]= 0.0048181;  
         bcoef[4]= -0.1932526;  
         bcoef[5]= -0.1381226;
         bcoef[6]= -0.2884586; 

esphi = e * sin(lat_center);
chi = 2.0 * atan(tan((HALF_PI + lat_center)/2.0) * 
            pow(((1.0 - esphi)/(1.0 + esphi)),(e/2.0))) - HALF_PI;
sincos(chi,&sin_p26,&cos_p26);


/* Report parameters to the user
  -----------------------------*/
ptitle("ALASKA CONFORMAL"); 
radius2(r_major,r_minor);
cenlon(lon_center);
cenlat(lat_center);
offsetp(false_easting,false_northing);
return(OK);
}

/* ALASKA CONFORMAL forward equations--mapping lat,long to x,y
  ----------------------------------------------------------*/
long alconfor(lon, lat, x, y)
double lon;			/* (I) Longitude */
double lat;			/* (I) Latitude */
double *x;			/* (O) X projection coordinate */
double *y;			/* (O) Y projection coordinate */

{
double dlon;
double sinlon,coslon;
double sinphi,cosphi;
double esphi;
double g;
double s;
double xp;
double yp;
double ar;
double ai;
double br;
double bi;
double arn;
double ain;
double chi;
double r;
long j;


/* Forward equations
  -----------------*/
dlon = adjust_lon( lon - lon_center);

/* caluclate x' and y' for Oblique Stereographic Proj for LAT/LONG
----------------------------------------------------------------*/
sincos(dlon,&sinlon,&coslon);
esphi = e * sin(lat);
chi = 2.0 * atan(tan((HALF_PI + lat) / 2.0) * 
            pow(((1.0 - esphi) / (1.0 + esphi)),(e/2.0))) - HALF_PI;
sincos(chi,&sinphi,&cosphi);
g = sin_p26 * sinphi + cos_p26 * cosphi * coslon;
s = 2.0 / (1.0 + g);
xp = s * cosphi * sinlon;
yp = s * (cos_p26 * sinphi - sin_p26 * cosphi * coslon);

/* Use Knuth algorithm for summing complex terms, to convert
   Oblique Stereographic to Modified-Stereographic coord
----------------------------------------------------------*/
r = xp + xp;
s = xp*xp + yp*yp;
ar = acoef[n];
ai = bcoef[n];
br = acoef[n -1];
bi = bcoef[n -1];
for (j =2; j <= n; j++)
   {
   arn = br + r * ar;
   ain = bi + r * ai; 
   if (j < n)
      {
      br = acoef[n - j] - s * ar;
      bi = bcoef[n - j] - s * ai;
      ar = arn;
      ai = ain;
      }
   }
br = -s * ar;
bi = -s * ai;
ar = arn;
ai = ain;
*x = (xp * ar - yp * ai + br) * r_major + false_easting;
*y = (yp * ar + xp * ai + bi) * r_major + false_northing;

return(OK);
}
