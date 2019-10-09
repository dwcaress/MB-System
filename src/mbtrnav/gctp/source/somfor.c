/*******************************************************************************
NAME                          SPACE OBLIQUE MERCATOR (SOM)

PURPOSE:	The first method of Transforming input longitude and
		latitude to Easting and Northing for the SOM projection.
		The longitude and latitude must be in radians.  The
		Easting and Northing values will be returned in meters.

PROGRAM HISTORY
PROGRAMMER              DATE            REASON
----------              ----            ------
D. Steinwand            July, 1992
T. Mittan		Mar, 1993
S. Nelson		Nov, 1993	Changed error message

ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  "Software Documentation for GCTP General Cartographic Transformation
    Package", U.S. Geological Survey National Mapping Division, May 1982.
*******************************************************************************/
#include <stdio.h>
#include "cproj.h"
#define LANDSAT_RATIO 0.5201613

static double lon_center,a,b,a2,a4,c1,c3,q,t,u,w,xj,p21,sa,ca,es,s,start;
static double som_series();
static double false_easting;
static double false_northing;

long somforint(r_major,r_minor,satnum,path,alf_in,lon,false_east,false_north,
	time, start1,flag)

double r_major;			/* major axis				*/
double r_minor;			/* minor axis				*/
long satnum;			/* Landsat satellite number (1,2,3,4,5) */
long path;			/* Landsat path number */
double alf_in;
double lon;
double false_east;		/* x offset in meters			*/
double false_north;		/* y offset in meters			*/
double time;
long   start1;
long   flag;
{
long i;
double alf,e2c,e2s,one_es;
double dlam,fb,fa2,fa4,fc1,fc3,suma2,suma4,sumc1,sumc3,sumb;

/* Place parameters in static storage for common use
  -------------------------------------------------*/
false_easting = false_east;
false_northing = false_north;
a = r_major;
b = r_minor;
es = 1.0 - SQUARE(r_minor/r_major);
if (flag != 0)
  {
  alf = alf_in;
  p21 = time / 1440.0;
  lon_center = lon; 
  start =  start1;
  }
else
  {
  if (satnum < 4)
    {
    alf = 99.092 * D2R;
    p21=103.2669323/1440.0;
    lon_center = (128.87 - (360.0/251.0 * path)) * D2R;
    }
  else
    {
    alf = 98.2 * D2R;
    p21=98.8841202/1440.0;
    lon_center = (129.30 - (360.0/233.0 * path)) * D2R;
    /*
    lon_center = (-129.30557714 - (360.0/233.0 * path)) * D2R;
    */
    }
  start=0.0;
  }

/* Report parameters to the user (to device set up prior to this call)
  -------------------------------------------------------------------*/
ptitle("SPACE OBLIQUE MERCATOR");
radius2(a,b);
if (flag == 0)
   {
   genrpt_long(path,     "Path Number:    ");
   genrpt_long(satnum,   "Satellite Number:    ");
   }
genrpt(alf*R2D,       "Inclination of Orbit:    ");
genrpt(lon_center*R2D,"Longitude of Ascending Orbit:    ");
offsetp(false_easting,false_northing);
genrpt(LANDSAT_RATIO, "Landsat Ratio:    ");

ca=cos(alf);
if (fabs(ca)<1.e-9) ca=1.e-9;
sa=sin(alf);
e2c=es*ca*ca;
e2s=es*sa*sa;
w=(1.0-e2c)/(1.0-es);
w=w*w-1.0;
one_es=1.0-es;
q = e2s / one_es;
t = (e2s*(2.0-es)) / (one_es*one_es);
u= e2c / one_es;
xj = one_es*one_es*one_es;
dlam=0.0;
som_series(&fb,&fa2,&fa4,&fc1,&fc3,&dlam);
suma2=fa2;
suma4=fa4;
sumb=fb;
sumc1=fc1;
sumc3=fc3;
for(i=9;i<=81;i+=18)
   {
   dlam=i;
   som_series(&fb,&fa2,&fa4,&fc1,&fc3,&dlam);
   suma2=suma2+4.0*fa2;
   suma4=suma4+4.0*fa4;
   sumb=sumb+4.0*fb;
   sumc1=sumc1+4.0*fc1;
   sumc3=sumc3+4.0*fc3;
   }
for(i=18; i<=72; i+=18)
   {
   dlam=i;
   som_series(&fb,&fa2,&fa4,&fc1,&fc3,&dlam);
   suma2=suma2+2.0*fa2;
   suma4=suma4+2.0*fa4;
   sumb=sumb+2.0*fb;
   sumc1=sumc1+2.0*fc1;
   sumc3=sumc3+2.0*fc3;
   }

dlam=90.0;
som_series(&fb,&fa2,&fa4,&fc1,&fc3,&dlam);
suma2=suma2+fa2;
suma4=suma4+fa4;
sumb=sumb+fb;
sumc1=sumc1+fc1;
sumc3=sumc3+fc3;
a2=suma2/30.0;
a4=suma4/60.0;
b=sumb/30.0;
c1=sumc1/15.0;
c3=sumc3/45.0;
return(OK);
}

