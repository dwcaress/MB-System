#include "algorithms/reconstruction.h"

#include "algorithms/seam_blending.h"
#include "io/point_spill.h"
#include "util/logger.h"
#include "util/memory.h"
#include "util/stopwatch.h"

using mbmesh::Stopwatch;

#include <algorithm>
#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>

namespace {

// A chunk grid larger than this is refused outright rather than risking the
// machine; callers are expected to choose a chunk size well below it.
constexpr double maximum_chunk_grid_cells = 1.5e8;

// Prefixes a message with the chunk it belongs to, so lines from chunks being
// reconstructed at the same time can be told apart.
[[nodiscard]] std::string labeled(const std::string &label, const std::string &message) {
    return label.empty() ? message : label + ": " + message;
}

// A stage's completion line. A single pass has no per-chunk progress, so its
// stages are reported at INFO; with a label the chunked driver reports the chunk.
void log_stage_completed(const char *tag, const std::string &label, const std::string &message) {
    if (label.empty()) {
        LOG_INFO(tag, message);
    } else {
        LOG_DEBUG(tag, labeled(label, message));
    }
}

[[nodiscard]] double snap_down(double value, double cell_size) {
    return std::floor(value / cell_size) * cell_size;
}

[[nodiscard]] double snap_up(double value, double cell_size) {
    return std::ceil(value / cell_size) * cell_size;
}

} // namespace

const char *reconstruction_status_message(ReconstructionStatus status) {
    switch (status) {
    case ReconstructionStatus::Ok:
        return "ok";
    case ReconstructionStatus::NoOrientedSamples:
        return "normal estimation produced no oriented samples";
    case ReconstructionStatus::EmptyField:
        return "screened Poisson reconstruction produced an empty field";
    case ReconstructionStatus::EmptyMesh:
        return "marching cubes produced an empty mesh";
    case ReconstructionStatus::TrimmedAway:
        return "support trimming removed the entire mesh; adjust the trimming thresholds or disable trimming";
    }
    return "unknown reconstruction failure";
}

