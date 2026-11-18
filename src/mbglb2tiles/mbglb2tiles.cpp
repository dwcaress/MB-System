/*
 * mbglb2tiles: OGC 3D Tiles of levels of detail from a GLB mesh of mbmesh or mbgrd2gltf.
 *
 * The mesh is read, welded (the primitives of a GLB each have their own copies of the vertices on their edges), put in the local east-north-up
 * frame at the middle of its bounds, and made coarser level by level by clustering vertices on a lattice that doubles in size at each level. Each
 * level is cut into the tiles of a lattice of squares that also doubles, so that four tiles of a level are replaced by one of the next. The tiles
 * are GLBs (Y-up, Draco-compressed unless asked not to) and tileset.json is the tree of them, with the transform that puts the frame on the Earth.
 */

#include "glb_reader.h"
#include "mesh_levels.h"

#include "algorithms/seam_blending.h"
#include "io/coordinate_frame.h"
#include "io/glb_writer.h"
#include "io/provenance.h"
#include "io/tileset_writer.h"
#include "io/cesium_convert.h"
#include "io/cesium_writer.h"
#include "io/x3dom_writer.h"
#include "util/logger.h"
#include "util/memory.h"
#include "util/parallel.h"
#include "util/stopwatch.h"

#include "draco_mesh.h"
#include "mb_define.h"

#include <getopt.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <thread>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace {

// The tile of the finest level is this many times the spacing of the mesh, as the block of mbgrd2gltf is 256 cells. The lattice of the clustering of a
// level is the spacing times two to the power of the level, and so is the tile, so a tile has about as many vertices at every level.
constexpr double cells_per_tile = 256.0;

struct Options {
    std::filesystem::path input;
    std::filesystem::path output;
    double tile_size = 0.0;          // 0: chosen from the spacing of the mesh
    double weld_tolerance = 0.0;     // 0: chosen from the quantization the file says
    int threads = 0;                 // 0: the cores, at most 8
    bool threads_given = false;
    mbdraco::Settings draco;
    bool draco_precision_given = false;
    bool draco_normal_bits_given = false;
    bool verbose = false;
    bool write_html = false;
    std::string x3dom_url;           // empty: the default of the page writer
    bool write_cesium_html = false;  // --cesium-html: a page for CesiumJS as well
    std::string cesium_url;          // empty: the default of the page writer
    std::vector<X3DomUserViewpoint> viewpoints;   // --geoviewpoint, for the pages
    double lift = 0.0;               // meters up along the local vertical, in the transform of tileset.json
    bool lift_requested = false;     // --lift as --geoorigin of mbmesh works: without it no lift; with no value, decided from the depth of the mesh
    bool lift_given = false;         // with a value
    double sse = 1.0;                // the maximumScreenSpaceError of the page's GeoTileset, in pixels
    bool sse_given = false;
    bool colors = true;              // the tiles are colored by height, unless --no-colors
};

struct TileRecord {
    std::string uri;
    double minimum[3];
    double maximum[3];
};

