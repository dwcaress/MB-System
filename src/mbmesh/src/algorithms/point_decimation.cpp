#include "algorithms/point_decimation.h"

#include <cmath>

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

struct PointDecimator::Impl {
    Vec3 cloud_origin;
    double cell_size;
    std::map<GridKey, CellAccumulator> cells;
};

PointDecimator::PointDecimator(const Vec3 &cloud_origin, double cell_size) : impl_(new Impl{cloud_origin, cell_size, {}}) {}

PointDecimator::~PointDecimator() {
    delete impl_;
}

void PointDecimator::add(const CollectedPoint &point) {
    CellAccumulator &cell = impl_->cells[GridKey(point.point, impl_->cloud_origin, impl_->cell_size)];
    cell.point_sum += point.point;
    cell.origin_sum += point.origin;
    cell.count++;
}

std::size_t PointDecimator::cell_count() const {
    return impl_->cells.size();
}

void PointDecimator::for_each(const std::function<void(const CollectedPoint &)> &visitor) const {
    for (const auto &entry : impl_->cells) {
        const CellAccumulator &cell = entry.second;
        if (cell.count > 0) {
            visitor(CollectedPoint{cell.point_sum / static_cast<double>(cell.count), cell.origin_sum / static_cast<double>(cell.count)});
        }
    }
}

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

    PointDecimator decimator(bounds.min, options.cell_size);
    for (const CollectedPoint &collected_point : collected_points) {
        decimator.add(collected_point);
    }
    CollectedPointCloud().swap(collected_points);   // the input is no longer needed

    CollectedPointCloud decimated_points;
    decimated_points.reserve(decimator.cell_count());
    decimator.for_each([&](const CollectedPoint &point) { decimated_points.push_back(point); });
    return decimated_points;
}
