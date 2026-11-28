#pragma once

#include <functional>
#include <map>

#include "../data_types/geometry.h"

struct PointDecimationOptions {
    // Enable voxel-grid averaging before normal estimation and reconstruction.
    bool decimate = false;

    // Edge length, in point-cloud coordinate units, of each averaging voxel.
    // Smaller values preserve more detail but increase every downstream cost.
    double cell_size = 0.25;
};

CollectedPointCloud point_decimation(CollectedPointCloud collected_points, PointDecimationOptions options);

// The same averaging, for points that arrive a piece at a time: add() each point, then visit the averages. Memory
// grows with the number of occupied voxels, not the number of points. cloud_origin is the corner the voxels are
// counted from (point_decimation() uses the minimum corner of the cloud's bounds).
class PointDecimator {
public:
    PointDecimator(const Vec3 &cloud_origin, double cell_size);
    ~PointDecimator();

    void add(const CollectedPoint &point);
    [[nodiscard]] std::size_t cell_count() const;
    // Calls the visitor with the average of each occupied voxel, in voxel order.
    void for_each(const std::function<void(const CollectedPoint &)> &visitor) const;

private:
    struct Impl;
    Impl *impl_;
};
