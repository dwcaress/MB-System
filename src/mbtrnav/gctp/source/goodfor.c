/*******************************************************************************
NAME                        GOODE'S HOMOLOSINE

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the Goode's Homolosine projection.  The
		longitude and latitude must be in radians.  The Easting
		and Northing values will be returned in meters.

PROGRAMMER              DATE
----------              ----
D. Steinwand, EROS      May, 1991; Updated Sept, 1992; Updated Feb 1993
S. Nelson, EDC		Jun, 1993	Make changes in precision and number
					of iterations.

ALGORITHM REFERENCES

1.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.

2.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

3.  Goode, J.P., 1925,  The Homolosine projection:  a new device for 
    portraying the Earth's surface entire:  Assoc. Am. Geographers, Annals, 
    v. 15, p. 119-125
*******************************************************************************/
#include "cproj.h"

/* Variables common to all subroutines in this code file
  -----------------------------------------------------*/
static double R;		/* Radius of the earth (sphere) */
static double lon_center[12];	/* Central meridians, one for each region */
static double feast[12];	/* False easting, one for each region */

/* Initialize the Goode`s Homolosine projection
  --------------------------------------------*/
long goodforint(r) 
double r; 			/* (I) Radius of the earth (sphere) */
{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;

/* Initialize central meridians for each of the 12 regions
  -------------------------------------------------------*/
lon_center[0] = -1.74532925199;		/* -100.0 degrees */
lon_center[1] = -1.74532925199;		/* -100.0 degrees */
lon_center[2] =  0.523598775598;	/*   30.0 degrees */
lon_center[3] =  0.523598775598;	/*   30.0 degrees */
lon_center[4] = -2.79252680319;		/* -160.0 degrees */
lon_center[5] = -1.0471975512;		/*  -60.0 degrees */
lon_center[6] = -2.79252680319;		/* -160.0 degrees */
lon_center[7] = -1.0471975512;		/*  -60.0 degrees */
lon_center[8] =  0.349065850399;	/*   20.0 degrees */
lon_center[9] =  2.44346095279;		/*  140.0 degrees */
lon_center[10] = 0.349065850399;	/*   20.0 degrees */
lon_center[11] = 2.44346095279;		/*  140.0 degrees */

/* Initialize false eastings for each of the 12 regions
  ----------------------------------------------------*/
feast[0] = R * -1.74532925199;
feast[1] = R * -1.74532925199;
feast[2] = R * 0.523598775598;
feast[3] = R * 0.523598775598;
feast[4] = R * -2.79252680319;
feast[5] = R * -1.0471975512;
feast[6] = R * -2.79252680319;
feast[7] = R * -1.0471975512;
feast[8] = R * 0.349065850399;
feast[9] = R * 2.44346095279;
feast[10] = R * 0.349065850399;
feast[11] = R * 2.44346095279;

/* Report parameters to the user
  -----------------------------*/
ptitle("GOODE'S HOMOLOSINE EQUAL-AREA"); 
radius(r);
return(OK);
}

/* Goode`s Homolosine forward equations--mapping lat,long to x,y
  -------------------------------------------------------------*/
long goodfor(lon, lat, x, y)
double lon;			/* (I) Longitude */
double lat;			/* (I) Latitude */
double *x;			/* (O) X projection coordinate */
double *y;			/* (O) Y projection coordinate */
{
double delta_lon;	/* Delta longitude (Given longitude - center */
double theta;
double delta_theta;
double constant;
long i;
long region;

/* Forward equations
  -----------------*/
if (lat >= 0.710987989993)	             /* if on or above 40 44' 11.8" */
   {
   if (lon <= -0.698131700798) region = 0;   /* If to the left of -40 */
   else region = 2;
   }
else if (lat >= 0.0)			     /* Between 0.0 and 40 44' 11.8" */
   {
   if (lon <= -0.698131700798) region = 1;   /* If to the left of -40 */
   else region = 3;
   }
else if (lat >= -0.710987989993)   	     /* Between 0.0 & -40 44' 11.8" */
   {
   if (lon <= -1.74532925199) region = 4;  	/* If between -180 and -100 */
   else if (lon <= -0.349065850399) region = 5;	/* If between -100 and -20 */
   else if (lon <= 1.3962634016) region = 8;	/* If between -20 and 80 */
   else region = 9;				/* If between 80 and 180 */
   }
else						/* Below -40 44' */
   {
   if (lon <= -1.74532925199) region = 6;       /* If between -180 and -100 */
   else if (lon <= -0.349065850399) region = 7;     /* If between -100 and -20 */
   else if (lon <= 1.3962634016) region = 10;   /* If between -20 and 80 */
   else region = 11;                            /* If between 80 and 180 */
   }

if (region==1||region==3||region==4||region==5||region==8||region==9)
   {
   delta_lon = adjust_lon(lon - lon_center[region]);
   *x = feast[region] + R * delta_lon * cos(lat);
   *y = R * lat;
   }
else
   {
   delta_lon = adjust_lon(lon - lon_center[region]);
   theta = lat;
   constant = PI * sin(lat);

/* Iterate using the Newton-Raphson method to find theta
  -----------------------------------------------------*/
   for (i=0;;i++)
      {
      delta_theta = -(theta + sin(theta) - constant) / (1.0 + cos(theta));
      theta += delta_theta;
      if (fabs(delta_theta) < EPSLN) break;
      if (i >= 50) 
         {
         p_error("Iteration failed to converge","goode-forward");
	 return(251);
         }
      }
   theta /= 2.0;

   /* If the latitude is 90 deg, force the x coordinate to be
      "0 + false easting" this is done here because of precision problems
      with "cos(theta)"
      ------------------------------------------------------------------*/
   if (PI / 2 - fabs(lat) < EPSLN)
      delta_lon = 0;
   *x = feast[region] + 0.900316316158 * R * delta_lon * cos(theta);
   *y = R * (1.4142135623731 * sin(theta) - 0.0528035274542 * sign(lat));
   }

return(OK);
}
