#include "settings.h"

#include "util/logger.h"

#include "math/kdtree.h"
#include "util/memory.h"
#include "util/stopwatch.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <limits>
#include <system_error>
#include <thread>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>
#include <unordered_map>
#include <utility>
#include <sstream>
#include <vector>

namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double minimum_feature_size = 0.01; // meters
constexpr double maximum_feature_size = 5.0;  // meters
constexpr double automatic_feature_size_spacing_scale = 2.0;
constexpr double maximum_poisson_grid_cells = 45.0e6;

// Target size of one chunk's Poisson grid. The solver holds seven grids of
// about 72 bytes per cell in all, so this is roughly 0.6 GB.
constexpr double target_chunk_grid_cells = 8.0e6;
constexpr double minimum_auto_chunk_cells = 16.0;
constexpr double maximum_auto_chunk_cells = 512.0;

// Automatic chunking aims for at least this many chunks per thread, so that all
// threads stay busy and uneven chunks at the survey edge balance out. A chunk is
// never made smaller than this many halos across, to keep the cost of the
// halo and padding, which every chunk reconstructs again, to a modest factor.
constexpr double target_chunks_per_thread = 2.0;

// When the level of detail is not given, it is the finest the data supports
// (twice the point spacing), unless that would need more Poisson cells than this
// across the survey's bounding box, in which case it is coarsened to fit. Work
// grows as the cube of the resolution, so a dense survey would otherwise ask for
// a centimeter-scale cell and run for days. This is only a default: -L is
// honored as given. The budget is about a quarter of an hour on four threads.
constexpr double automatic_work_budget_cells = 5.0e8;

// An explicitly requested level of detail that needs more cells than this gets a
// warning that the run may take hours.
constexpr double large_run_warning_cells = 2.0e9;
constexpr double minimum_chunk_halos = 4.0;

// Point spacing is measured inside blocks of consecutive points: each block is
// searched in full, and several blocks spread through the dataset are used. A
// dataset up to the product of the two is searched in one piece.
constexpr std::size_t spacing_block_size = 16384;
constexpr std::size_t spacing_block_count = 16;

struct PoissonGridEstimate {
    std::size_t nx = 0;
    std::size_t ny = 0;
    std::size_t nz = 0;
    double cell_count = 0.0;
};

void apply_cell_size_dependent_defaults(Options &options, double cell_size, double average_spacing);

Bounds3D collected_point_bounds(const CollectedPointCloud &points) {
    Bounds3D bounds(points.front().point, points.front().point);
    for (const CollectedPoint &point : points) {
        bounds.include(point.point);
    }
    return bounds;
}

GeodeticBounds collected_geodetic_bounds(const CollectedPointCloud &points, const CoordinateFrame &frame, double *mean_elevation) {
    GeodeticBounds bounds;
    bool initialized = false;
    double elevation_sum = 0.0;
    for (const CollectedPoint &point : points) {
        const GeodeticPosition position = local_to_geodetic(point.point, frame);
        elevation_sum += position.elevation;

        if (!initialized) {
            bounds.min_lon = position.longitude;
            bounds.max_lon = position.longitude;
            bounds.min_lat = position.latitude;
            bounds.max_lat = position.latitude;
            initialized = true;
            continue;
        }

        bounds.min_lon = std::min(bounds.min_lon, position.longitude);
        bounds.max_lon = std::max(bounds.max_lon, position.longitude);
        bounds.min_lat = std::min(bounds.min_lat, position.latitude);
        bounds.max_lat = std::max(bounds.max_lat, position.latitude);
    }
    if (mean_elevation != nullptr && !points.empty()) {
        *mean_elevation = elevation_sum / static_cast<double>(points.size());
    }
    return bounds;
}

// The blocks the spacing is measured in, as the index of the first point and the number of points: the whole
// dataset if it is small, otherwise spacing_block_count blocks spread evenly through it.
std::vector<std::pair<std::size_t, std::size_t>> spacing_blocks(std::size_t size) {
    std::vector<std::pair<std::size_t, std::size_t>> blocks;
    if (size <= spacing_block_size * spacing_block_count) {
        blocks.push_back({0, size});
    }
    else {
        for (std::size_t b = 0; b < spacing_block_count; b++) {
            const std::size_t first = b * (size - spacing_block_size) / (spacing_block_count - 1);
            blocks.push_back({first, spacing_block_size});
        }
    }
    return blocks;
}

