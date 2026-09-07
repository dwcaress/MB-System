#pragma once

#include <cstddef>

#include "../data_types/geometry.h"

#include "../include/math/eigen.h"
#include "../include/math/kdtree.h"
#include "../include/math/mat3.h"

struct NormalEstimationOptions {

    // Maximum number of neighboring samples used with the query point for the
    // local PCA plane fit. In radius mode this caps the search to the nearest
    // samples inside search_radius so dense surveys cannot produce unbounded
    // per-point neighborhoods.
    std::size_t k = 12;

    // Physical search radius used to gather neighbors. A value of zero falls
    // back to k-nearest neighbors only.
    double search_radius = 0.0;

    // Minimum neighboring samples required before trusting the radius-based
    // neighborhood. If the search radius is too sparse, k-nearest is used as a
    // fallback.
    std::size_t minimum_neighbors = 3;
};

/*******************************************************************************
 * Converts a CollectedPoint set to an OrientedPoint set using PCA (Principal
 * Component Analysis) normal estimation.
 ******************************************************************************/
OrientedPointCloud normal_estimation(
    const CollectedPointCloud &collected_points,
    NormalEstimationOptions options
);