ReconstructionResult reconstruct_chunk(
    const CollectedPointCloud &points,
    const ReconstructionOptions &options,
    const std::string &label) {

    ReconstructionResult result;
    const Stopwatch total_timer;
    Stopwatch stage_timer;

    // ------------------------------------------------------------------ normal estimation
    {
        const bool using_search_radius = options.normals.search_radius > 0.0;
        std::ostringstream message;
        message << "starting: search_radius=" << options.normals.search_radius
                << ", radius_mode=" << (using_search_radius ? "enabled" : "disabled")
                << ", k_nearest=" << options.normals.k
                << ", fallback_min_neighbors=" << options.normals.minimum_neighbors
                << ", fallback_to_knearest="
                << (using_search_radius ? "when radius neighborhood < minimum_neighbors" : "always");
        LOG_DEBUG("[normal-estimation]", labeled(label, message.str()));
    }

    result.oriented_points = normal_estimation(points, options.normals);
    if (result.oriented_points.empty()) {
        result.status = ReconstructionStatus::NoOrientedSamples;
        return result;
    }
    result.times.normals = stage_timer.seconds();
    log_stage_completed("[normal-estimation]", label, "completed: oriented " + std::to_string(result.oriented_points.size()) + " samples in " + stage_timer.text());
    stage_timer = Stopwatch();

    // ------------------------------------------------------------------ screened Poisson
    {
        std::ostringstream message;
        message << "starting: cell_size=" << options.poisson.cell_size
                << ", padding=" << options.poisson.padding
                << ", splat_radius=" << options.poisson.normal_splat_radius
                << ", iterations=" << options.poisson.solver_iterations
                << ", screening=" << (options.poisson.use_screening ? "enabled" : "disabled")
                << ", screening_weight=" << options.poisson.screening_weight;
        LOG_DEBUG("[poisson]", labeled(label, message.str()));
    }

    PoissonDiagnostics poisson_diagnostics;
    ScalarGrid3D poisson_surface = screened_poisson(result.oriented_points, options.poisson, &poisson_diagnostics);
    if (poisson_surface.values.empty()) {
        result.status = ReconstructionStatus::EmptyField;
        return result;
    }
    result.times.poisson = stage_timer.seconds();
    result.times.poisson_solve = poisson_diagnostics.solve_seconds;
    log_stage_completed("[poisson]", label, "completed: grid=" + std::to_string(poisson_surface.nx) + "x" + std::to_string(poisson_surface.ny) + "x" +
               std::to_string(poisson_surface.nz) + " in " + stage_timer.text());
    {
        // Where the Poisson time goes, and how far the solver got. The relative
        // residual is 1 at the start and falls toward 0 as the solver converges.
        std::ostringstream message;
        message << "solver " << poisson_solver_name(poisson_diagnostics.solver) << ", " << poisson_diagnostics.iterations
                << " iterations" << (poisson_diagnostics.solver == PoissonSolver::Sor ? " (relaxation " + std::to_string(poisson_diagnostics.relaxation).substr(0, 5) + ")" : std::string())
                << " on " << poisson_diagnostics.cells << " cells: splat " << mbmesh::format_duration(poisson_diagnostics.splat_seconds)
                << ", solve " << mbmesh::format_duration(poisson_diagnostics.solve_seconds) << "; relative residual 1";
        message << std::scientific << std::setprecision(2);
        if (poisson_diagnostics.residual_midway >= 0.0) {
            message << " -> " << poisson_diagnostics.residual_midway << " (after " << poisson_diagnostics.iterations / 2 << ")";
        }
        if (poisson_diagnostics.residual_final >= 0.0) {
            message << " -> " << poisson_diagnostics.residual_final << " (after " << poisson_diagnostics.iterations << ")";
        }
        log_stage_completed("[poisson]", label, message.str());
    }
    if (options.adjust_field) {
        // Not in the time of the Poisson stage or of marching cubes: for a chunk it is mostly waiting for the neighbors.
        const Stopwatch adjust_timer;
        options.adjust_field(&poisson_surface);
        result.times.seams = adjust_timer.seconds();
    }
    stage_timer = Stopwatch();

    // ------------------------------------------------------------------ marching cubes
    LOG_DEBUG("[marching-cubes]", labeled(label, "starting: iso_value=" + std::to_string(options.marching_cubes.iso_value)));

    result.raw_mesh = marching_cubes(poisson_surface, options.marching_cubes);

    // The scalar grid is by far the largest object in the pipeline; release it
    // before trimming rather than at the end of the function.
    poisson_surface = ScalarGrid3D();

    if (result.raw_mesh.vertices.empty() || result.raw_mesh.indices.empty()) {
        result.status = ReconstructionStatus::EmptyMesh;
        return result;
    }
    result.times.marching_cubes = stage_timer.seconds();
    log_stage_completed("[marching-cubes]", label, "completed: " + std::to_string(result.raw_mesh.vertices.size()) + " vertices, " +
               std::to_string(result.raw_mesh.indices.size() / 3) + " triangles in " + stage_timer.text());
    stage_timer = Stopwatch();

    // ------------------------------------------------------------------ support trimming
    if (options.trimming.enabled) {
        std::ostringstream message;
        message << "starting: radius="
                << ((options.trimming.support_radius > 0.0) ? std::to_string(options.trimming.support_radius) : "auto")
                << ", normal_offset="
                << ((options.trimming.max_normal_offset > 0.0) ? std::to_string(options.trimming.max_normal_offset) : "auto")
                << ", minimum_neighbors=" << options.trimming.minimum_neighbors
                << ", minimum_normal_alignment="
                << ((options.trimming.minimum_normal_alignment > 0.0) ? std::to_string(options.trimming.minimum_normal_alignment) : "disabled");
        LOG_DEBUG("[support-trimming]", labeled(label, message.str()));

        result.mesh = support_trimming(result.raw_mesh, result.oriented_points, options.trimming, &result.trimming_diagnostics);
    } else {
        LOG_DEBUG("[support-trimming]", labeled(label, "disabled; using raw mesh"));
        result.mesh = result.raw_mesh;
    }

    result.times.trimming = stage_timer.seconds();

    if (result.mesh.vertices.empty() || result.mesh.indices.empty()) {
        result.status = ReconstructionStatus::TrimmedAway;
        return result;
    }

    if (options.trimming.enabled) {
        const SupportTrimmingDiagnostics &diagnostics = result.trimming_diagnostics;
        std::ostringstream message;
        message << "completed: spacing=" << diagnostics.estimated_point_spacing
                << ", resolved_radius=" << diagnostics.resolved_support_radius
                << ", resolved_normal_offset=" << diagnostics.resolved_max_normal_offset
                << ", supported_vertices=" << diagnostics.supported_vertices << '/' << diagnostics.input_vertices
                << ", rejected=(neighbors=" << diagnostics.rejected_for_neighbors
                << ", offset=" << diagnostics.rejected_for_normal_offset
                << ", alignment=" << diagnostics.rejected_for_normal_alignment << ")"
                << ", restored_in_voids=" << diagnostics.void_vertices_restored << " (nearest sample median "
                << diagnostics.void_distance_median << " m, largest " << diagnostics.void_distance_maximum << " m), near_data_restored=" << diagnostics.near_vertices_restored
                << ", holes_restored=" << diagnostics.holes_restored << ", islands_removed=" << diagnostics.islands_removed
                << ", output=" << result.mesh.vertices.size() << " vertices, "
                << result.mesh.indices.size() / 3 << " triangles in " << stage_timer.text() << " (tree " << mbmesh::format_duration(diagnostics.tree_seconds)
                << ", vertices " << mbmesh::format_duration(diagnostics.vertex_seconds) << ", restoring " << mbmesh::format_duration(diagnostics.restore_seconds) << ")"
                << " (reconstruction total " << total_timer.text() << ")";
        log_stage_completed("[support-trimming]", label, message.str());
    }

    result.status = ReconstructionStatus::Ok;
    return result;
}

