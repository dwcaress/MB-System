#include "settings.h"

#include "math/kdtree.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

namespace {

constexpr double pi = 3.14159265358979323846;
constexpr double minimum_feature_size = 0.01; // meters
constexpr double maximum_feature_size = 5.0;  // meters
constexpr double automatic_feature_size_spacing_scale = 2.0;
constexpr double maximum_poisson_grid_cells = 45.0e6;
constexpr std::size_t maximum_spacing_samples = 8192;

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

GeodeticBounds collected_geodetic_bounds(const CollectedPointCloud &points, const CoordinateFrame &frame) {
    GeodeticBounds bounds;
    bool initialized = false;
    for (const CollectedPoint &point : points) {
        const GeodeticPosition position = {
            frame.origin.longitude + point.point.x / frame.meters_per_degree_lon,
            frame.origin.latitude + point.point.y / frame.meters_per_degree_lat,
            frame.origin.elevation + point.point.z,
        };

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
    return bounds;
}

std::vector<Vec3> sampled_positions(const CollectedPointCloud &points, std::size_t maximum_samples) {
    std::vector<Vec3> positions;
    if (points.empty() || maximum_samples == 0) {
        return positions;
    }

    const std::size_t sample_count = std::min(points.size(), maximum_samples);
    positions.reserve(sample_count);
    for (std::size_t i = 0; i < sample_count; i++) {
        const std::size_t point_index =
            sample_count == 1 ? 0 : i * (points.size() - 1) / (sample_count - 1);
        positions.push_back(points[point_index].point);
    }
    return positions;
}

SpacingMetadata estimate_point_spacing(const CollectedPointCloud &points) {
    SpacingMetadata metadata;

    const std::vector<Vec3> positions = sampled_positions(points, maximum_spacing_samples);
    metadata.sample_count = positions.size();
    if (positions.size() < 2) {
        return metadata;
    }

    const KDTree tree(positions);
    std::vector<double> distances;
    distances.reserve(positions.size());

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
    if (!metadata.lod_supported_by_spacing) {
        std::cerr << "mbmesh: [warning] resolved level_of_detail "
                  << options.level_of_detail
                  << " m is not supported by the estimated average point spacing "
                  << metadata.spacing.average
                  << " m; the result may be inaccurate\n";
    }

    if (metadata.normal_radius_exceeds_feature_limit) {
        std::cerr << "mbmesh: [warning] normal estimation radius "
                  << options.normals.search_radius
                  << " m exceeds 45% of requested feature size "
                  << options.level_of_detail
                  << " m; normals may smooth features smaller than the radius\n";
    }
}

} // namespace

void apply_spacing_driven_defaults(Options &options, DatalistMetadata *metadata) {
    const double fallback_feature_size =
        options.level_of_detail > vec3_epsilon ? options.level_of_detail : minimum_feature_size;
    const double average_spacing =
        metadata->spacing.average > vec3_epsilon ? metadata->spacing.average : fallback_feature_size;
    const double desired_feature_size =
        std::clamp(
            options.level_of_detail_requested
                ? options.level_of_detail
                : automatic_feature_size_spacing_scale * average_spacing,
            minimum_feature_size,
            maximum_feature_size);

    options.level_of_detail = desired_feature_size;

    if (!options.decimation_requested) {
        options.decimation.decimate = false;
        options.decimation.cell_size = std::max(0.5 * desired_feature_size, 0.5 * average_spacing);
    }

    const double poisson_cell_size =
        std::clamp(desired_feature_size / 4.0, average_spacing / 2.0, average_spacing);
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

    if (!read_datalist(options.input_datalist, reader_options, &preprocessed->read_result, error)) {
        return false;
    }

    if (preprocessed->read_result.points.empty()) {
        if (error != nullptr) {
            *error = "input contains no accepted soundings";
        }
        return false;
    }

    preprocessed->metadata.point_count = preprocessed->read_result.points.size();
    preprocessed->metadata.bounds = collected_point_bounds(preprocessed->read_result.points);
    preprocessed->metadata.geodetic_bounds = collected_geodetic_bounds(preprocessed->read_result.points, preprocessed->read_result.frame);
    preprocessed->metadata.spacing = estimate_point_spacing(preprocessed->read_result.points);

    apply_spacing_driven_defaults(options, &preprocessed->metadata);
    if (!adjust_poisson_grid_to_budget(options, preprocessed->metadata, error)) {
        return false;
    }
    print_warnings(options, preprocessed->metadata);

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
    std::cout << "mbmesh dataset metadata\n";
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
}
