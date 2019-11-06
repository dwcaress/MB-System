/*******************************************************************************
NAME                      ALASKA CONFORMAL 

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Alaska Conformal projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

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
long alconinvint(r_maj,r_min,false_east,false_north) 

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

temp = r_minor / r_major;
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

/* ALASKA CONFORMAL inverse equations--mapping x,y to lat/long
  ----------------------------------------------------------*/
long alconinv(x, y, lon, lat)
double x;			/* (O) X projection coordinate */
double y;			/* (O) Y projection coordinate */
double *lon;			/* (I) Longitude */
double *lat;			/* (I) Latitude */

{
double dlon;
double sinlon,coslon;
double esphi;
double r;
double s;
double br;
double bi;
double ai;
double ar;
double ci;
double cr;
double di;
double dr;
double arn;
double ain;
double crn;
double cin;
double fxyr;
double fxyi;
double fpxyr;
double fpxyi;
double xp,yp;
double den;
double dxp;
double dyp;
double ds;
double z;
double cosz;
double sinz;
double rh;
double chi;
double dphi;
double phi;
long j;
long nn;

/* Inverse equations
  -----------------*/
x = (x - false_easting) / r_major;
y = (y - false_northing) / r_major;
xp = x;
yp = y;
nn = 0;

/* Use Knuth algorithm for summing complex terms, to convert Modified-
   Stereographic conformal to Oblique Stereographic coordinates.
--------------------------------------------------------------------*/
do
  {
  r = xp + xp;
  s = xp * xp + yp * yp;
  ar = acoef[n];
  ai = bcoef[n];
  br = acoef[n -1];
  bi = bcoef[n - 1];
  cr = (double) (n) * ar;
  ci = (double) (n) * ai;
  dr = (double) (n -1) * br;
  di = (double) (n -1) * bi;

  for (j = 2; j <= n; j++)
      {
      arn = br + r * ar;
      ain = bi + r * ai;
      if (j < n)
        {
        br = acoef[n -j] - s * ar;
        bi = bcoef[n - j] - s * ai;
        ar = arn;
        ai = ain;
        crn = dr  + r * cr;
        cin = di  + r * ci;
        dr = (double) (n - j) * acoef[n -j] - s * cr;
        di = (double) (n - j) * bcoef[n -j] - s * ci;
        cr = crn;
        ci = cin;
        }
      }
  br = -s * ar;
  bi = -s * ai;
  ar = arn;
  ai = ain;
  fxyr = xp * ar - yp * ai + br - x;
  fxyi = yp * ar + xp * ai + bi - y;
  fpxyr = xp * cr - yp * ci + dr;
  fpxyi = yp * cr + xp * ci + ci;
  den = fpxyr * fpxyr + fpxyi * fpxyi;
  dxp = -(fxyr * fpxyr + fxyi * fpxyi) / den;
  dyp = -(fxyi * fpxyr - fxyr * fpxyi) / den;
  xp = xp + dxp;
  yp = yp + dyp;
  ds = fabs(dxp) + fabs(dyp);
  nn++;
  if (nn > 20)
     {
     p_error("Too many iterations in inverse","alcon-inv");
     return(235);
     }
  }
while (ds > EPSLN);

/* convert Oblique Stereographic coordinates to LAT/LONG
------------------------------------------------------*/
rh = sqrt(xp * xp + yp * yp);
z = 2.0 * atan(rh / 2.0);
sincos(z,&sinz,&cosz);
*lon = lon_center;
if (fabs(rh) <= EPSLN)
   {
   *lat = lat_center;
   return(OK);
   }
chi = asinz(cosz * sin_p26 + (yp * sinz * cos_p26) / rh);
nn = 0;
phi = chi;
do
  {
  esphi = e * sin(phi);
  dphi = 2.0 * atan(tan((HALF_PI + chi) / 2.0) * 
         pow(((1.0 + esphi) / (1.0 - esphi)),(e / 2.0))) - HALF_PI - phi;
  phi += dphi;
  nn++;
  if (nn > 20)
     {
     p_error("Too many iterations in inverse","alcon-inv");
     return(236);
     }
  }
while(fabs(dphi) > EPSLN);

*lat = phi;
*lon = adjust_lon (lon_center + atan2((xp * sinz), (rh * cos_p26 * cosz - yp *
                   sin_p26 * sinz)));
     

return(OK);
}
