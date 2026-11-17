#include <algorithm>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <cmath>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "settings.h"

#include "util/logger.h"
#include "util/stopwatch.h"

#include "io/coordinate_frame.h"
#include "io/xyz_writer.h"
#include "draco_mesh.h"
#include "io/glb_writer.h"
#include "io/tileset_writer.h"
#include "io/point_spill.h"
#include "util/memory.h"
#include "io/track.h"
#include "io/cesium_convert.h"
#include "io/cesium_writer.h"
#include "io/x3dom_writer.h"

#include "algorithms/coverage_footprint.h"
#include "algorithms/seam_blending.h"
#include "algorithms/point_decimation.h"
#include "algorithms/normal_estimation.h"
#include "algorithms/reconstruction.h"
#include "algorithms/screened_poisson.h"
#include "algorithms/marching_cubes.h"
#include "algorithms/support_trimming.h"

// ========================================================================================================

Options parse_options(int argc, char **argv);

void print_usage();

namespace {

// Output-frame conversion: local ENU -> ECEF, minus the GeoOrigin's ECEF
// position when one is in use. Directions only rotate; they do not translate.

Vec3 output_position(const Vec3 &local, const CoordinateFrame &frame, const Vec3 &offset) {
    return local_to_ecef(local, frame) - offset;
}

Mesh output_mesh(const Mesh &mesh, const CoordinateFrame &frame, const Vec3 &offset) {
    Mesh out;
    out.indices = mesh.indices;
    out.vertices.reserve(mesh.vertices.size());
    for (const Vec3 &vertex : mesh.vertices) {
        out.vertices.push_back(output_position(vertex, frame, offset));
    }
    out.normals.reserve(mesh.normals.size());
    for (const Vec3 &normal : mesh.normals) {
        out.normals.push_back(local_vector_to_ecef(normal, frame));
    }
    return out;
}

PointCloud output_points(const PointCloud &points, const CoordinateFrame &frame, const Vec3 &offset) {
    PointCloud out;
    out.reserve(points.size());
    for (const Vec3 &point : points) {
        out.push_back(output_position(point, frame, offset));
    }
    return out;
}

OrientedPointCloud output_oriented_points(const OrientedPointCloud &points, const CoordinateFrame &frame, const Vec3 &offset) {
    OrientedPointCloud out;
    out.reserve(points.size());
    for (const OrientedPoint &point : points) {
        OrientedPoint converted(output_position(point.point, frame, offset), local_vector_to_ecef(point.normal, frame));
        converted.lambdas = point.lambdas;
        out.push_back(converted);
    }
    return out;
}

CollectedPointCloud output_collected_points(const CollectedPointCloud &points, const CoordinateFrame &frame, const Vec3 &offset) {
    CollectedPointCloud out;
    out.reserve(points.size());
    for (const CollectedPoint &point : points) {
        out.push_back({output_position(point.point, frame, offset), output_position(point.origin, frame, offset)});
    }
    return out;
}

void append_mesh(Mesh *destination, const Mesh &source) {
    const unsigned int offset = static_cast<unsigned int>(destination->vertices.size());
    const bool had_normals = destination->normals.size() == destination->vertices.size();
    const bool has_normals = source.normals.size() == source.vertices.size();
    destination->vertices.insert(destination->vertices.end(), source.vertices.begin(), source.vertices.end());
    if (had_normals && has_normals) {
        destination->normals.insert(destination->normals.end(), source.normals.begin(), source.normals.end());
    } else {
        destination->normals.clear();
    }
    destination->indices.reserve(destination->indices.size() + source.indices.size());
    for (const unsigned int index : source.indices) {
        destination->indices.push_back(index + offset);
    }
}

} // namespace

// Where output vertices go. GLB outputs are WGS84 ECEF; with a GeoOrigin they
// are offsets from the GeoOrigin's ECEF position (exactly as in mbgrd2gltf).
struct OutputPlacement {
    bool use_geoorigin = false;
    GeodeticPosition geoorigin;
    X3DomGeoOrigin view;
};

// The point-based outputs of a survey whose points are on disk hold at most this many points, every nth of them, because
// they are written from memory (the xyz files are written in full, from the chunk files).
constexpr std::size_t maximum_output_points = 5000000;

// Deletes the temporary files of a run that spilled its soundings to disk, however the run ends.
struct WorkDirectoryCleanup {
    std::filesystem::path directory;
    ~WorkDirectoryCleanup() {
        std::error_code ignored;
        std::filesystem::remove(directory / "soundings.bin", ignored);
        std::filesystem::remove_all(directory / "chunks", ignored);
        std::filesystem::remove(directory, ignored);   // only if nothing else is in it
    }
};

bool write_outputs(
    const Mesh &mesh,
    const Mesh &raw_mesh,
    const PointCloud &points,
    const OrientedPointCloud &oriented_points,
    const CollectedPointCloud &collected_points,
    const CoordinateFrame &frame,
    const OutputPlacement &placement,
    const std::vector<TrackPoint> &track,
    const Options &options,
    const ChunkBins *bins,
    std::string *error);

void print_html_viewer_command(
    const std::filesystem::path &directory,
    const std::vector<std::string> &page_names);

// ========================================================================================================

namespace {

// The page of a directory input: every GLB beneath it. Returns false after logging an error.
bool write_survey_page(const std::filesystem::path &survey_directory, const std::string &x3dom_url, const std::vector<std::string> &geoviewpoints,
                       bool label_chunks, bool cesium_html, const std::string &cesium_url) {
    const std::filesystem::path html_path = survey_page_path();
    X3DomWriterOptions html_options;
    html_options.title = viewer_title(survey_directory);
    html_options.nested_groups = true;
    if (!x3dom_url.empty()) {
        html_options.x3dom_url = x3dom_url;
    }
    std::string error;
    std::vector<std::string> mismatches;
    html_options.models = find_survey_models(survey_directory, html_path.parent_path(), &html_options.use_geoorigin, &html_options.geo, &mismatches);
    for (const std::string &mismatch : mismatches) {
        LOG_ERROR("[html] GeoOrigin mismatch: " + mismatch);
    }
    if (html_options.models.empty()) {
        LOG_ERROR("no .glb files beneath " + survey_directory.string());
        return false;
    }
    if (!html_options.models.front().uri.empty() && html_options.models.front().uri.rfind("../", 0) == 0) {
        LOG_WARN("the directory is not beneath the current directory, so a web server run here cannot serve its GLBs; run mbmesh in the survey directory or above it");
    }
    for (const std::string &text : geoviewpoints) {
        X3DomUserViewpoint viewpoint;
        std::string viewpoint_error;
        if (!parse_user_viewpoint(text, &viewpoint, &viewpoint_error)) {
            LOG_ERROR(viewpoint_error);
            return false;
        }
        html_options.user_viewpoints.push_back(viewpoint);
    }
    if (label_chunks) {
        std::size_t without_labels = 0;
        html_options.chunk_labels = find_survey_chunk_labels(survey_directory, html_options.use_geoorigin, html_options.geo, &without_labels);
        if (html_options.chunk_labels.empty()) {
            LOG_WARN("--label-chunks: no chunk file beneath " + survey_directory.string() + " says where its chunk is" +
                     (without_labels > 0 ? " (" + std::to_string(without_labels) + " chunk files made by an earlier version of mbmesh do not; run it again)" : std::string(" (there are no chunk files)")));
        } else if (without_labels > 0) {
            LOG_WARN("--label-chunks: " + std::to_string(without_labels) + " chunk files made by an earlier version of mbmesh do not say where their chunk is, and are not drawn");
        }
    }
    html_options.log_messages = mbmesh::Logger::captured_messages();
    html_options.log_messages_omitted = mbmesh::Logger::captured_omitted();
    if (!write_glb_x3dom_file(html_path, html_options, &error)) {
        LOG_ERROR(error);
        return false;
    }
    LOG_INFO("[html]", "wrote " + html_path.string() + ", offering " + std::to_string(html_options.models.size()) + " GLB files");
    std::vector<std::string> page_names = {html_path.filename().string()};
    if (cesium_html) {
        // The same models for CesiumJS, placed by the GeoOrigin of the first; it has no chunk outlines or labels.
        CesiumPageOptions cesium_options;
        cesium_options.title = viewer_title(survey_directory);
        cesium_options.cesium_url = cesium_url;
        cesium_options.models = cesium_models(html_options.models);
        if (html_options.use_geoorigin) {
            const Vec3 origin = geodetic_to_ecef(GeodeticPosition{html_options.geo.origin_lon, html_options.geo.origin_lat, html_options.geo.origin_elevation});
            cesium_options.model_origin[0] = origin.x;
            cesium_options.model_origin[1] = origin.y;
            cesium_options.model_origin[2] = origin.z;
        }
        if (!cesium_user_viewpoints(html_options.user_viewpoints, &cesium_options.user_viewpoints, &error)) {
            LOG_ERROR("invalid --geoviewpoint:", error);
            return false;
        }
        std::filesystem::path cesium_path = html_path;
        cesium_path.replace_filename(html_path.stem().string() + "_cesium.html");
        if (!write_cesium_page(cesium_path, cesium_options, &error)) {
            LOG_ERROR(error);
            return false;
        }
        LOG_INFO("[cesium]", "wrote " + cesium_path.string() + ", offering " + std::to_string(cesium_options.models.size()) + " models, with the globe hidden; the check box in the page shows it");
        page_names.push_back(cesium_path.filename().string());
    }
    print_html_viewer_command(html_path.parent_path(), page_names);
    return true;
}

} // namespace

