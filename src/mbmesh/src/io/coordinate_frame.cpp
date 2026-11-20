#include "io/coordinate_frame.h"

#include <cmath>

namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double degrees_to_radians = pi / 180.0;
constexpr double radians_to_degrees = 180.0 / pi;
constexpr double wgs84_semi_major_axis = 6378137.0;
constexpr double wgs84_first_eccentricity_squared = 6.6943799901413165e-3;

[[nodiscard]] bool is_finite_vec3(const Vec3 &value) {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

} // namespace

Vec3 geodetic_to_ecef(const GeodeticPosition &position) {
    const double lon = position.longitude * degrees_to_radians;
    const double lat = position.latitude * degrees_to_radians;
    const double sin_lat = std::sin(lat);
    const double cos_lat = std::cos(lat);
    const double prime_vertical_radius =
        wgs84_semi_major_axis / std::sqrt(1.0 - wgs84_first_eccentricity_squared * sin_lat * sin_lat);

    return Vec3(
        (prime_vertical_radius + position.elevation) * cos_lat * std::cos(lon),
        (prime_vertical_radius + position.elevation) * cos_lat * std::sin(lon),
        ((1.0 - wgs84_first_eccentricity_squared) * prime_vertical_radius + position.elevation) * sin_lat);
}

GeodeticPosition ecef_to_geodetic(const Vec3 &ecef) {
    const double p = std::hypot(ecef.x, ecef.y);

    GeodeticPosition position;
    position.longitude = std::atan2(ecef.y, ecef.x) * radians_to_degrees;

    // Fixed-point iteration on latitude; converges to well under a millimeter
    // in a handful of steps for any point near the Earth's surface.
    double lat = std::atan2(ecef.z, p * (1.0 - wgs84_first_eccentricity_squared));
    double height = 0.0;
    for (int iteration = 0; iteration < 8; iteration++) {
        const double sin_lat = std::sin(lat);
        const double cos_lat = std::cos(lat);
        const double prime_vertical_radius =
            wgs84_semi_major_axis / std::sqrt(1.0 - wgs84_first_eccentricity_squared * sin_lat * sin_lat);

        // Use whichever form is well conditioned for this latitude.
        height = std::fabs(cos_lat) > 0.7071
                     ? p / cos_lat - prime_vertical_radius
                     : ecef.z / sin_lat - prime_vertical_radius * (1.0 - wgs84_first_eccentricity_squared);

        lat = std::atan2(ecef.z, p * (1.0 - wgs84_first_eccentricity_squared * prime_vertical_radius / (prime_vertical_radius + height)));
    }

    position.latitude = lat * radians_to_degrees;
    position.elevation = height;
    return position;
}

CoordinateFrame coordinate_frame_from_origin(const GeodeticPosition &origin) {
    const double lon = origin.longitude * degrees_to_radians;
    const double lat = origin.latitude * degrees_to_radians;
    const double sin_lon = std::sin(lon);
    const double cos_lon = std::cos(lon);
    const double sin_lat = std::sin(lat);
    const double cos_lat = std::cos(lat);

    CoordinateFrame frame;
    frame.origin = origin;
    frame.origin_ecef = geodetic_to_ecef(origin);
    frame.east = Vec3(-sin_lon, cos_lon, 0.0);
    frame.north = Vec3(-sin_lat * cos_lon, -sin_lat * sin_lon, cos_lat);
    frame.up = Vec3(cos_lat * cos_lon, cos_lat * sin_lon, sin_lat);
    return frame;
}

Vec3 ecef_to_local(const Vec3 &ecef, const CoordinateFrame &frame) {
    const Vec3 offset = ecef - frame.origin_ecef;
    return Vec3(dot(offset, frame.east), dot(offset, frame.north), dot(offset, frame.up));
}

Vec3 local_to_ecef(const Vec3 &local, const CoordinateFrame &frame) {
    return frame.origin_ecef + frame.east * local.x + frame.north * local.y + frame.up * local.z;
}

Vec3 geodetic_to_local(const GeodeticPosition &position, const CoordinateFrame &frame) {
    return ecef_to_local(geodetic_to_ecef(position), frame);
}

GeodeticPosition local_to_geodetic(const Vec3 &local, const CoordinateFrame &frame) {
    return ecef_to_geodetic(local_to_ecef(local, frame));
}

Vec3 local_vector_to_ecef(const Vec3 &vector, const CoordinateFrame &frame) {
    return frame.east * vector.x + frame.north * vector.y + frame.up * vector.z;
}

bool is_valid_frame(const CoordinateFrame &frame) {
    return std::isfinite(frame.origin.longitude) &&
           std::isfinite(frame.origin.latitude) &&
           std::isfinite(frame.origin.elevation) &&
           is_finite_vec3(frame.origin_ecef) &&
           is_finite_vec3(frame.east) &&
           is_finite_vec3(frame.north) &&
           is_finite_vec3(frame.up);
}