// The nearest-neighbor distances within each block of positions.
SpacingMetadata spacing_from_blocks(const std::vector<std::vector<Vec3>> &blocks) {
    SpacingMetadata metadata;
    std::vector<double> distances;
    for (const std::vector<Vec3> &positions : blocks) {
        const KDTree tree(positions);
        for (std::size_t i = 0; i < positions.size(); i++) {
            const std::vector<KDTree::Neighbor> neighbors = tree.k_nearest(positions[i], 1, i);
            if (neighbors.empty()) {
                continue;
            }

            const double distance_squared = neighbors.front().distance_squared;
            if (distance_squared > vec3_epsilon * vec3_epsilon) {
                distances.push_back(std::sqrt(distance_squared));
            }
        }
    }
    metadata.sample_count = distances.size();

    if (distances.empty()) {
        return metadata;
    }

    std::sort(distances.begin(), distances.end());
    const double distance_sum = std::accumulate(distances.begin(), distances.end(), 0.0);

    metadata.minimum = distances.front();
    metadata.maximum = distances.back();
    metadata.average = distance_sum / static_cast<double>(distances.size());
    metadata.median = distances[distances.size() / 2];
    return metadata;
}

// Estimates the spacing of the soundings from nearest-neighbor distances.
//
// Consecutive soundings are spatially adjacent, since they come from consecutive
// beams of consecutive pings, so a block of consecutive points holds real
// neighborhoods and its nearest-neighbor distances are the true local spacing.
// Searching a subsample spread over the whole dataset instead, as this once did
// with 8192 points, finds neighbors that are far apart and overstates the
// spacing by about sqrt(N / 8192), so that a dense survey looked sparse and the
// reconstruction was far coarser than the data. Soundings from other, overlapping
// lines are not in the same block, so this is the spacing along one line, which
// is the conservative choice.
SpacingMetadata estimate_point_spacing_in_blocks(const CollectedPointCloud &points) {
    if (points.size() < 2) {
        return SpacingMetadata();
    }

    std::vector<std::vector<Vec3>> blocks;
    for (const auto &block : spacing_blocks(points.size())) {
        std::vector<Vec3> positions;
        positions.reserve(block.second);
        for (std::size_t i = 0; i < block.second; i++) {
            positions.push_back(points[block.first + i].point);
        }
        blocks.push_back(std::move(positions));
    }
    return spacing_from_blocks(blocks);
}

// The statistics that preprocess_datalist() takes from the points, taken instead from the file of spilled soundings, which
// is read once in pieces: the bounds, the geodetic bounds and mean elevation, the spacing (from the same blocks of
// consecutive points, picked out of the pieces as they go by) and a sample of the points.
// The sensor position of each ping, in the local frame, which the spilled soundings name instead of holding.
std::vector<Vec3> ping_origins(const DatalistReadResult &read_result) {
    std::vector<Vec3> origins;
    origins.reserve(read_result.track.size());
    for (const TrackPoint &track_point : read_result.track) {
        origins.push_back(track_point.position);
    }
    return origins;
}

bool spilled_statistics(const DatalistReadResult &read_result, DatalistMetadata *metadata, CollectedPointCloud *sample, std::string *error) {
    constexpr std::size_t piece_points = 4000000;
    constexpr std::size_t maximum_samples = 1000000;
    const std::size_t total = read_result.stats.soundings_accepted;
    const std::size_t stride = std::max<std::size_t>(1, total / maximum_samples);

    const std::vector<std::pair<std::size_t, std::size_t>> blocks = spacing_blocks(total);
    std::vector<std::vector<Vec3>> block_positions(blocks.size());

    bool first_point = true;
    Bounds3D bounds;
    GeodeticBounds geodetic;
    double elevation_sum = 0.0;
    sample->clear();
    sample->reserve(maximum_samples + 1);

    const std::vector<Vec3> origins = ping_origins(read_result);
    const bool read = read_spill_pieces(read_result.spill_path, origins, piece_points, [&](CollectedPointCloud &piece, std::size_t first_index) {
        for (std::size_t n = 0; n < piece.size(); n++) {
            CollectedPoint &point = piece[n];
            point.point = ecef_to_local(point.point, read_result.frame);

            const GeodeticPosition position = local_to_geodetic(point.point, read_result.frame);
            elevation_sum += position.elevation;
            if (first_point) {
                bounds = Bounds3D(point.point, point.point);
                geodetic = {position.longitude, position.longitude, position.latitude, position.latitude};
                first_point = false;
            }
            else {
                bounds.include(point.point);
                geodetic.min_lon = std::min(geodetic.min_lon, position.longitude);
                geodetic.max_lon = std::max(geodetic.max_lon, position.longitude);
                geodetic.min_lat = std::min(geodetic.min_lat, position.latitude);
                geodetic.max_lat = std::max(geodetic.max_lat, position.latitude);
            }

            const std::size_t index = first_index + n;
            if (index % stride == 0) {
                sample->push_back(point);
            }
            for (std::size_t b = 0; b < blocks.size(); b++) {
                if (index >= blocks[b].first && index < blocks[b].first + blocks[b].second) {
                    block_positions[b].push_back(point.point);
                }
            }
        }
        return true;
    }, error);
    if (!read) {
        return false;
    }
    if (first_point) {
        if (error != nullptr) {
            *error = "the file of spilled soundings is empty";
        }
        return false;
    }

    metadata->point_count = total;
    metadata->bounds = bounds;
    metadata->geodetic_bounds = geodetic;
    metadata->mean_elevation = elevation_sum / static_cast<double>(total);
    metadata->spacing = total < 2 ? SpacingMetadata() : spacing_from_blocks(block_positions);
    return true;
}

