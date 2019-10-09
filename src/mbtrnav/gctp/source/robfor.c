/*******************************************************************************
NAME                            ROBINSON 

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the Robinson projection.  The
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
static double pr[21];
static double xlr[21];

/* Initialize the ROBINSON projection
  ---------------------------------*/
long robforint(r, center_long,false_east,false_north) 

double r; 			/* (I) Radius of the earth (sphere) 	*/
double center_long;		/* (I) Center longitude 		*/
double false_east;		/* x offset in meters			*/
double false_north;		/* y offset in meters			*/
{
long i;

/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;
lon_center = center_long;
false_easting = false_east;
false_northing = false_north;

         pr[1]= -0.062; 
         xlr[1]=0.9986; 
         pr[2]=0.0; 
         xlr[2]=1.0; 
         pr[3]=0.062; 
         xlr[3]=0.9986;
         pr[4]=0.124;   
         xlr[4]=0.9954;  
         pr[5]=0.186;
         xlr[5]=0.99; 
         pr[6]=0.248;  
         xlr[6]=0.9822; 
         pr[7]=0.31;     
         xlr[7]=0.973;    
         pr[8]=0.372;      
         xlr[8]=0.96;
         pr[9]=0.434; 
         xlr[9]=0.9427;
         pr[10]=0.4958;
         xlr[10]=0.9216;
         pr[11]=0.5571;  
         xlr[11]=0.8962;  
         pr[12]=0.6176;
         xlr[12]=0.8679;
         pr[13]=0.6769; 
         xlr[13]=0.835;  
         pr[14]=0.7346;
         xlr[14]=0.7986;
         pr[15]=0.7903;  
         xlr[15]=0.7597;  
         pr[16]=0.8435;
         xlr[16]=0.7186;
         pr[17]=0.8936; 
         xlr[17]=0.6732; 
         pr[18]=0.9394; 
         xlr[18]=0.6213;
         pr[19]=0.9761;  
         xlr[19]=0.5722;  
         pr[20]=1.0;  
         xlr[20]=0.5322;

         for (i = 0; i < 21; i++)
            xlr[i] *= 0.9858;   

/* Report parameters to the user
  -----------------------------*/
ptitle("ROBINSON"); 
radius(r);
cenlon(center_long);
offsetp(false_easting,false_northing);
return(OK);
}

/* Robinson forward equations--mapping lat,long to x,y
  ------------------------------------------------------------*/
long robfor(lon, lat, x, y)
double lon;			/* (I) Longitude */
double lat;			/* (I) Latitude */
double *x;			/* (O) X projection coordinate */
double *y;			/* (O) Y projection coordinate */

{
double dlon;
double p2;
long ip1;

/* Forward equations
  -----------------*/
dlon = adjust_lon(lon - lon_center);
p2 = fabs(lat / 5.0 / .01745329252);
ip1 = (long) (p2 - EPSLN);

/* Stirling's interpolation formula (using 2nd Diff.)
---------------------------------------------------*/
p2 -= (double) ip1;
*x = R * (xlr[ip1 + 2] + p2 * (xlr[ip1 + 3] - xlr[ip1 + 1]) / 2.0 +
          p2 * p2 * (xlr[ip1 + 3] - 2.0 * xlr[ip1 + 2] + xlr[ip1 + 1])/2.0) * 
          dlon + false_easting;

if (lat >= 0)
   *y = R * (pr[ip1 + 2] + p2 * (pr[ip1 + 3] - pr[ip1 +1]) / 2.0 + p2 * p2 *
            (pr[ip1 + 3] - 2.0 * pr[ip1 + 2] + pr[ip1 + 1]) / 2.0) * PI / 2.0 +
	    false_northing;
else
   *y = -R * (pr[ip1 + 2] + p2 * (pr[ip1 + 3] - pr[ip1 +1]) / 2.0 + p2 * p2 *
             (pr[ip1 + 3] - 2.0 * pr[ip1 + 2] + pr[ip1 + 1]) / 2.0) * PI / 2.0 +
	     false_northing;

return(OK);
}
