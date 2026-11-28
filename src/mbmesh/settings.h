#pragma once

#include <cstddef>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include "draco_mesh.h"
#include "algorithms/marching_cubes.h"
#include "algorithms/normal_estimation.h"
#include "algorithms/point_decimation.h"
#include "algorithms/reconstruction.h"
#include "algorithms/screened_poisson.h"
#include "algorithms/support_trimming.h"
#include "io/glb_writer.h"
#include "data_types/geometry.h"
#include "data_types/swathfile.h"
#include "io/datalist_reader.h"
#include "io/point_spill.h"

enum class GeoOriginMode {
    None,       // absolute ECEF vertices (default)
    Auto,       // -G with no value: center of the data bounds and mean elevation
    Specified,  // -G lon,lat,elev
};

struct Options {
    bool help_requested = false;
    bool metadata_requested = false;
    bool verbose = false;

    bool write_html = false;

    // --cesium-html: also write a page for CesiumJS (see io/cesium_writer.h), and --cesium-url, the directory of the CesiumJS build it loads, if not the default.
    bool write_cesium_html = false;
    std::string cesium_url;

    // --label-chunks: the --html page draws the outline and the file name of each chunk.
    bool label_chunks = false;
    // Draco compression of the triangle meshes written to GLB files (--no-draco, --draco-precision, --draco-normal-bits).
    mbdraco::Settings draco;   // on, 0.001 m and 10 bits unless changed, as in mbgrd2gltf (src/mbdraco)

    // --tileset: write the chunks as an OGC 3D Tiles tileset in <output>/tileset (see io/tileset_writer.h).
    bool write_tileset = false;

    // --x3dom-url: the X3DOM script of the --html page, if not the default.
    std::string x3dom_url;

    // --geoviewpoint, repeatable: GeoViewpoints for the --html page, as given (see parse_user_viewpoint()).
    std::vector<std::string> geoviewpoints;

    // --lift, as --geoorigin works: not given, no lift; given without a value, the lift is decided from the lowest point of the mesh
    // (default_tileset_lift()); given with --lift=<meters>, that many meters, along the local vertical, in the transform of tileset.json.
    // The tiles of --tileset are colored by height (COLOR_0, the Haxby table, bounds from the whole survey) unless --no-colors.
    bool tileset_colors = true;
    bool tileset_lift_requested = false;
    bool tileset_lift_given = false;   // with a value
    double tileset_lift = 0.0;
    bool write_local_xyz = false;
    bool write_ecef_xyz = false;
    bool write_oriented_ply = false;
    bool write_pointcloud_glb = false;
    // --pointcloud-colormap: how pointcloud.glb is colored by height.
    PointColorMap pointcloud_colormap = PointColorMap::HaxbyLinear;
    bool write_normal_glb = false;
    bool write_origin_glb = false;
    bool write_track_glb = false;
    bool write_raw_mesh_glb = false;

    std::filesystem::path input_datalist;
    // The output directory: -O, or else mbmesh_output in the directory of the input datalist.
    std::filesystem::path output_directory;

    bool use_bounds = false;
    GeographicBounds bounds;

    // --geoorigin / -G, with the same meaning as in mbgrd2gltf. Output
    // vertices are WGS84 ECEF coordinates; with a GeoOrigin they are offsets
    // from that point's ECEF position instead.
    GeoOriginMode geoorigin_mode = GeoOriginMode::None;
    GeodeticPosition geoorigin;   // used when geoorigin_mode == Specified

    // Smallest feature size, in local meters, that mbmesh should try to preserve.
    double level_of_detail = 0.5;
    bool level_of_detail_requested = false;
    bool decimation_requested = false;

    // Chunked reconstruction. --chunk-size forces it on with the given chunk size
    // in meters; otherwise it turns on automatically when a single Poisson grid
    // would be too large. chunking and use_chunking are resolved from these in
    // preprocess_datalist().
    bool chunk_size_requested = false;
    double chunk_size = 0.0;
    // --chunk, repeatable, and with ranges: only these chunks are reconstructed (see parse_chunk_selection()). Empty is all.
    std::set<std::pair<int, int>> chunk_selection;
    std::size_t max_chunks = 0;
    // --chunk-halo: meters of neighboring points each chunk reads beyond its edge. Zero chooses it (see resolve_chunking()).
    bool chunk_halo_requested = false;
    double chunk_halo = 0.0;
    // --seam-blend: meters over which the field of a chunk is matched to its neighbors' on the edges it shares with them;
    // zero does not. Unless given, resolve_chunking() chooses it.
    bool seam_blend_requested = false;
    double seam_blend = 0.0;
    // --seam-method solve (the default): the neighbors' fields are boundary values of the solve, not blended into it afterward.
    bool seam_solve = true;
    std::size_t threads = 0;   // chunks reconstructed at once; 0 chooses automatically
    bool use_chunking = false;
    ChunkingOptions chunking;

