#include <stdio.h>
#include "NavUtils.h"
#include "MathP.h"

// Defined constants for use with gctpc 
#define Geographic 0
#define UTM 1
#define WGS84Spheroid 12
#define Radians  0           // Radians
#define Meters   2           // Meters
#define SilentMode -1
#define NameLen   256
#define Ignored -1

extern "C" {
void gctp(double *incoor, long *insys, long *inzone, double *inparm,
	 long *inunit, long *indatum, long *ipr, char *efile, 
	 long *jpr, char *pfile, double *outcoor, long *outsys,
	 long *outzone, double *outparm, long *outunit,
	 long *outdatum, char *fn27, char *fn83, long *iflg); 
};


int NavUtils::geoToUtm(double latitude, double longitude, long utmZone,
		       double *northing, double *easting)
{
  long insys     = Geographic;
  long inzone    = Ignored;
  long inunit    = Radians;  
  long indatum   = WGS84Spheroid;
  long ipr       = SilentMode;
  long jpr       = SilentMode;

  long outsys    = UTM;   
  long outunit   = Meters;
  long outdatum  = WGS84Spheroid;

  long outzone   = utmZone;

  double inparm[15];

  double inputCoord[2];
  inputCoord[0] = longitude;  
  inputCoord[1] = latitude;  

  double outparm[15];
  double outputCoord[2];

  char errorFile[32];
  char projMsgFile[32];
  char nad27File[32];
  char nad83File[32];
  long errorFlag;

  gctp(inputCoord, &insys, &inzone, inparm, &inunit, &indatum, &ipr,
       errorFile, &jpr, projMsgFile, outputCoord, &outsys, &outzone, outparm, 
       &outunit, &outdatum, nad27File, nad83File, &errorFlag);

  *easting = outputCoord[0]; 
  *northing = outputCoord[1];

  return 0;
}


int NavUtils::utmToGeo(double northing, double easting, long utmZone,
		       double *latitude, double *longitude)
{
  long insys     = UTM;

  long inzone    = utmZone;

  long inunit    = Meters;
  long indatum   = WGS84Spheroid;
  long ipr       = SilentMode;
  long jpr       = SilentMode;

  long outsys    = Geographic;
  long outunit   = Radians;
  long outdatum  = WGS84Spheroid;

  long outzone   = Ignored;

  double inparm[15];

  double inputCoord[2];
  inputCoord[0] = easting;
  inputCoord[1] = northing;

  double outparm[15];
  double outputCoord[2];

  char errorFile[32];
  char projMsgFile[32];
  char nad27File[32];
  char nad83File[32];
  long errorFlag;

  gctp(inputCoord, &insys, &inzone, inparm, &inunit, &indatum, &ipr,
       errorFile, &jpr, projMsgFile, outputCoord, &outsys, &outzone, outparm, 
       &outunit, &outdatum, nad27File, nad83File, &errorFlag);

  *longitude = outputCoord[0]; 
  *latitude = outputCoord[1];

  return 0;
}

///////////////////////////////////////////////////////////////////
// Convert geographic coordinates (in radians) to UTM zone
// NOTE Latitude range is -pi/2 to pi/2 and longitude is -pi to pi.
// On input error returns 0, else > 0.
// Always check for return values > 0.
// https://gis.stackexchange.com/questions/13291/computing-utm-zone-from-lat-long-point
// Usage with units of radians:
//   unsigned uzone = geoToUtmZone(radLat, radLong);
// Usage with units of degrees:
//   #include <MathP.h>    // for rad-deg-rad conversions
//   unsigned uzone = geoToUtmZone(Math::degToRad(degLat), Math::degToRad(degLong));
//
unsigned int NavUtils::geoToUtmZone(double latitude, double longitude)
{
  if (longitude > PI   || longitude < -PI)
  {
    printf("NavUtils::geoToUtmZone - input error: Lon %.2f\n", longitude);
    printf("NavUtils::geoToUtmZone - -PI <= Lon <= PI\n");
    return 0;
  }

  if (latitude  > PI/2 || latitude  < -PI/2)
  {
    printf("NavUtils::geoToUtmZone - input error: Lat %.2f\n", latitude);
    printf("NavUtils::geoToUtmZone - -PI/2 <= Lat <= PI/2\n");
    return 0;
  }

  // Convert to degrees for this computation
  //
  double lat = Math::radToDeg(latitude), lon = Math::radToDeg(longitude);

  // Initialize using the default calculation
  //
  unsigned int ZoneNumber = floor((lon + 180)/6) + 1;
  // Norway
  if (lat >= 56.0 && lat < 64.0 && lon >= 3.0 && lon < 12.0)
  {
    ZoneNumber = 32;
  }
  // Special zones for Svalbard
  else if (lat >= 72.0 && lat < 84.0)
  { 
    if      (lon >= 0.0  && lon <  9.0) ZoneNumber = 31;
    else if (lon >= 9.0  && lon < 21.0) ZoneNumber = 33;
    else if (lon >= 21.0 && lon < 33.0) ZoneNumber = 35;
    else if (lon >= 33.0 && lon < 42.0) ZoneNumber = 37;
  }

  return ZoneNumber;
}
