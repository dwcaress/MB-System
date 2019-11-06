/*******************************************************************************
NAME                       OBLIQUE MERCATOR (HOTINE) 

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the Oblique Mercator projection.  The
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
static double d;
static double el,u;
static double singam,cosgam;
static double sinaz,cosaz;


/* Initialize the Oblique Mercator  projection
  ------------------------------------------*/
long omerforint(r_maj,r_min,scale_fact,azimuth,lon_orig,lat_orig,false_east,
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
double ts;
double ts1,ts2;
double h,l;
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
   genrpt(lon1 * R2D,"Longitude of First Point:    ");
   genrpt(lat1 * R2D,"Latitude of First Point:    ");
   genrpt(lon2 * R2D,"Longitude of Second Point:    ");
   genrpt(lat2 * R2D,"Latitude of Second Point:    ");
   
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


/* Oblique Mercator forward equations--mapping lat,long to x,y
  ----------------------------------------------------------*/
long omerfor(lon, lat, x, y)
double lon;			/* (I) Longitude 		*/
double lat;			/* (I) Latitude 		*/
double *x;			/* (O) X projection coordinate 	*/
double *y;			/* (O) Y projection coordinate 	*/
{
double theta;		/* angle					*/
double sin_phi, cos_phi;/* sin and cos value				*/
double b;		/* temporary values				*/
double c, t, tq;	/* temporary values				*/
double con, n, ml;	/* cone constant, small m			*/
double q,us,vl;
double ul,vs;
double s;
double dlon;
double ts1;

/* Forward equations
  -----------------*/
sin_phi = sin(lat);
dlon = adjust_lon(lon - lon_origin);
vl = sin(bl * dlon);
if (fabs(fabs(lat) - HALF_PI) > EPSLN)
   {
   ts1 = tsfnz(e,lat,sin_phi);
   q = el / (pow(ts1,bl));
   s = .5 * (q - 1.0 / q);
   t = .5 * (q + 1.0/ q);
   ul = (s * singam - vl * cosgam) / t;
   con = cos(bl * dlon);
   if (fabs(con) < .0000001)
      {
      us = al * bl * dlon;
      }
   else
      {
      us = al * atan((s * cosgam + vl * singam) / con)/bl;
      if (con < 0)
         us = us + PI * al / bl;
      }
   }
else
   {
   if (lat >= 0)
      ul = singam;
   else
      ul = -singam;
   us = al * lat / bl;
   }
if (fabs(fabs(ul) - 1.0) <= EPSLN)
   {
   p_error("Point projects into infinity","omer-for");
   return(205);
   }
vs = .5 * al * log((1.0 - ul)/(1.0 + ul)) / bl;
us = us - u;
*x = false_easting + vs * cosaz + us * sinaz;
*y = false_northing + us * cosaz - vs * sinaz;

return(OK);
}