// Number of chunks of this size that the survey's horizontal extent touches.
std::size_t chunks_covering(const Bounds3D &bounds, double chunk_size) {
    const double chunks_x = std::floor(bounds.max.x / chunk_size) - std::floor(bounds.min.x / chunk_size) + 1.0;
    const double chunks_y = std::floor(bounds.max.y / chunk_size) - std::floor(bounds.min.y / chunk_size) + 1.0;
    return static_cast<std::size_t>(chunks_x * chunks_y);
}

// The largest vertical extent of the points within any one chunk of this size. A
// chunk's Poisson grid is as tall as the relief inside it, not as tall as the whole
// survey's relief, so using the survey's full depth range makes chunks needlessly
// small on a sloping seafloor. A sample of the points is enough.
double tallest_chunk(const CollectedPointCloud &points, double chunk_size) {
    constexpr std::size_t maximum_samples = 1000000;
    const std::size_t stride = std::max<std::size_t>(1, points.size() / maximum_samples);

    std::unordered_map<long long, std::pair<double, double>> extent;
    for (std::size_t n = 0; n < points.size(); n += stride) {
        const long long i = static_cast<long long>(std::floor(points[n].point.x / chunk_size));
        const long long j = static_cast<long long>(std::floor(points[n].point.y / chunk_size));
        const long long key = (i << 32) ^ (j & 0xffffffffLL);
        const double z = points[n].point.z;
        auto found = extent.find(key);
        if (found == extent.end()) {
            extent.emplace(key, std::make_pair(z, z));
        } else {
            found->second.first = std::min(found->second.first, z);
            found->second.second = std::max(found->second.second, z);
        }
    }

    double tallest = 0.0;
    for (const auto &entry : extent) {
        tallest = std::max(tallest, entry.second.second - entry.second.first);
    }
    return tallest;
}

// Number of chunks of this size that contain points. A survey along a diagonal
// track touches far fewer chunks than its bounding box suggests. A sample of the
// points is enough, since this only steers the choice of chunk size.
std::size_t chunks_occupied(const CollectedPointCloud &points, double chunk_size) {
    constexpr std::size_t maximum_samples = 1000000;
    const std::size_t stride = std::max<std::size_t>(1, points.size() / maximum_samples);

    std::set<std::pair<long long, long long>> occupied;
    for (std::size_t n = 0; n < points.size(); n += stride) {
        occupied.insert({static_cast<long long>(std::floor(points[n].point.x / chunk_size)),
                         static_cast<long long>(std::floor(points[n].point.y / chunk_size))});
    }
    return occupied.size();
}

PoissonGridEstimate estimate_poisson_grid(const Bounds3D &bounds, double cell_size, double padding) {
    PoissonGridEstimate estimate;
    if (cell_size <= 0.0 || !std::isfinite(cell_size) || !std::isfinite(padding)) {
        return estimate;
    }

    const Vec3 size = bounds.size() + Vec3(2.0 * padding);
    estimate.nx = static_cast<std::size_t>(std::ceil(std::max(0.0, size.x) / cell_size)) + 1;
    estimate.ny = static_cast<std::size_t>(std::ceil(std::max(0.0, size.y) / cell_size)) + 1;
    estimate.nz = static_cast<std::size_t>(std::ceil(std::max(0.0, size.z) / cell_size)) + 1;
    estimate.cell_count = static_cast<double>(estimate.nx) *
                          static_cast<double>(estimate.ny) *
                          static_cast<double>(estimate.nz);
    return estimate;
}

void apply_cell_size_dependent_defaults(Options &options, double cell_size, double average_spacing) {
    const double resolved_cell_size = std::max(cell_size, minimum_feature_size * 0.05);

    options.poisson.cell_size = resolved_cell_size;
    options.poisson.padding = std::clamp(4.0 * resolved_cell_size, 2.0 * resolved_cell_size, 5.0);
    options.poisson.normal_splat_radius = std::max(2.0 * resolved_cell_size, 1.5 * average_spacing);

    options.trimming.support_radius = std::max(4.0 * resolved_cell_size, 2.5 * average_spacing);
    options.trimming.max_normal_offset = std::max(2.0 * resolved_cell_size, 1.25 * average_spacing);
}

