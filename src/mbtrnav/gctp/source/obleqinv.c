/*******************************************************************************
NAME                          OBLATED EQUAL-AREA

PURPOSE:	Transforms input Easting and Northing to longitude and
		latitude for the Oblated Equal Area projection.  The
		Easting and Northing must be in meters.  The longitude
		and latitude values will be returned in radians.

PROGRAM HISTORY
PROGRAMMER              DATE            REASON
----------              ----            ------
D. Steinwand            May, 1991     
S. Nelson		Nov, 1993	Added "double adjust_lon()" function
					declaration statement.

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

long obleqinvint(r, center_long, center_lat, shape_m, shape_n, angle,false_east,
	    false_north)
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

long obleqinv(x, y, lon, lat)

double x;		/* (I) X projection coordinate */
double y;		/* (I) Y projection coordinate */
double *lon;		/* (O) Longitude */
double *lat;		/* (O) Latitude */
{
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

/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
N = (n / 2.0) * asin(y / (n * R));
temp = x / (m * R) * cos(2.0 * N / n) / cos(N);
M = (m / 2.0) * asin(temp);
x_prime = 2.0 * sin(M);
y_prime = 2.0 * sin(N) * cos(2.0 * M / m) / cos(M);
temp = sqrt(x_prime * x_prime + y_prime * y_prime) / 2.0;
z = 2.0 * asin(temp);
Az = atan2(x_prime, y_prime);
diff_angle = Az - theta;
sincos(diff_angle, &sin_diff_angle, &cos_diff_angle);
sincos(z, &sin_z, &cos_z);
*lat = asin(sin_lat_o * cos_z + cos_lat_o * sin_z * cos_diff_angle);
*lon = adjust_lon(lon_center + atan2((sin_z * sin_diff_angle), (cos_lat_o *
		 cos_z - sin_lat_o * sin_z * cos_diff_angle)));
return(OK);
}

