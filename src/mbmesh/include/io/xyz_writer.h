#pragma once

#include <string>

#include "../data_types/geometry.h"
#include "../data_types/swathfile.h"

[[nodiscard]] bool write_local_xyz_pointcloud(
    const PointCloud &pointcloud,
    const std::string &path,
    std::string *error_message = nullptr);

[[nodiscard]] bool write_ecef_xyz_pointcloud(
    const PointCloud &pointcloud,
    const CoordinateFrame &frame,
    const std::string &path,
    std::string *error_message = nullptr);

[[nodiscard]] bool write_ply_oriented_pointcloud(
    const OrientedPointCloud &oriented_pointcloud,
    const CoordinateFrame &frame,
    const std::string &path,
    std::string *error_message = nullptr);