void print_warnings(const Options &options, const DatalistMetadata &metadata) {
    if (metadata.automatic_lod_before_limit > 0.0) {
        LOG_INFO("automatic level of detail raised from", metadata.automatic_lod_before_limit, "m to", options.level_of_detail,
                 "m to keep the reconstruction within about", automatic_work_budget_cells,
                 "Poisson cells; use -L to choose the level of detail");
    }

    if (options.level_of_detail_requested) {
        const PoissonGridEstimate grid = estimate_poisson_grid(metadata.bounds, options.poisson.cell_size, options.poisson.padding);
        if (grid.cell_count > large_run_warning_cells) {
            LOG_WARN("level of detail", options.level_of_detail, "m needs a Poisson cell of", options.poisson.cell_size,
                     "m, about", grid.cell_count, "cells across the survey; this may take hours, so consider a larger -L");
        }
    }

    if (!metadata.lod_supported_by_spacing) {
        LOG_WARN("resolved level_of_detail", options.level_of_detail,
                 "m is not supported by the estimated average point spacing", metadata.spacing.average,
                 "m; the result may be inaccurate");
    }

    if (metadata.normal_radius_exceeds_feature_limit) {
        LOG_WARN("normal estimation radius", options.normals.search_radius,
                 "m exceeds 45% of requested feature size", options.level_of_detail,
                 "m; normals may smooth features smaller than the radius");
    }
}

} // namespace

SpacingMetadata estimate_point_spacing(const CollectedPointCloud &points) {
    return estimate_point_spacing_in_blocks(points);
}

void apply_spacing_driven_defaults(Options &options, DatalistMetadata *metadata) {
    const double fallback_feature_size =
        options.level_of_detail > vec3_epsilon ? options.level_of_detail : minimum_feature_size;
    const double average_spacing =
        metadata->spacing.average > vec3_epsilon ? metadata->spacing.average : fallback_feature_size;
    double desired_feature_size =
        std::clamp(
            options.level_of_detail_requested
                ? options.level_of_detail
                : automatic_feature_size_spacing_scale * average_spacing,
            minimum_feature_size,
            maximum_feature_size);

    // The Poisson cell is a quarter of the level of detail, but no finer than half
    // the point spacing, since the data cannot support more. It is deliberately
    // not capped at the spacing: oversampled data should be reconstructed at the
    // level of detail that was asked for, not at the finest scale in the data.
    const auto cell_for = [&](double feature_size) {
        return std::max(feature_size / 4.0, average_spacing / 2.0);
    };
    const auto grid_cells_for = [&](double feature_size) {
        const double cell = std::max(cell_for(feature_size), minimum_feature_size * 0.05);
        const double padding = std::clamp(4.0 * cell, 2.0 * cell, 5.0);
        return estimate_poisson_grid(metadata->bounds, cell, padding).cell_count;
    };

    metadata->automatic_lod_before_limit = 0.0;
    if (!options.level_of_detail_requested && grid_cells_for(desired_feature_size) > automatic_work_budget_cells) {
        metadata->automatic_lod_before_limit = desired_feature_size;
        while (grid_cells_for(desired_feature_size) > automatic_work_budget_cells && desired_feature_size < maximum_feature_size) {
            desired_feature_size = std::min(maximum_feature_size, desired_feature_size * 1.1);
        }
    }

    options.level_of_detail = desired_feature_size;

    if (!options.decimation_requested) {
        options.decimation.decimate = false;
        options.decimation.cell_size = std::max(0.5 * desired_feature_size, 0.5 * average_spacing);
    }

    const double poisson_cell_size = cell_for(desired_feature_size);
    apply_cell_size_dependent_defaults(options, poisson_cell_size, average_spacing);

    const double normal_radius =
        std::max(2.5 * average_spacing, 2.5 * options.poisson.cell_size);
    metadata->normal_radius_exceeds_feature_limit =
        normal_radius > desired_feature_size;

    options.normals.search_radius = normal_radius;
    options.normals.k = static_cast<std::size_t>(std::clamp(
        std::lround(pi * std::pow(normal_radius / average_spacing, 2.0)),
        24L,
        48L));
    options.normals.minimum_neighbors = 6;

    options.poisson.screening_weight =
        std::clamp(4.0 * options.poisson.cell_size / average_spacing, 1.0, 6.0);
    options.poisson.solver_iterations = std::clamp(
        static_cast<int>(std::round(220.0 + 80.0 / std::sqrt(desired_feature_size))),
        200,
        500);
    options.poisson.use_screening = true;
    options.poisson.iso_value = 0.0;

    options.marching_cubes.iso_value = 0.0;

    options.trimming.enabled = true;
    options.trimming.minimum_neighbors = 3;
    options.trimming.minimum_normal_alignment = 0.0;

    metadata->lod_supported_by_spacing =
        desired_feature_size / average_spacing >= automatic_feature_size_spacing_scale;
}