int main(int argc, char **argv) {
    const std::time_t started_at = std::time(nullptr);

    using mbmesh::LogLevel;
    using mbmesh::Logger;
    using mbmesh::Stopwatch;

    const Stopwatch run_timer;

    // INFO until the options are parsed, so option errors are reported; -V then
    // raises it to DEBUG for progress messages, as in mbgrd2gltf.
    Logger::set_level(LogLevel::INFO);

    Options options = parse_options(argc, argv);
    if (options.verbose) {
        Logger::set_level(LogLevel::DEBUG);
    }

    if (options.help_requested) {
        print_usage();
        return 0;
    }

    // Keep the log for the processing log in the HTML page.
    if (options.write_html || options.write_cesium_html) {
        Logger::start_capture();
    }

    {
        std::string command_line;
        for (int i = 0; i < argc; i++) {
            const std::string argument = argv[i];
            if (i > 0) {
                command_line += ' ';
            }
            command_line += argument.find(' ') == std::string::npos ? argument : "'" + argument + "'";
        }
        LOG_INFO("[command]", command_line);
    }

    LOG_INFO("[startup]", "input=" + options.input_datalist.string() + " output=" + options.output_directory.string());

    if (options.input_datalist.empty()) {
        LOG_ERROR("input datalist path is empty; use -I <path>");
        return 1;
    }

    {
        std::error_code directory_error;
        if (std::filesystem::is_directory(options.input_datalist, directory_error)) {
            return write_survey_page(options.input_datalist, options.x3dom_url, options.geoviewpoints, options.label_chunks, options.write_cesium_html, options.cesium_url) ? 0 : 1;
        }
    }

    LOG_DEBUG("[input]", "reading datalist " + options.input_datalist.string());

    std::string error;
    PreprocessedDatalist preprocessed;
    const WorkDirectoryCleanup work_cleanup{work_directory(options)};
    const Stopwatch input_timer;
    if (!preprocess_datalist(options, &preprocessed, &error)) {
        LOG_ERROR(error);
        return 1;
    }

    LOG_INFO("[input]", "completed: accepted " + std::to_string(preprocessed.read_result.stats.soundings_accepted) +
                            " of " + std::to_string(preprocessed.read_result.stats.soundings_read) +
                            " soundings from " + std::to_string(preprocessed.read_result.stats.files_read) +
                            " files in " + input_timer.text() + ", " + mbmesh::peak_memory_text());

    if (options.metadata_requested) {
        print_datalist_metadata(preprocessed, options);
        return 0;
    }

    // ====================================================================================================
    // Output Placement (--geoorigin)
    // ====================================================================================================

    OutputPlacement placement;
    {
        const GeodeticBounds &bounds = preprocessed.metadata.geodetic_bounds;
        placement.use_geoorigin = resolve_geoorigin(options, preprocessed.metadata, &placement.geoorigin);

        // Viewpoint values follow mbgrd2gltf: center of the bounds, framed to fill a 45 degree view.
        X3DomGeoOrigin &view = placement.view;
        view.center_lat = (bounds.min_lat + bounds.max_lat) / 2.0;
        view.center_lon = (bounds.min_lon + bounds.max_lon) / 2.0;
        view.mean_elevation = preprocessed.metadata.mean_elevation;
        constexpr double pi = 3.14159265358979323846;
        const double lat_extent_m = (bounds.max_lat - bounds.min_lat) * 111000.0;
        const double lon_extent_m = (bounds.max_lon - bounds.min_lon) * 111000.0 * std::cos(view.center_lat * pi / 180.0);
        constexpr double field_of_view = 0.785398;
        view.view_altitude = (std::max(lat_extent_m, lon_extent_m) / 2.0) / std::tan(field_of_view / 2.0);

        // The viewpoint in the coordinates of the GLBs: ECEF, minus the GeoOrigin's ECEF position if there is one.
        {
            GeodeticPosition middle;
            middle.longitude = view.center_lon;
            middle.latitude = view.center_lat;
            middle.elevation = view.mean_elevation;
            const Vec3 middle_ecef = geodetic_to_ecef(middle);
            const Vec3 origin_ecef = placement.use_geoorigin ? geodetic_to_ecef(placement.geoorigin) : Vec3(0.0, 0.0, 0.0);
            const double middle_array[3] = {middle_ecef.x, middle_ecef.y, middle_ecef.z};
            const double origin_array[3] = {origin_ecef.x, origin_ecef.y, origin_ecef.z};
            set_local_viewpoint(&view, middle_array, origin_array);
        }

        if (placement.use_geoorigin) {
            std::ostringstream message;
            message << std::fixed << std::setprecision(6)
                    << (options.geoorigin_mode == GeoOriginMode::Auto ? "using automatic GeoOrigin (data center): "
                                                                       : "using user-specified GeoOrigin: ")
                    << placement.geoorigin.longitude << "," << placement.geoorigin.latitude << ","
                    << std::setprecision(2) << placement.geoorigin.elevation;
            LOG_INFO("[output]", message.str());

            view.origin_lat = placement.geoorigin.latitude;
            view.origin_lon = placement.geoorigin.longitude;
            view.origin_elevation = placement.geoorigin.elevation;
        } else {
            LOG_WARN("no GeoOrigin: GLB vertices are absolute ECEF stored as 32-bit floats (about 0.5 m resolution); "
                        "use --geoorigin to preserve sub-meter detail");
        }
    }

    // The settings the reconstruction will use, for the record.
    {
        std::ostringstream settings;
        settings << std::setprecision(6) << "level of detail " << options.level_of_detail << " m, Poisson cell " << options.poisson.cell_size
                 << " m, normal radius " << options.normals.search_radius << " m, point spacing " << preprocessed.metadata.spacing.median
                 << " m (median), " << preprocessed.metadata.point_count << " soundings; Poisson solver " << poisson_solver_name(options.poisson_solver)
                 << ", ";
        if (const int iterations = reconstruction_options(options).poisson.solver_iterations; iterations > 0) {
            settings << iterations << " iterations; ";
        } else {
            settings << "iterations chosen from the grid size; ";
        }
        if (options.use_chunking) {
            settings << "chunked reconstruction, chunks of " << options.chunking.chunk_size << " m, halo " << options.chunking.halo << " m, "
                     << options.chunking.threads << " threads";
        } else {
            settings << "single-pass reconstruction";
        }
        LOG_INFO("[settings]", settings.str());

        // The rest of the settings, in plain language, with the option that sets each one.
        auto join = [](const std::vector<std::string> &names) {
            std::string text;
            for (const std::string &name : names) {
                text += (text.empty() ? "" : ", ") + name;
            }
            return text.empty() ? std::string("none") : text;
        };
        {
            std::ostringstream text;
            text << std::setprecision(6) << "area: ";
            if (options.use_bounds) {
                text << "only soundings within W " << options.bounds.degrees_W << " E " << options.bounds.degrees_E << " S " << options.bounds.degrees_S
                     << " N " << options.bounds.degrees_N << " degrees (-R)";
            } else {
                text << "all of the soundings in the datalist (no -R)";
            }
            LOG_INFO("[settings]", text.str());
        }
        {
            std::ostringstream text;
            text << std::setprecision(6) << "soundings: ";
            if (options.decimation.decimate) {
                text << "averaged within cells of " << options.decimation.cell_size << " m before reconstruction (--decimate)";
            } else {
                text << "all used, none averaged (no --decimate)";
            }
            text << "; more than " << options.spill_points << " are kept on disk instead of in memory (--spill-points)";
            LOG_INFO("[settings]", text.str());
        }
        {
            std::ostringstream text;
            text << std::setprecision(6) << "normals: estimated from the soundings within " << options.normals.search_radius << " m, using up to "
                 << options.normals.k << " neighbors and at least " << options.normals.minimum_neighbors << "; Poisson: padding " << options.poisson.padding
                 << " m around the data, normals spread over " << options.poisson.normal_splat_radius << " m, screening weight "
                 << options.poisson.screening_weight;
            LOG_INFO("[settings]", text.str());
        }
        {
            std::ostringstream text;
            text << std::setprecision(6) << "trimming: " << (options.trimming.enabled ? "mesh is removed where the soundings do not support it" : "off")
                 << "; a vertex needs " << options.trimming.minimum_neighbors << " soundings within " << options.trimming.support_radius
                 << " m and an average distance from them, along the surface normal, of at most " << options.trimming.max_normal_offset << " m";
            if (options.trimming.minimum_normal_alignment > 0.0) {
                text << " and normals that agree to " << options.trimming.minimum_normal_alignment;
            }
            LOG_INFO("[settings]", text.str());
        }
        {
            std::ostringstream text;
            text << std::setprecision(6);
            if (options.void_size > 0.0) {
                text << "voids: gaps in the surveyed surface up to " << options.void_size
                     << " m across are filled in the mesh (--void-size) where soundings surround them; mesh more than " << 0.5 * options.void_size
                     << " m from any sounding stays trimmed, as do pieces smaller than " << 0.25 * 3.141592653589793 * 0.25 * options.void_size * options.void_size
                     << " square meters that are not joined to the rest";
            } else {
                text << "voids: not filled (--void-size 0); the mesh is trimmed wherever the soundings are sparse";
            }
            LOG_INFO("[settings]", text.str());
        }
        {
            std::ostringstream text;
            text << std::setprecision(6);
            if (options.island_size > 0.0) {
                text << "islands: when the survey is reconstructed in chunks, pieces of the mesh not joined to the rest, up to "
                     << 3.141592653589793 * 0.25 * options.island_size * options.island_size
                     << " square meters (--island-size " << options.island_size << " m across), are removed if no sounding is within "
                     << options.trimming.support_radius << " m of most of their vertices";
            } else {
                text << "islands: kept (--island-size 0)";
            }
            LOG_INFO("[settings]", text.str());
        }
        {
            std::ostringstream text;
            text << std::setprecision(6) << "chunks: ";
            if (!options.chunk_selection.empty()) {
                text << "only ";
                if (options.chunk_selection.size() == 1) {
                    text << "chunk " << options.chunk_selection.begin()->first << "," << options.chunk_selection.begin()->second;
                } else if (options.chunk_selection.size() <= 6) {
                    text << "chunks ";
                    std::size_t shown = 0;
                    for (const auto &selected : options.chunk_selection) {
                        const std::size_t before = shown++;
                        text << (before == 0 ? "" : (shown == options.chunk_selection.size() ? (before == 1 ? " and " : ", and ") : ", ")) << selected.first << "," << selected.second;
                    }
                } else {
                    text << options.chunk_selection.size() << " chunks";
                }
                text << " (--chunk) " << (options.chunk_selection.size() == 1 ? "is" : "are") << " reconstructed";
            } else {
                text << "all chunks are reconstructed";
            }
            if (options.max_chunks > 0) {
                text << ", at most " << options.max_chunks << " of them (--max-chunks)";
            }
            if (options.use_chunking) {
                if (options.chunking.seam_blend > 0.0) {
                    if (options.chunking.seam_solve) {
                        text << "; each chunk is solved with the fields of its west and south neighbors as boundary values on their shared edges (--seam-method solve)";
                    } else {
                        text << "; the fields of neighboring chunks are matched on their shared edges over " << options.chunking.seam_blend << " m (--seam-blend)";
                    }
                } else {
                    text << "; the fields of neighboring chunks are not matched on their shared edges (--seam-blend 0)";
                }
            }
            LOG_INFO("[settings]", text.str());
        }
        {
            std::ostringstream text;
            text << std::setprecision(6) << "placement: ";
            if (options.geoorigin_mode == GeoOriginMode::None) {
                text << "vertices are absolute Earth-centered coordinates (no --geoorigin)";
            } else {
                text << "vertices are offsets from the GeoOrigin lon " << placement.geoorigin.longitude << " lat " << placement.geoorigin.latitude
                     << " elevation " << placement.geoorigin.elevation << " m (--geoorigin" << (options.geoorigin_mode == GeoOriginMode::Auto ? ", chosen from the data" : "") << ")";
            }
            if (options.write_tileset) {
                text << "; tileset " << (!options.tileset_lift_requested ? std::string("not lifted (no --lift)")
                                         : options.tileset_lift_given  ? "lifted " + std::to_string(options.tileset_lift) + " m (--lift)"
                                                                       : std::string("lifted enough to put its lowest point 10 m above the ellipsoid (--lift)"));
            }
            LOG_INFO("[settings]", text.str());
        }
        {
            std::ostringstream text;
            text << std::setprecision(6);
            if (!options.draco.enabled) {
                text << "compression: none (--no-draco); the meshes are written as arrays of 32-bit floats and integers";
            } else if (!mbdraco::available()) {
                text << "compression: none; this build of mbmesh does not include Draco, so the meshes are written as arrays of 32-bit floats and integers";
            } else {
                text << "compression: Draco (KHR_draco_mesh_compression) for the triangle meshes (mesh.glb, the raw mesh, the chunks and the tileset tiles), with positions kept to "
                     << options.draco.precision << " m (--draco-precision) and normals in " << options.draco.normal_bits
                     << " bits (--draco-normal-bits); the point cloud, normals, sensor positions and track are not compressed, as the extension is for triangles";
            }
            LOG_INFO("[settings]", text.str());
        }
        {
            std::vector<std::string> outputs;
            if (options.write_html) outputs.push_back(options.label_chunks ? "viewer page with chunk outlines and names (--html, --label-chunks)" : "viewer page (--html)");
            if (options.write_cesium_html) outputs.push_back("CesiumJS viewer page (--cesium-html)");
            if (options.write_tileset) outputs.push_back("3D Tiles tileset (--tileset)");
            if (options.write_raw_mesh_glb) outputs.push_back("raw mesh (--raw-mesh-glb)");
            if (options.write_pointcloud_glb) outputs.push_back(std::string("point cloud colored by ") + point_color_map_name(options.pointcloud_colormap) + " (--pointcloud-glb)");
            if (options.write_normal_glb) outputs.push_back("normals (--normal-glb)");
            if (options.write_origin_glb) outputs.push_back("sensor positions (--origin-glb)");
            if (options.write_track_glb) outputs.push_back("track (--track-glb)");
            if (options.write_local_xyz) outputs.push_back("local XYZ (--local-xyz)");
            if (options.write_ecef_xyz) outputs.push_back("Earth-centered XYZ (--ecef-xyz)");
            if (options.write_oriented_ply) outputs.push_back("oriented PLY (--oriented-ply)");
            LOG_INFO("[settings]", "outputs, besides mesh.glb and the chunks: " + join(outputs) + "; into " + options.output_directory.string());
        }
    }

    // What is recorded in every GLB and in the HTML page about where the models are
    // and how they were made. One tree, so that the files cannot disagree.
    RunInfo run;
    {
        const GeodeticBounds &bounds = preprocessed.metadata.geodetic_bounds;
        const DatalistReadResult &read_result = preprocessed.read_result;

        run.title = viewer_title(viewer_web_root(options.output_directory));
        run.version = mbsystem_version();
        {
            std::string command_line;
            for (int i = 0; i < argc; i++) {
                const std::string argument = argv[i];
                command_line += (i > 0 ? " " : "") + (argument.find(' ') == std::string::npos ? argument : "'" + argument + "'");
            }
            run.command_line = command_line;
        }
        std::error_code cwd_error;
        run.working_directory = std::filesystem::current_path(cwd_error).string();
        run.started_at = iso_utc_time(static_cast<double>(started_at));
        if (const char *user = std::getenv("USER")) {
            run.user = user;
        } else if (const char *user_name = std::getenv("LOGNAME")) {
            run.user = user_name;
        }
        char host[256] = "";
        if (gethostname(host, sizeof(host) - 1) == 0) {
            run.host = host;
        }

        std::error_code absolute_error;
        const std::filesystem::path datalist = std::filesystem::absolute(options.input_datalist, absolute_error);
        run.datalist = absolute_error ? options.input_datalist.string() : datalist.lexically_normal().string();
        run.formats = read_result.formats;
        run.file_count = read_result.files.size();
        run.files.assign(read_result.files.begin(), read_result.files.begin() + std::min(read_result.files.size(), maximum_listed_files));
        run.pings = read_result.stats.pings_read;
        run.soundings_read = read_result.stats.soundings_read;
        run.soundings_accepted = read_result.stats.soundings_accepted;
        run.soundings_flagged = read_result.stats.soundings_flagged;
        run.soundings_out_of_bounds = read_result.stats.soundings_out_of_bounds;
        run.bounds_applied = options.use_bounds;
        run.bounds_west = options.bounds.degrees_W;
        run.bounds_east = options.bounds.degrees_E;
        run.bounds_south = options.bounds.degrees_S;
        run.bounds_north = options.bounds.degrees_N;
        if (!read_result.track.empty()) {
            const auto times = std::minmax_element(read_result.track.begin(), read_result.track.end(),
                                                   [](const TrackPoint &a, const TrackPoint &b) { return a.time < b.time; });
            run.first_time = iso_utc_time(times.first->time);
            run.last_time = iso_utc_time(times.second->time);
        }

        run.geoorigin_mode = !placement.use_geoorigin ? "none" : (options.geoorigin_mode == GeoOriginMode::Auto ? "automatic" : "specified");
        if (placement.use_geoorigin) {
            const Vec3 center = geodetic_to_ecef(placement.geoorigin);
            run.has_center = true;
            run.center_x = center.x;
            run.center_y = center.y;
            run.center_z = center.z;
            run.center_longitude = placement.geoorigin.longitude;
            run.center_latitude = placement.geoorigin.latitude;
            run.center_height = placement.geoorigin.elevation;
        }
        run.extent_west = bounds.min_lon;
        run.extent_south = bounds.min_lat;
        run.extent_east = bounds.max_lon;
        run.extent_north = bounds.max_lat;
        run.mean_elevation = preprocessed.metadata.mean_elevation;

        run.level_of_detail = options.level_of_detail;
        run.poisson_cell_size = options.poisson.cell_size;
        run.poisson_solver = poisson_solver_name(options.poisson_solver);
        run.poisson_iterations = reconstruction_options(options).poisson.solver_iterations;   // 0: chosen from each grid
        run.normal_search_radius = options.normals.search_radius;
        run.point_spacing = preprocessed.metadata.spacing.median;
        run.chunked = options.use_chunking;
        run.chunk_size = options.chunking.chunk_size;
        run.halo = options.chunking.halo;
        run.seam_blend = options.chunking.seam_blend;
        run.threads = options.use_chunking ? static_cast<std::size_t>(options.chunking.threads) : 1;
    }
    set_glb_run_metadata(run_metadata(run));
    {
        set_glb_compression(options.draco);
    }

    // The OGC 3D Tiles tileset: one tile for each chunk, the surface of the chunk in the local east-north-up frame, written Y-up as glTF requires,
    // and tileset.json, whose root transform is that frame in ECEF. It does not depend on the GeoOrigin.
    std::vector<TilesetTile> tileset_tiles;
    MetaValue tileset_metadata;
    std::vector<double> tileset_color_bounds;   // the heights the colors of the tiles change between, empty without colors
    const std::filesystem::path tileset_directory = options.output_directory / "tileset";
    if (options.write_tileset) {
        const CoordinateFrame &frame = preprocessed.read_result.frame;
        RunInfo tileset_run = run;
        tileset_run.local_frame = true;
        tileset_run.geoorigin_mode = "none";
        tileset_run.has_center = true;
        tileset_run.center_x = frame.origin_ecef.x;
        tileset_run.center_y = frame.origin_ecef.y;
        tileset_run.center_z = frame.origin_ecef.z;
        tileset_run.center_longitude = frame.origin.longitude;
        tileset_run.center_latitude = frame.origin.latitude;
        tileset_run.center_height = frame.origin.elevation;
        tileset_metadata = run_metadata(tileset_run);
        if (options.tileset_colors) {
            const Bounds3D &survey = preprocessed.metadata.bounds;
            tileset_color_bounds = haxby_height_bounds(frame.origin.elevation + survey.min.z, frame.origin.elevation + survey.max.z);
            set_metadata_height_colors(&tileset_metadata, 0.0, tileset_color_bounds);
        }

        std::error_code tileset_error;
        const std::size_t removed = remove_previous_tileset(tileset_directory);
        if (removed > 0) {
            LOG_INFO("[tileset]", "replacing the tileset already in " + tileset_directory.string() + " (" + std::to_string(removed) + " files and directories removed)");
        }
        std::filesystem::create_directories(tileset_directory / "L0", tileset_error);
        if (tileset_error) {
            LOG_ERROR("error creating tileset directory '" + tileset_directory.string() + "': " + tileset_error.message());
            return 1;
        }
    }
    const auto add_tileset_tile = [&](const Mesh &local_mesh, int i, int j) {
        TilesetTile tile;
        if (!mesh_bounds(local_mesh, tile.minimum, tile.maximum)) {
            return;
        }
        tile.uri = "L0/tile_" + std::to_string(i) + "_" + std::to_string(j) + ".glb";
        std::string tile_error;
        GlbContent content{"tileset_tile", "Surface of one tile of the 3D Tiles tileset, in the east-north-up frame at the center, Y-up", true, i, j};
        content.metadata = &tileset_metadata;
        Mesh colored = tileset_content_mesh(local_mesh);
        if (!tileset_color_bounds.empty()) {
            // Colored by the height of each vertex, which is the local up of the tile (its z, before the Y-up of the file) over the frame's height.
            Mesh by_height = local_mesh;
            color_mesh_by_height(&by_height, preprocessed.read_result.frame.origin.elevation, tileset_color_bounds);
            colored.colors = std::move(by_height.colors);
        }
        if (!write_mesh_glb_file(tileset_directory / tile.uri, colored, &tile_error, content)) {
            LOG_WARN(tile_error);
            return;
        }
        tileset_tiles.push_back(tile);
    };

    CollectedPointCloud collected_points = std::move(preprocessed.read_result.points);

    // ====================================================================================================
    // Points on disk: a survey with more soundings than fit in memory is sorted into one file per chunk, with the
    // decimation applied on the way, and only the chunks being reconstructed are in memory.
    // ====================================================================================================

    // Where the soundings are, and how high, for the support trimming to tell a void from the edge of the survey.
    std::unique_ptr<CoverageFootprint> footprint;
    if (options.void_size > 0.0 || options.island_size > 0.0) {
        const Bounds3D &b = preprocessed.metadata.bounds;
        footprint = std::make_unique<CoverageFootprint>(b.min.x, b.min.y, b.max.x, b.max.y,
                                                        (options.void_size > 0.0 ? options.void_size : options.island_size) / 32.0);
    }

    std::unique_ptr<ChunkBins> bins;
    std::size_t binned_points = 0;
    if (preprocessed.read_result.spilled) {
        const Stopwatch binning_timer;
        bins = std::make_unique<ChunkBins>(work_directory(options) / "chunks", options.chunking.chunk_size, options.chunking.halo);
        bins->set_footprint(footprint.get());
        bins->keep_only_chunks(options.chunking.only_chunks);
        if (!options.decimation.decimate && options.chunking.only_chunks.empty()) {
            // Every sounding goes into a chunk file, and into the halos of its neighbors too.
            const double chunk = options.chunking.chunk_size;
            const double halo = options.chunking.halo;
            const double need = static_cast<double>(preprocessed.read_result.stats.soundings_accepted) * spilled_point_bytes *
                                ((chunk + 2 * halo) * (chunk + 2 * halo)) / (chunk * chunk);
            std::error_code space_error;
            const auto space = std::filesystem::space(work_directory(options), space_error);
            if (!space_error && need > static_cast<double>(space.available)) {
                LOG_ERROR("the chunk files of " + std::to_string(preprocessed.read_result.stats.soundings_accepted) + " soundings need about " +
                          mbmesh::format_bytes(need) + " but only " + mbmesh::format_bytes(static_cast<double>(space.available)) +
                          " of disk is free. Average the soundings with --decimate (a cell size in meters near the level of detail, "
                          "which makes the files far smaller), or use -O to put the results on a disk with room");
                return 1;
            }
        }
        if (!bins->open(&error) || !bin_spilled_points(options, &preprocessed, bins.get(), &binned_points, &error)) {
            LOG_ERROR(error);
            return 1;
        }
        std::size_t halo_points = 0;
        for (const auto &entry : bins->chunks()) {
            halo_points += entry.second.total;
        }
        LOG_INFO("[binning]", "sorted " + std::to_string(binned_points) + " points into " + std::to_string(bins->chunks().size()) +
                                  " chunk files, " + std::to_string(halo_points) + " points with the halos (" +
                                  mbmesh::format_bytes(static_cast<double>(halo_points) * static_cast<double>(spilled_point_bytes)) + ") in " +
                                  binning_timer.text() + ", " + mbmesh::peak_memory_text());
    }

    // ====================================================================================================
    // Point Decimation
    // ====================================================================================================

    const std::size_t input_sample_count = collected_points.size();

    CollectedPointCloud decimated_points;

    if (bins != nullptr) {
        LOG_DEBUG("[decimation]", options.decimation.decimate ? "done while sorting the points into chunks" : "disabled");
    } else if (options.decimation.decimate) {

        LOG_DEBUG("[decimation]", "starting: cell_size=" + std::to_string(options.decimation.cell_size));

        decimated_points = point_decimation(std::move(collected_points), options.decimation);

        if (decimated_points.empty()) {
            LOG_ERROR("point decimation produced no samples");
            return 1;
        }
        LOG_DEBUG("[decimation]", "completed: retained " + std::to_string(decimated_points.size()) + " of " +
                        std::to_string(input_sample_count) + " samples");

    } else {
        LOG_DEBUG("[decimation]", "disabled; using " + std::to_string(input_sample_count) + " input samples");
        decimated_points = std::move(collected_points);
    }

    // ====================================================================================================
    // Reconstruction: normal estimation, screened Poisson, marching cubes, support trimming
    // ====================================================================================================

    ReconstructionOptions reconstruction = reconstruction_options(options);
    if (footprint != nullptr) {
        if (bins == nullptr) {
            for (const CollectedPoint &point : decimated_points) {
                footprint->add(point.point.x, point.point.y, point.point.z);
            }
        }
        if (options.void_size > 0.0) {
            reconstruction.trimming.footprint = footprint.get();
            reconstruction.trimming.max_void_distance = 0.5 * options.void_size;
            reconstruction.trimming.max_hole_area = 3.141592653589793 * 0.25 * options.void_size * options.void_size;
            reconstruction.trimming.min_island_area = 0.25 * reconstruction.trimming.max_hole_area;
        }
        LOG_DEBUG("[footprint]", "raster of the soundings: " + std::to_string(footprint->cell_count()) + " cells of " + std::to_string(footprint->cell_size()) +
                                     " m (" + mbmesh::format_bytes(static_cast<double>(footprint->cell_count()) * 8.0) + ")");
    }

    Mesh clean_mesh;
    Mesh raw_mesh;
    std::vector<X3DomChunkLabel> chunk_labels;   // for --label-chunks
    OrientedPointCloud oriented_points;
    const Stopwatch reconstruction_timer;

    if (!options.use_chunking) {
        ReconstructionResult result = reconstruct_chunk(decimated_points, reconstruction);
        if (result.status != ReconstructionStatus::Ok) {
            LOG_ERROR(reconstruction_status_message(result.status));
            return 1;
        }
        clean_mesh = std::move(result.mesh);
        raw_mesh = std::move(result.raw_mesh);
        oriented_points = std::move(result.oriented_points);
    }
    else {
        const bool keep_raw_mesh = options.write_raw_mesh_glb;
        const bool keep_oriented_points = options.write_normal_glb || options.write_oriented_ply;
        // Points on disk: the point outputs are written from memory, so they hold every nth point.
        const std::size_t output_stride = bins != nullptr ? std::max<std::size_t>(1, (binned_points + maximum_output_points - 1) / maximum_output_points) : 1;
        std::size_t oriented_seen = 0;

        const std::filesystem::path chunks_directory = options.output_directory / "chunks";
        std::error_code filesystem_error;
        std::filesystem::create_directories(chunks_directory, filesystem_error);
        if (filesystem_error) {
            LOG_ERROR("error creating chunk directory '" + chunks_directory.string() + "': " + filesystem_error.message());
            return 1;
        }

        const Vec3 chunk_offset = placement.use_geoorigin ? geodetic_to_ecef(placement.geoorigin) : Vec3(0.0, 0.0, 0.0);
        const CoordinateFrame &chunk_frame = preprocessed.read_result.frame;

        const ChunkCallback on_chunk = [&](ChunkResult &chunk) {
            const std::string chunk_name = "chunk_" + std::to_string(chunk.i) + "_" + std::to_string(chunk.j) + ".glb";

            // Where the chunk is: the square that it owns, at the mean height of its surface, and its name at the middle, raised to one
            // text height over the highest point of the chunk, along the local vertical. Kept in the chunk file, for the page of the survey
            // directory, and drawn in this run's page with --label-chunks.
            GlbContent chunk_content{"chunk", "Surface of one reconstruction chunk; the chunks together make mesh.glb", true, chunk.i, chunk.j};
            X3DomChunkLabel label;
            const bool has_label = !chunk.result.mesh.vertices.empty();
            if (has_label) {
                const double size = options.chunking.chunk_size;
                double mean_z = 0.0;
                double highest_z = std::numeric_limits<double>::lowest();
                for (const Vec3 &vertex : chunk.result.mesh.vertices) {
                    mean_z += vertex.z;
                    highest_z = std::max(highest_z, vertex.z);
                }
                mean_z /= static_cast<double>(chunk.result.mesh.vertices.size());
                label.name = chunk_name;
                label.text_size = 0.1 * size;
                const double x0 = chunk.i * size;
                const double y0 = chunk.j * size;
                const double corner_x[4] = {x0, x0 + size, x0 + size, x0};
                const double corner_y[4] = {y0, y0, y0 + size, y0 + size};
                for (int corner = 0; corner < 4; corner++) {
                    const Vec3 at = output_position(Vec3(corner_x[corner], corner_y[corner], mean_z), chunk_frame, chunk_offset);
                    label.corners[corner][0] = at.x;
                    label.corners[corner][1] = at.y;
                    label.corners[corner][2] = at.z;
                    const GeodeticPosition geodetic = ecef_to_geodetic(local_to_ecef(Vec3(corner_x[corner], corner_y[corner], mean_z), chunk_frame));
                    chunk_content.chunk_label.corners[corner][0] = geodetic.longitude;
                    chunk_content.chunk_label.corners[corner][1] = geodetic.latitude;
                    chunk_content.chunk_label.corners[corner][2] = geodetic.elevation;
                }
                const Vec3 middle = output_position(Vec3(x0 + 0.5 * size, y0 + 0.5 * size, mean_z), chunk_frame, chunk_offset);
                label.center[0] = middle.x;
                label.center[1] = middle.y;
                label.center[2] = middle.z;
                label.rise = (highest_z - mean_z) + label.text_size;
                const GeodeticPosition at_middle = ecef_to_geodetic(local_to_ecef(Vec3(x0 + 0.5 * size, y0 + 0.5 * size, mean_z), chunk_frame));
                label.latitude = at_middle.latitude;
                label.longitude = at_middle.longitude;
                chunk_content.chunk_label.present = true;
                chunk_content.chunk_label.center[0] = at_middle.longitude;
                chunk_content.chunk_label.center[1] = at_middle.latitude;
                chunk_content.chunk_label.center[2] = at_middle.elevation;
                chunk_content.chunk_label.rise = label.rise;
                chunk_content.chunk_label.text_size = label.text_size;
            }

            std::string chunk_error;
            if (!write_mesh_glb_file(chunks_directory / chunk_name, output_mesh(chunk.result.mesh, chunk_frame, chunk_offset), &chunk_error, chunk_content)) {
                LOG_WARN(chunk_error);
            }

            if (options.write_tileset) {
                add_tileset_tile(chunk.result.mesh, chunk.i, chunk.j);
            }

            if (options.label_chunks && has_label) {
                chunk_labels.push_back(label);
            }

            append_mesh(&clean_mesh, chunk.result.mesh);
            if (keep_raw_mesh) {
                append_mesh(&raw_mesh, chunk.result.raw_mesh);
            }
            if (keep_oriented_points) {
                if (output_stride == 1) {
                    oriented_points.insert(oriented_points.end(), chunk.result.oriented_points.begin(), chunk.result.oriented_points.end());
                } else {
                    for (const OrientedPoint &oriented : chunk.result.oriented_points) {
                        if (oriented_seen++ % output_stride == 0) {
                            oriented_points.push_back(oriented);
                        }
                    }
                }
            }
            return true;
        };

        ChunkingStats chunking_stats;
        const bool reconstructed = bins != nullptr ? reconstruct_chunked(*bins, reconstruction, options.chunking, on_chunk, &chunking_stats, &error)
                                                    : reconstruct_chunked(decimated_points, reconstruction, options.chunking, on_chunk, &chunking_stats, &error);
        if (!reconstructed) {
            LOG_ERROR(error);
            return 1;
        }

        LOG_INFO("[chunking]", "completed: " + std::to_string(chunking_stats.chunks_processed) + " chunks written, " +
                        std::to_string(chunking_stats.chunks_failed) + " failed, " +
                        std::to_string(chunking_stats.chunks_skipped_sparse) + " skipped as sparse, " +
                        std::to_string(chunking_stats.chunks_empty_after_crop) + " empty");

        if (clean_mesh.vertices.empty() || clean_mesh.indices.empty()) {
            LOG_ERROR("chunked reconstruction produced no mesh; adjust the level of detail or chunk size");
            return 1;
        }

        // The meshes of chunks whose fields were matched meet at the same points, which are separate vertices in each of the
        // chunk meshes: made one, so that mesh.glb has no seams, as an edge or as a gap.
        if (options.chunking.seam_blend > 0.0) {
            const Stopwatch weld_timer;
            constexpr double weld_tolerance = 1.0e-6;   // meters, far more than the rounding of the same point and less than any detail
            const std::size_t welded = weld_coincident_vertices(&clean_mesh, weld_tolerance);
            const std::size_t welded_raw = keep_raw_mesh ? weld_coincident_vertices(&raw_mesh, weld_tolerance) : 0;
            LOG_INFO("[chunking]", "seams: " + std::to_string(welded) + " vertices of the chunk meshes that coincide with another were merged in mesh.glb" +
                                       (keep_raw_mesh ? " (" + std::to_string(welded_raw) + " in the raw mesh)" : std::string()) + ", in " + weld_timer.text());
        }

        // Small pieces that are not joined to the rest and have no soundings near them.
        if (options.island_size > 0.0 && footprint != nullptr) {
            const Stopwatch island_timer;
            PieceRemoval removed;
            remove_unsupported_pieces(&clean_mesh, *footprint, options.trimming.support_radius,
                                      3.141592653589793 * 0.25 * options.island_size * options.island_size, &removed);
            LOG_INFO("[chunking]", "islands: " + std::to_string(removed.pieces) + " pieces of up to " +
                                       std::to_string(3.141592653589793 * 0.25 * options.island_size * options.island_size) +
                                       " square meters with no soundings near them removed from mesh.glb (" + std::to_string(removed.triangles) +
                                       " triangles), in " + island_timer.text());
            if (clean_mesh.vertices.empty() || clean_mesh.indices.empty()) {
                LOG_ERROR("removing the islands left no mesh; use --island-size 0");
                return 1;
            }
        }
    }

    // ====================================================================================================
    // Write Outputs
    // ====================================================================================================

    if (options.verbose) {
        std::ostringstream verbose_message;
        verbose_message << "starting: directory=" << options.output_directory
                        << ", glb=enabled"
                        << ", html=" << (options.write_html ? "enabled" : "disabled")
                        << ", local_xyz=" << (options.write_local_xyz ? "enabled" : "disabled")
                        << ", ecef_xyz=" << (options.write_ecef_xyz ? "enabled" : "disabled")
                        << ", oriented_ply=" << (options.write_oriented_ply ? "enabled" : "disabled")
                        << ", pointcloud_glb=" << (options.write_pointcloud_glb ? "enabled" : "disabled")
                        << ", normal_glb=" << (options.write_normal_glb ? "enabled" : "disabled")
                        << ", origin_glb=" << (options.write_origin_glb ? "enabled" : "disabled")
                        << ", track_glb=" << (options.write_track_glb ? "enabled" : "disabled")
                        << ", raw_mesh_glb=" << (options.write_raw_mesh_glb ? "enabled" : "disabled");
        LOG_DEBUG("[output]", verbose_message.str());
    }

    PointCloud output_points;
    if (bins != nullptr) {
        // Every nth point, from the chunk files, for the outputs that are written from memory.
        const std::size_t stride = std::max<std::size_t>(1, (binned_points + maximum_output_points - 1) / maximum_output_points);
        std::size_t seen = 0;
        const bool needed = options.write_pointcloud_glb || options.write_origin_glb;
        if (needed && !bins->for_each_owned([&](const CollectedPointCloud &owned) {
                for (const CollectedPoint &point : owned) {
                    if (seen++ % stride == 0) {
                        decimated_points.push_back(point);
                        output_points.push_back(point.point);
                    }
                }
                return true;
            }, &error)) {
            LOG_ERROR(error);
            return 1;
        }
        if (stride > 1 && (needed || options.write_normal_glb || options.write_oriented_ply)) {
            LOG_INFO("[output]", "the points are on disk: pointcloud.glb, origins.glb, normals.glb and the oriented PLY hold one point in " + std::to_string(stride) +
                                     " of the " + std::to_string(binned_points) + " (at most " + std::to_string(maximum_output_points) +
                                     "); the xyz files hold them all");
        }
    } else {
        output_points.reserve(decimated_points.size());
        for (const CollectedPoint &collected_point : decimated_points) {
            output_points.push_back(collected_point.point);
        }
    }

    if (options.write_tileset) {
        if (!options.use_chunking) {
            add_tileset_tile(clean_mesh, 0, 0);   // one tile
        }
        std::string tileset_error;
        // The lift: GeoTileset takes the height of its camera as at least 1 m, so tiles below the ellipsoid never refine.
        const CoordinateFrame &tileset_frame = preprocessed.read_result.frame;
        double lowest_height = std::numeric_limits<double>::max();
        for (const TilesetTile &tile : tileset_tiles) {
            lowest_height = std::min(lowest_height, tileset_frame.origin.elevation + tile.minimum[2]);
        }
        const double lift = !options.tileset_lift_requested ? 0.0
                            : options.tileset_lift_given    ? options.tileset_lift
                                                            : (tileset_tiles.empty() ? 0.0 : default_tileset_lift(lowest_height));
        MetaValue tileset_extras = tileset_metadata;
        set_metadata_lift(&tileset_extras, lift);
        if (!options.tileset_lift_requested && !tileset_tiles.empty() && lowest_height < 0.0) {
            LOG_WARN("[tileset]", "the lowest point of the tileset is at " + std::to_string(lowest_height) + " m, below the ellipsoid: X3DOM's GeoTileset takes the height of the camera as at least 1 m, so its tiles will not refine; use --lift to lift the tileset");
        }
        if (lift != 0.0) {
            LOG_INFO("[tileset]", "lifted " + std::to_string(lift) + " m above the ellipsoid" + (options.tileset_lift_given ? " (--lift=" + std::to_string(lift) + ")" : " (--lift: enough to put the lowest point, at " + std::to_string(lowest_height) + " m, 10 m above it)") +
                                      ", so every height in the tileset is that much more than the true one");
        }
        if (!write_tileset_json(tileset_directory / "tileset.json", lifted_frame(tileset_frame, lift), tileset_tiles, tileset_extras,
                                "MB-System mbmesh " + mbsystem_version(), &tileset_error)) {
            LOG_ERROR(tileset_error);
            return 1;
        }
        LOG_INFO("[tileset]", "wrote " + (tileset_directory / "tileset.json").string() + " with " + std::to_string(tileset_tiles.size()) + " tiles");
    }

    LOG_INFO("[reconstruction]", "completed in " + reconstruction_timer.text() + (options.use_chunking ? " on " + std::to_string(options.chunking.threads) + " threads" : std::string(", single pass")) +
                                     ", " + mbmesh::peak_memory_text());

    if (!write_outputs(clean_mesh, raw_mesh, output_points, oriented_points, decimated_points, preprocessed.read_result.frame, placement, preprocessed.read_result.track, options, bins.get(), &error)) {
        LOG_ERROR(error);
        return 1;
    }
    bins.reset();

    LOG_DEBUG("[output]", "completed");
    LOG_INFO("[complete]", "wrote " + (options.output_directory / "mesh.glb").string() + ", total " + run_timer.text() + ", " + mbmesh::peak_memory_text());

    // ====================================================================================================
    // X3DOM Viewer
    // ====================================================================================================

    std::vector<std::string> viewer_pages;   // the pages that were written, for the command that serves them
    const std::filesystem::path web_root = viewer_web_root(options.output_directory);

    if (options.write_html) {
        // Written last so that the processing log in the page holds the whole run.
        // The page is in the parent of the output directory, so that the parent is the
        // root of the web server and GLBs beside the output directory can be shown too.
        const std::filesystem::path html_path = viewer_page_path(options.output_directory);

        X3DomWriterOptions html_options;
        html_options.title = viewer_title(web_root);
        html_options.use_geoorigin = placement.use_geoorigin;
        html_options.geo = placement.view;
        for (const std::string &text : options.geoviewpoints) {
            X3DomUserViewpoint viewpoint;
            std::string viewpoint_error;
            if (!parse_user_viewpoint(text, &viewpoint, &viewpoint_error)) {
                LOG_ERROR(viewpoint_error);
                return 1;
            }
            html_options.user_viewpoints.push_back(viewpoint);
        }
        html_options.metadata = run_metadata(run);
        html_options.geo_summary = geo_metadata_summary(run);
        html_options.models = find_viewer_models(web_root, options.output_directory, options.write_tileset);
        if (options.label_chunks) {
            if (chunk_labels.empty()) {
                LOG_WARN("--label-chunks: no chunks were written (the run was not chunked, or no chunk had a surface), so there is nothing to label");
            }
            html_options.chunk_labels = chunk_labels;
        }
        if (!options.x3dom_url.empty()) {
            html_options.x3dom_url = options.x3dom_url;
        }
        // The log goes in its own file, which the pages load, so that they stay small and can be edited by hand.
        const std::filesystem::path log_path = viewer_log_path(options.output_directory);
        if (!write_log_file(log_path, Logger::captured_messages(), Logger::captured_omitted(), &error)) {
            LOG_ERROR(error);
            return 1;
        }
        html_options.log_url = viewer_log_url(options.output_directory);
        if (!write_glb_x3dom_file(html_path, html_options, &error)) {
            LOG_ERROR(error);
            return 1;
        }
        LOG_INFO("[html]", "wrote " + html_path.string() + ", showing " + std::to_string(html_options.models.size()) + " GLB files");

        // The same page inside the output directory, for a server that is rooted higher up, as one
        // rooted at a survey directory is: it shows the GLBs of this run and no others.
        {
            X3DomWriterOptions inner_options = html_options;
            inner_options.title = viewer_title(web_root);
            inner_options.log_url = log_path.filename().string();
            inner_options.models = find_viewer_models(web_root, options.output_directory, options.write_tileset, true);
            const std::filesystem::path inner_path = viewer_inner_page_path(options.output_directory);
            if (!write_glb_x3dom_file(inner_path, inner_options, &error)) {
                LOG_ERROR(error);
                return 1;
            }
            LOG_INFO("[html]", "wrote " + inner_path.string() + ", showing " + std::to_string(inner_options.models.size()) + " GLB files");
        }

        viewer_pages.push_back(html_path.filename().string());
    }

    if (options.write_cesium_html) {
        // The page of CesiumJS shows the same GLBs and tileset, in the parent of the output directory (mbmesh_output_cesium.html), and again inside the output
        // directory (mbmesh_cesium.html) for a server that is rooted there. It has no chunk outlines or labels.
        const std::filesystem::path log_path = viewer_log_path(options.output_directory);
        if (!options.write_html && !write_log_file(log_path, Logger::captured_messages(), Logger::captured_omitted(), &error)) {
            LOG_ERROR(error);
            return 1;
        }
        CesiumPageOptions cesium_options;
        cesium_options.title = viewer_title(web_root);
        cesium_options.cesium_url = options.cesium_url;
        cesium_options.log_uri = viewer_log_url(options.output_directory);
        cesium_options.log_label = "mbmesh log";
        if (placement.use_geoorigin) {
            const Vec3 origin = geodetic_to_ecef(placement.geoorigin);
            cesium_options.model_origin[0] = origin.x;
            cesium_options.model_origin[1] = origin.y;
            cesium_options.model_origin[2] = origin.z;
        }
        std::vector<X3DomUserViewpoint> viewpoints;
        for (const std::string &text : options.geoviewpoints) {
            X3DomUserViewpoint viewpoint;
            std::string viewpoint_error;
            if (!parse_user_viewpoint(text, &viewpoint, &viewpoint_error)) {
                LOG_ERROR(viewpoint_error);
                return 1;
            }
            viewpoints.push_back(viewpoint);
        }
        if (!cesium_user_viewpoints(viewpoints, &cesium_options.user_viewpoints, &error)) {
            LOG_ERROR("invalid --geoviewpoint:", error);
            return 1;
        }
        cesium_options.models = cesium_models(find_viewer_models(web_root, options.output_directory, options.write_tileset));
        const std::filesystem::path cesium_path = cesium_page_path(options.output_directory);
        if (!write_cesium_page(cesium_path, cesium_options, &error)) {
            LOG_ERROR(error);
            return 1;
        }
        LOG_INFO("[cesium]", "wrote " + cesium_path.string() + ", showing " + std::to_string(cesium_options.models.size()) + " models, with the globe hidden; the check box in the page shows it");
        {
            CesiumPageOptions inner_options = cesium_options;
            inner_options.log_uri = log_path.filename().string();
            inner_options.models = cesium_models(find_viewer_models(web_root, options.output_directory, options.write_tileset, true));
            std::filesystem::path inner_path = viewer_inner_page_path(options.output_directory);
            inner_path.replace_filename(inner_path.stem().string() + "_cesium.html");
            if (!write_cesium_page(inner_path, inner_options, &error)) {
                LOG_ERROR(error);
                return 1;
            }
            LOG_INFO("[cesium]", "wrote " + inner_path.string() + ", showing " + std::to_string(inner_options.models.size()) + " models");
        }
        viewer_pages.push_back(cesium_path.filename().string());
    }

    if (!viewer_pages.empty()) {
        print_html_viewer_command(web_root, viewer_pages);
    }

    return 0;
}