long somfor(lon, lat, y, x)

double lon;		/* (I) Longitude 		*/
double lat;		/* (I) Latitude 		*/
double *x;		/* (O) X projection coordinate 	*/
double *y;		/* (O) Y projection coordinate 	*/
{
long n,i,l;
double delta_lon;
double rlm,tabs,tlam,xlam,c,xlamt,ab2,sc1,ab1,xlamp,sav;
double d,sdsq,sd,tanlg,xtan,tphi,dp,dd,ds,rlm2;
double scl,tlamp,conv,delta_lat,radlt,radln;
double temp;
char   errorbuf[80];

/* Forward equations
  -----------------*/
conv=1.e-7;
delta_lat=lat;
delta_lon= lon-lon_center;

/* Test for latitude and longitude approaching 90 degrees
   ----------------------------------------------------*/
if (delta_lat>1.570796) delta_lat=1.570796;
if (delta_lat<-1.570796) delta_lat= -1.570796;
radlt=delta_lat;
radln=delta_lon;
if(delta_lat>=0.0)tlamp=PI/2.0; 
if(start!= 0.0)tlamp=2.5*PI;
if(delta_lat<0.0) tlamp=1.5*PI;
n=0;

L230:  sav=tlamp;
       l=0;
       xlamp=radln+p21*tlamp;
       ab1=cos(xlamp);
       if(fabs(ab1)<conv) xlamp=xlamp-1.e-7;
       if(ab1>=0.0) scl=1.0;
       if(ab1<0.0) scl= -1.0;
       ab2=tlamp-(scl)*sin(tlamp)*HALF_PI;
L240:  xlamt=radln+p21*sav;
       c=cos(xlamt);
       if (fabs(c)<1.e-7) xlamt=xlamt-1.e-7;
       xlam=(((1.0-es)*tan(radlt)*sa)+sin(xlamt)*ca)/c;
       tlam=atan(xlam);
       tlam=tlam+ab2;
       tabs=fabs(sav)-fabs(tlam);
       if(fabs(tabs)<conv) goto L250;
       l=l+1;
       if (l > 50) goto L260;
       sav=tlam;
       goto L240;

/* Adjust for confusion at beginning and end of landsat orbits
  -----------------------------------------------------------*/
L250:  rlm=PI*LANDSAT_RATIO;
       rlm2=rlm+2.0*PI;
       n++;
       if(n>=3) goto L300;
       if(tlam>rlm&&tlam<rlm2) goto L300;
       if(tlam<rlm)tlamp=2.50*PI;
       if(tlam>=rlm2) tlamp=HALF_PI;
       goto L230;
L260:  sprintf(errorbuf,"50 iterations without conv\n");
       p_error(errorbuf,"som-forward");
       return(214);

/* tlam computed - now compute tphi
  --------------------------------*/
L300: ds=sin(tlam);
      dd=ds*ds;
      dp=sin(radlt);
      tphi=asin(((1.0-es)*ca*dp-sa*cos(radlt)*sin(xlamt))/sqrt(1.0-es*dp*dp));

/* compute x and y
  ---------------*/
xtan = (PI/4.0) + (tphi/2.0);
tanlg = log(tan(xtan));
sd=sin(tlam);
sdsq=sd*sd;
s=p21*sa*cos(tlam)*sqrt((1.0+t*sdsq)/((1.0+w*sdsq)*(1.0+q*sdsq)));
d=sqrt(xj*xj+s*s);
*x=b*tlam+a2*sin(2.0*tlam)+a4*sin(4.0*tlam)-tanlg*s/d;
*x = a* *x;
*y=c1*sd+c3*sin(3.0*tlam)+tanlg*xj/d;
*y = a* *y;

/* Negate x & swap x,y
  -------------------*/
temp=  *x;
*x= *y + false_easting;
*y=temp + false_northing;;
return(OK);
}
/* Series to calculate a,b,c coefficients to convert from transform
   latitude,longitude to Space Oblique Mercator (SOM) rectangular coordinates

   Mathematical analysis by John Snyder 6/82
  --------------------------------------------------------------------------*/
static double som_series(fb,fa2,fa4,fc1,fc3,dlam)
double *fb,*fa2,*fa4,*fc1,*fc3,*dlam;
{
double sd,sdsq,h,sq,fc;

*dlam= *dlam*0.0174532925;               /* Convert dlam to radians */
sd=sin(*dlam);
sdsq=sd*sd;
s=p21*sa*cos(*dlam)*sqrt((1.0+t*sdsq)/((1.0+w*sdsq)*(1.0+q*sdsq)));
h=sqrt((1.0+q*sdsq)/(1.0+w*sdsq))*(((1.0+w*sdsq)/((1.0+q*sdsq)*(1.0+
     q*sdsq)))-p21*ca);
sq=sqrt(xj*xj+s*s);
*fb=(h*xj-s*s)/sq;
*fa2= *fb*cos(2.0* *dlam);
*fa4= *fb*cos(4.0* *dlam);
fc=s*(h+xj)/sq;
*fc1=fc*cos(*dlam);
*fc3=fc*cos(3.0* *dlam);
}