/******************************************************************************/

double round_to_cells(double length, double cell_size) {
    if (cell_size <= 0.0) {
        return length;
    }
    return std::max(1.0, std::ceil(length / cell_size - 1.0e-9)) * cell_size;
}

double chunk_halo(const ReconstructionOptions &options) {
    const double cell = options.poisson.cell_size;

    // Points within `reach` of a chunk can affect its triangles, through normal
    // splatting or through support trimming.
    const double trimming_reach =
        options.trimming.enabled ? (options.trimming.support_radius > 0.0 ? options.trimming.support_radius : 6.0 * cell) : 0.0;
    const double reach = std::max(options.poisson.normal_splat_radius, trimming_reach);

    // Those points need correct normals, which need their own neighbors.
    const double normal_reach = options.normals.search_radius > 0.0 ? options.normals.search_radius : 4.0 * cell;

    return round_to_cells(reach + normal_reach + 4.0 * cell, cell);
}

Mesh crop_mesh_to_chunk(const Mesh &mesh, double min_x, double max_x, double min_y, double max_y) {
    Mesh cropped;
    const bool has_normals = mesh.normals.size() == mesh.vertices.size();
    constexpr unsigned int unmapped = std::numeric_limits<unsigned int>::max();
    std::vector<unsigned int> remap(mesh.vertices.size(), unmapped);

    for (std::size_t t = 0; t + 2 < mesh.indices.size(); t += 3) {
        const unsigned int a = mesh.indices[t];
        const unsigned int b = mesh.indices[t + 1];
        const unsigned int c = mesh.indices[t + 2];

        const double centroid_x = (mesh.vertices[a].x + mesh.vertices[b].x + mesh.vertices[c].x) / 3.0;
        const double centroid_y = (mesh.vertices[a].y + mesh.vertices[b].y + mesh.vertices[c].y) / 3.0;
        if (centroid_x < min_x || centroid_x >= max_x || centroid_y < min_y || centroid_y >= max_y) {
            continue;
        }

        for (const unsigned int old_index : {a, b, c}) {
            if (remap[old_index] == unmapped) {
                remap[old_index] = static_cast<unsigned int>(cropped.vertices.size());
                cropped.vertices.push_back(mesh.vertices[old_index]);
                if (has_normals) {
                    cropped.normals.push_back(mesh.normals[old_index]);
                }
            }
            cropped.indices.push_back(remap[old_index]);
        }
    }
    return cropped;
}

