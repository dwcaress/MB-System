/*******************************************************************************
NAME                       OBLIQUE MERCATOR (HOTINE)

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Oblique Mercator projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

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
static double azimuth;
static double r_major;		/* major axis 				*/
static double r_minor;		/* minor axis 				*/
static double scale_factor;	/* scale factor				*/
static double lon_origin;	/* center longitude			*/
static double lat_origin;	/* center latitude			*/
static double e,es;		/* eccentricity constants		*/
static double false_northing;	/* y offset in meters			*/
static double false_easting;	/* x offset in meters			*/
static double sin_p20,cos_p20;	/* sin and cos values			*/
static double bl;
static double al;
static double ts;
static double d;
static double el,u;
static double singam,cosgam;
static double sinaz,cosaz;


/* Initialize the Oblique Mercator  projection
  ------------------------------------------*/
long omerinvint(r_maj,r_min,scale_fact,azimuth,lon_orig,lat_orig,false_east,
	    false_north,lon1,lat1,lon2,lat2,mode) 

double r_maj;			/* major axis			*/
double r_min;			/* minor axis			*/
double scale_fact;		/* scale factor			*/
double azimuth;			/* azimuth east of north	*/
double lon_orig;		/* longitude of origin		*/
double lat_orig;		/* center latitude		*/
double false_east;		/* x offset in meters		*/
double false_north;		/* y offset in meters		*/
double lon1;			/* fist point to define central line	*/
double lat1;			/* fist point to define central line	*/
double lon2;			/* second point to define central line	*/
double lat2;			/* second point to define central line	*/
long   mode;			/* which format type A or B	*/
{
double temp;			/* temporary variable		*/
double con,com;
double h,l,ts1,ts2;
double j,p,dlon;
double f,g,gama;
double sinphi,cosphi;

/* Place parameters in static storage for common use
  -------------------------------------------------*/
r_major = r_maj;
r_minor = r_min;
scale_factor = scale_fact;
lat_origin = lat_orig;
false_northing = false_north;
false_easting = false_east;

temp = r_minor / r_major;
es = 1.0 - SQUARE(temp);
e = sqrt(es);

sincos(lat_origin,&sin_p20,&cos_p20);
con = 1.0 - es * sin_p20 * sin_p20;
com = sqrt(1.0 - es);
bl = sqrt(1.0 + es * pow(cos_p20,4.0)/(1.0 - es));
al = r_major * bl * scale_factor * com / con;
if (fabs(lat_origin) < EPSLN)
   {
   ts = 1.0;
   d = 1.0;
   el = 1.0;
   }
else
   {
   ts = tsfnz(e,lat_origin,sin_p20);
   con = sqrt(con);
   d = bl * com / (cos_p20 * con);
   if ((d * d - 1.0) > 0.0)
      {
      if (lat_origin >= 0.0)
         f = d + sqrt(d * d - 1.0);
      else
         f = d - sqrt(d * d - 1.0);
      }
   else
      f = d;
   el = f * pow(ts,bl);
   }

/* Report parameters to the user that are the same for both formats
  ---------------------------------------------------------------*/
ptitle("OBLIQUE MERCATOR (HOTINE)"); 
radius2(r_major, r_minor);
genrpt(scale_factor,"Scale Factor at C. Meridian:    ");
offsetp(false_easting,false_northing);

if (mode != 0)
   {
   g = .5 * (f - 1.0/f);
   gama = asinz(sin(azimuth) / d);
   lon_origin = lon_orig - asinz(g * tan(gama))/bl;

   /* Report parameters common to format B
   -------------------------------------*/
   genrpt(azimuth * R2D,"Azimuth of Central Line:    ");
   cenlon(lon_origin);
   cenlat(lat_origin);
   
   con = fabs(lat_origin);
   if ((con > EPSLN) && (fabs(con - HALF_PI) > EPSLN))
      {
      sincos(gama,&singam,&cosgam);
      sincos(azimuth,&sinaz,&cosaz);
      if (lat_origin >= 0)
         u =  (al / bl) * atan(sqrt(d*d - 1.0)/cosaz);
      else
         u =  -(al / bl) * atan(sqrt(d*d - 1.0)/cosaz);
      }
   else
      {
      p_error("Input data error","omer-init");
      return(201);
      }
   }
else
   {
   sinphi = sin(lat1);
   ts1 = tsfnz(e,lat1,sinphi);
   sinphi = sin(lat2);
   ts2 = tsfnz(e,lat2,sinphi);
   h = pow(ts1,bl);
   l = pow(ts2,bl);
   f = el/h;
   g = .5 * (f - 1.0/f);
   j = (el * el - l * h)/(el * el + l * h);
   p = (l - h) / (l + h);
   dlon = lon1 - lon2;
   if (dlon < -PI)
      lon2 = lon2 - 2.0 * PI;
   if (dlon > PI)
      lon2 = lon2 + 2.0 * PI;
   dlon = lon1 - lon2;
   lon_origin = .5 * (lon1 + lon2) - atan(j * tan(.5 * bl * dlon)/p)/bl;
   dlon  = adjust_lon(lon1 - lon_origin);
   gama = atan(sin(bl * dlon)/g);
   azimuth = asinz(d * sin(gama));
   
   /* Report parameters common to format A
   -------------------------------------*/
   genrpt(lon1 * R2D,"Longitude of First Point:     ");
   genrpt(lat1 * R2D,"Latitude of First Point:      ");
   genrpt(lon2 * R2D,"Longitude of Second Point:    ");
   genrpt(lat2 * R2D,"Latitude of Second Point:     ");
   
   if (fabs(lat1 - lat2) <= EPSLN)
      {
      p_error("Input data error","omer-init");
      return(202);
      }
   else
      con = fabs(lat1);
   if ((con <= EPSLN) || (fabs(con - HALF_PI) <= EPSLN))
      {
      p_error("Input data error","omer-init");
      return(202);
      }
   else 
   if (fabs(fabs(lat_origin) - HALF_PI) <= EPSLN)
      {
      p_error("Input data error","omer-init");
      return(202);
      }
      
   sincos(gama,&singam,&cosgam);
   sincos(azimuth,&sinaz,&cosaz);
   if (lat_origin >= 0)
      u =  (al/bl) * atan(sqrt(d * d - 1.0)/cosaz);
   else
      u = -(al/bl) * atan(sqrt(d * d - 1.0)/cosaz);
   }
return(OK);
}


