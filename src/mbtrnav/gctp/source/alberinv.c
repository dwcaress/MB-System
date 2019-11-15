/*******************************************************************************
NAME                     ALBERS CONICAL EQUAL-AREA 

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Albers Conical Equal Area projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

PROGRAMMER              DATE
----------              ----
T. Mittan,       	Feb, 1992
S. Nelson		Feb, 1996	Made a modification to the
					assignment to "con" enclosing the
					section 1.0 - e3 in parenthesis.

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
  static double r_major;        /* major axis                           */
  static double r_minor;        /* minor axis                           */
  static double c;              /* constant c                           */
  static double e3;             /* eccentricity                         */
  static double es;		/* eccentricity squared			*/
  static double rh;             /* heigth above elipsoid                */
  static double ns0;            /* ratio between meridians              */
  static double lon_center;     /* center longitude                     */
  static double false_easting;  /* x offset in meters                   */
  static double false_northing; /* y offset in meters                   */

/* Initialize the Albers projection
  -------------------------------*/
long alberinvint(r_maj,r_min,lat1,lat2,lon0,lat0,false_east,false_north)

double r_maj;                   /* major axis                           */
double r_min;                   /* minor axis                           */
double lat1;                    /* first standard parallel              */
double lat2;                    /* second standard parallel             */
double lon0;                    /* center longitude                     */
double lat0;                    /* center lattitude                     */
double false_east;              /* x offset in meters                   */
double false_north;             /* y offset in meters                   */
{
double sin_po,cos_po;		/* sine and cos values			*/
double con;			/* temporary variable			*/
double temp;			/* temporary variable			*/
double ms1;   		        /* small m 1                            */
double ms2;            		/* small m 2                            */
double qs0;            		/* small q 0                            */
double qs1;            		/* small q 1                            */
double qs2;            		/* small q 2                            */

false_easting = false_east;
false_northing = false_north;
lon_center = lon0;
if (fabs(lat1 + lat2) < EPSLN)
   {
  p_error("Equal latitudes for Standard Parallels on opposite sides of equator"
	  ,"alber-invinit");
   return(31);
   }
r_major = r_maj;
r_minor = r_min;
temp = r_minor / r_major;
es = 1.0 - SQUARE(temp);
e3 = sqrt(es);

sincos(lat1, &sin_po, &cos_po);
con = sin_po;

ms1 = msfnz(e3,sin_po,cos_po);
qs1 = qsfnz(e3,sin_po,cos_po);

sincos(lat2,&sin_po,&cos_po);

ms2 = msfnz(e3,sin_po,cos_po);
qs2 = qsfnz(e3,sin_po,cos_po);

sincos(lat0,&sin_po,&cos_po);

qs0 = qsfnz(e3,sin_po,cos_po);

if (fabs(lat1 - lat2) > EPSLN)
   ns0 = (ms1 * ms1 - ms2 *ms2)/ (qs2 - qs1);
else
   ns0 = con;
c = ms1 * ms1 + ns0 * qs1;
rh = r_major * sqrt(c - ns0 * qs0)/ns0;

/* Report parameters to the user
  -----------------------------*/
ptitle("ALBERS CONICAL EQUAL-AREA");
radius2(r_major, r_minor);
stanparl(lat1,lat2);
cenlonmer(lon_center);
origin(lat0);
offsetp(false_easting,false_northing);

return(OK);
}

/* Albers Conical Equal Area inverse equations--mapping x,y to lat/long
  -------------------------------------------------------------------*/
long alberinv(x, y, lon, lat)
double x;			/* (O) X projection coordinate 	*/
double y;			/* (O) Y projection coordinate 	*/
double *lon;			/* (I) Longitude 		*/
double *lat;			/* (I) Latitude 		*/
{
double rh1;			/* height above ellipsoid	*/
double qs;			/* function q			*/
double con;			/* temporary sign value		*/
double theta;			/* angle			*/
long   flag;			/* error flag;			*/


flag = 0;
x -= false_easting;
y = rh - y + false_northing;;
if (ns0 >= 0)
   {
   rh1 = sqrt(x * x + y * y);
   con = 1.0;
   }
else
   {
   rh1 = -sqrt(x * x + y * y);
   con = -1.0;
   }
theta = 0.0;
if (rh1 != 0.0)
   theta = atan2(con * x, con * y);
con = rh1 * ns0 / r_major;
qs = (c - con * con) / ns0;
if (e3 >= 1e-10)
   {
   con = 1 - .5 * (1.0 - es) * log((1.0 - e3) / (1.0 + e3))/e3;
   if (fabs(fabs(con) - fabs(qs)) > .0000000001 )
      {
      *lat = phi1z(e3,qs,&flag);
      if (flag != 0)
         return(flag);
      }
   else
      {
      if (qs >= 0)
         *lat = .5 * PI;
      else
         *lat = -.5 * PI;
      }
   }
else
   {
   *lat = phi1z(e3,qs,&flag);
   if (flag != 0)
      return(flag);
   }

*lon = adjust_lon(theta/ns0 + lon_center);

return(OK);
}