namespace {

// The chunked driver. The points of the chunks come from points, held in memory, or from bins, on disk; exactly one is given.
bool reconstruct_chunked_from(
    const CollectedPointCloud *points,
    const ChunkBins *bins,
    const ReconstructionOptions &options,
    const ChunkingOptions &chunking,
    const ChunkCallback &on_chunk,
    ChunkingStats *stats,
    std::string *error) {

    ChunkingStats local_stats;
    ChunkingStats &counts = stats != nullptr ? *stats : local_stats;
    counts = ChunkingStats();

    const double cell = options.poisson.cell_size;
    const double chunk = chunking.chunk_size;
    const double halo = chunking.halo;

    if (cell <= 0.0 || chunk <= 0.0 || halo < 0.0) {
        if (error != nullptr) {
            *error = "chunked reconstruction needs a positive cell size and chunk size and a non-negative halo";
        }
        return false;
    }
    if (points != nullptr && points->size() >= std::numeric_limits<std::uint32_t>::max()) {
        if (error != nullptr) {
            *error = "chunked reconstruction supports fewer than 2^32 points";
        }
        return false;
    }

    // ---------------------------------------------------------------- bin points into chunks
    //
    // A point is stored in the chunk that owns it and in every neighboring chunk
    // whose halo reaches it. In memory only indices are stored, not copies of
    // points; on disk the chunk files were written beforehand (see ChunkBins).

    struct Chunk {
        std::vector<std::uint32_t> indices;
        std::size_t owned = 0;
    };
    std::map<std::pair<int, int>, Chunk> chunks;

    if (points != nullptr) {
        for (std::size_t n = 0; n < points->size(); n++) {
            const double x = (*points)[n].point.x;
            const double y = (*points)[n].point.y;
            for_each_chunk_of_point(x, y, chunk, halo, [&](int i, int j, bool owned) {
                if (!chunking.selects(i, j)) {
                    return;
                }
                Chunk &entry = chunks[{i, j}];
                entry.indices.push_back(static_cast<std::uint32_t>(n));
                if (owned) {
                    entry.owned++;
                }
            });
        }
    }
    else {
        for (const auto &entry : bins->chunks()) {
            if (!chunking.selects(entry.first.first, entry.first.second)) {
                continue;
            }
            chunks[entry.first].owned = entry.second.owned;   // the points stay on disk
        }
    }

    for (const auto &entry : chunks) {
        if (entry.second.owned > 0) {
            counts.chunks_with_points++;
        }
    }
    LOG_INFO("[chunking]", "chunk_size=" + std::to_string(chunk) + " m, halo=" + std::to_string(halo) + " m, " +
               std::to_string(counts.chunks_with_points) + " chunks contain points");

    // ---------------------------------------------------------------- choose the chunks to run
    struct Job {
        int i = 0;
        int j = 0;
        std::size_t owned = 0;
        std::vector<std::uint32_t> indices;
    };
    std::vector<Job> jobs;

    for (auto &entry : chunks) {
        Chunk &current = entry.second;
        if (current.owned == 0) {
            continue;   // the ring of halo-only chunks around the survey
        }
        if (current.owned < chunking.minimum_chunk_points) {
            counts.chunks_skipped_sparse++;
            LOG_DEBUG("[chunking]", "chunk " + std::to_string(entry.first.first) + "," + std::to_string(entry.first.second) +
                                        " skipped: only " + std::to_string(current.owned) + " points");
            continue;
        }
        if (chunking.max_chunks > 0 && jobs.size() >= chunking.max_chunks) {
            break;
        }
        jobs.push_back({entry.first.first, entry.first.second, current.owned, std::move(current.indices)});
    }
    chunks.clear();

    // ---------------------------------------------------------------- reconstruct chunks, possibly in parallel
    //
    // Workers take chunks in order. Finished chunks are handed to on_chunk strictly
    // in chunk order, so the output does not depend on the thread count.

    enum class Disposition { Pending, Mesh, Failed, EmptyCrop };
    struct Outcome {
        Disposition disposition = Disposition::Pending;
        ChunkResult chunk;
    };
    std::vector<Outcome> outcomes(jobs.size());

    // Start the chunks with the most points first. The last chunks to start decide
    // when the run ends, so starting small ones last keeps the threads evenly
    // loaded. Delivery stays in chunk order regardless.
    //
    // With the seams blended a chunk waits for the chunks to its west and south, which come before it in chunk order, so
    // the chunks must be started in that order: otherwise every thread could be holding a chunk that waits for one that
    // no thread has started.
    const bool blend_seams = chunking.seam_blend > 0.0;
    std::vector<std::size_t> start_order(jobs.size());
    for (std::size_t n = 0; n < start_order.size(); n++) {
        start_order[n] = n;
    }
    if (!blend_seams) {
        std::stable_sort(start_order.begin(), start_order.end(), [&](std::size_t a, std::size_t b) {
            return jobs[a].owned > jobs[b].owned;
        });
    }

    // The final field of each chunk on its east and north edges, kept until the chunks to its east and north have taken it.
    struct SeamPlanes {
        std::mutex mutex;
        std::condition_variable changed;
        std::set<std::pair<int, int>> finished;   // chunks that have published their edges, or never will
        std::map<std::pair<int, int>, std::shared_ptr<FieldPlane>> east;
        std::map<std::pair<int, int>, std::shared_ptr<FieldPlane>> north;
    } seam;
    std::set<std::pair<int, int>> job_chunks;
    for (const Job &job : jobs) {
        job_chunks.insert({job.i, job.j});
    }
    std::atomic<bool> abandoned{false};   // a run that has failed or stopped does not wait for neighbors
    const auto abandon_seams = [&]() {
        abandoned = true;
        std::lock_guard<std::mutex> lock(seam.mutex);
        seam.changed.notify_all();
    };

    std::mutex state_mutex;
    std::size_t completed_jobs = 0;
    const Stopwatch reconstruction_timer;
    constexpr double progress_interval_seconds = 60.0;
    double last_progress_seconds = 0.0;
    ReconstructionTimes stage_totals;   // summed over the chunks, so over threads as well
    std::size_t next_job = 0;
    std::size_t next_emit = 0;
    bool stop = false;
    bool failed = false;
    std::string failure;

    auto run_job = [&](Job &job, Outcome &outcome) -> bool {
        const int i = job.i;
        const int j = job.j;
        const std::string name = "chunk " + std::to_string(i) + "," + std::to_string(j);

        CollectedPointCloud chunk_points;
        if (points != nullptr) {
            chunk_points.reserve(job.indices.size());
            for (const std::uint32_t index : job.indices) {
                chunk_points.push_back((*points)[index]);
            }
            std::vector<std::uint32_t>().swap(job.indices);
        }
        else {
            std::string read_error;
            if (!bins->read_chunk(i, j, &chunk_points, &read_error)) {
                std::lock_guard<std::mutex> lock(state_mutex);
                failed = true;
                failure = read_error;
                abandon_seams();
                return false;
            }
        }
        double min_z = std::numeric_limits<double>::max();
        double max_z = std::numeric_limits<double>::lowest();
        for (const CollectedPoint &chunk_point : chunk_points) {
            min_z = std::min(min_z, chunk_point.point.z);
            max_z = std::max(max_z, chunk_point.point.z);
        }

        // The grid covers the chunk, its halo and the Poisson padding, with every
        // face on the global cell lattice so neighboring chunks share grid nodes.
        const double xy_margin = halo + options.poisson.padding;
        ReconstructionOptions chunk_options = options;
        chunk_options.trimming.threads = static_cast<int>(std::max<std::size_t>(chunking.threads, 1));
        // Threads that have no chunk to work on (the last chunks of a run, or those that wait for their neighbors) share the sweeps of the solver.
        chunk_options.poisson.threads = static_cast<int>(std::max<std::size_t>(chunking.threads, 1));
        chunk_options.poisson.use_fixed_domain = true;
        chunk_options.poisson.domain_bounds = Bounds3D(
            Vec3(snap_down(i * chunk - xy_margin, cell),
                 snap_down(j * chunk - xy_margin, cell),
                 snap_down(min_z - options.poisson.padding, cell)),
            Vec3(snap_up((i + 1) * chunk + xy_margin, cell),
                 snap_up((j + 1) * chunk + xy_margin, cell),
                 snap_up(max_z + options.poisson.padding, cell)));

        const Vec3 size = chunk_options.poisson.domain_bounds.size();
        const double grid_cells = (size.x / cell + 1.0) * (size.y / cell + 1.0) * (size.z / cell + 1.0);
        if (grid_cells > maximum_chunk_grid_cells) {
            std::ostringstream message;
            message << name << " needs a Poisson grid of " << grid_cells << " cells, which exceeds "
                    << maximum_chunk_grid_cells << "; use a smaller --chunk-size or a coarser level of detail";
            std::lock_guard<std::mutex> lock(state_mutex);
            failed = true;
            failure = message.str();
            abandon_seams();
            return false;
        }

        LOG_DEBUG("[chunking]", name + ": " + std::to_string(job.owned) + " owned of " + std::to_string(chunk_points.size()) +
                       " points, grid " + std::to_string(static_cast<long long>(grid_cells)) + " cells");

        ChunkResult &chunk_result = outcome.chunk;
        chunk_result.i = i;
        chunk_result.j = j;
        chunk_result.owned_points = job.owned;
        chunk_result.input_points = chunk_points.size();

        // Whatever happens to this chunk, the chunks that wait for it are let go.
        struct SeamRelease {
            SeamRelease(SeamPlanes &planes, std::pair<int, int> chunk_key) : seam(planes), key(chunk_key) {}
            SeamRelease(const SeamRelease &) = delete;
            SeamRelease &operator=(const SeamRelease &) = delete;
            ~SeamRelease() {
                std::lock_guard<std::mutex> lock(seam.mutex);
                seam.finished.insert(key);
                seam.changed.notify_all();
            }
            SeamPlanes &seam;
            std::pair<int, int> key;
        };
        std::unique_ptr<SeamRelease> release;
        if (blend_seams) {
            release = std::make_unique<SeamRelease>(seam, std::make_pair(i, j));
            // The final fields of the chunks to the west and the south, on the edges they share with this chunk, once they have them.
            const auto take_neighbors = [&](std::shared_ptr<FieldPlane> *west, std::shared_ptr<FieldPlane> *south) {
                std::unique_lock<std::mutex> lock(seam.mutex);
                for (const std::pair<int, int> &neighbor : {std::make_pair(i - 1, j), std::make_pair(i, j - 1)}) {
                    if (job_chunks.count(neighbor) != 0) {
                        seam.changed.wait(lock, [&] { return abandoned.load() || seam.finished.count(neighbor) != 0; });
                    }
                }
                if (abandoned) {
                    return false;
                }
                const auto east_of_west = seam.east.find({i - 1, j});
                if (east_of_west != seam.east.end()) {
                    *west = east_of_west->second;
                    seam.east.erase(east_of_west);
                }
                const auto north_of_south = seam.north.find({i, j - 1});
                if (north_of_south != seam.north.end()) {
                    *south = north_of_south->second;
                    seam.north.erase(north_of_south);
                }
                return true;
            };
            const auto neighbors_text = [&](bool west, bool south) {
                return std::string(west ? "the west " : "") + std::string(west && south ? "and " : "") + std::string(south ? "the south " : "") +
                       std::string(west || south ? "neighbor" : "no neighbor");
            };
            // This chunk's own final field on its east and north edges, for the chunks that come after it.
            const auto publish = [&](const ScalarGrid3D &grid) {
                const long east_edge = std::lround((i + 1) * chunk / cell);
                const long north_edge = std::lround((j + 1) * chunk / cell);
                const long west_edge = std::lround(i * chunk / cell);
                const long south_edge = std::lround(j * chunk / cell);
                auto east = std::make_shared<FieldPlane>(extract_field_plane(grid, 0, east_edge, south_edge, north_edge));
                auto north = std::make_shared<FieldPlane>(extract_field_plane(grid, 1, north_edge, west_edge, east_edge));
                std::lock_guard<std::mutex> lock(seam.mutex);
                seam.east[{i, j}] = std::move(east);
                seam.north[{i, j}] = std::move(north);
                seam.finished.insert({i, j});
                seam.changed.notify_all();
            };
            if (chunking.seam_solve) {
                // The neighbors' fields are boundary values of the solve, so the chunk waits for them before it.
                chunk_options.poisson.fix_nodes = [&, take_neighbors, neighbors_text](ScalarGrid3D *grid, std::vector<unsigned char> *fixed) -> std::size_t {
                    const Stopwatch wait_timer;
                    std::shared_ptr<FieldPlane> west;
                    std::shared_ptr<FieldPlane> south;
                    if (!take_neighbors(&west, &south)) {
                        return 0;
                    }
                    const std::size_t count = fix_field_planes(grid, west.get(), south.get(), fixed);
                    LOG_DEBUG("[chunking]", name + ": solved with the field of " + neighbors_text(west != nullptr, south != nullptr) + " fixed on " +
                                                std::to_string(count) + " nodes, after waiting " + wait_timer.text());
                    return count;
                };
                chunk_options.adjust_field = [&, publish](ScalarGrid3D *grid) { publish(*grid); };
            } else {
                chunk_options.adjust_field = [&, take_neighbors, neighbors_text, publish](ScalarGrid3D *grid) {
                    std::shared_ptr<FieldPlane> west;
                    std::shared_ptr<FieldPlane> south;
                    if (!take_neighbors(&west, &south)) {
                        return;
                    }
                    const SeamBlendResult blended = blend_field_to_planes(grid, west.get(), south.get(), chunking.seam_blend);
                    LOG_DEBUG("[chunking]", name + ": field matched to " + neighbors_text(west != nullptr, south != nullptr) + ", " +
                                                std::to_string(blended.nodes_changed) + " nodes changed, by at most " + std::to_string(blended.largest_correction));
                    publish(*grid);
                };
            }
        }
        chunk_result.result = reconstruct_chunk(chunk_points, chunk_options, name);
        chunk_points = CollectedPointCloud();

        if (chunk_result.result.status != ReconstructionStatus::Ok) {
            LOG_DEBUG("[chunking]", name + " produced no mesh: " + reconstruction_status_message(chunk_result.result.status));
            outcome.disposition = Disposition::Failed;
            const ReconstructionTimes times = chunk_result.result.times;   // the progress line reports them
            chunk_result.result = ReconstructionResult();
            chunk_result.result.times = times;
            return true;
        }

        // Keep only what this chunk owns.
        const double min_x = i * chunk;
        const double max_x = (i + 1) * chunk;
        const double min_y = j * chunk;
        const double max_y = (j + 1) * chunk;

        ReconstructionResult &result = chunk_result.result;
        result.mesh = crop_mesh_to_chunk(result.mesh, min_x, max_x, min_y, max_y);
        result.raw_mesh = crop_mesh_to_chunk(result.raw_mesh, min_x, max_x, min_y, max_y);

        OrientedPointCloud owned_oriented;
        for (const OrientedPoint &oriented : result.oriented_points) {
            if (oriented.point.x >= min_x && oriented.point.x < max_x && oriented.point.y >= min_y && oriented.point.y < max_y) {
                owned_oriented.push_back(oriented);
            }
        }
        result.oriented_points = std::move(owned_oriented);

        if (result.mesh.indices.empty()) {
            LOG_DEBUG("[chunking]", name + " has no triangles inside the chunk");
            outcome.disposition = Disposition::EmptyCrop;
            const ReconstructionTimes times = chunk_result.result.times;   // the progress line reports them
            chunk_result.result = ReconstructionResult();
            chunk_result.result.times = times;
            return true;
        }

        LOG_DEBUG("[chunking]", name + " completed: " + std::to_string(result.mesh.vertices.size()) + " vertices, " +
                       std::to_string(result.mesh.indices.size() / 3) + " triangles");
        outcome.disposition = Disposition::Mesh;
        return true;
    };

    auto worker = [&]() {
        while (true) {
            std::size_t n = 0;
            {
                std::lock_guard<std::mutex> lock(state_mutex);
                if (stop || failed || next_job >= jobs.size()) {
                    return;
                }
                n = start_order[next_job++];
            }

            Outcome outcome;
            const Stopwatch chunk_timer;
            if (!run_job(jobs[n], outcome)) {
                return;
            }

            std::lock_guard<std::mutex> lock(state_mutex);
            completed_jobs++;
            {
                const ReconstructionTimes &times = outcome.chunk.result.times;
                stage_totals.normals += times.normals;
                stage_totals.poisson += times.poisson;
                stage_totals.poisson_solve += times.poisson_solve;
                stage_totals.marching_cubes += times.marching_cubes;
                stage_totals.trimming += times.trimming;
                stage_totals.seams += times.seams;

                // One line per chunk at DEBUG; at INFO only a short progress line, and
                // not more than one a minute, because a survey can have hundreds of chunks.
                const ChunkResult &done = outcome.chunk;
                std::ostringstream message;
                message << "chunk " << completed_jobs << " of " << jobs.size() << " (" << jobs[n].i << "," << jobs[n].j << ") ";
                if (outcome.disposition == Disposition::Mesh) {
                    message << "done in " << chunk_timer.text() << ": " << done.result.mesh.vertices.size() << " vertices, "
                            << done.result.mesh.indices.size() / 3 << " triangles";
                } else if (outcome.disposition == Disposition::EmptyCrop) {
                    message << "done in " << chunk_timer.text() << ": no triangles inside the chunk";
                } else {
                    message << "produced no mesh after " << chunk_timer.text();
                }
                message << " (normals " << mbmesh::format_duration(times.normals) << ", poisson "
                        << mbmesh::format_duration(times.poisson) << ", marching cubes "
                        << mbmesh::format_duration(times.marching_cubes) << ", trimming "
                        << mbmesh::format_duration(times.trimming) << (blend_seams ? ", seams " + mbmesh::format_duration(times.seams) : std::string()) << ")";
                LOG_DEBUG("[chunking]", message.str());

                const double elapsed = reconstruction_timer.seconds();
                if (completed_jobs < jobs.size() && elapsed - last_progress_seconds >= progress_interval_seconds) {
                    last_progress_seconds = elapsed;
                    // The largest chunks start first, so the estimate is pessimistic early on.
                    // Padded so that the numbers line up from one line to the next.
                    std::ostringstream progress;
                    progress << std::setw(static_cast<int>(std::to_string(jobs.size()).size())) << completed_jobs << " of " << jobs.size()
                             << " chunks, " << std::setw(2) << 100 * completed_jobs / jobs.size() << "%, "
                             << mbmesh::format_minutes(elapsed) << " elapsed, about "
                             << mbmesh::format_minutes(elapsed * static_cast<double>(jobs.size() - completed_jobs) /
                                                       static_cast<double>(completed_jobs))
                             << " left, " << mbmesh::peak_memory_text();
                    LOG_INFO("[chunking]", progress.str());
                }
            }
            outcomes[n] = std::move(outcome);
            while (!stop && next_emit < outcomes.size() && outcomes[next_emit].disposition != Disposition::Pending) {
                Outcome &ready = outcomes[next_emit++];
                switch (ready.disposition) {
                case Disposition::Failed:
                    counts.chunks_failed++;
                    break;
                case Disposition::EmptyCrop:
                    counts.chunks_empty_after_crop++;
                    break;
                case Disposition::Mesh:
                    counts.chunks_processed++;
                    if (on_chunk && !on_chunk(ready.chunk)) {
                        stop = true;
                        abandon_seams();
                    }
                    break;
                case Disposition::Pending:
                    break;
                }
                ready.chunk = ChunkResult();   // release the chunk's meshes
            }
        }
    };

    const std::size_t thread_count = std::max<std::size_t>(1, std::min(chunking.threads, jobs.size()));
    if (thread_count == 1) {
        worker();
    }
    else {
        std::vector<std::thread> pool;
        pool.reserve(thread_count);
        for (std::size_t t = 0; t < thread_count; t++) {
            pool.emplace_back(worker);
        }
        for (std::thread &thread : pool) {
            thread.join();
        }
    }

    if (failed) {
        if (error != nullptr) {
            *error = failure;
        }
        return false;
    }
    if (completed_jobs > 0) {
        LOG_INFO("[chunking]", "time summed over all chunks and threads: normals " + mbmesh::format_duration(stage_totals.normals) +
                                   ", poisson " + mbmesh::format_duration(stage_totals.poisson) + " (solver iterations " +
                                   mbmesh::format_duration(stage_totals.poisson_solve) + "), marching cubes " +
                                   mbmesh::format_duration(stage_totals.marching_cubes) + ", trimming " +
                                   mbmesh::format_duration(stage_totals.trimming));
    }
    return true;
}

} // namespace

bool reconstruct_chunked(
    const CollectedPointCloud &points,
    const ReconstructionOptions &options,
    const ChunkingOptions &chunking,
    const ChunkCallback &on_chunk,
    ChunkingStats *stats,
    std::string *error) {
    return reconstruct_chunked_from(&points, nullptr, options, chunking, on_chunk, stats, error);
}

bool reconstruct_chunked(
    const ChunkBins &bins,
    const ReconstructionOptions &options,
    const ChunkingOptions &chunking,
    const ChunkCallback &on_chunk,
    ChunkingStats *stats,
    std::string *error) {
    return reconstruct_chunked_from(nullptr, &bins, options, chunking, on_chunk, stats, error);
}
