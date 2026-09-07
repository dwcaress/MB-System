#pragma once

#include <algorithm>

#include "../data_types/geometry.h"

struct ScreenedPoissonOptions {
    // Regular-grid spacing in point-cloud coordinate units. This is the main
    // reconstruction resolution and memory-control parameter.
    double cell_size = 0.25;

    // Empty margin around the sample bounds. At least a few cells are needed
    // for the implicit field to transition before reaching the grid boundary.
    double padding = 0.75;

    // Radius over which each oriented sample contributes to grid nodes.
    // Approximately two grid cells gives continuous support without excessive
    // smoothing for the default resolution.
    double normal_splat_radius = 0.5;

    // Strength of positional constraints at observed samples. Larger values
    // adhere more closely to samples but can reproduce more measurement noise.
    double screening_weight = 4.0;

    // Number of Jacobi relaxation passes used to solve the grid equation.
    // More iterations improve convergence at a proportional runtime cost.
    int solver_iterations = 200;

    // Apply sample-position constraints in addition to the normal field.
    bool use_screening = true;

    // Scalar value assigned to the observed surface after centering the field.
    // MarchingCubesOptions::iso_value should normally use the same value.
    double iso_value = 0.0;
};

/******************************************************************************/

/******************************************************************************
 * Extracts an implicit surface from an OrientedPoint set.
 ******************************************************************************/

ScalarGrid3D screened_poisson(const OrientedPointCloud &oriented_points, ScreenedPoissonOptions options);