// ========================================================================================================

Options parse_options(int argc, char **argv) {

    Options options;

    // No arguments at all: show the usage, as mbgrd2gltf does, rather than an error about a missing input.
    if (argc < 2) {
        options.help_requested = true;
        return options;
    }

    struct SingleDashLongOption {
        const char *single_dash;
        const char *double_dash;
    };

    static const SingleDashLongOption single_dash_long_options[] = {
        {"-html", "--html"},
        {"-cesium-html", "--cesium-html"},
        {"-local-xyz", "--local-xyz"},
        {"-ecef-xyz", "--ecef-xyz"},
        {"-xyz", "--xyz"},
        {"-oriented-ply", "--oriented-ply"},
        {"-pointcloud-glb", "--pointcloud-glb"},
        {"-normal-glb", "--normal-glb"},
        {"-origin-glb", "--origin-glb"},
        {"-track-glb", "--track-glb"},
        {"-tileset", "--tileset"},
        {"-raw-mesh-glb", "--raw-mesh-glb"},
        {"-decimate", "--decimate"},
        {"-diagnostics", "--diagnostics"},
        {"-all-outputs", "--all-outputs"},
    };

    for (int i = 1; i < argc; i++) {
        for (const SingleDashLongOption &option_alias : single_dash_long_options) {
            if (std::strcmp(argv[i], option_alias.single_dash) == 0) {
                argv[i] = const_cast<char *>(option_alias.double_dash);
                break;
            }
        }
    }

    // --metadata was renamed --info. Say so, rather than ignoring an unknown
    // option and starting a full reconstruction, which can run for hours.
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--metadata") == 0 || std::strcmp(argv[i], "-metadata") == 0) {
            LOG_ERROR("the --metadata option has been renamed --info");
            std::exit(1);
        }
    }

    static const struct option long_options[] = {
        {"input", required_argument, nullptr, 'I'},
        {"output", required_argument, nullptr, 'O'},
        {"bounds", required_argument, nullptr, 'R'},
        {"lod", required_argument, nullptr, 'L'},
        {"level-of-detail", required_argument, nullptr, 'L'},
        {"info", no_argument, nullptr, 1000},
        {"html", no_argument, nullptr, 1001},
        {"local-xyz", no_argument, nullptr, 1002},
        {"ecef-xyz", no_argument, nullptr, 1003},
        {"xyz", no_argument, nullptr, 1004},
        {"oriented-ply", no_argument, nullptr, 1005},
        {"pointcloud-glb", no_argument, nullptr, 1006},
        {"normal-glb", no_argument, nullptr, 1007},
        {"origin-glb", no_argument, nullptr, 1008},
        {"raw-mesh-glb", no_argument, nullptr, 1009},
        {"decimate", required_argument, nullptr, 1010},
        {"geoorigin", optional_argument, nullptr, 'G'},
        {"chunk-size", required_argument, nullptr, 1013},
        {"chunk-halo", required_argument, nullptr, 1027},
        {"label-chunks", no_argument, nullptr, 1028},
        {"seam-blend", required_argument, nullptr, 1029},
        {"seam-method", required_argument, nullptr, 1031},
        {"no-draco", no_argument, nullptr, 1032},
        {"draco-precision", required_argument, nullptr, 1033},
        {"draco-normal-bits", required_argument, nullptr, 1034},
        {"chunk", required_argument, nullptr, 1014},
        {"max-chunks", required_argument, nullptr, 1015},
        {"threads", required_argument, nullptr, 1016},
        {"track-glb", no_argument, nullptr, 1017},
        {"tileset", no_argument, nullptr, 1019},
        {"x3dom-url", required_argument, nullptr, 1020},
        {"cesium-html", no_argument, nullptr, 1050},
        {"cesium-url", required_argument, nullptr, 1051},
        {"poisson-solver", required_argument, nullptr, 1021},
        {"poisson-iterations", required_argument, nullptr, 1022},
        {"geoviewpoint", required_argument, nullptr, 1018},
        {"lift", optional_argument, nullptr, 1023},
        {"no-colors", no_argument, nullptr, 1040},
        {"pointcloud-colormap", required_argument, nullptr, 1024},
        {"spill-points", required_argument, nullptr, 1025},
        {"void-size", required_argument, nullptr, 1026},
        {"island-size", required_argument, nullptr, 1030},
        {"primitive-size", required_argument, nullptr, 1035},
        {"diagnostics", no_argument, nullptr, 1011},
        {"all-outputs", no_argument, nullptr, 1012},
        {"verbose", no_argument, nullptr, 'V'},
        {"help", no_argument, nullptr, 'H'},
        {nullptr, 0, nullptr, 0},
    };

    int c = 0;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "I:O:R:L:G::VHh", long_options, &option_index)) != -1) {

        switch (c) {

        case 'I':
            options.input_datalist = optarg;
            break;

        case 'O':
            options.output_directory = optarg;
            break;

        case 'R': {
            double west = 0.0, east = 0.0, south = 0.0, north = 0.0;
            if (std::sscanf(optarg, "%lf/%lf/%lf/%lf", &west, &east, &south, &north) == 4) {
                options.bounds.degrees_W = west;
                options.bounds.degrees_E = east;
                options.bounds.degrees_S = south;
                options.bounds.degrees_N = north;
                options.use_bounds = true;
            } else {
                LOG_WARN("invalid bounds argument: " + std::string(optarg));
            }
            break;
        }

        case 'L':
            options.level_of_detail = std::strtod(optarg, nullptr);
            options.level_of_detail_requested = true;
            break;

        case 'G':
            // Same syntax and meaning as mbgrd2gltf: no value selects the
            // automatic GeoOrigin; otherwise all of lon,lat,elev are required.
            // As in mbgrd2gltf, the value must be attached (--geoorigin=V, -GV).
            if (optarg == nullptr) {
                options.geoorigin_mode = GeoOriginMode::Auto;
            } else {
                const std::string geoorigin_string(optarg);
                const std::size_t comma1 = geoorigin_string.find(',');
                const std::size_t comma2 = geoorigin_string.rfind(',');
                if (comma1 == std::string::npos || comma2 == std::string::npos || comma1 == comma2) {
                    LOG_ERROR("GeoOrigin format must be lon,lat,elev");
                    std::exit(1);
                }
                options.geoorigin.longitude = std::atof(geoorigin_string.substr(0, comma1).c_str());
                options.geoorigin.latitude = std::atof(geoorigin_string.substr(comma1 + 1, comma2 - comma1 - 1).c_str());
                options.geoorigin.elevation = std::atof(geoorigin_string.substr(comma2 + 1).c_str());
                options.geoorigin_mode = GeoOriginMode::Specified;
            }
            break;

        case 1018: {
            X3DomUserViewpoint parsed;
            std::string viewpoint_error;
            if (!parse_user_viewpoint(optarg, &parsed, &viewpoint_error)) {
                LOG_ERROR("invalid --geoviewpoint:", viewpoint_error);
                std::exit(1);
            }
            options.geoviewpoints.push_back(optarg);
            break;
        }

        case 'V':
            options.verbose = true;
            break;

        case 1000:
            options.metadata_requested = true;
            break;

        case 1001:
            options.write_html = true;
            break;

        case 1002:
            options.write_local_xyz = true;
            break;

        case 1003:
            options.write_ecef_xyz = true;
            break;

        case 1004:
            options.write_local_xyz = true;
            options.write_ecef_xyz = true;
            break;

        case 1005:
            options.write_oriented_ply = true;
            break;

        case 1006:
            options.write_pointcloud_glb = true;
            break;

        case 1007:
            options.write_normal_glb = true;
            break;

        case 1008:
            options.write_origin_glb = true;
            break;

        case 1009:
            options.write_raw_mesh_glb = true;
            break;

        case 1010:
            options.decimation.decimate = true;
            options.decimation.cell_size = std::strtod(optarg, nullptr);
            options.decimation_requested = true;
            break;

        case 1013:
            options.chunk_size = std::strtod(optarg, nullptr);
            options.chunk_size_requested = true;
            break;

        case 1028:
            options.label_chunks = true;
            break;

        case 1029: {
            char *end = nullptr;
            const double width = std::strtod(optarg, &end);
            if (end == optarg || *end != '\0' || !std::isfinite(width) || width < 0.0) {
                LOG_ERROR("invalid --seam-blend:", std::string("'") + optarg + "' is not a width in meters, zero or more");
                std::exit(1);
            }
            options.seam_blend = width;
            options.seam_blend_requested = true;
            break;
        }

        case 1032:
            options.draco.enabled = false;
            break;

        case 1033: {
            std::string reason;
            if (!mbdraco::parse_precision(optarg, &options.draco.precision, &reason)) {
                LOG_ERROR("invalid --draco-precision:", reason);
                std::exit(1);
            }
            break;
        }

        case 1034: {
            std::string reason;
            if (!mbdraco::parse_normal_bits(optarg, &options.draco.normal_bits, &reason)) {
                LOG_ERROR("invalid --draco-normal-bits:", reason);
                std::exit(1);
            }
            break;
        }

        case 1031: {
            const std::string method = optarg;
            if (method != "blend" && method != "solve") {
                LOG_ERROR("invalid --seam-method:", std::string("'") + optarg + "' is not blend or solve");
                std::exit(1);
            }
            options.seam_solve = method == "solve";
            break;
        }

        case 1027: {
            char *end = nullptr;
            const double halo = std::strtod(optarg, &end);
            if (end == optarg || *end != '\0' || !std::isfinite(halo) || halo <= 0.0) {
                LOG_ERROR("invalid --chunk-halo:", std::string("'") + optarg + "' is not a size in meters, more than zero");
                std::exit(1);
            }
            options.chunk_halo = halo;
            options.chunk_halo_requested = true;
            break;
        }

        case 1014: {
            std::string chunk_error;
            if (!parse_chunk_selection(optarg, &options.chunk_selection, &chunk_error)) {
                LOG_ERROR("invalid --chunk:", chunk_error);
                std::exit(1);
            }
            break;
        }

        case 1015:
            options.max_chunks = static_cast<std::size_t>(std::strtoull(optarg, nullptr, 10));
            break;

        case 1017:
            options.write_track_glb = true;
            break;

        case 1019:
            options.write_tileset = true;
            break;

        case 1020:
            options.x3dom_url = optarg;
            break;

        case 1050:
            options.write_cesium_html = true;
            break;

        case 1051:
            options.cesium_url = optarg;
            break;

        case 1024:
            if (!parse_point_color_map(optarg, &options.pointcloud_colormap)) {
                LOG_ERROR("invalid --pointcloud-colormap:", std::string("'") + optarg + "' is not one of " + point_color_map_names());
                std::exit(1);
            }
            break;

        case 1025: {
            char *end = nullptr;
            const unsigned long long count = std::strtoull(optarg, &end, 10);
            if (end == optarg || *end != '\0' || optarg[0] == '-') {
                LOG_ERROR("invalid --spill-points:", std::string("'") + optarg + "' is not a whole number of soundings, zero or more");
                std::exit(1);
            }
            options.spill_points = static_cast<std::size_t>(count);
            break;
        }

        case 1026: {
            char *end = nullptr;
            const double size = std::strtod(optarg, &end);
            if (end == optarg || *end != '\0' || !std::isfinite(size) || size < 0.0) {
                LOG_ERROR("invalid --void-size:", std::string("'") + optarg + "' is not a size in meters, zero or more");
                std::exit(1);
            }
            options.void_size = size;
            break;
        }

        case 1035: {
            char *end = nullptr;
            const double size = std::strtod(optarg, &end);
            if (end == optarg || *end != '\0' || !std::isfinite(size) || size < 0.0) {
                LOG_ERROR("invalid --primitive-size:", std::string("'") + optarg + "' is not a size in meters, zero or more");
                std::exit(1);
            }
            options.primitive_size = size;
            break;
        }

        case 1030: {
            char *end = nullptr;
            const double size = std::strtod(optarg, &end);
            if (end == optarg || *end != '\0' || !std::isfinite(size) || size < 0.0) {
                LOG_ERROR("invalid --island-size:", std::string("'") + optarg + "' is not a size in meters, zero or more");
                std::exit(1);
            }
            options.island_size = size;
            break;
        }

        case 1040:
            options.tileset_colors = false;
            break;

        case 1023: {
            options.tileset_lift_requested = true;
            if (optarg == nullptr) {
                break;   // the default lift
            }
            char *end = nullptr;
            options.tileset_lift = std::strtod(optarg, &end);
            if (end == optarg || *end != '\0' || !std::isfinite(options.tileset_lift) || options.tileset_lift < 0.0) {
                LOG_ERROR("invalid --lift:", std::string("'") + optarg + "' is not a number of meters, zero or more");
                std::exit(1);
            }
            options.tileset_lift_given = true;
            break;
        }

        case 1021:
            if (!parse_poisson_solver(optarg, &options.poisson_solver)) {
                LOG_ERROR("invalid --poisson-solver:", std::string("'") + optarg + "' is not one of " + poisson_solver_names());
                std::exit(1);
            }
            break;

        case 1022: {
            char *end = nullptr;
            const long iterations = std::strtol(optarg, &end, 10);
            if (end == optarg || *end != '\0' || iterations < 1 || iterations > 100000) {
                LOG_ERROR("invalid --poisson-iterations:", std::string("'") + optarg + "' is not a whole number from 1 to 100000");
                std::exit(1);
            }
            options.poisson_iterations = static_cast<int>(iterations);
            break;
        }

        case 1016:
            options.threads = static_cast<std::size_t>(std::strtoull(optarg, nullptr, 10));
            break;

        case 1011:
            options.write_pointcloud_glb = true;
            options.write_normal_glb = true;
            options.write_origin_glb = true;
            break;

        case 1012:
            options.write_html = true;
            options.write_local_xyz = true;
            options.write_ecef_xyz = true;
            options.write_oriented_ply = true;
            options.write_pointcloud_glb = true;
            options.write_normal_glb = true;
            options.write_origin_glb = true;
            options.write_track_glb = true;
            options.write_raw_mesh_glb = true;
            options.write_tileset = true;
            break;

        case 'H':
        case 'h':
            options.help_requested = true;
            break;

        default:
            // getopt has already named the option. An unknown option must not be
            // ignored: it could be a mistyped option that changes the result.
            LOG_ERROR("unrecognized or incomplete option; use -H for usage");
            std::exit(1);
        }
    }

    if (options.input_datalist.empty() && optind < argc) {
        options.input_datalist = argv[optind++];
    }

    // A directory input writes the survey page, which takes --geoviewpoint without --html.
    std::error_code input_error;
    const bool directory_input = !options.input_datalist.empty() && std::filesystem::is_directory(options.input_datalist, input_error);
    if (!options.geoviewpoints.empty() && !options.write_html && !options.write_cesium_html && !directory_input) {
        LOG_ERROR("--geoviewpoint adds a viewpoint to the page that --html or --cesium-html writes; use one of them too");
        std::exit(1);
    }
    if (options.label_chunks && !options.write_html && !directory_input) {
        LOG_ERROR("--label-chunks draws the chunks in the page that --html writes; use --html too");
        std::exit(1);
    }
    if (options.tileset_lift_requested && !options.write_tileset) {
        LOG_ERROR("--lift lifts the tileset that --tileset writes; use --tileset too");
        std::exit(1);
    }

    // Without -O the output goes beside the datalist, so that a run from a survey directory with
    // "-I multibeam/datalistp.mb-1" writes multibeam/mbmesh_output.
    if (options.output_directory.empty()) {
        options.output_directory = options.input_datalist.parent_path() / "mbmesh_output";
    }

    // Anything left over was not understood. Ignoring it can silently change the
    // result: "--geoorigin 122.4,36.6,0" is --geoorigin with no value (the
    // automatic GeoOrigin) followed by a stray argument.
    if (optind < argc) {
        LOG_ERROR("unexpected argument", argv[optind],
                  "- an option value must be attached to its option, for example --geoorigin=lon,lat,elev or -Glon,lat,elev; use -H for usage");
        std::exit(1);
    }

    return options;
}