void print_usage() {
    std::cout <<
        "mbglb2tiles: an OGC 3D Tiles tileset of levels of detail from a GLB mesh made by mbmesh or mbgrd2gltf\n\n"
        "usage: mbglb2tiles -I mesh.glb [-O outputdir] [options]\n\n"
        "  -I, --input <file.glb>       The mesh: a GLB of mbmesh or mbgrd2gltf (with the georeference in its extras)\n"
        "  -O, --output <outputdir>     Where tileset.json and the tiles go [tileset beside the input, where mbmesh --tileset writes its tileset]\n"
        "      --tile-size <meters>     The side of the tiles of the finest level [256 times the spacing of the mesh, to two digits]; each level\n"
        "                               above has tiles of twice the side, until one tile holds everything\n"
        "      --weld-tolerance <m>     Vertices of the file this close are one [twice the quantization step the file records, or 0.001]\n"
        "      --threads <n>            Tiles written at once [the cores, at most 8]\n"
        "      --html                   Also write a viewer page, <outputdir>.html beside the output directory, that shows the tileset with an X3DOM\n"
        "                               GeoTileset node, and print the command to serve and view it\n"
        "      --cesium-html            Also write a viewer page for CesiumJS, <outputdir>_cesium.html beside the output directory. The globe of Cesium\n"
        "                               would hide data below the ellipsoid, so the page starts with it hidden and has a check box that shows it,\n"
        "                               translucent. The tileset is shown at its true height whatever --lift says\n"
        "      --cesium-url <url>       The directory of the CesiumJS build of the --cesium-html page, with Cesium.js and Widgets/widgets.css in it\n"
        "                               [https://cesium.com/downloads/cesiumjs/releases/1.114/Build/Cesium]\n"
        "      --geoviewpoint <lon,lat,elev,clon,clat,celev>\n"
        "                               A viewpoint for the --html and --cesium-html pages, which can be repeated: the position of the camera and the\n"
        "                               point it looks at, geodetic, true heights. Ten numbers, lon,lat,elev,ax,ay,az,angle,clon,clat,celev, give the\n"
        "                               orientation too, as an axis and an angle in the local frame (as a GeoViewpoint takes it; 1 0 0 -1.5708 looks\n"
        "                               straight down). The pages offer them in the Viewpoint menu as the viewpoints of the user. The Copy GeoViewpoint\n"
        "                               button of the X3DOM page and the Copy viewpoint button of the Cesium page give the option for the view\n"
        "      --x3dom-url <url>        The X3DOM script of the --html page [https://www.x3dom.org/x3dom/dist/x3dom-full.js]; a GeoTileset needs a\n"
        "                               build of X3DOM that has it\n"
        "      --lift[=<meters>]        Lift the tileset above the ellipsoid along the local vertical, in the transform of tileset.json:\n"
        "                               X3DOM's GeoTileset does not refine tiles for a camera below the ellipsoid, as for seafloor data. With no value,\n"
        "                               enough to put the lowest point of the mesh 10 m above the ellipsoid, to a meter, if it is lower; a value must be\n"
        "                               attached: --lift=900. Without --lift the tileset has the true heights\n"
        "      --sse <pixels>           With --html, the maximumScreenSpaceError of the page's GeoTileset: a tile is replaced by its children when\n"
        "                               its geometric error would show larger than this [1; X3DOM's own default is 8, too coarse for tiles of\n"
        "                               centimeter errors]\n"
        "      --no-colors              Do not color the tiles by height. By default every vertex has a color (COLOR_0) from the Haxby table\n"
        "                               over the heights of the whole mesh, so that all tiles and levels agree; the page can turn the colors off\n"
        "      --no-draco               Write the tiles as arrays [compressed with Draco]\n"
        "      --draco-precision <m>    With Draco, keep positions to this many meters [0.001]\n"
        "      --draco-normal-bits <n>  With Draco, the bits of each normal, 2-30 [10]\n"
        "  -V, --verbose                Log the stages of every level\n"
        "  -h, --help                   This text\n";
}

[[nodiscard]] double round_to_two_digits(double value) {
    const double scale = std::pow(10.0, std::floor(std::log10(value)) - 1.0);
    return std::round(value / scale) * scale;
}

[[nodiscard]] bool parse_positive(const char *text, double *value) {
    char *end = nullptr;
    *value = std::strtod(text, &end);
    return end != text && *end == '\0' && std::isfinite(*value) && *value > 0.0;
}

} // namespace

