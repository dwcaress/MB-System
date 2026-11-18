/*--------------------------------------------------------------------
 *    The MB-system:	viewpoint.cpp	10/24/2026
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

#include "viewpoint.h"
#include "wgs84.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace mbgrd2gltf {

namespace {

// The rotation with the given matrix, as an axis and an angle.
void matrix_to_axis_angle(const double r[3][3], double axis[3], double* angle) {
  const double trace = r[0][0] + r[1][1] + r[2][2];
  const double cosine = std::max(-1.0, std::min(1.0, (trace - 1.0) / 2.0));
  *angle = std::acos(cosine);
  double a[3] = {r[2][1] - r[1][2], r[0][2] - r[2][0], r[1][0] - r[0][1]};
  double length = std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
  if (length < 1.0e-9) {
    // No rotation, or a half turn: keep the answer defined.
    if (cosine > 0.0) {
      axis[0] = 0.0;
      axis[1] = 0.0;
      axis[2] = 1.0;
      *angle = 0.0;
      return;
    }
    // A half turn about the axis with the largest diagonal term.
    int k = 0;
    for (int i = 1; i < 3; i++) {
      if (r[i][i] > r[k][k]) {
        k = i;
      }
    }
    double v[3];
    v[k] = std::sqrt(std::max(0.0, (r[k][k] + 1.0) / 2.0));
    for (int i = 0; i < 3; i++) {
      if (i != k) {
        v[i] = (v[k] > 0.0) ? (r[i][k] + r[k][i]) / (4.0 * v[k]) : 0.0;
      }
    }
    const double n = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    for (int i = 0; i < 3; i++) {
      axis[i] = n > 0.0 ? v[i] / n : (i == 2 ? 1.0 : 0.0);
    }
    return;
  }
  for (int i = 0; i < 3; i++) {
    axis[i] = a[i] / length;
  }
}

void normalize3(double v[3]) {
  const double length = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  if (length > 0.0) {
    for (int i = 0; i < 3; i++) {
      v[i] /= length;
    }
  }
}

void cross3(const double a[3], const double b[3], double out[3]) {
  out[0] = a[1] * b[2] - a[2] * b[1];
  out[1] = a[2] * b[0] - a[0] * b[2];
  out[2] = a[0] * b[1] - a[1] * b[0];
}

} // namespace

bool parse_user_viewpoint(const std::string& text, UserViewpoint* viewpoint, std::string* error) {
  std::vector<double> numbers;
  std::size_t start = 0;
  while (true) {
    const std::size_t comma = text.find(',', start);
    const std::string field = text.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
    char *end = nullptr;
    const double value = std::strtod(field.c_str(), &end);
    if (field.empty() || end != field.c_str() + field.size() || !std::isfinite(value)) {
      if (error != nullptr) {
        *error = "viewpoint value '" + field + "' is not a number in '" + text + "'";
      }
      return false;
    }
    numbers.push_back(value);
    if (comma == std::string::npos) {
      break;
    }
    start = comma + 1;
  }

  if (numbers.size() != 6 && numbers.size() != 10) {
    if (error != nullptr) {
      *error = "a viewpoint takes 6 numbers, lon,lat,elev,clon,clat,celev, or 10, lon,lat,elev,ax,ay,az,angle,clon,clat,celev, "
          "with no spaces; got " + std::to_string(numbers.size()) + " in '" + text + "'";
    }
    return false;
  }

  UserViewpoint result;
  result.longitude = numbers[0];
  result.latitude = numbers[1];
  result.elevation = numbers[2];
  result.center_longitude = numbers[numbers.size() - 3];
  result.center_latitude = numbers[numbers.size() - 2];
  result.center_elevation = numbers[numbers.size() - 1];
  if (std::fabs(result.latitude) > 90.0 || std::fabs(result.center_latitude) > 90.0) {
    if (error != nullptr) {
      *error = "a latitude of the viewpoint '" + text + "' is beyond 90 degrees; the order is lon,lat,elev";
    }
    return false;
  }
  if (numbers.size() == 10) {
    result.has_orientation = true;
    for (int i = 0; i < 3; i++) {
      result.axis[i] = numbers[3 + static_cast<std::size_t>(i)];
    }
    result.angle = numbers[6];
    if (result.axis[0] * result.axis[0] + result.axis[1] * result.axis[1] + result.axis[2] * result.axis[2] <= 0.0) {
      if (error != nullptr) {
        *error = "the orientation axis of the viewpoint '" + text + "' is zero";
      }
      return false;
    }
  }
  *viewpoint = result;
  return true;
}

bool geoviewpoint_orientation(const UserViewpoint& viewpoint, double axis[3], double* angle, std::string* error) {
  if (viewpoint.has_orientation) {
    for (int i = 0; i < 3; i++)
      axis[i] = viewpoint.axis[i];
    normalize3(axis);
    *angle = viewpoint.angle;
    return true;
  }

  double position[3], center[3];
  wgs84_geodetic_to_ecef(viewpoint.longitude, viewpoint.latitude, viewpoint.elevation, position);
  wgs84_geodetic_to_ecef(viewpoint.center_longitude, viewpoint.center_latitude, viewpoint.center_elevation, center);

  // The camera looks along its -Z with +Y at the top and X to the right: Z points back from the view.
  double back[3] = {position[0] - center[0], position[1] - center[1], position[2] - center[2]};
  if (back[0] * back[0] + back[1] * back[1] + back[2] * back[2] < 1.0e-12) {
    if (error != nullptr)
      *error = "the viewpoint and its center of rotation are the same point";
    return false;
  }
  normalize3(back);

  // The local east, north and up axes at the position, in ECEF.
  const double phi = viewpoint.latitude * pi / 180.0;
  const double lambda = viewpoint.longitude * pi / 180.0;
  const double east[3] = {-std::sin(lambda), std::cos(lambda), 0.0};
  const double north[3] = {-std::sin(phi) * std::cos(lambda), -std::sin(phi) * std::sin(lambda), std::cos(phi)};
  const double up[3] = {std::cos(phi) * std::cos(lambda), std::cos(phi) * std::sin(lambda), std::sin(phi)};

  double right[3];
  cross3(up, back, right);
  if (right[0] * right[0] + right[1] * right[1] + right[2] * right[2] < 1.0e-12)
    cross3(north, back, right); // looking straight up or down
  normalize3(right);
  double top[3];
  cross3(back, right, top);

  // X3DOM's local frame for a GeoViewpoint has X east, Y up and Z south (minus north). The orientation is
  // the rotation from that frame to the camera: the frame's transpose times the camera's axes.
  const double frame[3][3] = {{east[0], east[1], east[2]}, {up[0], up[1], up[2]}, {-north[0], -north[1], -north[2]}};
  const double camera[3][3] = {{right[0], top[0], back[0]}, {right[1], top[1], back[1]}, {right[2], top[2], back[2]}};
  double o[3][3];
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++)
      o[i][j] = frame[i][0] * camera[0][j] + frame[i][1] * camera[1][j] + frame[i][2] * camera[2][j];
  }
  matrix_to_axis_angle(o, axis, angle);

  // The same rotation has two spellings, an axis and angle or the opposite axis and the opposite angle.
  // Choose the one whose largest axis component is positive, so that looking straight down is written
  // as 1 0 0 -1.5708, and write values that are zero to the digits written as zero.
  int largest = 0;
  for (int i = 1; i < 3; i++) {
    if (std::fabs(axis[i]) > std::fabs(axis[largest]))
      largest = i;
  }
  if (axis[largest] < 0.0) {
    for (int i = 0; i < 3; i++)
      axis[i] = -axis[i];
    *angle = -*angle;
  }
  for (int i = 0; i < 3; i++) {
    if (std::fabs(axis[i]) < 5.0e-9)
      axis[i] = 0.0;
  }
  return true;
}

} // namespace mbgrd2gltf
