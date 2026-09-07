#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

#include "algorithms/marching_cubes.h"
#include "algorithms/normal_estimation.h"
#include "algorithms/point_decimation.h"
#include "algorithms/screened_poisson.h"
#include "algorithms/support_trimming.h"
#include "data_types/geometry.h"
#include "data_types/swathfile.h"
#include "io/datalist_reader.h"

struct Options {
    bool help_requested = false;
    bool metadata_requested = false;
    bool verbose = false;

    bool write_html = false;
    bool write_local_xyz = false;
    bool write_ecef_xyz = false;
    bool write_oriented_ply = false;
    bool write_pointcloud_glb = false;
    bool write_normal_glb = false;
    bool write_origin_glb = false;
    bool write_raw_mesh_glb = false;

    std::filesystem::path input_datalist;
    std::filesystem::path output_directory = "mbmesh_output";

    bool use_bounds = false;
    GeographicBounds bounds;

    // Smallest feature size, in local meters, that mbmesh should try to preserve.
    double level_of_detail = 0.5;
    bool level_of_detail_requested = false;
    bool decimation_requested = false;

    PointDecimationOptions decimation;
    NormalEstimationOptions normals;
    ScreenedPoissonOptions poisson;
    MarchingCubesOptions marching_cubes;
    SupportTrimmingOptions trimming;
};

struct SpacingMetadata {
    double minimum = 0.0;
    double maximum = 0.0;
    double average = 0.0;
    double median = 0.0;
    std::size_t sample_count = 0;
};

struct GeodeticBounds {
    double min_lon = 0.0;
    double max_lon = 0.0;
    double min_lat = 0.0;
    double max_lat = 0.0;
};

struct DatalistMetadata {
    Bounds3D bounds;
    GeodeticBounds geodetic_bounds;
    SpacingMetadata spacing;
    std::size_t point_count = 0;
    bool lod_supported_by_spacing = true;
    bool normal_radius_exceeds_feature_limit = false;
};

struct PreprocessedDatalist {
    DatalistReadResult read_result;
    DatalistMetadata metadata;
};

void apply_spacing_driven_defaults(Options &options, DatalistMetadata *metadata);
bool adjust_poisson_grid_to_budget(Options &options, const DatalistMetadata &metadata, std::string *error = nullptr);

bool preprocess_datalist(
    Options &options,
    PreprocessedDatalist *preprocessed,
    std::string *error);

void print_datalist_metadata(
    const PreprocessedDatalist &preprocessed,
    const Options &options);
