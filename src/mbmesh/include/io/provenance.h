#pragma once

/*
 * The georeferencing and provenance metadata of mbmesh's output, as one tree.
 *
 * The same tree is written into every GLB as asset.extras and into the viewer
 * page as X3D metadata (WorldInfo, GeoMetadata), so the two cannot disagree. The
 * names are described in GEOREFERENCE.md, which proposes them for other software
 * to adopt.
 */

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "draco_mesh.h"

// An ordered JSON-like value: a string, number, boolean, array or object.
struct MetaValue {
    enum class Type { String, Number, Bool, Array, Object };

    Type type = Type::Object;
    std::string text;
    double number = 0.0;
    bool flag = false;
    std::vector<MetaValue> items;                              // array elements
    std::vector<std::pair<std::string, MetaValue>> members;    // object members, in order

    [[nodiscard]] static MetaValue from_string(const std::string &value);
    [[nodiscard]] static MetaValue from_number(double value);
    [[nodiscard]] static MetaValue from_bool(bool value);
    [[nodiscard]] static MetaValue make_array();
    [[nodiscard]] static MetaValue make_object();

    MetaValue &set(const std::string &key, MetaValue value);   // object: add or replace a member
    MetaValue &add(MetaValue value);                           // array: append an element
    [[nodiscard]] const MetaValue *find(const std::string &key) const;
};

// The text of a number as it appears in the X3D metadata and in flatten():
// integers as integers, other values to 12 significant digits.
[[nodiscard]] std::string metadata_number_text(double value);

// Every leaf of the tree as a ("dotted.path", "text") pair, in order. An array
// element is written as path[index]. Members whose names start with @ (JSON-LD
// keywords) are not values and are left out. This is what the page prints as WorldInfo
// lines, and what the tests compare between the GLB and the page.
[[nodiscard]] std::vector<std::pair<std::string, std::string>> flatten_metadata(const MetaValue &value, const std::string &path = "");

// A time in seconds since 1970 as ISO 8601 UTC, "2026-10-22T16:09:29Z".
[[nodiscard]] std::string iso_utc_time(double unix_seconds);

// Everything the metadata says about one run.
struct RunInfo {
    // The survey, for titles.
    std::string title;

    // The software and the execution.
    std::string program = "mbmesh";
    std::string version;
    std::string command_line;
    std::string working_directory;
    std::string started_at;   // ISO 8601 UTC
    std::string user;
    std::string host;

    // The inputs.
    std::string datalist;
    std::vector<int> formats;
    std::size_t file_count = 0;
    std::vector<std::string> files;   // at most maximum_listed_files
    std::size_t pings = 0;
    std::size_t soundings_read = 0;
    std::size_t soundings_accepted = 0;
    std::size_t soundings_flagged = 0;
    std::size_t soundings_out_of_bounds = 0;
    bool bounds_applied = false;      // -R
    double bounds_west = 0.0, bounds_east = 0.0, bounds_south = 0.0, bounds_north = 0.0;
    std::string first_time;           // ISO 8601 UTC of the first and last ping with an accepted sounding
    std::string last_time;

    // Where the output is.
    std::string geoorigin_mode = "none";   // "none", "automatic" or "specified"
    bool has_center = false;
    // The vertices are not ECEF offsets from the center but are in the east-north-up frame at the center, written
    // Y-up as glTF requires (X east, Y up, Z south), as the content of a 3D Tiles tileset whose transform places
    // them. Needs has_center; the center is then the origin of that frame.
    bool local_frame = false;
    double center_x = 0.0, center_y = 0.0, center_z = 0.0;   // ECEF, meters
    double center_longitude = 0.0, center_latitude = 0.0, center_height = 0.0;
    double extent_west = 0.0, extent_south = 0.0, extent_east = 0.0, extent_north = 0.0;
    double mean_elevation = 0.0;

    // How it was made.
    double level_of_detail = 0.0;
    double poisson_cell_size = 0.0;
    std::string poisson_solver;
    int poisson_iterations = 0;
    double normal_search_radius = 0.0;
    double point_spacing = 0.0;   // median nearest-neighbor distance
    bool chunked = false;
    double chunk_size = 0.0;
    double halo = 0.0;
    double seam_blend = 0.0;   // meters over which the chunks are matched on their edges, zero for not
    std::size_t threads = 1;
};

// The most input files listed by name; the count of all of them is always given.
constexpr std::size_t maximum_listed_files = 1000;

// The three blocks of asset.extras: "georeference", "provenance" and "MB-System".
[[nodiscard]] MetaValue run_metadata(const RunInfo &run);

// The keyword and value strings of the X3D GeoMetadata summary field, which takes
// only the keywords the X3D specification lists. There is always an even number.
[[nodiscard]] std::vector<std::string> geo_metadata_summary(const RunInfo &run);

// What one GLB holds, added to the run metadata under "MB-System.content".
struct GlbContent {
    std::string role;          // "mesh", "raw_mesh", "chunk", "pointcloud", "normals", "origin_rays" or "track"
    std::string description;
    bool has_index = false;   // a chunk or a tile of a tileset, with its index
    int index_i = 0;
    int index_j = 0;
    // Where a chunk is, for the page of a survey directory to draw its outline and name (--label-chunks): the corners of the
    // square that the chunk owns at the mean height of its surface, in order around it, and the middle, each as longitude,
    // latitude and height above the ellipsoid; how far above the middle the name is, in meters along the local vertical;
    // and the height of its text. Written for each chunk, when present.
    struct ChunkLabelPlace {
        bool present = false;
        double corners[4][3] = {};
        double center[3] = {};
        double rise = 0.0;
        double text_size = 1.0;
    } chunk_label;
    int primitives = 0;       // how many primitives the triangle mesh of this file is in, when the writer says
    // How the geometry in this file is compressed, if it is (KHR_draco_mesh_compression): filled in by the writer.
    mbdraco::Compression compression;
    // Metadata to write in place of the run metadata that set_glb_run_metadata() set, for a file whose
    // georeference differs, such as a tileset's content. The content of this file is added to it.
    const MetaValue *metadata = nullptr;
};

[[nodiscard]] MetaValue content_metadata(const GlbContent &content);