namespace {

// A whole number, or FIRST..LAST (both included). False if the text is not one.
bool parse_chunk_range(const std::string &text, long *first, long *last) {
    const auto whole = [](const std::string &part, long *value) {
        if (part.empty()) {
            return false;
        }
        char *end = nullptr;
        errno = 0;
        const long number = std::strtol(part.c_str(), &end, 10);
        if (end == part.c_str() || *end != '\0' || errno != 0 || number < -1000000000L || number > 1000000000L) {
            return false;
        }
        *value = number;
        return true;
    };
    const std::size_t dots = text.find("..");
    if (dots == std::string::npos) {
        return whole(text, first) && (*last = *first, true);
    }
    return whole(text.substr(0, dots), first) && whole(text.substr(dots + 2), last) && *first <= *last;
}

} // namespace

bool parse_chunk_selection(const std::string &text, std::set<std::pair<int, int>> *chunks, std::string *error) {
    const auto fail = [&](const std::string &why) {
        if (error != nullptr) {
            *error = "'" + text + "' is not chunks: " + why + " (expected I,J, where I and J are each a whole number or a range FIRST..LAST, for example 2,-10 or 2..3,-11..-10)";
        }
        return false;
    };
    const std::size_t comma = text.find(',');
    if (comma == std::string::npos || text.find(',', comma + 1) != std::string::npos) {
        return fail("it needs one comma, between I and J");
    }
    long i_first = 0, i_last = 0, j_first = 0, j_last = 0;
    if (!parse_chunk_range(text.substr(0, comma), &i_first, &i_last)) {
        return fail("I is not a whole number or a range with FIRST not more than LAST");
    }
    if (!parse_chunk_range(text.substr(comma + 1), &j_first, &j_last)) {
        return fail("J is not a whole number or a range with FIRST not more than LAST");
    }
    constexpr long maximum_chunks = 100000;
    if ((i_last - i_first + 1) > maximum_chunks || (j_last - j_first + 1) > maximum_chunks ||
        (i_last - i_first + 1) * (j_last - j_first + 1) > maximum_chunks) {
        return fail("it names more than " + std::to_string(maximum_chunks) + " chunks");
    }
    for (long i = i_first; i <= i_last; i++) {
        for (long j = j_first; j <= j_last; j++) {
            chunks->insert({static_cast<int>(i), static_cast<int>(j)});
        }
    }
    return true;
}

ReconstructionOptions reconstruction_options(const Options &options) {
    ReconstructionOptions reconstruction;
    reconstruction.normals = options.normals;
    reconstruction.poisson = options.poisson;
    reconstruction.poisson.solver = options.poisson_solver;
    if (options.poisson_iterations > 0) {
        reconstruction.poisson.solver_iterations = options.poisson_iterations;
    } else if (options.poisson_solver == PoissonSolver::Sor) {
        // The count chosen from the level of detail is meant for jacobi; sor chooses
        // its own from the size of each grid.
        reconstruction.poisson.solver_iterations = 0;
    }
    reconstruction.marching_cubes = options.marching_cubes;
    reconstruction.trimming = options.trimming;
    return reconstruction;
}

