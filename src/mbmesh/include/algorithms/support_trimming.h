#pragma once

#include <cstddef>

#include "../data_types/geometry.h"
#include "coverage_footprint.h"

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

    // Where the soundings are, seen from above, with their heights, for the whole survey. A mesh vertex that was
    // rejected only for having too few samples within the support radius is kept if soundings surround it in the
    // tangent plane of the mesh, which the footprint tells (see surrounded_by_samples() in support_trimming.cpp): the
    // sonar returns thin out in the depressions of the seafloor and under overhangs, and the surface bridges the gap
    // from the rest of the surface that surrounds it. A vertex at the edge of the survey, or on a sheet that hangs
    // under the surveyed surface, has none on its far side, and stays trimmed, as does one rejected because the
    // samples disagree with the surface. Null trims every rejected vertex.
    const CoverageFootprint *footprint = nullptr;

    // A vertex is only kept in a void if the nearest sample is no farther than this, so that the surface does not
    // reach across a footprint that other parts of the survey fill. Zero sets no limit. A positive value also turns on
    // the put-back of a vertex rejected only for too few neighbors that has a sample within a few support radii.
    double max_void_distance = 0.0;

    // A patch of rejected triangles inside the surface, which the open boundary of the raw mesh does not touch, of up
    // to this many square meters, is put back whatever the reason it was rejected for: small holes are not the edge
    // of the survey. A vertex of the patch that was rejected for too few neighbors must also have samples all around
    // it, which is not tested beyond the distance of the nearest sample without a footprint. Zero leaves the patches.
    double max_hole_area = 0.0;

    // A piece of the trimmed surface that is not joined to the rest, is smaller than this many square meters, and
    // is not at the edge of the reconstructed region is taken out: sound reflected from the water column leaves
    // specks, and the end of a sheet of extrapolated surface can be left as one. Zero keeps them.
    double min_island_area = 0.0;

    // Threads that test the vertices (one: the calling thread alone). The result does not depend on it.
    int threads = 1;
};

struct SupportTrimmingDiagnostics {
    double estimated_point_spacing = 0.0;
    double resolved_support_radius = 0.0;
    double resolved_max_normal_offset = 0.0;

    // Where the time went, in seconds: the tree of the samples, the test of every vertex, and the rest (the vertices put
    // back, the holes, the islands, and the mesh that is left).
    double tree_seconds = 0.0;
    double vertex_seconds = 0.0;
    double restore_seconds = 0.0;

    std::size_t input_vertices = 0;
    std::size_t supported_vertices = 0;
    std::size_t rejected_for_neighbors = 0;
    std::size_t rejected_for_normal_offset = 0;
    std::size_t rejected_for_normal_alignment = 0;

    std::size_t void_vertices_restored = 0;   // rejected for too few samples, but surrounded by soundings
    double void_distance_median = 0.0;        // distance of those vertices to the nearest sample, median and largest
    double void_distance_maximum = 0.0;
    std::size_t near_vertices_restored = 0;   // rejected for too few neighbors, but a sounding is within a few support radii
    std::size_t islands_removed = 0;          // small pieces of surface, not joined to the rest, taken out
    std::size_t holes_restored = 0;           // patches of rejected triangles surrounded by kept ones, put back

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

// What remove_unsupported_pieces took out.
struct PieceRemoval {
    std::size_t pieces = 0;      // connected pieces of the mesh removed
    std::size_t triangles = 0;
    std::size_t vertices = 0;
};

// The mesh of a survey in chunks, put together, can have pieces that are not joined to the rest and have no soundings near
// them: the leftovers of the sheets that the trimming of each chunk took the connection of away from, or reflections that
// the chunks that they were in held and trimmed in part. This removes the pieces of mesh (triangles that share
// vertices) with an area of at most max_area that are not supported: fewer than half of their vertices have a sounding
// within tolerance (in three dimensions, as the footprint knows them). A small piece with soundings under it, a
// real rock or a ledge that the trimming cut from the rest, stays.
void remove_unsupported_pieces(Mesh *mesh, const CoverageFootprint &footprint, double tolerance, double max_area, PieceRemoval *removed = nullptr);
