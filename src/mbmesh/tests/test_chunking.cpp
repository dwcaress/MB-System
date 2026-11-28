#include "algorithms/reconstruction.h"
#include "util/stopwatch.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <unistd.h>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr double survey_size = 30.0;     // meters, square
constexpr double chunk_size = 10.0;       // meters, 3 x 3 chunks
constexpr double sample_spacing = 0.25;

double surface_height(double x, double y) {
    return 1.5 * std::sin(0.35 * x) * std::cos(0.25 * y) + 0.2 * x * 0.05;
}

bool check(bool condition, const std::string &message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message.c_str());
    }
    return condition;
}

// A survey of a gently varying seafloor with the sonar 20 m above it, so every
// normal is oriented upward.
CollectedPointCloud make_survey() {
    CollectedPointCloud points;
    const int count = static_cast<int>(std::round(survey_size / sample_spacing));
    for (int ix = 0; ix < count; ix++) {
        for (int iy = 0; iy < count; iy++) {
            const double x = ix * sample_spacing;
            const double y = iy * sample_spacing;
            const double z = surface_height(x, y);
            points.push_back({Vec3(x, y, z), Vec3(x, y, z + 20.0)});
        }
    }
    return points;
}

ReconstructionOptions make_options() {
    ReconstructionOptions options;
    options.normals.k = 24;
    options.normals.search_radius = 0.8;
    options.normals.minimum_neighbors = 6;

    options.poisson.cell_size = 0.5;
    options.poisson.padding = 2.0;
    options.poisson.normal_splat_radius = 1.0;
    options.poisson.screening_weight = 3.0;
    options.poisson.solver_iterations = 300;
    options.poisson.use_screening = true;
    options.poisson.iso_value = 0.0;

    options.marching_cubes.iso_value = 0.0;

    options.trimming.enabled = true;
    options.trimming.support_radius = 1.5;
    options.trimming.max_normal_offset = 0.5;
    options.trimming.minimum_neighbors = 3;
    return options;
}

double max_height_error(const Mesh &mesh) {
    double worst = 0.0;
    for (const Vec3 &vertex : mesh.vertices) {
        worst = std::max(worst, std::fabs(vertex.z - surface_height(vertex.x, vertex.y)));
    }
    return worst;
}

// True if (x, y) lies inside some triangle of the mesh, seen from above.
bool covered(const Mesh &mesh, double x, double y) {
    constexpr double slack = 1.0e-9;
    for (std::size_t t = 0; t + 2 < mesh.indices.size(); t += 3) {
        const Vec3 &a = mesh.vertices[mesh.indices[t]];
        const Vec3 &b = mesh.vertices[mesh.indices[t + 1]];
        const Vec3 &c = mesh.vertices[mesh.indices[t + 2]];
        const double d1 = (b.x - a.x) * (y - a.y) - (b.y - a.y) * (x - a.x);
        const double d2 = (c.x - b.x) * (y - b.y) - (c.y - b.y) * (x - b.x);
        const double d3 = (a.x - c.x) * (y - c.y) - (a.y - c.y) * (x - c.x);
        const bool has_negative = d1 < -slack || d2 < -slack || d3 < -slack;
        const bool has_positive = d1 > slack || d2 > slack || d3 > slack;
        if (!(has_negative && has_positive)) {
            return true;
        }
    }
    return false;
}

void append(Mesh *destination, const Mesh &source) {
    const unsigned int offset = static_cast<unsigned int>(destination->vertices.size());
    destination->vertices.insert(destination->vertices.end(), source.vertices.begin(), source.vertices.end());
    for (const unsigned int index : source.indices) {
        destination->indices.push_back(index + offset);
    }
}

} // namespace