bool resolve_chunking(Options &options, const DatalistMetadata &metadata, const CollectedPointCloud *points, std::string *error) {
    options.use_chunking = false;

    const double cell = options.poisson.cell_size;
    const PoissonGridEstimate whole = estimate_poisson_grid(metadata.bounds, cell, options.poisson.padding);

    const bool forced = options.chunk_size_requested || !options.chunk_selection.empty() || options.max_chunks > 0;
    if (!forced && whole.cell_count <= target_chunk_grid_cells) {
        return true;   // small enough to reconstruct in one piece
    }

    const ReconstructionOptions reconstruction = reconstruction_options(options);
    // The halo that keeps the stages near a chunk edge exact is the least that is safe, and the default. --chunk-halo sets
    // it: a larger one reads more neighboring points, and also changes the chunk size and the points each chunk's iso-value
    // is found from, which moved the surfaces of neighboring chunks apart at their edges in a test on a multibeam survey.
    const double minimum_halo = chunk_halo(reconstruction);
    double halo = minimum_halo;
    if (options.chunk_halo_requested) {
        if (!(options.chunk_halo > 0.0) || !std::isfinite(options.chunk_halo)) {
            if (error != nullptr) {
                *error = "--chunk-halo must be a positive number of meters";
            }
            return false;
        }
        halo = round_to_cells(options.chunk_halo, cell);
        if (halo < minimum_halo) {
            LOG_WARN("--chunk-halo", options.chunk_halo, "m is less than the", minimum_halo,
                     "m that the reach of the normal estimation, splatting and trimming needs: the mesh near chunk edges may differ from a single run");
        }
    }

    // Each chunk holds its own Poisson grid, so cap the automatic thread count to
    // keep memory modest. Ask for more with --threads.
    constexpr std::size_t automatic_thread_limit = 4;
    const std::size_t hardware = std::max<std::size_t>(1, std::thread::hardware_concurrency());
    const std::size_t threads = options.threads > 0 ? options.threads : std::min(hardware, automatic_thread_limit);

    double chunk_size = 0.0;
    if (options.chunk_size_requested) {
        if (!(options.chunk_size > 0.0) || !std::isfinite(options.chunk_size)) {
            if (error != nullptr) {
                *error = "--chunk-size must be a positive number of meters";
            }
            return false;
        }
        chunk_size = round_to_cells(options.chunk_size, cell);
    }
    else {
        // Pick the largest chunk whose grid, including its halo and padding, holds
        // no more than about target_chunk_grid_cells. The grid is as tall as the
        // tallest relief inside one chunk of that size, measured from the points
        // (or the survey's full depth range if no points are given).
        const double margin = 2.0 * (halo + options.poisson.padding);
        const double smallest_chunk = minimum_auto_chunk_cells * cell;
        chunk_size = maximum_auto_chunk_cells * cell;
        while (chunk_size > smallest_chunk) {
            const double depth = points != nullptr ? tallest_chunk(*points, chunk_size) : metadata.bounds.size().z;
            const double side = (chunk_size + margin) / cell + 1.0;
            const double depth_cells = (depth + 2.0 * options.poisson.padding) / cell + 1.0;
            if (side * side * depth_cells <= target_chunk_grid_cells) {
                break;
            }
            chunk_size = std::max(smallest_chunk, round_to_cells(0.85 * chunk_size, cell));
        }

        // The memory budget alone can give a few huge chunks, which leaves most
        // threads idle. Shrink the chunk until there are enough chunks to keep
        // every thread busy, down to a floor set by the halo.
        const double minimum_chunk = std::max(minimum_auto_chunk_cells * cell, round_to_cells(minimum_chunk_halos * halo, cell));
        const std::size_t wanted_chunks = static_cast<std::size_t>(std::ceil(target_chunks_per_thread * static_cast<double>(threads)));
        const auto chunk_count = [&](double size) {
            return points != nullptr ? chunks_occupied(*points, size) : chunks_covering(metadata.bounds, size);
        };
        while (chunk_size > minimum_chunk && chunk_count(chunk_size) < wanted_chunks) {
            chunk_size = std::max(minimum_chunk, round_to_cells(0.85 * chunk_size, cell));
        }
    }

    options.use_chunking = true;
    options.chunking.chunk_size = chunk_size;
    options.chunking.halo = halo;
    // Over 1 m on each side of an edge, or a quarter of a chunk's width if the chunks are smaller than 4 m.
    options.chunking.seam_solve = options.seam_solve;
    options.chunking.seam_blend = options.seam_blend_requested ? options.seam_blend : std::min(1.0, 0.25 * chunk_size);
    options.chunking.only_chunks = options.chunk_selection;
    options.chunking.max_chunks = options.max_chunks;

    options.chunking.threads = threads;
    return true;
}

bool resolve_geoorigin(const Options &options, const DatalistMetadata &metadata, GeodeticPosition *origin) {
    if (options.geoorigin_mode == GeoOriginMode::Specified) {
        *origin = options.geoorigin;
        return true;
    }
    if (options.geoorigin_mode == GeoOriginMode::Auto) {
        // Same convention as mbgrd2gltf: center of the bounds, mean elevation.
        origin->longitude = (metadata.geodetic_bounds.min_lon + metadata.geodetic_bounds.max_lon) / 2.0;
        origin->latitude = (metadata.geodetic_bounds.min_lat + metadata.geodetic_bounds.max_lat) / 2.0;
        origin->elevation = metadata.mean_elevation;
        return true;
    }
    return false;
}

