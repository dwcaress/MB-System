#pragma once

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
