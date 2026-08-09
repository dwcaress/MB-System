#pragma once

#include <vector>

#include "../math/vec3.h"

struct CollectedPoint {
    Vec3 point;
    Vec3 origin;
};

struct OrientedPoint {
    Vec3 point;
    Vec3 normal;
    Vec3 lambdas;
    OrientedPoint(const Vec3 &point, const Vec3 &normal) : point(point), normal(normal), lambdas(Vec3(0.0)) {}
};

using PointCloud = std::vector<Vec3>;
using CollectedPointCloud = std::vector<CollectedPoint>;
using OrientedPointCloud = std::vector<OrientedPoint>;

struct Bounds3D {
    Vec3 min;
    Vec3 max;

    Bounds3D() : min(Vec3(0.0)), max(Vec3(0.0)) {}
    Bounds3D(const Vec3 &min_value, const Vec3 &max_value) : min(min_value), max(max_value) {}

    [[nodiscard]] Vec3 size() const { return max - min; }
    [[nodiscard]] Vec3 center() const { return (min + max) * 0.5; }

    void expand(double padding) {
        const Vec3 offset(padding);
        min -= offset;
        max += offset;
    }

    void include(const Vec3 &p) {
        if (p.x < min.x) {
            min.x = p.x;
        }
        if (p.y < min.y) {
            min.y = p.y;
        }
        if (p.z < min.z) {
            min.z = p.z;
        }
        if (p.x > max.x) {
            max.x = p.x;
        }
        if (p.y > max.y) {
            max.y = p.y;
        }
        if (p.z > max.z) {
            max.z = p.z;
        }
    }
};

struct ReconstructionDomain {
    Bounds3D bounds;
    Vec3 origin;
    double cell_size = 1.0;
    int nx = 0;
    int ny = 0;
    int nz = 0;

    ReconstructionDomain(const std::vector<OrientedPoint> &oriented_points, double padding, double cells) {

        Bounds3D point_bounds(oriented_points[0].point, oriented_points[0].point);
        for (const OrientedPoint &oriented_point : oriented_points) {
            point_bounds.include(oriented_point.point);
        }
        point_bounds.expand(padding);

        bounds = point_bounds;
        origin = point_bounds.min;
        cell_size = cells;
        nx = static_cast<int>(std::ceil(bounds.size().x / cell_size)) + 1;
        ny = static_cast<int>(std::ceil(bounds.size().y / cell_size)) + 1;
        nz = static_cast<int>(std::ceil(bounds.size().z / cell_size)) + 1;
    }
};

struct ScalarGrid3D {
    Vec3 origin;
    double cell_size = 1.0;
    std::size_t nx = 0;
    std::size_t ny = 0;
    std::size_t nz = 0;
    std::vector<double> values;

    ScalarGrid3D() = default;

    ScalarGrid3D(Vec3 origin, double cell_size, int nx, int ny, int nz) :
        origin(origin),
        cell_size(cell_size),
        nx(nx),
        ny(ny),
        nz(nz),
        values(static_cast<std::size_t>(nx * ny * nz), 0.0) {}

    ScalarGrid3D(ReconstructionDomain domain) :
        origin(domain.origin),
        cell_size(domain.cell_size),
        nx(domain.nx), ny(domain.ny),
        nz(domain.nz),
        values(nx * ny * nz) {}

    bool contains(int x, int y, int z) const {
        return x >= 0 && x < nx && y >= 0 && y < ny && z >= 0 && z < nz;
    }

    std::size_t index(int x, int y, int z) const {
        return static_cast<std::size_t>(z) * static_cast<std::size_t>(ny) * static_cast<std::size_t>(nx) +
               static_cast<std::size_t>(y) * static_cast<std::size_t>(nx) +
               static_cast<std::size_t>(x);
    }

    double &at(int x, int y, int z) { return values[index(x, y, z)]; }
    double at(int x, int y, int z) const { return values[index(x, y, z)]; }

    Vec3 position(int x, int y, int z) const {
        return origin + Vec3(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)) * cell_size;
    }
};

struct VectorGrid3D {
    Vec3 origin;
    double cell_size = 1.0;
    int nx = 0;
    int ny = 0;
    int nz = 0;
    std::vector<Vec3> values;

    VectorGrid3D() = default;

    VectorGrid3D(ReconstructionDomain domain) :
        origin(domain.origin),
        cell_size(domain.cell_size),
        nx(domain.nx), ny(domain.ny),
        nz(domain.nz),
        values(nx * ny * nz,Vec3(0.0)) {}

    bool contains(int x, int y, int z) const {
        return x >= 0 && x < nx && y >= 0 && y < ny && z >= 0 && z < nz;
    }

    std::size_t index(int x, int y, int z) const {
        return static_cast<std::size_t>(z) * static_cast<std::size_t>(ny) * static_cast<std::size_t>(nx) +
               static_cast<std::size_t>(y) * static_cast<std::size_t>(nx) +
               static_cast<std::size_t>(x);
    }

    Vec3 &at(int x, int y, int z) { return values[index(x, y, z)]; }
    Vec3 at(int x, int y, int z) const { return values[index(x, y, z)]; }

    Vec3 position(int x, int y, int z) const {
        return origin + Vec3(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)) * cell_size;
    }
};

struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<unsigned int> indices;
};
