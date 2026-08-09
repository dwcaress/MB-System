#include "synthetic_data.h"
#include "algorithms/normal_estimation.h"
#include "io/glb_writer.h"
#include "math/kdtree.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>

namespace {

[[nodiscard]] double perturbed_plane_height(double x, double y) {
    return -perturbed_plane_sdf(x, y, 0.0);
}

[[nodiscard]] CollectedPointCloud sample_collected_perturbed_plane(
    double half_extent,
    double spacing,
    const Vec3 &sensor_origin)
{
    CollectedPointCloud pointcloud;
    if (half_extent <= 0.0 || spacing <= 0.0) {
        return pointcloud;
    }

    for (double y = -half_extent; y <= half_extent + 0.5 * spacing; y += spacing) {
        for (double x = -half_extent; x <= half_extent + 0.5 * spacing; x += spacing) {
            const Vec3 point(x, y, perturbed_plane_height(x, y));
            pointcloud.push_back(CollectedPoint{point, sensor_origin});
        }
    }

    return pointcloud;
}

[[nodiscard]] bool validate_normals_face_origin(
    const CollectedPointCloud &collected_points,
    const OrientedPointCloud &oriented_points)
{
    if (collected_points.size() != oriented_points.size()) {
        std::fprintf(stderr,
                     "Point count mismatch: %zu collected, %zu oriented\n",
                     collected_points.size(),
                     oriented_points.size());
        return false;
    }

    double min_alignment = 1.0;
    double average_alignment = 0.0;
    std::size_t rejected = 0;

    for (std::size_t i = 0; i < oriented_points.size(); ++i) {
        const Vec3 to_origin = normalize(collected_points[i].origin - collected_points[i].point);
        const Vec3 normal = normalize(oriented_points[i].normal);
        const double alignment = dot(normal, to_origin);
        min_alignment = std::min(min_alignment, alignment);
        average_alignment += alignment;

        if (alignment <= 0.0) {
            ++rejected;
        }
    }

    average_alignment /= static_cast<double>(oriented_points.size());
    std::printf("Normal/source alignment: min %.3f, average %.3f, rejected %zu/%zu\n",
                min_alignment,
                average_alignment,
                rejected,
                oriented_points.size());

    return rejected == 0;
}

[[nodiscard]] bool write_visualization_glbs(
    const std::filesystem::path &output_dir,
    const CollectedPointCloud &collected_points,
    const OrientedPointCloud &oriented_points)
{
    std::string error;

    PointCloud points;
    points.reserve(oriented_points.size());
    for (const OrientedPoint &oriented_point : oriented_points) {
        points.push_back(oriented_point.point);
    }

    const std::filesystem::path pointcloud_path = output_dir / "normal-estimation-points.glb";
    if (!write_pointcloud_glb_file(pointcloud_path, points, &error)) {
        std::fprintf(stderr, "Failed to write point cloud GLB: %s\n", error.c_str());
        return false;
    }

    const std::filesystem::path normals_path = output_dir / "normal-estimation-orientedPointCloud.glb";
    if (!write_normal_lines_glb_file(normals_path, oriented_points, 0.25, &error)) {
        std::fprintf(stderr, "Failed to write oriented point cloud GLB: %s\n", error.c_str());
        return false;
    }

    const std::filesystem::path origins_path = output_dir / "normal-estimation-origin-rays.glb";
    if (!write_origin_ray_lines_glb_file(origins_path, collected_points, 0.25, &error)) {
        std::fprintf(stderr, "Failed to write origin ray GLB: %s\n", error.c_str());
        return false;
    }

    std::printf("Wrote %s\n", pointcloud_path.string().c_str());
    std::printf("Wrote %s\n", normals_path.string().c_str());
    std::printf("Wrote %s\n", origins_path.string().c_str());
    return true;
}

[[nodiscard]] bool validate_bounded_radius_search() {
    std::vector<Vec3> points;
    for (int i = 0; i < 10; i++) {
        points.push_back(Vec3(static_cast<double>(i), 0.0, 0.0));
    }

    const KDTree tree(points);
    const std::vector<KDTree::Neighbor> bounded =
        tree.bounded_radius_search_squared(Vec3(4.0, 0.0, 0.0), 9.0, 3, 4);
    const std::vector<KDTree::Neighbor> full =
        tree.radius_search_squared(Vec3(4.0, 0.0, 0.0), 9.0, 4);

    if (bounded.size() != 3 || full.size() < bounded.size()) {
        std::fprintf(stderr, "Unexpected bounded radius search result size\n");
        return false;
    }

    for (std::size_t i = 0; i < bounded.size(); i++) {
        if (bounded[i].index != full[i].index ||
            std::fabs(bounded[i].distance_squared - full[i].distance_squared) > vec3_epsilon) {
            std::fprintf(stderr, "Bounded radius search did not match nearest full-radius neighbors\n");
            return false;
        }
    }

    return true;
}

} // namespace

int main() {
    const std::filesystem::path output_dir = "src/mbmesh/output";
    std::filesystem::create_directories(output_dir);

    if (!validate_bounded_radius_search()) {
        return 1;
    }

    const Vec3 sensor_origin(0.0, 0.0, 3.0);
    const CollectedPointCloud collected_points =
        sample_collected_perturbed_plane(2.4, 0.12, sensor_origin);

    if (collected_points.empty()) {
        std::fprintf(stderr, "Expected non-empty collected point cloud\n");
        return 1;
    }

    NormalEstimationOptions options;
    options.k = 16;
    options.search_radius = 0.60;
    const OrientedPointCloud oriented_points = normal_estimation(collected_points, options);

    std::printf("Perturbed plane normal estimation: %zu points\n",
                oriented_points.size());

    if (oriented_points.empty()) {
        std::fprintf(stderr, "Expected non-empty oriented point cloud\n");
        return 1;
    }

    if (!validate_normals_face_origin(collected_points, oriented_points)) {
        std::fprintf(stderr, "Expected all normals to face the synthetic origin\n");
        return 1;
    }

    if (!write_visualization_glbs(output_dir, collected_points, oriented_points)) {
        return 1;
    }

    return 0;
}

/*
g++ -std=c++17 -I src/mbmesh/include -I src/mbgrd2gltf \
  src/mbmesh/tests/test_normal_estimation.cpp \
  src/mbmesh/src/algorithms/normal_estimation.cpp \
  src/mbmesh/src/io/glb_writer.cpp \
  -o /tmp/mbmesh_test_normal_estimation

/tmp/mbmesh_test_normal_estimation
*/
