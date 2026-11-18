/*
 * Regression test for the --geoviewpoint values of mbgrd2gltf: parsing, and the orientation of a viewpoint
 * as a GeoViewpoint takes it. In X3DOM that orientation is relative to a local frame at the position, so
 * that no rotation looks level to the north and '1 0 0 -1.5708' looks straight down with north at the top
 * and east on the right.
 */

#include "viewpoint.h"

#include <cmath>
#include <cstdio>
#include <string>

using namespace mbgrd2gltf;

namespace {

bool check(bool condition, const std::string& message) {
  if (!condition)
    std::fprintf(stderr, "FAIL: %s\n", message.c_str());
  return condition;
}

bool contains(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

} // namespace

int main() {
  bool ok = true;
  {
  UserViewpoint v;
  std::string error;
  ok &= check(parse_user_viewpoint("-122.4,36.6,-900,-122.39,36.61,-950", &v, &error) && !v.has_orientation && v.longitude == -122.4 &&
                  v.latitude == 36.6 && v.elevation == -900.0 && v.center_longitude == -122.39 && v.center_latitude == 36.61 &&
                  v.center_elevation == -950.0,
              "six numbers are a position and a center, longitude first as --geoorigin takes them");
  ok &= check(parse_user_viewpoint("-122.4,36.6,-900,1,0,0,-1.5708,-122.39,36.61,-950", &v, &error) && v.has_orientation && v.angle == -1.5708 &&
                  v.axis[0] == 1.0 && v.center_latitude == 36.61,
              "ten numbers are a position, an orientation and a center");
  ok &= check(!parse_user_viewpoint("1,2,3,4,5", &v, &error) && contains(error, "6 numbers"), "five numbers are refused");
  ok &= check(!parse_user_viewpoint("1,2,3,4,5,x", &v, &error) && contains(error, "'x'"), "a value that is not a number is refused and named");
  ok &= check(!parse_user_viewpoint("1,2,3,,5,6", &v, &error), "an empty value is refused");
  ok &= check(!parse_user_viewpoint("1 2 3 4 5 6", &v, &error), "spaces instead of commas are refused");
  ok &= check(!parse_user_viewpoint("1,2,3,0,0,0,1,4,5,6", &v, &error) && contains(error, "zero"), "a zero axis is refused");
  ok &= check(!parse_user_viewpoint("36.6,-122.4,-900,36.61,-122.39,-950", &v, &error) && contains(error, "lon,lat,elev"),
              "latitude and longitude swapped is caught when the latitude is beyond 90");

  // The orientation a GeoViewpoint takes, in X3DOM, is relative to a local frame: no rotation looks level
  // to the north, and '1 0 0 -1.5708' looks straight down with north at the top and east on the right.
  const auto rotate = [](const double axis[3], double angle, const double in[3], double out[3]) {
      const double c = std::cos(angle), s = std::sin(angle);
      const double dot = axis[0] * in[0] + axis[1] * in[1] + axis[2] * in[2];
      const double cross[3] = {axis[1] * in[2] - axis[2] * in[1], axis[2] * in[0] - axis[0] * in[2], axis[0] * in[1] - axis[1] * in[0]};
      for (int i = 0; i < 3; i++) {
          out[i] = in[i] * c + cross[i] * s + axis[i] * dot * (1.0 - c);
      }
  };
  const double ex[3] = {1, 0, 0}, ey[3] = {0, 1, 0}, ez[3] = {0, 0, 1};
  {
      // Looking straight down from 100 m above the center is the orientation 1 0 0 -1.5708.
      UserViewpoint down;
      ok &= check(parse_user_viewpoint("0,0,100,0,0,0", &down, &error), "a look-at viewpoint parses");
      double axis[3], angle = 0.0;
      ok &= check(geoviewpoint_orientation(down, axis, &angle, &error), "it has an orientation");
      double rx[3], ry[3], rz[3];
      rotate(axis, angle, ex, rx);
      rotate(axis, angle, ey, ry);
      rotate(axis, angle, ez, rz);
      // The same rotation as 1 0 0 -1.5708: X stays, Y goes to -Z, Z goes to Y.
      ok &= check(std::fabs(rx[0] - 1.0) < 1e-9 && std::fabs(ry[2] + 1.0) < 1e-9 && std::fabs(rz[1] - 1.0) < 1e-9,
                  "looking straight down is the orientation 1 0 0 -1.5708: east on the right, north at the top");
  }
  {
      // 100 m south of the center, looking north and level: no rotation.
      UserViewpoint level;
      ok &= check(parse_user_viewpoint("0,-0.0008983,0,0,0,0", &level, &error), "a level viewpoint parses");
      double axis[3], angle = 0.0;
      ok &= check(geoviewpoint_orientation(level, axis, &angle, &error) && std::fabs(angle) < 1e-4, "looking north and level is no rotation");
  }
  {
      // Looking east and level turns the view a quarter turn to the right about up (the frame's Y).
      UserViewpoint east_looking;
      ok &= check(parse_user_viewpoint("-0.0008983,0,0,0,0,0", &east_looking, &error), "an east-looking viewpoint parses");
      double axis[3], angle = 0.0;
      ok &= check(geoviewpoint_orientation(east_looking, axis, &angle, &error), "it has an orientation");
      double rz[3];
      rotate(axis, angle, ez, rz);
      ok &= check(std::fabs(rz[0] + 1.0) < 1e-4, "looking east turns the view's back toward west (frame -X)");
  }
  {
      UserViewpoint same;
      double axis[3], angle = 0.0;
      ok &= check(parse_user_viewpoint("1,2,3,1,2,3", &same, &error) && !geoviewpoint_orientation(same, axis, &angle, &error) &&
                      contains(error, "same point"),
                  "a position equal to its center has no orientation");
  }
  {
      UserViewpoint given;
      ok &= check(parse_user_viewpoint("1,2,3,0,2,0,0.5,4,5,6", &given, &error), "ten numbers parse");
      double axis[3], angle = 0.0;
      ok &= check(geoviewpoint_orientation(given, axis, &angle, &error) && std::fabs(axis[1] - 1.0) < 1e-12 && angle == 0.5,
                  "a given orientation is used, with its axis made a unit vector");
  }
  }

  if (ok)
    std::printf("OK: viewpoint\n");
  return ok ? 0 : 1;
}
