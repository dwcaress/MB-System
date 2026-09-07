#include "io/xyz_writer.h"

#include <cstddef>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>

namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double degrees_to_radians = pi / 180.0;
constexpr double wgs84_semi_major_axis = 6378137.0;
constexpr double wgs84_first_eccentricity_squared = 6.6943799901413165e-3;

void set_error(std::string *error_message, const std::string &message) {
    if (error_message != nullptr) {
        *error_message = message;
    }
}

bool is_finite_vec3(const Vec3 &value) {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool is_valid_frame(const CoordinateFrame &frame) {
    return std::isfinite(frame.origin.longitude) &&
           std::isfinite(frame.origin.latitude) &&
           std::isfinite(frame.origin.elevation) &&
           std::isfinite(frame.meters_per_degree_lon) &&
           std::isfinite(frame.meters_per_degree_lat) &&
           frame.meters_per_degree_lon > 0.0 &&
           frame.meters_per_degree_lat > 0.0;
}

GeodeticPosition local_to_geodetic(const Vec3 &point, const CoordinateFrame &frame) {
    GeodeticPosition position;
    position.longitude = frame.origin.longitude + point.x / frame.meters_per_degree_lon;
    position.latitude = frame.origin.latitude + point.y / frame.meters_per_degree_lat;
    position.elevation = frame.origin.elevation + point.z;
    return position;
}

Vec3 geodetic_to_ecef(const GeodeticPosition &position) {
    const double lon = position.longitude * degrees_to_radians;
    const double lat = position.latitude * degrees_to_radians;
    const double sin_lat = std::sin(lat);
    const double cos_lat = std::cos(lat);
    const double sin_lon = std::sin(lon);
    const double cos_lon = std::cos(lon);
    const double prime_vertical_radius =
        wgs84_semi_major_axis / std::sqrt(1.0 - wgs84_first_eccentricity_squared * sin_lat * sin_lat);

    return Vec3(
        (prime_vertical_radius + position.elevation) * cos_lat * cos_lon,
        (prime_vertical_radius + position.elevation) * cos_lat * sin_lon,
        ((1.0 - wgs84_first_eccentricity_squared) * prime_vertical_radius + position.elevation) * sin_lat);
}

Vec3 local_to_ecef(const Vec3 &point, const CoordinateFrame &frame) {
    return geodetic_to_ecef(local_to_geodetic(point, frame));
}

Vec3 local_enu_vector_to_ecef(const Vec3 &vector, const GeodeticPosition &position) {
    const double lon = position.longitude * degrees_to_radians;
    const double lat = position.latitude * degrees_to_radians;
    const double sin_lat = std::sin(lat);
    const double cos_lat = std::cos(lat);
    const double sin_lon = std::sin(lon);
    const double cos_lon = std::cos(lon);

    return Vec3(
        -sin_lon * vector.x - sin_lat * cos_lon * vector.y + cos_lat * cos_lon * vector.z,
        cos_lon * vector.x - sin_lat * sin_lon * vector.y + cos_lat * sin_lon * vector.z,
        cos_lat * vector.y + sin_lat * vector.z);
}

} // namespace

bool write_local_xyz_pointcloud(const PointCloud &pointcloud, const std::string &path, std::string *error_message) {
    if (error_message != nullptr) {
        error_message->clear();
    }

    if (path.empty()) {
        set_error(error_message, "XYZ output path is empty");
        return false;
    }

    std::ofstream output(path);
    if (!output) {
        set_error(error_message, "Unable to create XYZ file: " + path);
        return false;
    }

    output << std::setprecision(17);
    for (std::size_t i = 0; i < pointcloud.size(); i++) {
        const Vec3 &point = pointcloud[i];
        if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) {
            set_error(error_message, "Non-finite point value at index " + std::to_string(i));
            return false;
        }

        output << point.x << ' ' << point.y << ' ' << point.z << '\n';
        if (!output) {
            set_error(error_message, "Error while writing XYZ file: " + path);
            return false;
        }
    }

    return true;
}