int main(int argc, char **argv) {
    mbmesh::Logger::set_level(mbmesh::LogLevel::INFO);
    Options options;
    static const struct option long_options[] = {
        {"input", required_argument, nullptr, 'I'},
        {"output", required_argument, nullptr, 'O'},
        {"tile-size", required_argument, nullptr, 1000},
        {"weld-tolerance", required_argument, nullptr, 1001},
        {"threads", required_argument, nullptr, 1002},
        {"html", no_argument, nullptr, 1006},
        {"x3dom-url", required_argument, nullptr, 1007},
        {"lift", optional_argument, nullptr, 1008},
        {"sse", required_argument, nullptr, 1009},
        {"no-colors", no_argument, nullptr, 1010},
        {"cesium-html", no_argument, nullptr, 1011},
        {"cesium-url", required_argument, nullptr, 1012},
        {"geoviewpoint", required_argument, nullptr, 1013},
        {"no-draco", no_argument, nullptr, 1003},
        {"draco-precision", required_argument, nullptr, 1004},
        {"draco-normal-bits", required_argument, nullptr, 1005},
        {"verbose", no_argument, nullptr, 'V'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}};
    int option;
    while ((option = getopt_long(argc, argv, "I:O:Vh", long_options, nullptr)) != -1) {
        std::string problem;
        switch (option) {
        case 'I':
            options.input = optarg;
            break;
        case 'O':
            options.output = optarg;
            break;
        case 1000:
            if (!parse_positive(optarg, &options.tile_size)) {
                LOG_ERROR("invalid --tile-size:", std::string("'") + optarg + "' is not a size in meters, more than zero");
                return 1;
            }
            break;
        case 1001:
            if (!parse_positive(optarg, &options.weld_tolerance)) {
                LOG_ERROR("invalid --weld-tolerance:", std::string("'") + optarg + "' is not a size in meters, more than zero");
                return 1;
            }
            break;
        case 1002:
            options.threads = std::max(1, std::atoi(optarg));
            options.threads_given = true;
            break;
        case 1006:
            options.write_html = true;
            break;
        case 1007:
            options.x3dom_url = optarg;
            break;
        case 1010:
            options.colors = false;
            break;
        case 1011:
            options.write_cesium_html = true;
            break;
        case 1012:
            options.cesium_url = optarg;
            break;
        case 1013: {
            X3DomUserViewpoint viewpoint;
            std::string viewpoint_error;
            double axis[3];
            double angle = 0.0;
            if (!parse_user_viewpoint(optarg, &viewpoint, &viewpoint_error) || !geoviewpoint_orientation(viewpoint, axis, &angle, &viewpoint_error)) {
                LOG_ERROR("invalid --geoviewpoint:", viewpoint_error);
                return 1;
            }
            options.viewpoints.push_back(viewpoint);
            break;
        }
        case 1008: {
            options.lift_requested = true;
            if (optarg == nullptr) {
                break;   // the default lift
            }
            char *end = nullptr;
            options.lift = std::strtod(optarg, &end);
            if (end == optarg || *end != '\0' || !std::isfinite(options.lift)) {
                LOG_ERROR("invalid --lift:", std::string("'") + optarg + "' is not a height in meters");
                return 1;
            }
            options.lift_given = true;
            break;
        }
        case 1009: {
            char *end = nullptr;
            options.sse = std::strtod(optarg, &end);
            if (end == optarg || *end != '\0' || !(options.sse >= 1.0e-3)) {
                LOG_ERROR("invalid --sse:", std::string("'") + optarg + "' is not a number of pixels, at least 0.001");
                return 1;
            }
            options.sse_given = true;
            break;
        }
        case 1003:
            options.draco.enabled = false;
            break;
        case 1004:
            if (!mbdraco::parse_precision(optarg, &options.draco.precision, &problem)) {
                LOG_ERROR("invalid --draco-precision:", problem);
                return 1;
            }
            options.draco_precision_given = true;
            break;
        case 1005:
            if (!mbdraco::parse_normal_bits(optarg, &options.draco.normal_bits, &problem)) {
                LOG_ERROR("invalid --draco-normal-bits:", problem);
                return 1;
            }
            options.draco_normal_bits_given = true;
            break;
        case 'V':
            options.verbose = true;
            mbmesh::Logger::set_level(mbmesh::LogLevel::DEBUG);
            break;
        case 'h':
            print_usage();
            return 0;
        default:
            print_usage();
            return 1;
        }
    }
    if (options.input.empty()) {
        LOG_ERROR("no input: give a GLB with -I");
        print_usage();
        return 1;
    }
    const bool output_given = !options.output.empty();
    if (!output_given) {
        options.output = options.input.parent_path() / "tileset";
    }
    if (options.threads <= 0) {
        options.threads = static_cast<int>(std::min(8u, std::max(1u, std::thread::hardware_concurrency())));
    }
    const mbmesh::Stopwatch run_timer;

    std::string command_line;
    // The log of the run is kept for the page, whatever the console level.
    mbmesh::Logger::start_capture();
    {
        for (int k = 0; k < argc; k++) {
            command_line += (k == 0 ? "" : " ") + std::string(argv[k]);
        }
        LOG_INFO("[command]", command_line);
        LOG_INFO("[settings]", "mbglb2tiles " + std::string(MB_VERSION) + ": tiles of levels of detail from " + options.input.string());
        LOG_INFO("[settings]", "output: " + options.output.string() + (output_given ? " (--output)" : " (default: tileset beside the input, where mbmesh --tileset writes its tileset)"));
        LOG_INFO("[settings]", "threads: " + std::to_string(options.threads) + (options.threads_given ? " (--threads)" : " (default: the cores, at most 8)"));
        LOG_INFO("[settings]", options.draco.enabled ? "Draco compression of the tiles: on (default; --no-draco turns it off), positions to " +
                                                           std::to_string(options.draco.precision) + " m" + (options.draco_precision_given ? " (--draco-precision)" : " (default)") + ", normals in " +
                                                           std::to_string(options.draco.normal_bits) + " bits" + (options.draco_normal_bits_given ? " (--draco-normal-bits)" : " (default)")
                                                     : std::string("Draco compression of the tiles: off (--no-draco), the tiles are written as arrays"));
        LOG_INFO("[settings]", std::string("tile size: ") + (options.tile_size > 0.0 ? std::to_string(options.tile_size) + " m at the finest level (--tile-size)"
                                                                                       : "256 times the spacing of the mesh at the finest level (default; decided once the mesh is read)"));
        LOG_INFO("[settings]", std::string("weld tolerance: ") + (options.weld_tolerance > 0.0 ? std::to_string(options.weld_tolerance) + " m (--weld-tolerance)"
                                                                                                : "twice the Draco step that the file records, at least 1 mm (default; decided once the file is read)"));
        LOG_INFO("[settings]", options.write_html ? "viewer page: yes (--html), X3DOM from " + (options.x3dom_url.empty() ? std::string("https://www.x3dom.org/x3dom/dist/x3dom-full.js (default)") : options.x3dom_url + " (--x3dom-url)") +
                                                        ", maximumScreenSpaceError " + std::to_string(options.sse) + " px" + (options.sse_given ? " (--sse)" : " (default)")
                                                  : std::string("viewer page: no (--html writes one)"));
        LOG_INFO("[settings]", options.write_cesium_html ? "Cesium page: yes (--cesium-html), CesiumJS from " + (options.cesium_url.empty() ? std::string(default_cesium_url()) + " (default)" : options.cesium_url + " (--cesium-url)")
                                                         : std::string("Cesium page: no (--cesium-html writes one)"));
        LOG_INFO("[settings]", options.viewpoints.empty() ? std::string("viewpoints of the user: none (--geoviewpoint adds one)")
                                                          : "viewpoints of the user: " + std::to_string(options.viewpoints.size()) + " (--geoviewpoint)");
        LOG_INFO("[settings]", !options.lift_requested ? std::string("lift: none, the true heights (default; --lift lifts the tileset above the ellipsoid)")
                                  : options.lift_given    ? "lift: " + std::to_string(options.lift) + " m (--lift)"
                                                          : std::string("lift: decided from the depth of the mesh once it is read (--lift)"));
    }

    // ---- Read the mesh and put it in the local frame ----
    GlbMesh glb;
    std::string error;
    {
        const mbmesh::Stopwatch timer;
        std::error_code size_error;
        const auto input_size = std::filesystem::file_size(options.input, size_error);
        LOG_INFO("[input]", "reading " + options.input.string() + (size_error ? std::string() : " (" + std::to_string(input_size / (1024 * 1024)) + " MiB)"));
        if (!read_mesh_glb(options.input, &glb, &error, options.threads)) {
            LOG_ERROR(error);
            return 1;
        }
        LOG_INFO("[input]", "read " + std::to_string(glb.mesh.vertices.size()) + " vertices and " + std::to_string(glb.mesh.indices.size() / 3) + " triangles from " +
                                std::to_string(glb.primitives) + " primitives (" + std::to_string(glb.draco_primitives) + " Draco) in " + timer.text());
    }
    if (!glb.has_center) {
        LOG_WARN("the vertices of the file are absolute ECEF stored as 32-bit floats, which resolve about 0.5 m: the tiles cannot be better than that; make the mesh with --geoorigin");
    }
    Mesh &mesh = glb.mesh;
    double ecef_minimum[3] = {1e300, 1e300, 1e300}, ecef_maximum[3] = {-1e300, -1e300, -1e300};
    for (Vec3 &vertex : mesh.vertices) {
        if (glb.has_center) {
            vertex = vertex + glb.center;
        }
        const double values[3] = {vertex.x, vertex.y, vertex.z};
        for (int k = 0; k < 3; k++) {
            ecef_minimum[k] = std::min(ecef_minimum[k], values[k]);
            ecef_maximum[k] = std::max(ecef_maximum[k], values[k]);
        }
    }
    const Vec3 middle(0.5 * (ecef_minimum[0] + ecef_maximum[0]), 0.5 * (ecef_minimum[1] + ecef_maximum[1]), 0.5 * (ecef_minimum[2] + ecef_maximum[2]));
    const CoordinateFrame frame = coordinate_frame_from_origin(ecef_to_geodetic(middle));
    for (Vec3 &vertex : mesh.vertices) {
        vertex = ecef_to_local(vertex, frame);
    }
    {
        std::ostringstream text;
        text << std::fixed << std::setprecision(6) << "local east-north-up frame at the middle of the bounds of the mesh: longitude " << frame.origin.longitude << ", latitude "
             << frame.origin.latitude << ", height " << std::setprecision(2) << frame.origin.elevation << " m; the vertices of the file are "
             << (glb.has_center ? "relative to a center (the GeoOrigin of the mesh)" : "absolute ECEF");
        LOG_INFO("[frame]", text.str());
    }

    // ---- Weld the copies of the vertices on the edges of the primitives, and make the normals ----
    {
        const mbmesh::Stopwatch timer;
        const double tolerance = options.weld_tolerance > 0.0 ? options.weld_tolerance : (glb.position_step > 0.0 ? std::max(0.001, 2.0 * glb.position_step) : 0.001);
        const std::size_t before = mesh.vertices.size();
        LOG_INFO("[weld]", "joining the vertices within " + std::to_string(tolerance) + " m of each other, the copies on the edges of the primitives" +
                               (options.weld_tolerance > 0.0 ? " (--weld-tolerance)" : (glb.position_step > 0.0 ? " (default: twice the Draco step of " + std::to_string(glb.position_step) + " m that the file records)" : " (default: 1 mm)")) +
                               (mesh.vertices.size() > 5000000 ? std::string("; this takes minutes for tens of millions of vertices") : std::string()));
        weld_coincident_vertices(&mesh, tolerance);
        LOG_INFO("[weld]", "making the normals again from the joined surface");
        compute_vertex_normals(&mesh);
        LOG_INFO("[weld]", "vertices within " + std::to_string(tolerance) + " m joined: " + std::to_string(before) + " to " + std::to_string(mesh.vertices.size()) + " in " + timer.text());
    }

    // ---- The lattices ----
    double minimum[3] = {1e300, 1e300, 1e300}, maximum[3] = {-1e300, -1e300, -1e300};
    for (const Vec3 &vertex : mesh.vertices) {
        const double values[3] = {vertex.x, vertex.y, vertex.z};
        for (int k = 0; k < 3; k++) {
            minimum[k] = std::min(minimum[k], values[k]);
            maximum[k] = std::max(maximum[k], values[k]);
        }
    }
    const double spacing = native_spacing(mesh);
    if (!(spacing > 0.0)) {
        LOG_ERROR("the mesh has no area seen from above, so there is no spacing to choose tiles from");
        return 1;
    }
    const double tile_size = options.tile_size > 0.0 ? options.tile_size : round_to_two_digits(cells_per_tile * spacing);
    const double extent = std::max(maximum[0] - minimum[0], maximum[1] - minimum[1]);
    int top = 0;
    while (tile_size * std::pow(2.0, top) <= extent * (1.0 + 1e-9)) {
        top++;
    }
    const Vec3 anchor(minimum[0], minimum[1], minimum[2]);
    LOG_INFO("[levels]", "spacing of the mesh " + std::to_string(spacing) + " m (area seen from above divided by the vertices, square root); tiles of " + std::to_string(tile_size) +
                             " m at the finest level" + (options.tile_size > 0.0 ? " (--tile-size)" : " (default: 256 times the spacing, to two digits)") + ", doubling at each level until one tile holds the mesh, over " +
                             std::to_string(extent) + " m: " + std::to_string(top + 1) + " levels, the last of one tile of " + std::to_string(tile_size * std::pow(2.0, top)) + " m");
    for (int level = 0; level <= top; level++) {
        LOG_INFO("[levels]", "level " + std::to_string(level) + ": tiles of " + std::to_string(tile_size * std::pow(2.0, level)) + " m, " +
                                 (level == 0 ? std::string("the mesh itself, geometric error 0") : "vertices clustered in cubes of " + std::to_string(spacing * std::pow(2.0, level)) + " m, geometric error " + std::to_string(spacing * std::pow(2.0, level)) + " m"));
    }

    // The metadata of the tiles: the frame they are in, and where it is.
    RunInfo run;
    run.program = "mbglb2tiles";
    run.version = MB_VERSION;
    run.datalist = std::filesystem::absolute(options.input).string();
    run.command_line = command_line;
    run.started_at = iso_utc_time(static_cast<double>(std::time(nullptr)));
    {
        const char *user = std::getenv("USER");
        run.user = user != nullptr ? user : "";
        char host[256] = "";
        if (gethostname(host, sizeof(host) - 1) == 0) {
            run.host = host;
        }
        std::error_code ignored;
        run.working_directory = std::filesystem::current_path(ignored).string();
    }
    run.geoorigin_mode = "none";
    run.has_center = true;
    run.local_frame = true;
    run.center_x = frame.origin_ecef.x;
    run.center_y = frame.origin_ecef.y;
    run.center_z = frame.origin_ecef.z;
    run.center_longitude = frame.origin.longitude;
    run.center_latitude = frame.origin.latitude;
    run.center_height = frame.origin.elevation;
    {
        double west = 1e300, south = 1e300, east = -1e300, north = -1e300;
        for (const double x : {minimum[0], maximum[0]}) {
            for (const double y : {minimum[1], maximum[1]}) {
                const GeodeticPosition corner = local_to_geodetic(Vec3(x, y, 0.5 * (minimum[2] + maximum[2])), frame);
                west = std::min(west, corner.longitude);
                east = std::max(east, corner.longitude);
                south = std::min(south, corner.latitude);
                north = std::max(north, corner.latitude);
            }
        }
        run.extent_west = west;
        run.extent_south = south;
        run.extent_east = east;
        run.extent_north = north;
        run.mean_elevation = frame.origin.elevation + 0.5 * (minimum[2] + maximum[2]);
    }
    // The lift: X3DOM's GeoTileset takes the height of the camera as at least 1 m above the ellipsoid, so tiles of data below it
    // are never close enough to refine. The transform of the tileset puts the frame this much higher.
    const double lowest_height = frame.origin.elevation + minimum[2];
    if (!options.lift_requested) {
        options.lift = 0.0;
        if (lowest_height < 0.0) {
            LOG_WARN("[lift]", "the lowest point of the mesh is at " + std::to_string(lowest_height) + " m, below the ellipsoid: X3DOM's GeoTileset takes the height of the camera as at least 1 m, so its tiles will not refine; use --lift to lift the tileset");
        }
    } else if (!options.lift_given) {
        options.lift = default_tileset_lift(lowest_height);
    }
    const CoordinateFrame placed_frame = lifted_frame(frame, options.lift);
    LOG_INFO("[lift]", "the lowest point of the mesh is at height " + std::to_string(lowest_height) + " m above the ellipsoid; the tileset is " +
                           (options.lift != 0.0 ? "lifted " + std::to_string(options.lift) + " m" : std::string("not lifted")) +
                           (!options.lift_requested ? " (no --lift)" : options.lift_given ? " (--lift)" : " (--lift: enough to put the lowest point 10 m above the ellipsoid, to a meter)") +
                           (options.lift != 0.0 ? ", so every height in the tileset is that much more than the true one" : ""));
    MetaValue metadata = run_metadata(run);
    set_metadata_lift(&metadata, options.lift);
    std::vector<double> color_bounds;   // the heights the colors change between, empty without colors
    if (options.colors) {
        color_bounds = haxby_height_bounds(frame.origin.elevation + minimum[2], frame.origin.elevation + maximum[2]);
        set_metadata_height_colors(&metadata, 0.0, color_bounds);
        LOG_INFO("[colors]", "tiles colored by height with the Haxby table over " + std::to_string(color_bounds.front()) + " to " + std::to_string(color_bounds.back()) + " m (--no-colors turns it off)");
    }
    set_glb_compression(options.draco);

    std::error_code directory_error;
    {
        const std::size_t removed = remove_previous_tileset(options.output);
        if (removed > 0) {
            LOG_INFO("[output]", "replacing the tileset already in " + options.output.string() + " (" + std::to_string(removed) + " files and directories removed)");
        }
    }
    std::filesystem::create_directories(options.output, directory_error);
    if (directory_error) {
        LOG_ERROR("cannot create " + options.output.string() + ": " + directory_error.message());
        return 1;
    }

    // ---- Each level: coarsen, cut into tiles, write them ----
    std::vector<std::map<std::pair<int, int>, TileRecord>> records(static_cast<std::size_t>(top) + 1);
    Mesh coarse;
    std::size_t tiles_written = 0, bytes_written = 0;
    for (int level = 0; level <= top; level++) {
        const mbmesh::Stopwatch level_timer;
        const double size = tile_size * std::pow(2.0, level);
        const Mesh *level_mesh = &mesh;
        LOG_INFO("[level]", "level " + std::to_string(level) + " of " + std::to_string(top) + ": " + (level == 0 ? "cutting the mesh into tiles" : "clustering the mesh, then cutting it into tiles"));
        if (level > 0) {
            if (!cluster_mesh(mesh, anchor, spacing * std::pow(2.0, level), &coarse, &error)) {
                LOG_ERROR(error);
                return 1;
            }
            compute_vertex_normals(&coarse);
            level_mesh = &coarse;
        }
        const std::vector<MeshTile> tiles = cut_mesh(*level_mesh, minimum[0], minimum[1], size);
        const std::filesystem::path directory = options.output / ("L" + std::to_string(level));
        std::filesystem::create_directories(directory, directory_error);
        LOG_INFO("[level]", "level " + std::to_string(level) + ": writing " + std::to_string(tiles.size()) + " tiles to " + directory.string() + " on " + std::to_string(options.threads) + " threads");
        std::vector<TileRecord> made(tiles.size());
        std::atomic<bool> failed(false);
        std::mutex error_mutex;
        mbmesh::parallel_for_ranges(0, static_cast<long>(tiles.size()), options.threads, [&](long first, long last) {
            for (long n = first; n < last && !failed; n++) {
                const MeshTile &tile = tiles[static_cast<std::size_t>(n)];
                TileRecord &record = made[static_cast<std::size_t>(n)];
                record.uri = "L" + std::to_string(level) + "/tile_" + std::to_string(tile.i) + "_" + std::to_string(tile.j) + ".glb";
                if (!mesh_bounds(tile.mesh, record.minimum, record.maximum)) {
                    continue;
                }
                GlbContent content{"tileset_tile", "Level " + std::to_string(level) + " of " + std::to_string(top) + " of the tileset, in the east-north-up frame at the center, Y-up", true, tile.i, tile.j};
                content.metadata = &metadata;
                std::string tile_error;
                Mesh content_mesh = tileset_content_mesh(tile.mesh);
                if (!color_bounds.empty()) {
                    Mesh by_height = tile.mesh;
                    color_mesh_by_height(&by_height, frame.origin.elevation, color_bounds);
                    content_mesh.colors = std::move(by_height.colors);
                }
                if (!write_mesh_glb_file(options.output / record.uri, content_mesh, &tile_error, content)) {
                    std::lock_guard<std::mutex> lock(error_mutex);
                    LOG_ERROR(tile_error);
                    failed = true;
                }
            }
        });
        if (failed) {
            return 1;
        }
        std::size_t level_bytes = 0;
        for (std::size_t n = 0; n < tiles.size(); n++) {
            records[static_cast<std::size_t>(level)][{tiles[n].i, tiles[n].j}] = made[n];
            std::error_code size_error;
            const auto file_size = std::filesystem::file_size(options.output / made[n].uri, size_error);
            level_bytes += size_error ? 0 : static_cast<std::size_t>(file_size);
        }
        tiles_written += tiles.size();
        bytes_written += level_bytes;
        LOG_INFO("[level]", "level " + std::to_string(level) + ": " + std::to_string(level_mesh->vertices.size()) + " vertices, " + std::to_string(level_mesh->indices.size() / 3) +
                                " triangles, " + std::to_string(tiles.size()) + " tiles, " + std::to_string(level_bytes / 1024) + " KiB, in " + level_timer.text());
    }

    // ---- The tree: a tile's children are the tiles of the level below that lie in its square ----
    LOG_INFO("[tileset]", "joining the " + std::to_string(tiles_written) + " tiles into a tree: the children of a tile are the tiles of the level below that lie in its square");
    std::vector<std::set<std::pair<int, int>>> exists(static_cast<std::size_t>(top) + 1);
    for (int level = 0; level <= top; level++) {
        for (const auto &entry : records[static_cast<std::size_t>(level)]) {
            exists[static_cast<std::size_t>(level)].insert(entry.first);
        }
        if (level > 0) {
            for (const auto &child : exists[static_cast<std::size_t>(level) - 1]) {
                exists[static_cast<std::size_t>(level)].insert({child.first / 2, child.second / 2});
            }
        }
    }
    std::function<TilesetNode(int, int, int)> build = [&](int level, int i, int j) {
        TilesetNode node;
        bool has_box = false;
        const auto found = records[static_cast<std::size_t>(level)].find({i, j});
        if (found != records[static_cast<std::size_t>(level)].end()) {
            node.uri = found->second.uri;
            std::copy(found->second.minimum, found->second.minimum + 3, node.minimum);
            std::copy(found->second.maximum, found->second.maximum + 3, node.maximum);
            has_box = true;
        }
        node.geometric_error = level == 0 ? 0.0 : spacing * std::pow(2.0, level);
        if (level > 0) {
            for (int a = 0; a < 2; a++) {
                for (int b = 0; b < 2; b++) {
                    if (exists[static_cast<std::size_t>(level) - 1].count({2 * i + a, 2 * j + b}) == 0) {
                        continue;
                    }
                    TilesetNode child = build(level - 1, 2 * i + a, 2 * j + b);
                    for (int k = 0; k < 3; k++) {
                        node.minimum[k] = has_box ? std::min(node.minimum[k], child.minimum[k]) : child.minimum[k];
                        node.maximum[k] = has_box ? std::max(node.maximum[k], child.maximum[k]) : child.maximum[k];
                    }
                    has_box = true;
                    node.children.push_back(std::move(child));
                }
            }
        }
        return node;
    };
    if (exists[static_cast<std::size_t>(top)].size() != 1) {
        LOG_ERROR("the top level has " + std::to_string(exists[static_cast<std::size_t>(top)].size()) + " tiles, not one");
        return 1;
    }
    const std::pair<int, int> root_index = *exists[static_cast<std::size_t>(top)].begin();
    const TilesetNode root = build(top, root_index.first, root_index.second);
    const std::filesystem::path tileset_path = options.output / "tileset.json";
    if (!write_tileset_tree_json(tileset_path, placed_frame, root, metadata, "MB-System mbglb2tiles " + std::string(MB_VERSION), &error)) {
        LOG_ERROR(error);
        return 1;
    }
    LOG_INFO("[tileset]", "wrote " + tileset_path.string() + " with " + std::to_string(tiles_written) + " tiles in " + std::to_string(top + 1) + " levels, " +
                              std::to_string(bytes_written / (1024 * 1024)) + " MiB");
    LOG_INFO("[complete]", "total " + run_timer.text() + ", " + mbmesh::peak_memory_text());

    // ---- The viewer pages ----
    const auto url_encode = [](const std::string &text) {
        std::string encoded;
        for (const unsigned char character : text) {
            if (std::isalnum(character) || character == '-' || character == '.' || character == '_' || character == '~' || character == '/') {
                encoded += static_cast<char>(character);
            } else {
                std::ostringstream hex;
                hex << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(character);
                encoded += hex.str();
            }
        }
        return encoded;
    };
    const std::filesystem::path web_root = viewer_web_root(options.output);
    const std::filesystem::path log_path = options.output / "mbglb2tiles.log";
    const std::string directory_name = options.output.has_filename() ? options.output.filename().string() : options.output.parent_path().filename().string();
    const std::filesystem::path absolute_root = std::filesystem::absolute(web_root);
    const auto write_log = [&]() {
        std::string log_error;
        if (!write_log_file(log_path, mbmesh::Logger::captured_messages(), mbmesh::Logger::captured_omitted(), &log_error)) {
            LOG_ERROR(log_error);
            return false;
        }
        return true;
    };

    // The pages that were written, for the command that serves them and the URL of each.
    std::vector<std::string> page_names;

    if (options.write_html) {
        const std::filesystem::path html_path = viewer_page_path(options.output);

        X3DomWriterOptions html_options;
        html_options.title = viewer_title(web_root);
        if (!options.x3dom_url.empty()) {
            html_options.x3dom_url = options.x3dom_url;
        }
        // The page has a GeoOrigin at the origin of the frame, as the tileset of mbmesh with --geoorigin does, and an overview from above.
        html_options.use_geoorigin = true;
        X3DomGeoOrigin &view = html_options.geo;
        view.origin_lat = frame.origin.latitude;
        view.origin_lon = frame.origin.longitude;
        view.origin_elevation = frame.origin.elevation;
        view.center_lat = 0.5 * (run.extent_south + run.extent_north);
        view.center_lon = 0.5 * (run.extent_west + run.extent_east);
        view.mean_elevation = run.mean_elevation;
        constexpr double pi = 3.14159265358979323846;
        const double lat_extent_m = (run.extent_north - run.extent_south) * 111000.0;
        const double lon_extent_m = (run.extent_east - run.extent_west) * 111000.0 * std::cos(view.center_lat * pi / 180.0);
        view.view_altitude = (std::max(lat_extent_m, lon_extent_m) / 2.0) / std::tan(0.785398 / 2.0);
        {
            const Vec3 middle_ecef = geodetic_to_ecef(GeodeticPosition{view.center_lon, view.center_lat, view.mean_elevation});
            const double middle_array[3] = {middle_ecef.x, middle_ecef.y, middle_ecef.z};
            const double origin_array[3] = {frame.origin_ecef.x, frame.origin_ecef.y, frame.origin_ecef.z};
            set_local_viewpoint(&view, middle_array, origin_array);
        }
        X3DomModel model;
        model.uri = url_encode(directory_name + "/tileset.json");
        model.max_screen_space_error = options.sse;
        model.height_colors = options.colors;
        model.lift_meters = options.lift;
        model.label = directory_name + " (OGC 3D Tiles, " + std::to_string(top + 1) + " levels)";
        model.group = directory_name;
        model.visible = true;
        model.is_tileset = true;
        html_options.models.push_back(model);
        html_options.user_viewpoints = options.viewpoints;
        html_options.metadata = metadata;
        html_options.geo_summary = geo_metadata_summary(run);
        if (!write_log()) {
            return 1;
        }
        html_options.log_url = url_encode(directory_name + "/mbglb2tiles.log");
        if (!write_glb_x3dom_file(html_path, html_options, &error)) {
            LOG_ERROR(error);
            return 1;
        }
        LOG_INFO("[html]", "wrote " + html_path.string() + ", showing the tileset and the log " + log_path.string());
        page_names.push_back(html_path.filename().string());
    }

    if (options.write_cesium_html) {
        const std::filesystem::path cesium_path = cesium_page_path(options.output);
        if (!write_log()) {
            return 1;
        }
        CesiumPageOptions cesium_options;
        cesium_options.title = viewer_title(web_root);
        cesium_options.cesium_url = options.cesium_url;
        CesiumModel tileset_model;
        tileset_model.uri = url_encode(directory_name + "/tileset.json");
        tileset_model.label = directory_name + " (OGC 3D Tiles, " + std::to_string(top + 1) + " levels)";
        tileset_model.visible = true;
        tileset_model.is_tileset = true;
        tileset_model.max_screen_space_error = options.sse;
        tileset_model.lift_meters = options.lift;
        tileset_model.height_colors = options.colors;
        tileset_model.log_uri = url_encode(directory_name + "/mbglb2tiles.log");
        tileset_model.log_label = "mbglb2tiles log";
        cesium_options.models.push_back(tileset_model);
        if (!cesium_user_viewpoints(options.viewpoints, &cesium_options.user_viewpoints, &error)) {
            LOG_ERROR("invalid --geoviewpoint:", error);
            return 1;
        }
        if (!write_cesium_page(cesium_path, cesium_options, &error)) {
            LOG_ERROR(error);
            return 1;
        }
        LOG_INFO("[cesium]", "wrote " + cesium_path.string() + ", showing the tileset with the globe hidden; the check box in the page shows it");
        page_names.push_back(cesium_path.filename().string());
    }
    if (!page_names.empty()) {
        LOG_INFO("[html]", std::string("to view ") + (page_names.size() > 1 ? "the pages" : page_names[0]) +
                               ", run this command in a terminal (Ctrl-C stops it), then open the URL" + (page_names.size() > 1 ? "s" : "") + ":");
        LOG_INFO("[html]", "  python3 -m http.server 8000 --bind 127.0.0.1 --directory '" + absolute_root.string() + "'");
        for (const std::string &name : page_names) {
            LOG_INFO("[html]", "  http://127.0.0.1:8000/" + url_encode(name));
        }
    }
    return 0;
}
