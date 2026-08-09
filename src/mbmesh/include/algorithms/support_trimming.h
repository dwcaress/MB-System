#pragma once

#include <cstddef>

#include "../data_types/geometry.h"

struct SupportTrimmingOptions {
    // Remove generated triangles that are not supported by nearby oriented
    // input samples. Disable this to return the marching-cubes mesh unchanged.
    bool enabled = true;

    // Maximum Euclidean distance used to gather supporting samples. A value
    // of zero automatically uses 2.5 times the estimated point spacing.
    double support_radius = 0.0;

    // Maximum weighted point-to-plane distance from a generated vertex to its
    // supporting samples. Zero automatically uses 1.5 times point spacing.
    double max_normal_offset = 0.0;

    // Minimum number of valid oriented samples required per mesh vertex.
    // Two tolerates survey edges and locally sparse data without accepting
    // completely isolated geometry.
    int minimum_neighbors = 2;

    // Minimum absolute dot product between generated and sample normals.
    // Zero disables this test; values near one require parallel normals.
    double minimum_normal_alignment = 0.0;
};

struct SupportTrimmingDiagnostics {
    double estimated_point_spacing = 0.0;
    double resolved_support_radius = 0.0;
    double resolved_max_normal_offset = 0.0;

    std::size_t input_vertices = 0;
    std::size_t supported_vertices = 0;
    std::size_t rejected_for_neighbors = 0;
    std::size_t rejected_for_normal_offset = 0;
    std::size_t rejected_for_normal_alignment = 0;

    std::size_t input_triangles = 0;
    std::size_t output_vertices = 0;
    std::size_t output_triangles = 0;
};

Mesh support_trimming(
    const Mesh &raw_mesh,
    const OrientedPointCloud &oriented_points,
    SupportTrimmingOptions options,
    SupportTrimmingDiagnostics *diagnostics = nullptr
);