bool write_ecef_xyz_pointcloud(
    const PointCloud &pointcloud,
    const CoordinateFrame &frame,
    const std::string &path,
    std::string *error_message) {
    if (error_message != nullptr) {
        error_message->clear();
    }

    if (path.empty()) {
        set_error(error_message, "ECEF XYZ output path is empty");
        return false;
    }

    if (!is_valid_frame(frame)) {
        set_error(error_message, "Coordinate frame is invalid for ECEF XYZ output");
        return false;
    }

    std::ofstream output(path);
    if (!output) {
        set_error(error_message, "Unable to create ECEF XYZ file: " + path);
        return false;
    }

    output << std::setprecision(17);
    for (std::size_t i = 0; i < pointcloud.size(); i++) {
        const Vec3 &point = pointcloud[i];
        if (!is_finite_vec3(point)) {
            set_error(error_message, "Non-finite point value at index " + std::to_string(i));
            return false;
        }

        const Vec3 ecef_point = local_to_ecef(point, frame);
        output << ecef_point.x << ' ' << ecef_point.y << ' ' << ecef_point.z << '\n';
        if (!output) {
            set_error(error_message, "Error while writing ECEF XYZ file: " + path);
            return false;
        }
    }

    return true;
}

bool write_ply_oriented_pointcloud(
    const OrientedPointCloud &oriented_pointcloud,
    const CoordinateFrame &frame,
    const std::string &path,
    std::string *error_message) {
    if (error_message != nullptr) {
        error_message->clear();
    }

    if (path.empty()) {
        set_error(error_message, "Oriented point cloud PLY output path is empty");
        return false;
    }

    if (!is_valid_frame(frame)) {
        set_error(error_message, "Coordinate frame is invalid for oriented point cloud PLY output");
        return false;
    }

    std::ofstream output(path);
    if (!output) {
        set_error(error_message, "Unable to create oriented point cloud PLY file: " + path);
        return false;
    }

    output << "ply\n"
           << "format ascii 1.0\n"
           << "element vertex " << oriented_pointcloud.size() << '\n'
           << "property double x\n"
           << "property double y\n"
           << "property double z\n"
           << "property double nx\n"
           << "property double ny\n"
           << "property double nz\n"
           << "property double lambda0\n"
           << "property double lambda1\n"
           << "property double lambda2\n"
           << "end_header\n";

    if (!output) {
        set_error(error_message, "Error while writing oriented point cloud PLY header: " + path);
        return false;
    }

    output << std::setprecision(17);
    for (std::size_t i = 0; i < oriented_pointcloud.size(); i++) {
        const OrientedPoint &oriented_point = oriented_pointcloud[i];
        if (!is_finite_vec3(oriented_point.point)) {
            set_error(error_message, "Non-finite oriented point value at index " + std::to_string(i));
            return false;
        }
        if (!is_finite_vec3(oriented_point.normal)) {
            set_error(error_message, "Non-finite oriented point normal at index " + std::to_string(i));
            return false;
        }
        if (!is_finite_vec3(oriented_point.lambdas)) {
            set_error(error_message, "Non-finite oriented point lambda value at index " + std::to_string(i));
            return false;
        }

        const GeodeticPosition geodetic_point = local_to_geodetic(oriented_point.point, frame);
        const Vec3 ecef_point = geodetic_to_ecef(geodetic_point);
        const Vec3 ecef_normal = normalize(local_enu_vector_to_ecef(oriented_point.normal, geodetic_point));
        output << ecef_point.x << ' ' << ecef_point.y << ' ' << ecef_point.z << ' '
               << ecef_normal.x << ' ' << ecef_normal.y << ' ' << ecef_normal.z << ' '
               << oriented_point.lambdas.x << ' ' << oriented_point.lambdas.y << ' ' << oriented_point.lambdas.z << '\n';
        if (!output) {
            set_error(error_message, "Error while writing oriented point cloud PLY file: " + path);
            return false;
        }
    }

    return true;
}
