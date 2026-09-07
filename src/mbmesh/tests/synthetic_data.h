#include "math/vec3.h"
#include "data_types/geometry.h"
#include "perlin_noise.h"

using SdfFunction = double (*)(double x, double y, double z);

constexpr double normal_step = 1e-3;
constexpr double pi = 3.14159265358979323846;

double plane_sdf(double x, double y, double z) {
    (void)x;
    (void)y;
    return z;
}

double tilted_plane_sdf(double x, double y, double z) {
    return z - (0.20 * x - 0.10 * y);
}

double perturbed_plane_sdf(double x, double y, double z) {
    const double ripple = 0.5 * perlin_noise(x * 0.35, y * 0.35, 0.0);
    const double wave = 0.1 * std::sin(x * 0.5) * std::cos(y * 0.35);
    return z - ripple - wave;
}

double sphere_sdf(double x, double y, double z) {
    constexpr double radius = 1.0;
    return Vec3(x, y, z).length() - radius;
}

double torus_sdf(double x, double y, double z) {
    constexpr double major_radius = 1.0;
    constexpr double minor_radius = 0.25;
    const double radial_distance = std::sqrt(x * x + y * y) - major_radius;
    return std::sqrt(radial_distance * radial_distance + z * z) - minor_radius;
}

Vec3 sdf_normal(SdfFunction sdf, const Vec3 &p) {
    const double dx = sdf(p.x + normal_step, p.y, p.z) - sdf(p.x - normal_step, p.y, p.z);
    const double dy = sdf(p.x, p.y + normal_step, p.z) - sdf(p.x, p.y - normal_step, p.z);
    const double dz = sdf(p.x, p.y, p.z + normal_step) - sdf(p.x, p.y, p.z - normal_step);
    return normalize(Vec3(dx, dy, dz));
}

PointCloud sample_sdf_surface_points(SdfFunction sdf, const Vec3 &min_corner, const Vec3 &max_corner, double interval, double surface_threshold) {

    PointCloud pointcloud;
    if (sdf == nullptr || interval <= 0.0 || surface_threshold < 0.0) {
        return pointcloud;
    }

    for (double z = min_corner.z; z <= max_corner.z; z += interval) {
        for (double y = min_corner.y; y <= max_corner.y; y += interval) {
            for (double x = min_corner.x; x <= max_corner.x; x += interval) {
                const Vec3 p(x, y, z);
                if (std::fabs(sdf(x, y, z)) <= surface_threshold) {
                    pointcloud.push_back(p);
                }
            }
        }
    }

    return pointcloud;
}

OrientedPointCloud sample_sdf_surface_oriented_points(SdfFunction sdf, const Vec3 &min_corner, const Vec3 &max_corner, double interval, double surface_threshold) {
    OrientedPointCloud pointcloud;
    if (sdf == nullptr || interval <= 0.0 || surface_threshold < 0.0) {
        return pointcloud;
    }

    for (double z = min_corner.z; z <= max_corner.z; z += interval) {
        for (double y = min_corner.y; y <= max_corner.y; y += interval) {
            for (double x = min_corner.x; x <= max_corner.x; x += interval) {
                const Vec3 p(x, y, z);
                if (std::fabs(sdf(x, y, z)) <= surface_threshold) {
                    pointcloud.push_back(OrientedPoint(p, sdf_normal(sdf, p)));
                }
            }
        }
    }

    return pointcloud;
}

ScalarGrid3D sample_sdf_grid(SdfFunction sdf, int grid_size, double cell_size) {

    const double extent = static_cast<double>(grid_size - 1) * cell_size;
    const Vec3 origin(-0.5 * extent, -0.5 * extent, -0.5 * extent);
    ScalarGrid3D grid(origin, cell_size, grid_size, grid_size, grid_size);

    for (int z = 0; z < grid.nz; z++) {
        for (int y = 0; y < grid.ny; y++) {
            for (int x = 0; x < grid.nx; x++) {
                const Vec3 position = grid.position(x, y, z);
                grid.at(x, y, z) = sdf(position.x, position.y, position.z);
            }
        }
    }

    return grid;
}
