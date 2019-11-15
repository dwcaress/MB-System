/*******************************************************************************
NAME                          OBLATED EQUAL-AREA

PURPOSE:	Transforms input longitude and latitude to Easting and
		Northing for the Oblated Equal Area projection.  The
		longitude and latitude must be in radians.  The Easting
		and Northing values will be returned in meters.

PROGRAM HISTORY
PROGRAMMER              DATE            
----------              ----           
D. Steinwand            May, 1991     

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

static double lon_center;
static double lat_o;
static double theta;
static double m;
static double n;
static double R;
static double sin_lat_o;
static double cos_lat_o;
static double false_easting;
static double false_northing;

long obleqforint(r, center_long, center_lat, shape_m, shape_n, angle,
	false_east, false_north)

double r;
double center_long;
double center_lat;
double shape_m,shape_n;
double angle;
double false_east;
double false_north;
{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;
lon_center = center_long;
lat_o = center_lat;
m = shape_m;
n = shape_n;
theta = angle;
false_easting = false_east;
false_northing = false_north;

/* Report parameters to the user (to device set up prior to this call)
  -------------------------------------------------------------------*/
ptitle("OBLATED EQUAL-AREA");
radius(R);
cenlon(lon_center);
cenlat(lat_o);
genrpt(m,"Parameter m:      ");
genrpt(n,"Parameter n:      ");
genrpt(theta,"Theta:      ");
offsetp(false_easting,false_northing);

/* Calculate the sine and cosine of the latitude of the center of the map
   and store in static storage for common use.
  -------------------------------------------*/
sincos(lat_o, &sin_lat_o, &cos_lat_o);
return(OK);
}

long obleqfor(lon, lat, x, y)

double lon;		/* (I) Longitude */
double lat;		/* (I) Latitude */
double *x;		/* (O) X projection coordinate */
double *y;		/* (O) Y projection coordinate */
{
double delta_lon;
double sin_delta_lon;
double cos_delta_lon;
double sin_lat;
double cos_lat;
double z;
double sin_z;
double cos_z;
double Az;
double sin_Az;
double cos_Az;
double temp;			/* Re-used temporary variable */
double x_prime;
double y_prime;
double M;
double N;
double diff_angle;
double sin_diff_angle;
double cos_diff_angle;

/* Forward equations
  -----------------*/
delta_lon = lon - lon_center;
sincos(lat, &sin_lat, &cos_lat);
sincos(delta_lon, &sin_delta_lon, &cos_delta_lon);
z = acos(sin_lat_o * sin_lat + cos_lat_o * cos_lat * cos_delta_lon);
Az = atan2(cos_lat * sin_delta_lon , cos_lat_o * sin_lat - sin_lat_o * 
	cos_lat * cos_delta_lon) + theta;
sincos(Az, &sin_Az, &cos_Az);
temp = 2.0 * sin(z / 2.0);
x_prime = temp * sin_Az;
y_prime = temp * cos_Az;
M = asin(x_prime / 2.0);
temp = y_prime / 2.0 * cos(M) / cos(2.0 * M / m);
N = asin(temp);
*y = n * R * sin(2.0 * N / n) + false_easting;
*x = m * R * sin(2.0 * M / m) * cos(N) / cos(2.0 * N / n) + false_northing;
return(OK);
}