/* Oblique Mercator inverse equations--mapping x,y to lat/long
  ----------------------------------------------------------*/
long omerinv(x, y, lon, lat)
double x;			/* (O) X projection coordinate 	*/
double y;			/* (O) Y projection coordinate 	*/
double *lon;			/* (I) Longitude 		*/
double *lat;			/* (I) Latitude 		*/
{
double delta_lon;	/* Delta longitude (Given longitude - center 	*/
double theta;		/* angle					*/
double delta_theta;	/* adjusted longitude				*/
double sin_phi, cos_phi;/* sin and cos value				*/
double b;		/* temporary values				*/
double c, t, tq;	/* temporary values				*/
double con, n, ml;	/* cone constant, small m			*/
double vs,us,q,s,ts1;
double vl,ul,bs;
double dlon;
long   flag;

/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
flag = 0;
vs = x * cosaz - y * sinaz;
us = y * cosaz + x * sinaz;
us = us + u;
q = exp(-bl * vs / al);
s = .5 * (q - 1.0/q);
t = .5 * (q + 1.0/q);
vl = sin(bl * us / al);
ul = (vl * cosgam + s * singam)/t;
if (fabs(fabs(ul) - 1.0) <= EPSLN)
   {
   *lon = lon_origin;
   if (ul >= 0.0)
      *lat = HALF_PI;
   else
      *lat = -HALF_PI;
   }
else
   {
   con = 1.0 / bl;
   ts1 = pow((el / sqrt((1.0 + ul) / (1.0 - ul))),con);
   *lat = phi2z(e,ts1,&flag);
   if (flag != 0)
      return(flag);
   con = cos(bl * us /al);
   theta = lon_origin - atan2((s * cosgam - vl * singam) , con)/bl;
   *lon = adjust_lon(theta);
   }
return(OK);
}
