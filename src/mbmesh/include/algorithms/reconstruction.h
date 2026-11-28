#pragma once

#include <cstddef>
#include <functional>
#include <set>
#include <utility>
#include <string>

#include "../data_types/geometry.h"
#include "../io/point_spill.h"
#include "marching_cubes.h"
#include "normal_estimation.h"
#include "screened_poisson.h"
#include "support_trimming.h"

/*******************************************************************************
 * Reconstruction of a surface mesh from a set of collected points, and a
 * chunked driver that reconstructs a large survey chunk by chunk.
 *
 * Chunking bounds memory and run time. The screened-Poisson grid covers the
 * bounding box of its input, so a single run over a whole survey needs a grid
 * that is mostly empty, and its cost grows quickly with size. Here the survey
 * is split into square horizontal chunks on a fixed lattice anchored at the
 * local-frame origin. Each chunk is reconstructed from its own points plus a
 * halo of neighboring points, and keeps only the triangles it owns.
 ******************************************************************************/

struct ReconstructionOptions {
    NormalEstimationOptions normals;
    ScreenedPoissonOptions poisson;
    MarchingCubesOptions marching_cubes;
    SupportTrimmingOptions trimming;

    // Called with the Poisson field before marching cubes, and may change it. The chunked driver uses it to make
    // the field of a chunk agree with those of its neighbors where they meet (see seam_blending.h).
    std::function<void(ScalarGrid3D *)> adjust_field;
};

enum class ReconstructionStatus {
    Ok,
    NoOrientedSamples,
    EmptyField,
    EmptyMesh,
    TrimmedAway,
};

[[nodiscard]] const char *reconstruction_status_message(ReconstructionStatus status);

// Wall-clock seconds spent in each stage of reconstruct_chunk(); stages that
// did not run are zero.
struct ReconstructionTimes {
    double normals = 0.0;
    double poisson = 0.0;
    double poisson_solve = 0.0;   // the part of poisson spent in the solver iterations
    double marching_cubes = 0.0;
    double trimming = 0.0;
    double seams = 0.0;   // waiting for the neighbors, and matching their fields
};

struct ReconstructionResult {
    ReconstructionStatus status = ReconstructionStatus::Ok;
    ReconstructionTimes times;
    Mesh raw_mesh;
    Mesh mesh;
    OrientedPointCloud oriented_points;
    SupportTrimmingDiagnostics trimming_diagnostics;
};

// Normal estimation, screened Poisson, marching cubes and support trimming for
// one set of points. Decimation is not included; apply it beforehand. Progress
// is logged through util/logger.h: at INFO level when label is empty (a single
// pass, where each stage is a long wait worth reporting) and at DEBUG level
// otherwise, because the chunked driver reports each chunk itself. label, if
// given, prefixes each message (the chunked driver passes the chunk name).
[[nodiscard]] ReconstructionResult reconstruct_chunk(
    const CollectedPointCloud &points,
    const ReconstructionOptions &options,
    const std::string &label = "");

/******************************************************************************/

struct ChunkingOptions {
    // Edge length of a chunk, in meters. Must be a whole number of Poisson
    // cells; use round_to_cells() to make it so.
    double chunk_size = 0.0;

    // Width of the ring of neighboring points each chunk also reads, in meters.
    // Use chunk_halo() to compute a value that covers every stage's reach.
    double halo = 0.0;

    // Chunks with fewer owned points than this are skipped.
    std::size_t minimum_chunk_points = 32;

    // Number of chunks reconstructed at the same time. Memory use grows with
    // this, since each chunk holds its own Poisson grid. Chunks are delivered to
    // the callback in chunk order whatever the thread count.
    std::size_t threads = 1;

    // Over this many meters, on both sides of each edge that a chunk shares with a chunk that was finished before it (the
    // one to its west and the one to its south), the field of the chunk is made to have the neighbor's values on the
    // edge, so that the two meshes meet exactly (see seam_blending.h). Zero leaves the chunks as they are solved, and
    // lets them be started in any order.
    double seam_blend = 0.0;

    // How the chunks are matched on their edges (when seam_blend is above zero): false by solving each chunk free and then
    // blending its field to the neighbors' over seam_blend; true by solving each chunk with the fields of its west and
    // south neighbors fixed on the edges they share, so that there is nothing to blend. A chunk then waits for those
    // neighbors before its solve, and not after it.
    bool seam_solve = false;

    // Testing aids: process only these chunks (all of them if the set is empty), or stop after a number of chunks.
    std::set<std::pair<int, int>> only_chunks;
    std::size_t max_chunks = 0;   // 0 = no limit

    [[nodiscard]] bool selects(int i, int j) const { return only_chunks.empty() || only_chunks.count({i, j}) != 0; }
};

struct ChunkingStats {
    std::size_t chunks_with_points = 0;
    std::size_t chunks_processed = 0;
    std::size_t chunks_skipped_sparse = 0;
    std::size_t chunks_failed = 0;
    std::size_t chunks_empty_after_crop = 0;
};

// One reconstructed chunk. The meshes are cropped to the chunk (see below) and
// oriented_points holds only the points the chunk owns.
struct ChunkResult {
    int i = 0;
    int j = 0;
    std::size_t owned_points = 0;
    std::size_t input_points = 0;
    ReconstructionResult result;
};

// Return false to stop processing.
using ChunkCallback = std::function<bool(ChunkResult &)>;

// Smallest halo, in meters, that lets a chunk's owned region see the same
// neighborhoods a single run would: the reach of splatting or trimming, plus
// the normal-estimation radius needed to orient points at that reach, plus a
// few cells of margin.
[[nodiscard]] double chunk_halo(const ReconstructionOptions &options);

// Rounds a length up to a whole number of Poisson cells (at least one).
[[nodiscard]] double round_to_cells(double length, double cell_size);

// Chunk (i, j) owns x in [i*size, (i+1)*size) and y in [j*size, (j+1)*size) in
// the local frame. A triangle belongs to the chunk that contains its centroid,
// so every triangle belongs to exactly one chunk.
[[nodiscard]] Mesh crop_mesh_to_chunk(const Mesh &mesh, double min_x, double max_x, double min_y, double max_y);

// Reconstructs a survey chunk by chunk, calling on_chunk for each chunk that
// produces a mesh, in order of chunk index. With more than one thread the
// callback is still called one chunk at a time. Returns false only on a hard
// error.
[[nodiscard]] bool reconstruct_chunked(
    const CollectedPointCloud &points,
    const ReconstructionOptions &options,
    const ChunkingOptions &chunking,
    const ChunkCallback &on_chunk,
    ChunkingStats *stats,
    std::string *error);

// The same, from points sorted into chunk files beforehand (see io/point_spill.h), so that only the chunks being
// reconstructed are in memory. The chunk size and halo of the bins are the ones to use in chunking.
[[nodiscard]] bool reconstruct_chunked(
    const ChunkBins &bins,
    const ReconstructionOptions &options,
    const ChunkingOptions &chunking,
    const ChunkCallback &on_chunk,
    ChunkingStats *stats,
    std::string *error);