void print_usage() {
    std::cout << "mbmesh generates 3D meshes from swath sonar bathymetry data.\n\n";
    std::cout << "mbmesh reads the soundings of an MB-System datalist, estimates normals from the neighbors of each\n";
    std::cout << "sounding, fits a surface to them with screened Poisson reconstruction, extracts it with marching\n";
    std::cout << "cubes and trims what the soundings do not support. The output is a glTF binary (GLB), mesh.glb, and,\n";
    std::cout << "optionally, an OGC 3D Tiles tileset and an X3DOM viewer page.\n\n";
    std::cout << "The vertices are in an Earth-Centered, Earth-Fixed (ECEF) Cartesian coordinate system, WGS 84, in\n";
    std::cout << "meters. ECEF is right-handed with its origin at the Earth's center of mass. A GeoOrigin (--geoorigin)\n";
    std::cout << "makes the vertices offsets from a point, which keeps the detail of a localized area in the 32-bit\n";
    std::cout << "floats of a GLB. Heights are above the ellipsoid, with no geoid correction.\n\n";
    std::cout << "usage: mbmesh -I datalist [-R west/east/south/north] [-O outputdir] [-L meters] [output options] [-V]\n\n";
    std::cout << "Required:\n";
    std::cout << "  -I, --input <datalist>       Input MB-System datalist file. A directory writes only a survey page\n";
    std::cout << "                               (mbmesh_survey.html) instead, see below\n\n";
    std::cout << "Optional:\n";
    std::cout << "  -O, --output <outputdir>     Output directory [mbmesh_output in the directory of the datalist]\n";
    std::cout << "  -R, --bounds <w/e/s/n>       Geographic bounds in degrees\n";
    std::cout << "  -L, --lod <meters>           Requested smallest feature size; auto if omitted\n";
    std::cout << "      --decimate <meters>      Enable voxel-grid point decimation\n";
    std::cout << "  -G, --geoorigin[=lon,lat,elev]\n";
    std::cout << "                               Write vertices as ECEF offsets from a GeoOrigin, as in mbgrd2gltf\n";
    std::cout << "                                 With values: use the specified lon,lat,elev\n";
    std::cout << "                                 Without values: use the data center and mean elevation\n";
    std::cout << "                                 Not specified: absolute ECEF coordinates (default)\n";
    std::cout << "                               GLB positions are 32-bit floats, so absolute ECEF (magnitudes of about\n";
    std::cout << "                               6.4 million meters) resolves only about 0.5 m; use a GeoOrigin to\n";
    std::cout << "                               preserve sub-meter detail\n";
    std::cout << "                               The value must be attached to the option, with an equals sign and no\n";
    std::cout << "                               spaces: --geoorigin=-122.4,36.6,0 or -G-122.4,36.6,0. A value in a\n";
    std::cout << "                               separate argument is not accepted\n";
    std::cout << "      --geoviewpoint=<values>  With --html or --cesium-html, add a GeoViewpoint to the page, which can be repeated. The values are\n";
    std::cout << "                               geodetic, in the order of --geoorigin, so they do not depend on the GeoOrigin, the\n";
    std::cout << "                               area or the data and can be used again: lon,lat,elev,clon,clat,celev, the camera\n";
    std::cout << "                               position and the point it looks at, or lon,lat,elev,ax,ay,az,angle,clon,clat,celev,\n";
    std::cout << "                               with the orientation as an axis and an angle, as a GeoViewpoint takes it. Heights are\n";
    std::cout << "                               above the ellipsoid. The Copy GeoViewpoint button of the page gives the ten-number form.\n";
    std::cout << "                               The value must be attached: --geoviewpoint=-122.4,36.6,-900,...\n";
    std::cout << "      --lift[=<meters>]        With --tileset, lift the tileset above the ellipsoid along the local\n";
    std::cout << "                               vertical, in the transform of tileset.json: X3DOM's GeoTileset does not refine tiles\n";
    std::cout << "                               for a camera below the ellipsoid, as over seafloor. The page lifts its other models\n";
    std::cout << "                               and the viewpoints the same. With no value, enough to put the lowest point of the mesh\n";
    std::cout << "                               10 m above the ellipsoid, to a meter, if it is lower; a value must be attached:\n";
    std::cout << "                               --lift=900. Without --lift the tileset has the true heights. Copy GeoViewpoint takes\n";
    std::cout << "                               the lift off again\n";
    std::cout << "      --no-colors              With --tileset, do not color the tiles by height. By default every vertex has a color\n";
    std::cout << "                               (COLOR_0) from the Haxby table over the heights of the whole survey, so all tiles agree;\n";
    std::cout << "                               the page can turn the colors off\n";
    std::cout << "      --chunk-size <meters>    Reconstruct in square chunks of about this size, writing chunks/chunk_I_J.glb\n";
    std::cout << "                               plus the merged mesh.glb; chunking turns on automatically for\n";
    std::cout << "                               surveys too large for one Poisson grid\n";
    std::cout << "      --chunk-halo <meters>    Neighboring points each chunk reads beyond its edge [what the normal estimation, splatting\n";
    std::cout << "                               and trimming need]. Less than that is allowed, with a warning; more changes the chunk size\n";
    std::cout << "      --chunk <i,j>            Reconstruct only chunk (i,j); chunk (i,j) covers x in [i*size,(i+1)*size)\n";
    std::cout << "                               and y in [j*size,(j+1)*size) in the local east-north frame. i and j can each be a range\n";
    std::cout << "                               first..last, as in 2..3,-11..-10, and the option can be repeated, to see a seam between chunks\n";
    std::cout << "      --max-chunks <n>         Stop after n chunks (for testing)\n";
    std::cout << "      --seam-blend <meters>    Over this width on each side of an edge between chunks, the field of a chunk is made to\n";
    std::cout << "                               match that of the chunk to its west or south on the edge, so that the meshes meet exactly,\n";
    std::cout << "                               with no step and no gap [1, or a quarter of a chunk if that is less]. 0 does not\n";
    std::cout << "      --label-chunks           With --html, draw the outline and the file name of each chunk in the page, with a check\n";
    std::cout << "                               box to switch them off, to see which chunk a seam or a void belongs to. Also for the survey\n";
    std::cout << "                               page of a directory input (-I directory), for the chunk files that say where they are\n";
    std::cout << "      --threads <n>            Chunks reconstructed at once when chunking [min(cores,4)]; each holds its own grid\n";
    std::cout << "      --spill-points <n>       A survey of more than n soundings [50000000] is not held in memory: the soundings go to a\n";
    std::cout << "                               file in mbmesh_work, beside the results, and are sorted into one file per chunk, so that\n";
    std::cout << "                               memory follows the size of a chunk, not of the survey; 0 always does. Needs chunking, which\n";
    std::cout << "                               it turns on for such a survey. The point outputs then hold at most 5 million points\n";
    std::cout << "      --void-size <m>          Put back voids of up to this many meters across [2] that support trimming would remove from\n";
    std::cout << "                               inside the surface: where the soundings thin out, in the depressions of the seafloor and\n";
    std::cout << "                               under overhangs, and the surveyed surface surrounds the gap. Gaps at the edge of the\n";
    std::cout << "                               survey stay trimmed. 0 trims them all\n";
    std::cout << "      --island-size <m>        When the survey is reconstructed in chunks, remove from mesh.glb the pieces that are not\n";
    std::cout << "                               joined to the rest, up to this many meters across [2] (the area of a disc), if no\n";
    std::cout << "                               sounding is near most of their vertices. Pieces with soundings under them stay. 0 keeps all\n";
    std::cout << "      --primitive-size <m>     Write mesh.glb and the raw mesh as one primitive for each square of this many meters of the\n";
    std::cout << "                               local frame, each with its own vertices and its own Draco stream, as mbgrd2gltf writes blocks\n";
    std::cout << "                               of 256 cells, so that a viewer decodes and draws them separately [256 Poisson cells]. 0 writes one\n";
    std::cout << "                               primitive. The vertices on the edges of the squares are in both\n";
    std::cout << "      --seam-method <name>     How chunks are matched on their edges: blend (solve each, then blend toward the neighbors over --seam-blend)\n";
    std::cout << "                               or solve (solve each with its west and south neighbors' fields fixed on the shared edges) [solve]\n";
    std::cout << "      --no-draco               Write the meshes as arrays. By default the triangle meshes of the GLB files (mesh.glb, the raw\n";
    std::cout << "                               mesh, the chunks and the tileset tiles) are compressed with Draco (KHR_draco_mesh_compression);\n";
    std::cout << "                               point clouds, normals, sensor positions and the track are never compressed\n";
    std::cout << "      --draco-precision <m>    With Draco, keep positions to this many meters [0.001]: each file gets the bits that make the\n";
    std::cout << "                               step of the quantization no more than this over its extent\n";
    std::cout << "      --draco-normal-bits <n>  With Draco, the bits of each normal, 2 to 30 [10]\n";
    std::cout << "      --poisson-solver <name>  How the Poisson grid equation is solved: " << poisson_solver_names() << " [sor]\n";
    std::cout << "      --poisson-iterations <n> Iterations of the solver, for either solver [sor: one per cell across the longest grid edge,\n";
    std::cout << "                               at least 50; jacobi: chosen from the level of detail, 200 to 500]. The\n";
    std::cout << "                               relative residual after them is logged with -V, and at INFO in a single pass\n";
    std::cout << "      --info                   Print dataset information and the reconstruction plan, then exit\n";
    std::cout << "  -V, --verbose                Enable progress and diagnostic logging\n";
    std::cout << "  -H, -h, --help               Print this help message\n\n";
    std::cout << "Output options:\n";
    std::cout << "      --html                   Also write an X3DOM viewer page, with a check box for each GLB and the\n";
    std::cout << "                               processing log, in the parent of the output directory (for example\n";
    std::cout << "                               mbmesh_output.html), and print the command to view it. The output directory\n";
    std::cout << "                               gets its own page too, mbmesh.html, without the GLBs beside the directory\n";
    std::cout << "      --cesium-html            Also write a viewer page for CesiumJS, with a check box for each GLB and tileset, beside the output\n";
    std::cout << "                               directory (mbmesh_output_cesium.html) and inside it (mbmesh_cesium.html). The globe of Cesium would hide data\n";
    std::cout << "                               below the ellipsoid, so the page starts with it hidden and has a check box that shows it, translucent. Tilesets\n";
    std::cout << "                               are shown at their true height whatever --lift says. It has no chunk outlines or labels\n";
    std::cout << "      --cesium-url <url>       The directory of the CesiumJS build of the --cesium-html page, with Cesium.js and Widgets/widgets.css in it\n";
    std::cout << "                               [https://cesium.com/downloads/cesiumjs/releases/1.114/Build/Cesium]\n";
    std::cout << "      --xyz                    Also write local and ECEF XYZ point clouds\n";
    std::cout << "      --local-xyz              Also write pointcloud-local.xyz\n";
    std::cout << "      --ecef-xyz               Also write pointcloud-ecef.xyz\n";
    std::cout << "      --oriented-ply           Also write oriented-pointcloud-ecef.ply\n";
    std::cout << "      --pointcloud-glb         Also write pointcloud.glb, colored by height\n";
    std::cout << "      --pointcloud-colormap <name>  How pointcloud.glb is colored by height, with the Haxby color table of mbm_grdplot:\n";
    std::cout << "                               " << point_color_map_names() << " [linear]. linear: 11 equal steps between rounded heights, as\n";
    std::cout << "                               mbm_grdplot -G5 does; equalized: steps with equal numbers of points, as mbm_grdplot -G5 -S;\n";
    std::cout << "                               none: one orange. The heights are listed in the file's description\n";
    std::cout << "      --normal-glb             Also write normals.glb\n";
    std::cout << "      --origin-glb             Also write origins.glb\n";
    std::cout << "      --track-glb              Also write track.glb, the sensor track as a black 3D line\n";
    std::cout << "      --tileset                Also write tileset/tileset.json and tileset/L0/tile_I_J.glb, the surface as an OGC 3D\n";
    std::cout << "                               Tiles tileset (a part of it is loaded as needed, so large surveys fit in a browser).\n";
    std::cout << "                               With --html the page shows it with a GeoTileset node, which needs a build of X3DOM\n";
    std::cout << "                               that has one: see --x3dom-url\n";
    std::cout << "      --x3dom-url <url>        The X3DOM script of the --html page [https://www.x3dom.org/x3dom/dist/x3dom-full.js]\n";
    std::cout << "      --raw-mesh-glb           Also write raw_mesh.glb before support trimming\n";
    std::cout << "      --diagnostics            Also write pointcloud, normal, and origin GLBs\n";
    std::cout << "      --all-outputs            Write all optional outputs\n\n";
    std::cout << "The default output is mesh.glb.\n\n";
    std::cout << "Survey page: when -I names a directory, for example a survey directory with multibeam and lidar\n";
    std::cout << "subdirectories, mbmesh reconstructs nothing. It writes mbmesh_survey.html in the current directory,\n";
    std::cout << "a viewer page with a check box for every .glb file beneath that directory, in accordions that mirror\n";
    std::cout << "its layout (the chunk files and the tiles of a tileset are left out; its tileset.json is offered; the track.glb files start switched on). Run it where a\n";
    std::cout << "web server will serve the directory, and the page loads the GLBs by relative path. The page\n";
    std::cout << "has an overview viewpoint over all the mbmesh GLBs found, with the GeoOrigin of the first, so GLBs from runs with another GeoOrigin\n";
    std::cout << "do not line up with it; each is logged as an ERROR. --geoviewpoint adds GeoViewpoints to its Viewpoint menu, as it does with --html;\n";
    std::cout << "--x3dom-url sets its X3DOM script. With --cesium-html it also writes mbmesh_survey_cesium.html, the same models for CesiumJS.\n";
    std::cout << "Single-dash long output options such as -html are accepted for legacy compatibility.\n";
}