bool adjust_poisson_grid_to_budget(Options &options, const DatalistMetadata &metadata, std::string *error) {
    PoissonGridEstimate estimate =
        estimate_poisson_grid(metadata.bounds, options.poisson.cell_size, options.poisson.padding);
    if (estimate.cell_count <= maximum_poisson_grid_cells) {
        return true;
    }

    if (error != nullptr) {
        std::ostringstream message;
        message << "Poisson grid estimate " << estimate.cell_count << " cells exceeds the current budget "
                << maximum_poisson_grid_cells << " at cell size " << options.poisson.cell_size
                << " m; reduce the requested LOD or run the reconstruction with chunked/adaptive processing";
        *error = message.str();
    }
    return false;
}

bool preprocess_datalist(Options &options, PreprocessedDatalist *preprocessed, std::string *error) {
    if (preprocessed == nullptr) {
        if (error != nullptr) {
            *error = "Preprocessed datalist result pointer is null";
        }
        return false;
    }

    DatalistReaderOptions reader_options;
    reader_options.use_bounds = options.use_bounds;
    reader_options.bounds = options.bounds;
    reader_options.verbose = options.verbose ? 1 : 0;
    reader_options.spill_points = options.spill_points;
    reader_options.spill_path = work_directory(options) / "soundings.bin";

    if (!read_datalist(options.input_datalist, reader_options, &preprocessed->read_result, error)) {
        return false;
    }

    if (preprocessed->read_result.stats.soundings_accepted == 0) {
        if (error != nullptr) {
            *error = "input contains no accepted soundings";
        }
        return false;
    }

    // Soundings that are too many for memory are on disk: their statistics are taken in one pass over the file, and
    // the chunk size from a sample.
    const bool spilled = preprocessed->read_result.spilled;
    const CollectedPointCloud *chunk_sample = &preprocessed->read_result.points;
    if (spilled) {
        const mbmesh::Stopwatch timer;
        LOG_INFO("[input]", "more than " + std::to_string(options.spill_points) + " soundings: " +
                                std::to_string(preprocessed->read_result.stats.soundings_accepted) + " are on disk in " +
                                preprocessed->read_result.spill_path.string() + " (" +
                                mbmesh::format_bytes(static_cast<double>(preprocessed->read_result.stats.soundings_accepted) *
                                                     static_cast<double>(spill_record_bytes)) + ")");
        if (!spilled_statistics(preprocessed->read_result, &preprocessed->metadata, &preprocessed->sample, error)) {
            return false;
        }
        chunk_sample = &preprocessed->sample;
        LOG_INFO("[input]", "measured the spilled soundings in " + timer.text() + ", " + mbmesh::peak_memory_text());
    }
    else {
        preprocessed->metadata.point_count = preprocessed->read_result.points.size();
        preprocessed->metadata.bounds = collected_point_bounds(preprocessed->read_result.points);
        preprocessed->metadata.geodetic_bounds = collected_geodetic_bounds(
            preprocessed->read_result.points, preprocessed->read_result.frame, &preprocessed->metadata.mean_elevation);
        preprocessed->metadata.spacing = estimate_point_spacing(preprocessed->read_result.points);
    }

    apply_spacing_driven_defaults(options, &preprocessed->metadata);
    if (!resolve_chunking(options, preprocessed->metadata, chunk_sample, error)) {
        return false;
    }

    // A survey that needs no chunking is small enough for memory whatever the threshold: read it back.
    if (spilled && !options.use_chunking && !options.metadata_requested) {
        DatalistReadResult &read_result = preprocessed->read_result;
        read_result.points.reserve(read_result.stats.soundings_accepted);
        const bool read = read_spill_pieces(read_result.spill_path, ping_origins(read_result), 4000000, [&](CollectedPointCloud &piece, std::size_t) {
            for (CollectedPoint &point : piece) {
                point.point = ecef_to_local(point.point, read_result.frame);
            }
            read_result.points.insert(read_result.points.end(), piece.begin(), piece.end());
            return true;
        }, error);
        if (!read) {
            return false;
        }
        std::error_code remove_error;
        std::filesystem::remove(read_result.spill_path, remove_error);
        read_result.spilled = false;
    }
    if (!options.use_chunking && !adjust_poisson_grid_to_budget(options, preprocessed->metadata, error)) {
        return false;
    }
    print_warnings(options, preprocessed->metadata);

    return true;
}

std::filesystem::path work_directory(const Options &options) {
    return options.output_directory / "mbmesh_work";
}

