#pragma once

#include <filesystem>
#include <string>

#include "../data_types/geometry.h"

[[nodiscard]] bool write_mesh_glb_file(
    const std::filesystem::path &path,
    const Mesh &mesh,
    std::string *error);

[[nodiscard]] bool write_pointcloud_glb_file(
    const std::filesystem::path &path,
    const PointCloud &pointcloud,
    std::string *error);

[[nodiscard]] bool write_normal_lines_glb_file(
    const std::filesystem::path &path,
    const OrientedPointCloud &pointcloud,
    double line_length,
    std::string *error);

[[nodiscard]] bool write_origin_ray_lines_glb_file(
    const std::filesystem::path &path,
    const CollectedPointCloud &pointcloud,
    double line_length,
    std::string *error);