namespace {

constexpr std::size_t maximum_diagnostic_glb_points = 2'000'000;

std::string shell_quote(const std::string &value) {
    std::string quoted = "'";
    for (const char character : value) {
        if (character == '\'') {
            quoted += "'\\''";
        } else {
            quoted += character;
        }
    }
    quoted += "'";
    return quoted;
}

} // namespace

// ========================================================================================================

bool write_outputs(
    const Mesh &mesh,
    const Mesh &raw_mesh,
    const PointCloud &points,
    const OrientedPointCloud &oriented_points,
    const CollectedPointCloud &collected_points,
    const CoordinateFrame &frame,
    const OutputPlacement &placement,
    const std::vector<TrackPoint> &track,
    const Options &options,
    const ChunkBins *bins,
    std::string *error)
{
    const Vec3 offset = placement.use_geoorigin ? geodetic_to_ecef(placement.geoorigin) : Vec3(0.0, 0.0, 0.0);

    if (options.output_directory.empty()) {
        if (error != nullptr) {
            *error = "Output directory path is empty";
        }
        return false;
    }

    std::error_code filesystem_error;
    std::filesystem::create_directories(options.output_directory, filesystem_error);

    if (filesystem_error) {
        if (error != nullptr) {
            *error = "Error creating output directory '" + options.output_directory.string() + "': " + filesystem_error.message();
        }
        return false;
    }

    // The xyz files hold every point. When the points are on disk they are written a chunk at a time.
    const auto write_xyz = [&](const std::filesystem::path &path, bool ecef) {
        if (bins == nullptr) {
            return ecef ? write_ecef_xyz_pointcloud(points, frame, path.string(), error) : write_local_xyz_pointcloud(points, path.string(), error);
        }
        bool first = true;
        bool ok = true;
        const bool read = bins->for_each_owned([&](const CollectedPointCloud &owned) {
            PointCloud chunk_points;
            chunk_points.reserve(owned.size());
            for (const CollectedPoint &point : owned) {
                chunk_points.push_back(point.point);
            }
            ok = ecef ? write_ecef_xyz_pointcloud(chunk_points, frame, path.string(), error, !first)
                      : write_local_xyz_pointcloud(chunk_points, path.string(), error, !first);
            first = false;
            return ok;
        }, error);
        return read && ok;
    };

    if (options.write_ecef_xyz) {
        const auto ecef_xyz_path = options.output_directory / "pointcloud-ecef.xyz";
        if (!write_xyz(ecef_xyz_path, true)) {
            return false;
        }
        LOG_DEBUG("[output]", "wrote " + ecef_xyz_path.string());
    }

    if (options.write_local_xyz) {
        const auto local_xyz_path = options.output_directory / "pointcloud-local.xyz";
        if (!write_xyz(local_xyz_path, false)) {
            return false;
        }
        LOG_DEBUG("[output]", "wrote " + local_xyz_path.string());
    }

    if (options.write_oriented_ply) {
        const auto oriented_ply_path = options.output_directory / "oriented-pointcloud-ecef.ply";
        if (!write_ply_oriented_pointcloud(oriented_points, frame, oriented_ply_path.string(), error)) {
            return false;
        }
        LOG_DEBUG("[output]", "wrote " + oriented_ply_path.string());
    }

    if (options.write_pointcloud_glb) {
        const auto pointcloud_glb_path = options.output_directory / "pointcloud.glb";
        // The points are colored by their height in the local frame, which is up and in meters.
        std::vector<double> heights;
        heights.reserve(points.size());
        for (const Vec3 &point : points) {
            heights.push_back(point.z);
        }
        if (!write_pointcloud_glb_file(pointcloud_glb_path, output_points(points, frame, offset), error,
                                         GlbContent{"pointcloud", "The soundings used for the reconstruction, as points"}, &heights,
                                       options.pointcloud_colormap)) {
            return false;
        }
        LOG_DEBUG("[output]", "wrote " + pointcloud_glb_path.string());
    }

    if (options.write_normal_glb) {
        const auto normals_glb_path = options.output_directory / "normals.glb";
        if (!write_normal_lines_glb_file(normals_glb_path, output_oriented_points(oriented_points, frame, offset), 0.25, error,
                                         GlbContent{"normals", "The estimated surface normal at each sounding, as short lines"})) {
            return false;
        }
        LOG_DEBUG("[output]", "wrote " + normals_glb_path.string());
    }

    if (options.write_origin_glb) {
        const auto origins_glb_path = options.output_directory / "origins.glb";
        if (!write_origin_ray_lines_glb_file(origins_glb_path, output_collected_points(collected_points, frame, offset), 0.25, error,
                                         GlbContent{"origin_rays", "A short line from each sounding toward the sensor that measured it"})) {
            return false;
        }
        LOG_DEBUG("[output]", "wrote " + origins_glb_path.string());
    }

    if (options.write_track_glb) {
        const auto track_glb_path = options.output_directory / "track.glb";
        // The track must be in exactly the coordinates of mesh.glb, so that the two
        // overlay: it goes through the same output_position(), with the same frame
        // and the same GeoOrigin offset (computed once, above), as every mesh.
        std::vector<std::vector<Vec3>> polylines = split_track(track);
        std::size_t track_points = 0;
        for (std::vector<Vec3> &polyline : polylines) {
            track_points += polyline.size();
            for (Vec3 &position : polyline) {
                position = output_position(position, frame, offset);
            }
        }
        if (!write_track_glb_file(track_glb_path, polylines, error,
                                  GlbContent{"track", "The sensor track, one line per continuous run of pings, in the same coordinates as mesh.glb"})) {
            return false;
        }
        LOG_DEBUG("[output]", "wrote " + track_glb_path.string() + ": " + std::to_string(track.size()) + " pings in " +
                                  std::to_string(polylines.size()) + " polylines, " +
                                  std::to_string(track_points) + " points drawn");
    }

    // One primitive for each block of 256 cells, as in mbgrd2gltf, unless --primitive-size says otherwise.
    const double primitive_block = options.primitive_size >= 0.0 ? options.primitive_size : 256.0 * options.poisson.cell_size;
    if (options.write_raw_mesh_glb) {
        const auto raw_mesh_glb_path = options.output_directory / "raw_mesh.glb";
        const std::vector<std::uint32_t> raw_groups = triangle_blocks(raw_mesh, primitive_block);
        if (!write_mesh_glb_file(raw_mesh_glb_path, output_mesh(raw_mesh, frame, offset), error,
                                  GlbContent{"raw_mesh", "The reconstructed surface before support trimming"}, &raw_groups)) {
            return false;
        }
        LOG_DEBUG("[output]", "wrote " + raw_mesh_glb_path.string());
    }

    const auto clean_mesh_glb_path = options.output_directory / "mesh.glb";
    const std::vector<std::uint32_t> mesh_groups = triangle_blocks(mesh, primitive_block);
    if (!write_mesh_glb_file(clean_mesh_glb_path, output_mesh(mesh, frame, offset), error,
                              GlbContent{"mesh", "The reconstructed seafloor surface"}, &mesh_groups)) {
        return false;
    }
    LOG_DEBUG("[output]", "wrote " + clean_mesh_glb_path.string());

    return true;
}

// ========================================================================================================

void print_html_viewer_command(
    const std::filesystem::path &directory,
    const std::vector<std::string> &page_names)
{
    // The viewer pages load their GLBs over HTTP, which browsers require for
    // local files. The directory is the web root, the parent of the output directory. Rather than starting a server and a browser, which would leave a
    // process running and hold the port, log the command to run, and the URL of each page.
    constexpr int port = 8000;

    std::error_code filesystem_error;
    std::filesystem::path absolute_directory = std::filesystem::absolute(directory, filesystem_error);
    if (filesystem_error) {
        absolute_directory = directory;
    }

    LOG_INFO("[html]", std::string("to view ") + (page_names.size() > 1 ? "the pages" : page_names.front()) +
                           ", run this command in a terminal (Ctrl-C stops it), then open the URL" + (page_names.size() > 1 ? "s" : "") + ":");
    LOG_INFO("[html]", "  python3 -m http.server " + std::to_string(port) + " --bind 127.0.0.1 --directory " +
                    shell_quote(absolute_directory.string()));
    for (const std::string &name : page_names) {
        LOG_INFO("[html]", "  http://127.0.0.1:" + std::to_string(port) + "/" + name);
    }
}