    // The grid solver and its number of iterations, from --poisson-solver and
    // --poisson-iterations. A count of 0 keeps the one chosen from the level of detail.
    PoissonSolver poisson_solver = PoissonSolver::Sor;
    int poisson_iterations = 0;

    // --spill-points: more accepted soundings than this are kept in a file on disk, in the work directory, instead of
    // in memory, and sorted into one file per chunk (see io/point_spill.h). Zero spills them all.
    std::size_t spill_points = 50000000;

    // --void-size: voids in the surveyed surface that the support trimming would remove, up to this many meters across,
    // are put back (see SupportTrimmingOptions::footprint). Zero keeps them out.
    double void_size = 2.0;
    // --island-size: pieces of the mesh of a survey reconstructed in chunks that are not joined to the rest and have no
    // soundings near them, up to this many meters across (as a disc), are removed. 0 keeps them.
    double island_size = 2.0;
    // --primitive-size: mesh.glb and the raw mesh are written as one primitive for each square of this many meters, as mbgrd2gltf writes
    // blocks of 256 cells. Negative, the default: 256 Poisson cells. Zero: one primitive.
    double primitive_size = -1.0;

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
    double mean_elevation = 0.0;   // mean accepted sounding elevation, meters above the ellipsoid
    std::size_t point_count = 0;
    bool lod_supported_by_spacing = true;
    bool normal_radius_exceeds_feature_limit = false;

    // Set when the automatic level of detail was coarsened to fit the work
    // budget: the level of detail the data alone would have given, in meters.
    double automatic_lod_before_limit = 0.0;
};

struct PreprocessedDatalist {
    DatalistReadResult read_result;
    DatalistMetadata metadata;

    // About a million of the soundings, evenly spread, in the local frame, when read_result.points is empty because
    // they were spilled: enough to choose the chunk size from.
    CollectedPointCloud sample;
};

// Resolves the GeoOrigin that --geoorigin selects. Returns false (and leaves
// *origin untouched) when no GeoOrigin was requested.
bool resolve_geoorigin(const Options &options, const DatalistMetadata &metadata, GeodeticPosition *origin);

// Parses the value of --chunk, "I,J", where each of I and J is a whole number or a range "FIRST..LAST" (both included,
// FIRST not more than LAST), for example "2,-10" or "2..3,-11..-10", and adds the chunks it names to *chunks. Returns
// false and says why in error if the text is not that, or names more than 100000 chunks.
[[nodiscard]] bool parse_chunk_selection(const std::string &text, std::set<std::pair<int, int>> *chunks, std::string *error);

// Builds the options the reconstruction stages use.
ReconstructionOptions reconstruction_options(const Options &options);

// Decides whether to chunk and, if so, sets options.use_chunking and
// options.chunking. Returns false (with *error) if the settings cannot work.
// If points is given, the chunk count is taken from the chunks that contain
// points; otherwise from the survey's bounding box.
bool resolve_chunking(Options &options, const DatalistMetadata &metadata, const CollectedPointCloud *points = nullptr, std::string *error = nullptr);

// Spacing of the soundings, from nearest-neighbor distances inside blocks of
// consecutive points (see settings.cpp). Exposed for testing.
SpacingMetadata estimate_point_spacing(const CollectedPointCloud &points);

void apply_spacing_driven_defaults(Options &options, DatalistMetadata *metadata);
bool adjust_poisson_grid_to_budget(Options &options, const DatalistMetadata &metadata, std::string *error = nullptr);

bool preprocess_datalist(
    Options &options,
    PreprocessedDatalist *preprocessed,
    std::string *error);

// The directory for the temporary files of a run that spills its soundings: beside the results, so that it is on a disk
// with room for them.
[[nodiscard]] std::filesystem::path work_directory(const Options &options);

// With spilled soundings and chunked reconstruction: reads the spill file, applies the decimation if it is on, and
// sorts the points into one file per chunk in bins (which must be open, with the chunk size and halo of
// options.chunking). *point_count is the number of points, after decimation, that the chunks own. The spill file is
// deleted afterward.
[[nodiscard]] bool bin_spilled_points(
    const Options &options,
    PreprocessedDatalist *preprocessed,
    ChunkBins *bins,
    std::size_t *point_count,
    std::string *error);

void print_datalist_metadata(
    const PreprocessedDatalist &preprocessed,
    const Options &options);