int main() {
    bool ok = true;

    const CollectedPointCloud points = make_survey();
    const ReconstructionOptions options = make_options();

    // ---- helpers
    ok &= check(std::fabs(round_to_cells(10.1, 0.5) - 10.5) < 1e-12, "round_to_cells rounds up to whole cells");
    ok &= check(std::fabs(round_to_cells(0.01, 0.5) - 0.5) < 1e-12, "round_to_cells returns at least one cell");

    const double halo = chunk_halo(options);
    ok &= check(halo >= options.poisson.normal_splat_radius + options.normals.search_radius, "halo covers splat reach plus normal radius");
    ok &= check(std::fabs(std::remainder(halo, options.poisson.cell_size)) < 1e-9, "halo is a whole number of cells");

    {
        Mesh square;
        square.vertices = {Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(1, 1, 0), Vec3(0, 1, 0), Vec3(5, 5, 0), Vec3(6, 5, 0), Vec3(6, 6, 0)};
        square.indices = {0, 1, 2, 0, 2, 3, 4, 5, 6};
        const Mesh cropped = crop_mesh_to_chunk(square, 0.0, 2.0, 0.0, 2.0);
        ok &= check(cropped.indices.size() == 6 && cropped.vertices.size() == 4, "crop keeps only triangles whose centroid is in the chunk");
        const Mesh edge = crop_mesh_to_chunk(square, 1.0, 2.0, 0.0, 2.0);
        ok &= check(edge.indices.empty() || edge.indices.size() == 3, "crop uses a half-open chunk");
    }

    // ---- one run over the whole survey
    const ReconstructionResult whole = reconstruct_chunk(points, options);
    ok &= check(whole.status == ReconstructionStatus::Ok, "whole-survey reconstruction succeeds");
    ok &= check(whole.times.normals > 0.0 && whole.times.poisson > 0.0 && whole.times.marching_cubes > 0.0,
                "reconstruct_chunk times the normal, Poisson and marching cubes stages");
    ok &= check(mbmesh::format_duration(12.34) == "12.3 s" && mbmesh::format_duration(125.06) == "2 min 5.1 s",
                "format_duration gives seconds, then minutes and seconds");
    ok &= check(mbmesh::format_minutes(191.3) == "  3.2 min" && mbmesh::format_minutes(2793.3) == " 46.6 min" &&
                    mbmesh::format_minutes(7261.0) == "121.0 min",
                "format_minutes gives minutes with a decimal, padded so the decimal points line up");
    ok &= check(whole.times.poisson_solve > 0.0 && whole.times.poisson_solve <= whole.times.poisson,
                "the solver iterations are timed and are part of the Poisson time");
    const double whole_error = max_height_error(whole.mesh);

    // ---- the solver reports how far it got
    {
        PoissonSolver solver = PoissonSolver::Jacobi;
        ok &= check(parse_poisson_solver("jacobi", &solver) && solver == PoissonSolver::Jacobi &&
                        std::string(poisson_solver_name(solver)) == "jacobi",
                    "the solver name jacobi round-trips");
        ok &= check(!parse_poisson_solver("bogus", &solver), "an unknown solver name is refused");

        const OrientedPointCloud oriented = whole.oriented_points;
        PoissonDiagnostics few, many;
        ScreenedPoissonOptions poisson = options.poisson;
        poisson.solver = PoissonSolver::Jacobi;   // the baseline that sor is compared with
        poisson.solver_iterations = 20;
        (void)screened_poisson(oriented, poisson, &few);
        poisson.solver_iterations = 200;
        (void)screened_poisson(oriented, poisson, &many);
        std::printf("residual: 20 iterations %.3e, 200 iterations %.3e (midway %.3e)\n", few.residual_final, many.residual_final,
                    many.residual_midway);
        ok &= check(few.cells > 0 && many.iterations == 200, "diagnostics report the grid size and the iterations");
        ok &= check(few.residual_final > 0.0 && few.residual_final < 1.0, "the relative residual starts below 1 after some iterations");
        ok &= check(many.residual_final < few.residual_final, "more iterations lower the residual");
        ok &= check(many.residual_midway > many.residual_final, "the residual falls between the midpoint and the end");
        ok &= check(many.splat_seconds > 0.0 && many.solve_seconds > 0.0, "splat and solve are timed separately");

        // sor reaches a given residual in far fewer sweeps than jacobi
        ok &= check(parse_poisson_solver("sor", &solver) && solver == PoissonSolver::Sor &&
                        std::string(poisson_solver_name(solver)) == "sor",
                    "the solver name sor round-trips");
        PoissonDiagnostics sor_few, sor_many;
        poisson.solver = PoissonSolver::Sor;
        poisson.solver_iterations = 20;
        const ScalarGrid3D sor_field = screened_poisson(oriented, poisson, &sor_few);
        poisson.solver_iterations = 100;
        (void)screened_poisson(oriented, poisson, &sor_many);
        std::printf("residual: sor 20 iterations %.3e, sor 100 iterations %.3e, relaxation %.3f\n", sor_few.residual_final,
                    sor_many.residual_final, sor_many.relaxation);
        ok &= check(sor_many.relaxation > 1.0 && sor_many.relaxation < 2.0, "the over-relaxation factor is between 1 and 2");
        ok &= check(sor_few.residual_final < few.residual_final, "20 sor iterations beat 20 jacobi iterations");
        ok &= check(sor_many.residual_final < many.residual_final, "100 sor iterations beat 200 jacobi iterations");

        // with no count given, sor chooses one from the grid size and converges well
        PoissonDiagnostics automatic;
        poisson.solver = PoissonSolver::Sor;
        poisson.solver_iterations = 0;
        (void)screened_poisson(oriented, poisson, &automatic);
        std::printf("sor with the automatic count: %d iterations, residual %.3e\n", automatic.iterations, automatic.residual_final);
        ok &= check(automatic.iterations >= 50 && automatic.residual_final < 1.0e-2, "sor chooses its own iteration count and converges");
        poisson.solver = PoissonSolver::Jacobi;
        PoissonDiagnostics automatic_jacobi;
        (void)screened_poisson(oriented, poisson, &automatic_jacobi);
        ok &= check(automatic_jacobi.iterations == 200, "jacobi with no count takes 200 iterations");

        // both solve the same equation, so given enough iterations they give the same field
        poisson.solver = PoissonSolver::Jacobi;
        poisson.solver_iterations = 4000;
        const ScalarGrid3D jacobi_field = screened_poisson(oriented, poisson);
        poisson.solver = PoissonSolver::Sor;
        poisson.solver_iterations = 400;
        const ScalarGrid3D converged_sor = screened_poisson(oriented, poisson);
        double largest_difference = 0.0;
        for (std::size_t n = 0; n < jacobi_field.values.size() && n < converged_sor.values.size(); n++) {
            largest_difference = std::max(largest_difference, std::fabs(jacobi_field.values[n] - converged_sor.values[n]));
        }
        std::printf("largest field difference, jacobi 4000 vs sor 400: %.3e\n", largest_difference);
        ok &= check(jacobi_field.values.size() == converged_sor.values.size() && largest_difference < 1.0e-2,
                    "converged jacobi and sor agree on the field");
        (void)sor_field;
    }
    std::printf("whole survey: %zu vertices, %zu triangles, max height error %.3f m\n",
                whole.mesh.vertices.size(), whole.mesh.indices.size() / 3, whole_error);

    // ---- the same survey in chunks
    ChunkingOptions chunking;
    chunking.chunk_size = chunk_size;
    chunking.halo = halo;

    Mesh merged;
    std::vector<ChunkResult> chunk_meshes;
    ChunkingStats stats;
    std::string error;
    const bool chunked_ok = reconstruct_chunked(
        points, options, chunking,
        [&](ChunkResult &chunk) {
            append(&merged, chunk.result.mesh);
            chunk_meshes.push_back(chunk);
            return true;
        },
        &stats, &error);
    ok &= check(chunked_ok, "chunked reconstruction succeeds: " + error);
    ok &= check(stats.chunks_processed == 9, "a 30 m survey in 10 m chunks gives 3 x 3 chunks, got " + std::to_string(stats.chunks_processed));
    ok &= check(stats.chunks_failed == 0 && stats.chunks_skipped_sparse == 0, "no chunk fails or is skipped");
    ok &= check(!chunk_meshes.empty() && std::all_of(chunk_meshes.begin(), chunk_meshes.end(), [](const ChunkResult &chunk) {
                    return chunk.result.times.poisson > 0.0;
                }),
                "every delivered chunk carries its stage times");

    const double chunked_error = max_height_error(merged);
    std::printf("chunked:      %zu vertices, %zu triangles, max height error %.3f m (%zu chunks)\n",
                merged.vertices.size(), merged.indices.size() / 3, chunked_error, stats.chunks_processed);

    // Chunking must not make the surface noticeably worse than one run does.
    ok &= check(chunked_error <= whole_error + 0.5 * options.poisson.cell_size,
                "chunked surface is as accurate as the whole-survey surface (" + std::to_string(chunked_error) +
                    " vs " + std::to_string(whole_error) + ")");

    // Each chunk owns only triangles whose centroids lie inside it.
    for (const ChunkResult &chunk : chunk_meshes) {
        const Mesh &mesh = chunk.result.mesh;
        bool inside = true;
        for (std::size_t t = 0; t + 2 < mesh.indices.size(); t += 3) {
            const Vec3 &a = mesh.vertices[mesh.indices[t]];
            const Vec3 &b = mesh.vertices[mesh.indices[t + 1]];
            const Vec3 &c = mesh.vertices[mesh.indices[t + 2]];
            const double cx = (a.x + b.x + c.x) / 3.0;
            const double cy = (a.y + b.y + c.y) / 3.0;
            inside = inside && cx >= chunk.i * chunk_size && cx < (chunk.i + 1) * chunk_size &&
                     cy >= chunk.j * chunk_size && cy < (chunk.j + 1) * chunk_size;
        }
        ok &= check(inside, "chunk " + std::to_string(chunk.i) + "," + std::to_string(chunk.j) + " owns only triangles inside it");
    }

    // No gaps: sample points away from the survey's outer edge, including
    // points right on the seams, must be covered by some triangle.
    int uncovered = 0;
    int samples = 0;
    for (double x = 4.0; x <= survey_size - 4.0; x += 0.7) {
        for (double y = 4.0; y <= survey_size - 4.0; y += 0.7) {
            samples++;
            if (!covered(merged, x, y)) {
                uncovered++;
            }
        }
    }
    for (const double seam : {10.0, 20.0}) {
        for (double along = 4.0; along <= survey_size - 4.0; along += 0.7) {
            samples += 2;
            if (!covered(merged, seam, along)) {
                uncovered++;
            }
            if (!covered(merged, along, seam)) {
                uncovered++;
            }
        }
    }
    std::printf("coverage:     %d of %d interior samples uncovered\n", uncovered, samples);
    ok &= check(uncovered == 0, "merged chunks leave no gaps in the interior");

    // The thread count must not change the result: same chunks, in the same
    // order, with identical meshes.
    {
        ChunkingOptions threaded = chunking;
        threaded.threads = 4;
        std::vector<std::pair<int, int>> order;
        Mesh threaded_merged;
        ok &= check(reconstruct_chunked(points, options, threaded, [&](ChunkResult &chunk) {
                        order.push_back({chunk.i, chunk.j});
                        append(&threaded_merged, chunk.result.mesh);
                        return true;
                    }, &stats, &error),
                    "threaded chunked reconstruction succeeds: " + error);

        bool same_order = order.size() == chunk_meshes.size();
        for (std::size_t n = 0; same_order && n < order.size(); n++) {
            same_order = order[n].first == chunk_meshes[n].i && order[n].second == chunk_meshes[n].j;
        }
        ok &= check(same_order, "chunks are delivered in chunk order whatever the thread count");

        bool same_mesh = threaded_merged.vertices.size() == merged.vertices.size() &&
                         threaded_merged.indices == merged.indices;
        for (std::size_t n = 0; same_mesh && n < merged.vertices.size(); n++) {
            same_mesh = merged.vertices[n].x == threaded_merged.vertices[n].x &&
                        merged.vertices[n].y == threaded_merged.vertices[n].y &&
                        merged.vertices[n].z == threaded_merged.vertices[n].z;
        }
        ok &= check(same_mesh, "four threads produce exactly the same mesh as one");

        // Stopping from the callback stops the run.
        std::size_t seen = 0;
        ok &= check(reconstruct_chunked(points, options, threaded, [&](ChunkResult &) {
                        seen++;
                        return false;
                    }, &stats, &error) && seen == 1,
                    "a callback returning false stops processing");
    }

    // Single-chunk and chunk-count limits.
    {
        ChunkingOptions one = chunking;
        one.only_chunks = {{1, 1}};
        std::size_t seen = 0;
        ok &= check(reconstruct_chunked(points, options, one, [&](ChunkResult &chunk) {
                        seen++;
                        return chunk.i == 1 && chunk.j == 1;
                    }, &stats, &error) && seen == 1,
                    "--chunk selects exactly one chunk");

        ChunkingOptions several = chunking;
        several.only_chunks = {{1, 1}, {0, 0}};
        std::set<std::pair<int, int>> delivered;
        ok &= check(reconstruct_chunked(points, options, several, [&](ChunkResult &chunk) {
                        delivered.insert({chunk.i, chunk.j});
                        return true;
                    }, &stats, &error) && delivered == std::set<std::pair<int, int>>({{0, 0}, {1, 1}}),
                    "--chunk twice selects exactly those two chunks");

        ChunkingOptions limited = chunking;
        limited.max_chunks = 2;
        seen = 0;
        ok &= check(reconstruct_chunked(points, options, limited, [&](ChunkResult &) {
                        seen++;
                        return true;
                    }, &stats, &error) && seen == 2,
                    "--max-chunks limits the number of chunks");
    }

    // The same survey from chunk files on disk gives the same chunks as from memory.
    {
        const std::filesystem::path directory = std::filesystem::temp_directory_path() / ("mbmesh_test_chunking_bins_" + std::to_string(getpid()));
        ChunkBins bins(directory, chunk_size, halo, 1000);   // a small buffer, so that the chunk files are appended to several times
        std::string bins_error;
        bool binned = bins.open(&bins_error);
        for (const CollectedPoint &point : points) {
            binned = binned && bins.add(point, &bins_error);
        }
        binned = binned && bins.finish(&bins_error);
        ok &= check(binned, "the points can be sorted into chunk files: " + bins_error);
        ok &= check(bins.owned_point_count() == points.size(), "the chunk files own every point once");

        std::vector<ChunkResult> memory_chunks;
        ok &= check(reconstruct_chunked(points, options, chunking, [&](ChunkResult &chunk) {
                        memory_chunks.push_back(chunk);
                        return true;
                    }, &stats, &error), "chunked reconstruction from memory succeeds: " + error);
        std::vector<ChunkResult> disk_chunks;
        ChunkingStats disk_stats;
        ok &= check(reconstruct_chunked(bins, options, chunking, [&](ChunkResult &chunk) {
                        disk_chunks.push_back(chunk);
                        return true;
                    }, &disk_stats, &error), "chunked reconstruction from chunk files succeeds: " + error);
        ok &= check(disk_stats.chunks_processed == stats.chunks_processed && disk_chunks.size() == memory_chunks.size(),
                    "chunk files give the same number of chunks as memory");
        bool same = disk_chunks.size() == memory_chunks.size();
        for (std::size_t n = 0; same && n < disk_chunks.size(); n++) {
            const Mesh &a = disk_chunks[n].result.mesh;
            const Mesh &b = memory_chunks[n].result.mesh;
            same = disk_chunks[n].i == memory_chunks[n].i && disk_chunks[n].j == memory_chunks[n].j &&
                   disk_chunks[n].input_points == memory_chunks[n].input_points && a.indices == b.indices && a.vertices.size() == b.vertices.size();
            for (std::size_t v = 0; same && v < a.vertices.size(); v++) {
                same = a.vertices[v].x == b.vertices[v].x && a.vertices[v].y == b.vertices[v].y && a.vertices[v].z == b.vertices[v].z;
            }
        }
        ok &= check(same, "chunks reconstructed from chunk files are identical to those from memory");

        // --chunk from chunk files keeps one chunk, as from memory.
        ChunkBins one_bins(directory / "one", chunk_size, halo);
        one_bins.keep_only_chunks({{1, 1}});
        bool one_binned = one_bins.open(&bins_error);
        for (const CollectedPoint &point : points) {
            one_binned = one_binned && one_bins.add(point, &bins_error);
        }
        one_binned = one_binned && one_bins.finish(&bins_error);
        ok &= check(one_binned && one_bins.chunks().size() == 1 && one_bins.chunks().begin()->first == std::make_pair(1, 1),
                    "only the requested chunk is kept when one chunk is asked for");

        bins.remove();
        std::error_code ignored;
        std::filesystem::remove_all(directory, ignored);
        ok &= check(!std::filesystem::exists(directory), "removing the bins deletes their directory");
    }

    std::printf(ok ? "chunking tests passed\n" : "chunking tests FAILED\n");
    return ok ? 0 : 1;
}
