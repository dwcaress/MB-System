/*******************************************************************************
NAME                            LAMBERT CONFORMAL CONIC 

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Lambert Conformal Conic projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

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
  static double r_major;                /* major axis                   */
  static double r_minor;                /* minor axis                   */
  static double es;                     /* eccentricity squared         */
  static double e;                      /* eccentricity                 */
  static double center_lon;             /* center longituted            */
  static double center_lat;             /* cetner latitude              */
  static double ns;                     /* ratio of angle between meridian*/
  static double f0;                     /* flattening of ellipsoid      */
  static double rh;                     /* height above ellipsoid       */
  static double false_easting;          /* x offset in meters           */
  static double false_northing;		/* y offset in meters		*/

/* Initialize the Lambert Conformal Conic projection
  ------------------------------------------------*/
long lamccinvint(r_maj,r_min,lat1,lat2,c_lon,c_lat,false_east,false_north)

double r_maj;				/* major axis			*/
double r_min;				/* minor axis			*/
double lat1;				/* first standard parallel	*/
double lat2;				/* second standard parallel	*/
double c_lon;				/* center longitude		*/
double c_lat;				/* center latitude		*/
double false_east;			/* x offset in meters		*/
double false_north;			/* y offset in meters		*/
{
double sin_po;				/* sin value			*/
double cos_po;				/* cos value			*/
double con;				/* temporary sin value		*/
double ms1;				/* small m 1			*/
double ms2;				/* small m 2			*/
double temp;				/* temporary variable		*/
double ts0;				/* small t 0			*/
double ts1;				/* small t 1			*/
double ts2;				/* small t 2			*/

r_major = r_maj;
r_minor = r_min;
false_easting = false_east;
false_northing = false_north;

/* Standard Parallels cannot be equal and on opposite sides of the equator
------------------------------------------------------------------------*/
if (fabs(lat1+lat2) < EPSLN)
   {
   p_error("Equal Latitiudes for St. Parallels on opposite sides of equator",
           "lamcc-inv");
   return(41);
   }

temp = r_minor / r_major;
es = 1.0 - SQUARE(temp);
e = sqrt(es);

center_lon = c_lon;
center_lat = c_lat;
sincos(lat1,&sin_po,&cos_po);
con = sin_po;
ms1 = msfnz(e,sin_po,cos_po);
ts1 = tsfnz(e,lat1,sin_po);
sincos(lat2,&sin_po,&cos_po);
ms2 = msfnz(e,sin_po,cos_po);
ts2 = tsfnz(e,lat2,sin_po);
sin_po = sin(center_lat);
ts0 = tsfnz(e,center_lat,sin_po);

if (fabs(lat1 - lat2) > EPSLN)
    ns = log (ms1/ms2)/ log (ts1/ts2);
else
    ns = con;
f0 = ms1 / (ns * pow(ts1,ns));
rh = r_major * f0 * pow(ts0,ns);



/* Report parameters to the user
  -----------------------------*/
ptitle("LAMBERT CONFORMAL CONIC"); 
radius2(r_major, r_minor);
stanparl(lat1,lat2);
cenlonmer(center_lon);
origin(c_lat);
offsetp(false_easting,false_northing);

return(OK);
}

/* Lambert Conformal Conic inverse equations--mapping x,y to lat/long
  -----------------------------------------------------------------*/
long lamccinv(x , y, lon, lat)
double x;			/* (O) X projection coordinate 	*/
double y;			/* (O) Y projection coordinate 	*/
double *lon;			/* (I) Longitude 		*/
double *lat;			/* (I) Latitude 		*/

{
double rh1;			/* height above ellipsoid	*/
double con;			/* sign variable		*/
double ts;			/* small t			*/
double theta;			/* angle			*/
long   flag;			/* error flag			*/

flag = 0;
x -= false_easting;
y = rh - y + false_northing;
 if (ns > 0)
    {
    rh1 = sqrt (x * x + y * y);
    con = 1.0;
    }
 else
    {
    rh1 = -sqrt (x * x + y * y);
    con = -1.0;
    }
 theta = 0.0;
 if (rh1 != 0)
    theta = atan2((con * x),(con * y));
 if ((rh1 != 0) || (ns > 0.0))
    {
    con = 1.0/ns;
    ts = pow((rh1/(r_major * f0)),con);
    *lat = phi2z(e,ts,&flag);
    if (flag != 0)
       return(flag);
    }
 else
    *lat = -HALF_PI;
 *lon = adjust_lon(theta/ns + center_lon);
 return(OK);
 }
