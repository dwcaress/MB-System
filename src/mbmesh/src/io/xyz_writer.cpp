#include "io/xyz_writer.h"

#include "io/coordinate_frame.h"

#include <cstddef>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>

namespace {

void set_error(std::string *error_message, const std::string &message) {
    if (error_message != nullptr) {
        *error_message = message;
    }
}

bool is_finite_vec3(const Vec3 &value) {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

} // namespace

bool write_local_xyz_pointcloud(const PointCloud &pointcloud, const std::string &path, std::string *error_message, bool append) {
    if (error_message != nullptr) {
        error_message->clear();
    }

    if (path.empty()) {
        set_error(error_message, "XYZ output path is empty");
        return false;
    }

    std::ofstream output(path, append ? std::ios::app : std::ios::out);
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
    std::string *error_message,
    bool append) {
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

    std::ofstream output(path, append ? std::ios::app : std::ios::out);
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

        const Vec3 ecef_point = local_to_ecef(oriented_point.point, frame);
        const Vec3 ecef_normal = normalize(local_vector_to_ecef(oriented_point.normal, frame));
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
