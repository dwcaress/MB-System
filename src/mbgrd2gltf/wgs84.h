/*--------------------------------------------------------------------
 *    The MB-system:	wgs84.h	10/24/2026
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * The WGS 84 ellipsoid, and the conversion from geodetic coordinates to
 * Earth-Centered Earth-Fixed (ECEF) coordinates, in one place for the grid
 * geometry and for the viewpoints of the page.
 */

#ifndef WGS84_H
#define WGS84_H

#include <cmath>

namespace mbgrd2gltf {

// Full double-precision pi. A value truncated to 3.1415926535 is off by 3e-11
// relative, which at ECEF magnitudes of millions of meters is a few tenths of a
// millimeter, and the error grows with longitude.
constexpr double pi = 3.14159265358979323846;

constexpr double wgs84_semi_major_axis = 6378137.0;
constexpr double wgs84_inverse_flattening = 298.257223563;

// WGS 84 geodetic coordinates (longitude and latitude in degrees, height in meters above the
// ellipsoid) to ECEF meters, in double precision.
inline void wgs84_geodetic_to_ecef(double longitude, double latitude, double altitude, double ecef[3]) {
  const double flattening = 1.0 / wgs84_inverse_flattening;
  const double e_squared = flattening * (2.0 - flattening);

  const double sin_lon = std::sin(longitude * (pi / 180.0));
  const double cos_lon = std::cos(longitude * (pi / 180.0));
  const double sin_lat = std::sin(latitude * (pi / 180.0));
  const double cos_lat = std::cos(latitude * (pi / 180.0));

  const double n = wgs84_semi_major_axis / std::sqrt(1.0 - e_squared * sin_lat * sin_lat);
  const double tmp = (n + altitude) * cos_lat;
  ecef[0] = tmp * cos_lon;
  ecef[1] = tmp * sin_lon;
  ecef[2] = (n * (1 - e_squared) + altitude) * sin_lat;
}

} // namespace mbgrd2gltf

#endif
