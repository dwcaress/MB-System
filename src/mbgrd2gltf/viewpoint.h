/*--------------------------------------------------------------------
 *    The MB-system:	viewpoint.h	10/24/2026
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Viewpoints that the user supplies with --geoviewpoint for the page that --html
 * writes. As in mbmesh they are geodetic, so that they do not depend on the
 * GeoOrigin or the grid.
 */

#ifndef VIEWPOINT_H
#define VIEWPOINT_H

#include <string>

namespace mbgrd2gltf {

// A GeoViewpoint the user supplies with --geoviewpoint: where the camera is and where it looks, as WGS 84
// geodetic coordinates, so that it does not depend on the GeoOrigin, the area or the grid, and can be
// used again in another run. The orientation, if given, is that of a GeoViewpoint: relative to the
// local frame at the position, as X3DOM takes it ('1 0 0 -1.5708' looks straight down with north at the
// top and east on the right). Heights are above the ellipsoid, like the GeoOrigin's elevation.
struct UserViewpoint {
  double longitude = 0.0;
  double latitude = 0.0;
  double elevation = 0.0;
  double center_longitude = 0.0;   // the center of rotation, and the point looked at without an orientation
  double center_latitude = 0.0;
  double center_elevation = 0.0;
  bool has_orientation = false;    // otherwise the camera looks from the position toward the center, with up at the top
  double axis[3] = {0.0, 0.0, 1.0};
  double angle = 0.0;
};

// Parses the value of --geoviewpoint, which is in the order of --geoorigin, longitude first: six numbers,
// lon,lat,elev,clon,clat,celev, the camera position and the point looked at; or ten,
// lon,lat,elev,ax,ay,az,angle,clon,clat,celev, with the orientation as an axis and an angle between the
// position and the center. Commas separate the numbers, with no spaces. The Copy GeoViewpoint button of the
// page gives the ten-number form. Returns false and says why in error.
[[nodiscard]] bool parse_user_viewpoint(const std::string& text, UserViewpoint* viewpoint, std::string* error);

// The orientation of a user viewpoint as a GeoViewpoint takes it, an axis and an angle relative to the local
// frame at the position: the one given, or, without one, the orientation that looks from the position toward
// the center with the top of the view toward local up, or toward north when looking straight up or down.
// Returns false if the position and center are the same point.
[[nodiscard]] bool geoviewpoint_orientation(const UserViewpoint& viewpoint, double axis[3], double* angle, std::string* error);

} // namespace mbgrd2gltf

#endif
