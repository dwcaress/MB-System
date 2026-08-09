#include "algorithms/point_decimation.h"

struct GridKey {
    int x;
    int y;
    int z;

    GridKey(Vec3 point, Vec3 origin, double cell_size) {
        x = static_cast<int>(std::floor((point.x - origin.x) / cell_size));
        y = static_cast<int>(std::floor((point.y - origin.y) / cell_size));
        z = static_cast<int>(std::floor((point.z - origin.z) / cell_size));
    }

    bool operator<(const GridKey &other) const noexcept {
        if (z != other.z) return z < other.z;
        if (y != other.y) return y < other.y;
        return x < other.x;
    }
};

struct CellAccumulator {
    Vec3 point_sum;
    Vec3 origin_sum;
    unsigned int count = 0;
};

CollectedPointCloud point_decimation(CollectedPointCloud collected_points, PointDecimationOptions options) {

    if (!options.decimate ||
        collected_points.empty() ||
        options.cell_size <= 0.0) {
        return collected_points;
    }

    Bounds3D bounds(collected_points[0].point, collected_points[0].point);
    for (const CollectedPoint &collected_point : collected_points) {
        bounds.include(collected_point.point);
    }

    const Vec3 cloud_origin = bounds.min;

    std::map<GridKey, CellAccumulator> cells;
    for (const CollectedPoint &collected_point : collected_points) {
        CellAccumulator &cell = cells[GridKey(collected_point.point, cloud_origin, options.cell_size)];
        cell.point_sum += collected_point.point;
        cell.origin_sum += collected_point.origin;
        cell.count++;
    }

    CollectedPointCloud decimated_points;
    decimated_points.reserve(cells.size());

    for (const auto &entry : cells) {
        const CellAccumulator &cell = entry.second;
        if (cell.count > 0) {
            CollectedPoint collected_point {
                cell.point_sum / static_cast<double>(cell.count),
                cell.origin_sum / static_cast<double>(cell.count)
            };
            decimated_points.push_back(collected_point);
        }
    }

    return decimated_points;
}
