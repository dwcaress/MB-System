#ifndef _NAVUTILS_H
#define _NAVUTILS_H

// Use temporarily!!!
#define MontereyUTM 10

/*
CLASS 
NavUtils

DESCRIPTION
Various navigation utilities, including projection transformation methods.

AUTHOR
Tom O'Reilly
*/

class NavUtils {

public:

  ///////////////////////////////////////////////////////////////////
  // Convert geographic coordinates (in radians) to UTM coordinates
  // (in meters)
  static int geoToUtm(double latitude, double longitude, long utmZone,
          double *northing, double *easting);

  ///////////////////////////////////////////////////////////////////
  // Convert geographic coordinates (in radians) to UTM zone
  // NOTE Latitude range is -pi/2 to pi/2 and longitude is -pi to pi.
  // On input error returns 0, else > 0.
  // IMPORTANT: Always check for return values > 0.
  // https://gis.stackexchange.com/questions/13291/computing-utm-zone-from-lat-long-point
  // Usage with units of radians:
  //   unsigned uzone = geoToUtmZone(radLat, radLong);
  // Usage with units of degrees:
  //   #include <MathP.h>    // for rad-deg-rad conversions
  //   unsigned uzone = geoToUtmZone(Math::degToRad(degLat), Math::degToRad(degLong));
  //
  static unsigned int geoToUtmZone(double latitude, double longitude);

  ///////////////////////////////////////////////////////////////////
  // Convert UTM coordinates (in meters) to geographic coordinates
  // (in radians)
  static int utmToGeo(double northing, double easting, long utmZone,
		      double *latitude, double *longitude);

};

#endif