bool bin_spilled_points(const Options &options, PreprocessedDatalist *preprocessed, ChunkBins *bins, std::size_t *point_count, std::string *error) {
    DatalistReadResult &read_result = preprocessed->read_result;
    constexpr std::size_t piece_points = 4000000;

    const std::vector<Vec3> origins = ping_origins(read_result);
    const auto to_local = [&](CollectedPointCloud &piece) {
        for (CollectedPoint &point : piece) {
            point.point = ecef_to_local(point.point, read_result.frame);
        }
    };

    bool ok = true;
    if (options.decimation.decimate) {
        // The voxels are counted from the minimum corner of the bounds, as point_decimation() does, so the points are the same.
        PointDecimator decimator(preprocessed->metadata.bounds.min, options.decimation.cell_size);
        ok = read_spill_pieces(read_result.spill_path, origins, piece_points, [&](CollectedPointCloud &piece, std::size_t) {
            to_local(piece);
            for (const CollectedPoint &point : piece) {
                decimator.add(point);
            }
            return true;
        }, error);
        if (ok) {
            LOG_INFO("[decimation]", "averaged " + std::to_string(read_result.stats.soundings_accepted) + " soundings into " +
                                         std::to_string(decimator.cell_count()) + " points, " + mbmesh::peak_memory_text());
            decimator.for_each([&](const CollectedPoint &point) {
                if (ok) {
                    ok = bins->add(point, error);
                }
            });
        }
    }
    else {
        ok = read_spill_pieces(read_result.spill_path, origins, piece_points, [&](CollectedPointCloud &piece, std::size_t) {
            to_local(piece);
            for (const CollectedPoint &point : piece) {
                if (!bins->add(point, error)) {
                    return false;
                }
            }
            return true;
        }, error);
    }
    if (ok) {
        ok = bins->finish(error);
    }
    if (!ok) {
        return false;
    }

    std::error_code remove_error;
    std::filesystem::remove(read_result.spill_path, remove_error);
    read_result.spilled = false;
    *point_count = bins->owned_point_count();
    return true;
}

void print_datalist_metadata(const PreprocessedDatalist &preprocessed, const Options &options) {
    const DatalistMetadata &metadata = preprocessed.metadata;
    const DatalistReaderStats &stats = preprocessed.read_result.stats;
    const Bounds3D &bounds = metadata.bounds;
    const Vec3 extent = bounds.size();
    const PoissonGridEstimate grid =
        estimate_poisson_grid(bounds, options.poisson.cell_size, options.poisson.padding);

    std::cout << std::setprecision(10);
    std::cout << "mbmesh dataset information and reconstruction plan\n";
    std::cout << "input:\n";
    std::cout << "  files_read: " << stats.files_read << '\n';
    std::cout << "  soundings_read: " << stats.soundings_read << '\n';
    std::cout << "  accepted_soundings: " << metadata.point_count << '\n';

    std::cout << "geodetic_bounds_degrees:\n";
    std::cout << "  min_lon: " << metadata.geodetic_bounds.min_lon << '\n';
    std::cout << "  max_lon: " << metadata.geodetic_bounds.max_lon << '\n';
    std::cout << "  min_lat: " << metadata.geodetic_bounds.min_lat << '\n';
    std::cout << "  max_lat: " << metadata.geodetic_bounds.max_lat << '\n';
    std::cout << "  extent_lon: " << (metadata.geodetic_bounds.max_lon - metadata.geodetic_bounds.min_lon) << '\n';
    std::cout << "  extent_lat: " << (metadata.geodetic_bounds.max_lat - metadata.geodetic_bounds.min_lat) << '\n';
    std::cout << "  point_count: " << metadata.point_count << '\n';

    std::cout << "estimated_spacing_meters:\n";
    std::cout << "  sample_count: " << metadata.spacing.sample_count << '\n';
    std::cout << "  minimum: " << metadata.spacing.minimum << '\n';
    std::cout << "  maximum: " << metadata.spacing.maximum << '\n';
    std::cout << "  average: " << metadata.spacing.average << '\n';
    std::cout << "  median: " << metadata.spacing.median << '\n';

    std::cout << "reconstruction:\n";
    std::cout << "  poisson_cell_size: " << options.poisson.cell_size << '\n';
    std::cout << "  whole_survey_grid_cells: " << grid.cell_count << '\n';
    std::cout << "  chunked: " << (options.use_chunking ? "yes" : "no") << '\n';
    if (options.use_chunking) {
        const double chunk = options.chunking.chunk_size;
        const double halo = options.chunking.halo;
        const double side = (chunk + 2.0 * (halo + options.poisson.padding)) / options.poisson.cell_size + 1.0;
        std::cout << "  chunk_size: " << chunk << '\n';
        std::cout << "  halo: " << halo << '\n';
        std::cout << "  chunks_with_points: " << chunks_occupied(preprocessed.read_result.spilled ? preprocessed.sample : preprocessed.read_result.points, chunk)
                  << " (of " << chunks_covering(metadata.bounds, chunk) << " in the survey's bounding box)\n";
        std::cout << "  threads: " << options.chunking.threads << '\n';
        std::cout << "  chunk_grid_columns: " << side * side << '\n';
    }
}
