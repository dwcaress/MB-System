/*******************************************************************************
NAME                        GOODE'S HOMOLOSINE

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Goode's Homolosine projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

PROGRAMMER              DATE
----------              ----
D. Steinwand, EROS      May, 1991; Updated Sept, 1992; Updated Feb 1993
S. Nelson, EDC		Jun, 1993	Made changes in precision.

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
long goodinvint(r) 
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

/* Goode`s Homolosine inverse equations--mapping x,y to lat,long 
  -------------------------------------------------------------*/
long goodinv(x, y, lon, lat)
double x;		/* (I) X projection coordinate */
double y;		/* (I) Y projection coordinate */
double *lon;		/* (O) Longitude */
double *lat;		/* (O) Latitude */
{
double arg;
double theta;
double temp;
long region;

/* Inverse equations
  -----------------*/
if (y >= R * 0.710987989993)                 /* if on or above 40 44' 11.8" */
   {
   if (x <= R * -0.698131700798) region = 0; /* If to the left of -40 */
   else region = 2;
   }
else if (y >= 0.0)                           /* Between 0.0 and 40 44' 11.8" */
   {
   if (x <= R * -0.698131700798) region = 1; /* If to the left of -40 */
   else region = 3;
   }
else if (y >= R * -0.710987989993)           /* Between 0.0 & -40 44' 11.8" */
   {
   if (x <= R * -1.74532925199) region = 4;     /* If between -180 and -100 */
   else if (x <= R * -0.349065850399) region = 5; /* If between -100 and -20 */
   else if (x <= R * 1.3962634016) region = 8;  /* If between -20 and 80 */
   else region = 9;                             /* If between 80 and 180 */
   }
else                                            /* Below -40 44' 11.8" */
   {
   if (x <= R * -1.74532925199) region = 6;     /* If between -180 and -100 */
   else if (x <= R * -0.349065850399) region = 7; /* If between -100 and -20 */
   else if (x <= R * 1.3962634016) region = 10; /* If between -20 and 80 */
   else region = 11;                            /* If between 80 and 180 */
   }
x = x - feast[region];

if (region==1||region==3||region==4||region==5||region==8||region==9)
   {
   *lat = y / R;
   if (fabs(*lat) > HALF_PI) 
      {
      p_error("Input data error","goode-inverse");
      return(252);
      }
   temp = fabs(*lat) - HALF_PI;
   if (fabs(temp) > EPSLN)
      {
      temp = lon_center[region] + x / (R * cos(*lat));
      *lon = adjust_lon(temp);
      }
   else *lon = lon_center[region];
   }
else
   {
   arg = (y + 0.0528035274542 * R * sign(y)) /  (1.4142135623731 * R);
   if (fabs(arg) > 1.0) return(IN_BREAK);
   theta = asin(arg);
   *lon = lon_center[region]+(x/(0.900316316158 * R * cos(theta)));
   if(*lon < -(PI + EPSLN)) return(IN_BREAK);
   arg = (2.0 * theta + sin(2.0 * theta)) / PI;
   if (fabs(arg) > 1.0) return(IN_BREAK);
   *lat = asin(arg);
   }
/* because of precision problems, long values of 180 deg and -180 deg
   may be mixed.
   ----------------------------------------------------------------*/
if (((x < 0) && (PI - *lon < EPSLN)) || ((x > 0) && (PI + *lon < EPSLN)))
   *lon = -(*lon);

/* Are we in a interrupted area?  If so, return status code of IN_BREAK.
  ---------------------------------------------------------------------*/
if (region == 0 && (*lon < -(PI + EPSLN) || *lon > -0.698131700798))
							return(IN_BREAK);
if (region == 1 && (*lon < -(PI + EPSLN) || *lon > -0.698131700798))
							return(IN_BREAK);
if (region == 2 && (*lon < -0.698131700798 || *lon > PI + EPSLN))
							return(IN_BREAK);
if (region == 3 && (*lon < -0.698131700798 || *lon > PI + EPSLN))
							return(IN_BREAK);
if (region == 4 && (*lon < -(PI + EPSLN) || *lon > -1.74532925199))
							return(IN_BREAK);
if (region == 5 && (*lon < -1.74532925199 || *lon > -0.349065850399))
							return(IN_BREAK);
if (region == 6 && (*lon < -(PI + EPSLN) || *lon > -1.74532925199))
							return(IN_BREAK);
if (region == 7 && (*lon < -1.74532925199 || *lon > -0.349065850399))
							return(IN_BREAK);
if (region == 8 && (*lon < -0.349065850399 || *lon > 1.3962634016))
							return(IN_BREAK);
if (region == 9 && (*lon < 1.3962634016|| *lon > PI + EPSLN))
							return(IN_BREAK);
if (region ==10 && (*lon < -0.349065850399 || *lon > 1.3962634016))
							return(IN_BREAK);
if (region ==11 && (*lon < 1.3962634016 || *lon > PI + EPSLN))
							return(IN_BREAK);
return(OK);
}

