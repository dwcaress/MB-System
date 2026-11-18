#include <algorithm>
#include "io/x3dom_writer.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

bool check(bool condition, const std::string &message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message.c_str());
    }
    return condition;
}

bool contains(const std::string &text, const std::string &part) {
    return text.find(part) != std::string::npos;
}

std::string write_page(const X3DomWriterOptions &options) {
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "mbmesh_test_x3dom.html";
    std::string error;
    if (!write_glb_x3dom_file(path, options, &error)) {
        std::fprintf(stderr, "FAIL: writing the page: %s\n", error.c_str());
        return "";
    }
    std::ifstream input(path);
    std::string page((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    std::filesystem::remove(path);
    return page;
}

X3DomModel model(const std::string &uri, const std::string &label, const std::string &group, bool visible) {
    X3DomModel m;
    m.uri = uri;
    m.label = label;
    m.group = group;
    m.visible = visible;
    return m;
}

void touch(const std::filesystem::path &path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path) << "x";
}

} // namespace

int main() {
    bool ok = true;

    X3DomGeoOrigin geo;
    geo.origin_lat = 36.6;
    geo.origin_lon = -122.4;
    geo.origin_elevation = -10.0;
    geo.center_lat = 36.61;
    geo.center_lon = -122.39;
    geo.mean_elevation = -50.0;
    geo.view_altitude = 100.0;

    // ---- the page has the controls and the processing log of mbgrd2gltf's page
    {
        X3DomWriterOptions options;
        options.title = "OceanImaging2018/20180923d1/multibeam";
        options.geo = geo;
        options.models = {model("mbmesh_output/mesh.glb", "mesh.glb", "mbmesh_output", true)};
        options.log_messages = {"INFO first message", "DEBUG second message"};
        const std::string page = write_page(options);

        ok &= check(contains(page, "https://www.x3dom.org/x3dom/dist/x3dom-full.js"), "the page loads X3DOM");
        ok &= check(contains(page, "Navigation Mode:") && contains(page, "</input> Stats") && contains(page, "</input> Log"),
                    "the page has the navigation mode and debug display controls");
        ok &= check(contains(page, "runtime.showAll()") && contains(page, "runtime.resetView()"), "the page has Show Everything and Reset View");
        ok &= check(contains(page, "id='model-title'>OceanImaging2018/20180923d1/multibeam<") &&
                        contains(page, "<title>OceanImaging2018/20180923d1/multibeam</title>"),
                    "the title is the survey directories");
        ok &= check(contains(page, "Show Processing Log") && contains(page, "id='log-panel'") && contains(page, "function toggleLogPanel()"),
                    "the page has the processing log panel and its button");
        ok &= check(contains(page, "Processing Log (2 messages)") &&
                        contains(page, "<div class='log-line'>INFO first message</div>") &&
                        contains(page, "<div class='log-line'>DEBUG second message</div>"),
                    "the log shows each message and their number");

        // No GeoOrigin: no GeoOrigin node, and the viewpoint does not refer to one.
        ok &= check(!contains(page, "<GeoOrigin") && !contains(page, "GRID_ORIGIN"), "without a GeoOrigin the page declares none");
        ok &= check(contains(page, "<GeoViewpoint DEF='OVERVIEW_GEO' id='OVERVIEW_GEO' description='Overview, GeoViewpoint with geoOrigin' position='36.610000 -122.390000 50.0'") &&
                        !contains(page, "<Viewpoint") && !contains(page, "bindViewpoint(this.value)"),
                    "the viewpoint is above the center of the data (latitude, longitude, elevation)");
    }

    // ---- the local viewpoint: in the coordinates of the models, looking straight down
    {
        // A viewpoint 100 m above a point on the equator at longitude 0, models relative to
        // an origin 1000 m east of it. ECEF there: x is up, y is east, z is north.
        X3DomGeoOrigin view;
        view.center_lat = 0.0;
        view.center_lon = 0.0;
        view.view_altitude = 100.0;
        const double center[3] = {6378137.0, 0.0, 0.0};
        const double origin[3] = {6378137.0, 1000.0, 0.0};
        set_local_viewpoint(&view, center, origin);
        ok &= check(view.has_local_view, "the local viewpoint is set");
        ok &= check(std::fabs(view.view_center[0]) < 1e-9 && std::fabs(view.view_center[1] + 1000.0) < 1e-9 && std::fabs(view.view_center[2]) < 1e-9,
                    "the center of rotation is the data center relative to the origin");
        ok &= check(std::fabs(view.view_position[0] - 100.0) < 1e-9 && std::fabs(view.view_position[1] + 1000.0) < 1e-9,
                    "the viewpoint is the altitude above the center, along up");

        // The rotation must send the viewpoint's X to east, Y to north and Z to up, so that the
        // viewing direction -Z is down and +Y, the top of the view, is north.
        const double c = std::cos(view.view_angle), s = std::sin(view.view_angle);
        const double *k = view.view_axis;
        const auto rotate = [&](const double v[3], double out[3]) {
            const double dot = k[0] * v[0] + k[1] * v[1] + k[2] * v[2];
            const double cross[3] = {k[1] * v[2] - k[2] * v[1], k[2] * v[0] - k[0] * v[2], k[0] * v[1] - k[1] * v[0]};
            for (int i = 0; i < 3; i++) {
                out[i] = v[i] * c + cross[i] * s + k[i] * dot * (1.0 - c);
            }
        };
        const double x_axis[3] = {1, 0, 0}, y_axis[3] = {0, 1, 0}, z_axis[3] = {0, 0, 1};
        double x[3], y[3], z[3];
        rotate(x_axis, x);
        rotate(y_axis, y);
        rotate(z_axis, z);
        ok &= check(std::fabs(x[1] - 1.0) < 1e-9, "the viewpoint's X is east (ECEF y at longitude 0)");
        ok &= check(std::fabs(y[2] - 1.0) < 1e-9, "the viewpoint's Y is north (ECEF z at the equator)");
        ok &= check(std::fabs(z[0] - 1.0) < 1e-9, "the viewpoint's Z is up (ECEF x at longitude 0, latitude 0)");

        X3DomWriterOptions options;
        options.title = "survey";
        options.use_geoorigin = true;
        options.geo = view;
        options.models = {model("mbmesh_output/mesh.glb", "mesh.glb", "mbmesh_output", true)};
        const std::string page = write_page(options);
        ok &= check(contains(page, "<Viewpoint DEF='OVERVIEW_LOCAL' id='OVERVIEW_LOCAL' description='Overview, Viewpoint in model coordinates' position='100.000 -1000.000 0.000'"),
                    "the page has a Viewpoint in the models' coordinates");
        ok &= check(contains(page, "<GeoViewpoint DEF='OVERVIEW_GEO'") && contains(page, "<GeoOrigin DEF='GRID_ORIGIN'") &&
                        page.find("<GeoOrigin DEF='GRID_ORIGIN'") > page.find("<GeoViewpoint DEF='OVERVIEW_GEO'") &&
                        page.find("<GeoOrigin DEF='GRID_ORIGIN'") < page.find("</GeoViewpoint>"),
                    "the page also has the GeoViewpoint, with the GeoOrigin as its child node");
        ok &= check(page.find("<Viewpoint DEF='OVERVIEW_LOCAL'") < page.find("<GeoViewpoint DEF='OVERVIEW_GEO'"),
                    "the Viewpoint comes first, so it is the one bound when the page opens");
        ok &= check(contains(page, "<option value='OVERVIEW_LOCAL'>") && contains(page, "<option value='OVERVIEW_GEO'>") && contains(page, "function bindViewpoint(id)"),
                    "the page has a menu to choose between the two");
    }

    // ---- the scene fills the window, whatever X3DOM does with the size of its canvas
    {
        X3DomWriterOptions options;
        options.title = "survey";
        options.geo = geo;
        options.models = {model("mbmesh_output/mesh.glb", "mesh.glb", "mbmesh_output", true)};
        const std::string page = write_page(options);
        ok &= check(contains(page, "x3d { display: block; width: 100vw !important; height: 100vh !important; }") &&
                        contains(page, "x3d canvas { display: block; width: 100% !important; height: 100% !important; }") &&
                        contains(page, "html, body { overflow: hidden; }"),
                    "the page sizes the scene and its canvas in viewport units, so that they cannot grow");
    }

    // ---- GeoViewpoints the user supplies with --geoviewpoint
    {
        X3DomUserViewpoint v;
        std::string error;
        ok &= check(parse_user_viewpoint("-122.4,36.6,-900,-122.39,36.61,-950", &v, &error) && !v.has_orientation && v.longitude == -122.4 &&
                        v.latitude == 36.6 && v.elevation == -900.0 && v.center_longitude == -122.39 && v.center_latitude == 36.61 &&
                        v.center_elevation == -950.0,
                    "six numbers are a position and a center, longitude first as --geoorigin takes them");
        ok &= check(parse_user_viewpoint("-122.4,36.6,-900,1,0,0,-1.5708,-122.39,36.61,-950", &v, &error) && v.has_orientation && v.angle == -1.5708 &&
                        v.axis[0] == 1.0 && v.center_latitude == 36.61,
                    "ten numbers are a position, an orientation and a center");
        ok &= check(!parse_user_viewpoint("1,2,3,4,5", &v, &error) && contains(error, "6 numbers"), "five numbers are refused");
        ok &= check(!parse_user_viewpoint("1,2,3,4,5,x", &v, &error) && contains(error, "'x'"), "a value that is not a number is refused and named");
        ok &= check(!parse_user_viewpoint("1,2,3,,5,6", &v, &error), "an empty value is refused");
        ok &= check(!parse_user_viewpoint("1 2 3 4 5 6", &v, &error), "spaces instead of commas are refused");
        ok &= check(!parse_user_viewpoint("1,2,3,0,0,0,1,4,5,6", &v, &error) && contains(error, "zero"), "a zero axis is refused");
        ok &= check(!parse_user_viewpoint("36.6,-122.4,-900,36.61,-122.39,-950", &v, &error) && contains(error, "lon,lat,elev"),
                    "latitude and longitude swapped is caught when the latitude is beyond 90");

        // The orientation a GeoViewpoint takes, in X3DOM, is relative to a local frame: no rotation looks level
        // to the north, and '1 0 0 -1.5708' looks straight down with north at the top and east on the right.
        const auto rotate = [](const double axis[3], double angle, const double in[3], double out[3]) {
            const double c = std::cos(angle), s = std::sin(angle);
            const double dot = axis[0] * in[0] + axis[1] * in[1] + axis[2] * in[2];
            const double cross[3] = {axis[1] * in[2] - axis[2] * in[1], axis[2] * in[0] - axis[0] * in[2], axis[0] * in[1] - axis[1] * in[0]};
            for (int i = 0; i < 3; i++) {
                out[i] = in[i] * c + cross[i] * s + axis[i] * dot * (1.0 - c);
            }
        };
        const double ex[3] = {1, 0, 0}, ey[3] = {0, 1, 0}, ez[3] = {0, 0, 1};
        {
            // Looking straight down from 100 m above the center is the orientation 1 0 0 -1.5708.
            X3DomUserViewpoint down;
            ok &= check(parse_user_viewpoint("0,0,100,0,0,0", &down, &error), "a look-at viewpoint parses");
            double axis[3], angle = 0.0;
            ok &= check(geoviewpoint_orientation(down, axis, &angle, &error), "it has an orientation");
            double rx[3], ry[3], rz[3];
            rotate(axis, angle, ex, rx);
            rotate(axis, angle, ey, ry);
            rotate(axis, angle, ez, rz);
            // The same rotation as 1 0 0 -1.5708: X stays, Y goes to -Z, Z goes to Y.
            ok &= check(std::fabs(rx[0] - 1.0) < 1e-9 && std::fabs(ry[2] + 1.0) < 1e-9 && std::fabs(rz[1] - 1.0) < 1e-9,
                        "looking straight down is the orientation 1 0 0 -1.5708: east on the right, north at the top");
        }
        {
            // 100 m south of the center, looking north and level: no rotation.
            X3DomUserViewpoint level;
            ok &= check(parse_user_viewpoint("0,-0.0008983,0,0,0,0", &level, &error), "a level viewpoint parses");
            double axis[3], angle = 0.0;
            ok &= check(geoviewpoint_orientation(level, axis, &angle, &error) && std::fabs(angle) < 1e-4, "looking north and level is no rotation");
        }
        {
            // Looking east and level turns the view a quarter turn to the right about up (the frame's Y).
            X3DomUserViewpoint east_looking;
            ok &= check(parse_user_viewpoint("-0.0008983,0,0,0,0,0", &east_looking, &error), "an east-looking viewpoint parses");
            double axis[3], angle = 0.0;
            ok &= check(geoviewpoint_orientation(east_looking, axis, &angle, &error), "it has an orientation");
            double rz[3];
            rotate(axis, angle, ez, rz);
            ok &= check(std::fabs(rz[0] + 1.0) < 1e-4, "looking east turns the view's back toward west (frame -X)");
        }
        {
            X3DomUserViewpoint same;
            double axis[3], angle = 0.0;
            ok &= check(parse_user_viewpoint("1,2,3,1,2,3", &same, &error) && !geoviewpoint_orientation(same, axis, &angle, &error) &&
                            contains(error, "same point"),
                        "a position equal to its center has no orientation");
        }
        {
            X3DomUserViewpoint given;
            ok &= check(parse_user_viewpoint("1,2,3,0,2,0,0.5,4,5,6", &given, &error), "ten numbers parse");
            double axis[3], angle = 0.0;
            ok &= check(geoviewpoint_orientation(given, axis, &angle, &error) && std::fabs(axis[1] - 1.0) < 1e-12 && angle == 0.5,
                        "a given orientation is used, with its axis made a unit vector");
        }

        // The page: GeoViewpoints after the overview ones, a menu entry for each, and a Copy GeoViewpoint button.
        X3DomGeoOrigin view;
        view.origin_lon = -122.4;
        view.origin_lat = 36.6;
        view.origin_elevation = -10.0;
        view.center_lat = 36.6;
        view.center_lon = -122.4;
        view.view_altitude = 100.0;
        const double center[3] = {6378137.0, 0.0, 0.0};
        set_local_viewpoint(&view, center, center);
        X3DomWriterOptions options;
        options.title = "survey";
        options.use_geoorigin = true;
        options.geo = view;
        options.models = {model("mbmesh_output/mesh.glb", "mesh.glb", "mbmesh_output", true)};
        X3DomUserViewpoint first, second;
        ok &= check(parse_user_viewpoint("-122.4,36.6,50,-122.4,36.6,0", &first, &error) &&
                        parse_user_viewpoint("-122.39,36.61,10,1,0,0,-1.5708,-122.39,36.61,0", &second, &error),
                    "two viewpoints parse");
        options.user_viewpoints = {first, second};
        const std::string page = write_page(options);
        ok &= check(contains(page, "<GeoViewpoint DEF='USER_GEOVIEW_1' id='USER_GEOVIEW_1' description='User GeoViewpoint 1' position='36.6000000 -122.4000000 50.000'") &&
                        contains(page, "centerOfRotation='36.6000000 -122.4000000 0.000'") &&
                        contains(page, "<GeoViewpoint DEF='USER_GEOVIEW_2' id='USER_GEOVIEW_2' description='User GeoViewpoint 2' position='36.6100000 -122.3900000 10.000' orientation='1.00000000 0.00000000 0.00000000 -1.57080000'"),
                    "each GeoViewpoint is written readably, latitude first as X3D takes it, with its orientation and center");
        ok &= check(page.find("<GeoViewpoint DEF='OVERVIEW_GEO'") < page.find("<GeoViewpoint DEF='USER_GEOVIEW_1'") &&
                        page.find("<GeoViewpoint DEF='USER_GEOVIEW_1'") < page.find("<GeoViewpoint DEF='USER_GEOVIEW_2'"),
                    "the user GeoViewpoints follow the overview viewpoints, in order");
        // The GeoOrigin is defined once, in the first GeoViewpoint that needs it, and used again.
        ok &= check(contains(page, "<GeoOrigin DEF='GRID_ORIGIN'") && page.find("<GeoOrigin DEF='GRID_ORIGIN'") < page.find("<GeoOrigin USE='GRID_ORIGIN'>") &&
                        page.find("<GeoOrigin DEF='GRID_ORIGIN'", page.find("<GeoOrigin DEF='GRID_ORIGIN'") + 1) == std::string::npos,
                    "the GeoOrigin is defined once and used by the user GeoViewpoints");
        ok &= check(contains(page, "<option value='USER_GEOVIEW_1'>") && contains(page, "<option value='USER_GEOVIEW_2'>"), "the menu has each user GeoViewpoint");
        ok &= check(contains(page, "onclick='copyGeoViewpoint()'") && contains(page, "function copyGeoViewpoint()") && contains(page, "'--geoviewpoint='") &&
                        contains(page, "var modelOrigin = ["),
                    "the page has the Copy GeoViewpoint button, its script and the GeoOrigin's ECEF position");
        ok &= check(contains(page, "<div id='copy-status'") && contains(page, "function showCopyStatus(message)") &&
                        contains(page, "option was copied to the clipboard") && contains(page, "The clipboard is not available here"),
                    "the page tells the user that the option was copied, or that the clipboard could not be used");

        // Without a GeoOrigin there is nothing to use.
        X3DomWriterOptions absolute = options;
        absolute.use_geoorigin = false;
        const std::string absolute_page = write_page(absolute);
        ok &= check(contains(absolute_page, "USER_GEOVIEW_1") && !contains(absolute_page, "GRID_ORIGIN"), "without a GeoOrigin the user GeoViewpoints have none");

        X3DomWriterOptions without = options;
        without.user_viewpoints.clear();
        ok &= check(!contains(write_page(without), "USER_GEOVIEW"), "no user GeoViewpoints, none in the page");

        // A lifted tileset (liftMeters in its tileset.json): the other models are in GeoTransforms that translate up (Y in X3DOM's
        // frame), the tileset is not, as it is lifted by its own file, and the heights of the viewpoints, the user's too, have the lift added.
        X3DomWriterOptions lifted = options;
        X3DomModel lifted_tileset = model("mbmesh_output/tileset/tileset.json", "tileset", "mbmesh_output", true);
        lifted_tileset.is_tileset = true;
        lifted_tileset.lift_meters = 1000.0;
        lifted_tileset.max_screen_space_error = 1.0;
        lifted.models.push_back(lifted_tileset);
        const std::string lifted_page = write_page(lifted);
        ok &= check(contains(lifted_page, "<GeoTransform id='transform_0'") && contains(lifted_page, "translation='0 1000.000 0'") &&
                        contains(lifted_page, "</GeoTransform>") && !contains(lifted_page, "<transform id='transform_0'>"),
                    "a page with a lifted tileset puts each other model in a GeoTransform with a translation up");
        ok &= check(contains(lifted_page, "<transform id='transform_1'>") && !contains(lifted_page, "<GeoTransform id='transform_1'") &&
                        contains(lifted_page, "<GeoTileset id='tileset_1' maximumScreenSpaceError='1' url="),
                    "the tileset is not in a GeoTransform, and has its maximumScreenSpaceError");
        ok &= check(contains(lifted_page, "position='36.6000000 -122.4000000 1050.000'") &&
                        contains(lifted_page, "centerOfRotation='36.6000000 -122.4000000 1000.000'"),
                    "the user GeoViewpoint heights have the lift added");
        ok &= check(contains(lifted_page, "var liftMeters = 1000.000;") && contains(lifted_page, "position[2] -= liftMeters;"),
                    "Copy GeoViewpoint takes the lift off");
        ok &= check(contains(write_page(options), "var liftMeters = 0.000;") && !contains(write_page(options), "<GeoTransform"),
                    "without a lift the page has plain transforms");
        // The scale object: axes at the center of rotation that the page resizes whenever a viewpoint changes, with a check box.
        ok &= check(contains(lifted_page, "<matrixTransform id='scale_axes_matrix'") && contains(lifted_page, "<text id='scale_axes_length'") &&
                        contains(lifted_page, "id='scale-axes-checkbox' type='checkbox' checked onclick='setScaleAxes(this.checked);'"),
                    "the page has the scale object and its check box");
        ok &= check(contains(lifted_page, "id='OVERVIEW_GEO'") && contains(lifted_page, "onviewpointChanged='updateScaleAxes()'") && contains(lifted_page, "function updateScaleAxes"),
                    "each viewpoint calls updateScaleAxes when it changes");
        X3DomWriterOptions without_axes = lifted;
        without_axes.scale_axes = false;
        ok &= check(!contains(write_page(without_axes), "<matrixTransform id='scale_axes_matrix'") && !contains(write_page(without_axes), "scale-axes-checkbox") &&
                        !contains(write_page(without_axes), "onviewpointChanged"),
                    "without scale_axes there is no scale object, no check box and no viewpoint hook");
        ok &= check(!contains(lifted_page, "id='color-checkbox-") && contains(lifted_page, "function setTilesetColors"), "a tileset without vertex colors has no color checkbox");
        lifted_tileset.height_colors = true;
        X3DomWriterOptions colored = lifted;
        colored.models.back() = lifted_tileset;
        ok &= check(contains(write_page(colored), "id='color-checkbox-1' type='checkbox' checked onclick='setTilesetColors(1, this.checked);'"), "a tileset with vertex colors has a checkbox that turns them off");

        // --label-chunks: the outline of each chunk and its name, with a check box, lifted with the models.
        X3DomWriterOptions labeled = options;
        for (int n = 0; n < 2; n++) {
            X3DomChunkLabel label;
            label.name = n == 0 ? "chunk_2_-10.glb" : "chunk_3_-10.glb";
            for (int corner = 0; corner < 4; corner++) {
                label.corners[corner][0] = 3.0 * (n + (corner == 1 || corner == 2 ? 1 : 0));
                label.corners[corner][1] = corner >= 2 ? 3.0 : 0.0;
                label.corners[corner][2] = -20.0;
            }
            label.center[0] = 3.0 * n + 1.5;
            label.center[1] = 1.5;
            label.center[2] = -20.0;
            label.text_size = 0.3;
            if (n == 0) {
                label.rise = 1.25;   // over the highest point of the chunk: a GeoTransform raises this one
                label.latitude = 36.3928;
                label.longitude = -122.304;
            }
            labeled.chunk_labels.push_back(label);
        }
        const std::string labeled_page = write_page(labeled);
        ok &= check(contains(labeled_page, "&quot;chunk_2_-10.glb&quot;") && contains(labeled_page, "&quot;chunk_3_-10.glb&quot;") &&
                        contains(labeled_page, "<billboard") && contains(labeled_page, "size='0.300'"),
                    "each chunk has its name as text that faces the viewer");
        ok &= check(contains(labeled_page, "coordIndex='0 1 2 3 0 -1 4 5 6 7 4 -1'") && contains(labeled_page, "<coordinate point='0.000 0.000 -20.000, 3.000 0.000 -20.000, "),
                    "each chunk has a closed outline of four corners");
        ok &= check(contains(labeled_page, "id='switch_chunks'") && contains(labeled_page, "Outlines and names") && contains(labeled_page, "<transform id='transform_chunks'>"),
                    "the chunks have a switch and a check box");
        ok &= check(contains(labeled_page, "<GeoTransform geoSystem='&quot;GD&quot; &quot;WE&quot;' geoCenter='36.3928000 -122.3040000 0' translation='0 1.250 0'>"),
                    "a label with a rise is in a GeoTransform that raises it along the local vertical");
        ok &= check(labeled_page.find("<GeoTransform") != std::string::npos && labeled_page.find("<GeoTransform", labeled_page.find("<GeoTransform") + 1) == std::string::npos,
                    "a label with no rise has no GeoTransform of its own");
        ok &= check(!contains(write_page(options), "switch_chunks") && !contains(write_page(options), "Outlines and names"),
                    "without chunk labels the page has neither");
        labeled.models.push_back(lifted_tileset);
        ok &= check(contains(write_page(labeled), "<GeoTransform id='transform_chunks'") && !contains(write_page(labeled), "<transform id='transform_chunks'>"),
                    "the chunk outlines are lifted with the tileset");
    }

    // ---- a tileset: a GeoTileset in the page, with the GeoOrigin, and the script that has it
    {
        X3DomWriterOptions options;
        options.title = "survey";
        options.use_geoorigin = true;
        options.geo = geo;
        options.x3dom_url = "http://127.0.0.1:8000/x3dom-full.js";
        X3DomModel tileset = model("mbmesh_output/tileset/tileset.json", "tileset (OGC 3D Tiles)", "mbmesh_output", true);
        tileset.is_tileset = true;
        options.models = {tileset, model("mbmesh_output/track.glb", "track.glb", "mbmesh_output", true)};
        const std::string page = write_page(options);
        ok &= check(contains(page, "<GeoTileset id='tileset_0' url='&quot;mbmesh_output/tileset/tileset.json&quot;'>") && contains(page, "</GeoTileset>"),
                    "the tileset is a GeoTileset whose url is an MFString");
        ok &= check(contains(page, "<GeoOrigin USE='GRID_ORIGIN'></GeoOrigin>\n            </GeoTileset>") ||
                        page.find("<GeoOrigin USE='GRID_ORIGIN'>", page.find("<GeoTileset")) < page.find("</GeoTileset>"),
                    "the GeoTileset has the GeoOrigin as its child node");
        ok &= check(contains(page, "<script type='text/javascript' src='http://127.0.0.1:8000/x3dom-full.js'></script>") &&
                        contains(page, "href='http://127.0.0.1:8000/x3dom.css'") && !contains(page, "www.x3dom.org/x3dom/dist"),
                    "the page loads the X3DOM script it was given, and the stylesheet beside it");
        ok &= check(contains(page, "<div id='geotileset-warning'") && contains(page, "x3dom.nodeTypes.GeoTileset") &&
                        contains(page, "The X3DOM script of this page, http://127.0.0.1:8000/x3dom-full.js, has no GeoTileset node"),
                    "a page with a tileset says so when the X3DOM script has no GeoTileset node");
        ok &= check(!contains(page, "<Inline id='inline_0'") && contains(page, "<Inline id='inline_1'"), "a tileset is not an Inline");
        ok &= check(contains(page, "{ uri: 'mbmesh_output/tileset/tileset.json', loaded: true }"), "a tileset needs no lazy loading");

        // Without a GeoOrigin the GeoTileset has none.
        X3DomWriterOptions absolute = options;
        absolute.use_geoorigin = false;
        const std::string absolute_page = write_page(absolute);
        X3DomWriterOptions no_tileset = options;
        no_tileset.models = {model("mbmesh_output/track.glb", "track.glb", "mbmesh_output", true)};
        ok &= check(!contains(write_page(no_tileset), "geotileset-warning"), "a page without a tileset has no such warning");
        ok &= check(contains(absolute_page, "<GeoTileset id='tileset_0'") && !contains(absolute_page, "GRID_ORIGIN"), "without a GeoOrigin the GeoTileset has none");

        // The default script.
        X3DomWriterOptions plain;
        plain.title = "survey";
        plain.geo = geo;
        plain.models = {model("mbmesh_output/mesh.glb", "mesh.glb", "mbmesh_output", true)};
        ok &= check(contains(write_page(plain), "src='https://www.x3dom.org/x3dom/dist/x3dom-full.js'") &&
                        contains(write_page(plain), "href='https://www.x3dom.org/x3dom/dist/x3dom.css'"),
                    "the default script and stylesheet are the ones on x3dom.org");
    }

    // ---- which models a run offers: a tileset takes the place of mesh.glb
    {
        const std::filesystem::path root = std::filesystem::temp_directory_path() / "mbmesh_test_models";
        std::filesystem::remove_all(root);
        touch(root / "mbmesh_output" / "mesh.glb");
        touch(root / "mbmesh_output" / "track.glb");
        const std::vector<X3DomModel> with = find_viewer_models(root, root / "mbmesh_output", true);
        const std::vector<X3DomModel> without = find_viewer_models(root, root / "mbmesh_output", false);
        ok &= check(with.size() == 3 && with[0].is_tileset && with[0].visible && with[0].uri == "mbmesh_output/tileset/tileset.json",
                    "a run with a tileset lists it first, shown");
        ok &= check(with[1].label == "mesh.glb" && !with[1].visible && with[2].label == "track.glb" && with[2].visible,
                    "mesh.glb is then not shown, since the tileset is the same surface, and the track is");
        ok &= check(without.size() == 2 && without[0].label == "mesh.glb" && without[0].visible && !without[0].is_tileset, "without a tileset, mesh.glb is shown");
        std::filesystem::remove_all(root);
    }

    // ---- a check box for every model; models that start switched off load when first wanted
    {
        X3DomWriterOptions options;
        options.title = "survey";
        options.geo = geo;
        options.models = {
            model("mbmesh_output/mesh.glb", "mesh.glb", "mbmesh_output", true),
            model("mbmesh_output/track.glb", "track.glb", "mbmesh_output", true),
            model("mbmesh_output/pointcloud.glb", "pointcloud.glb", "mbmesh_output", false),
            model("ZTopo.glb", "ZTopo.glb", "Other GLBs", false),
        };
        options.models[3].group_note = "Only if they use the same GeoOrigin & <so>.";
        const std::string page = write_page(options);

        ok &= check(contains(page, "id='model-checkbox-0' type='checkbox' checked onclick='toggleModel(0, this.checked);'") &&
                        contains(page, "id='model-checkbox-1' type='checkbox' checked onclick='toggleModel(1, this.checked);'"),
                    "mesh and track start checked");
        ok &= check(contains(page, "id='model-checkbox-2' type='checkbox' onclick='toggleModel(2, this.checked);'") &&
                        contains(page, "id='model-checkbox-3' type='checkbox' onclick='toggleModel(3, this.checked);'"),
                    "the other models start unchecked");
        ok &= check(contains(page, "<b>mbmesh_output</b>") && contains(page, "<b>Other GLBs</b>"), "models are listed under a heading for each group");
        ok &= check(page.find("<b>mbmesh_output</b>") == page.rfind("<b>mbmesh_output</b>"), "a group heading is written once");
        ok &= check(contains(page, "font-size:smaller;'>Only if they use the same GeoOrigin &amp; &lt;so&gt;.</td>"),
                    "a group note is written, escaped, under its heading");

        ok &= check(contains(page, "<switch id='switch_0' whichChoice='0'>") && contains(page, "<switch id='switch_2' whichChoice='-1'>"),
                    "each model has a switch, on or off at the start");
        ok &= check(contains(page, "<Inline id='inline_0' url='mbmesh_output/mesh.glb' nameSpaceName='model_0'") &&
                        contains(page, "<Inline id='inline_1' url='mbmesh_output/track.glb' nameSpaceName='model_1'"),
                    "models that start visible are loaded with their own namespace");
        ok &= check(!contains(page, "<Inline id='inline_2'") && !contains(page, "<Inline id='inline_3'"),
                    "models that start switched off are not loaded until they are wanted");
        ok &= check(contains(page, "{ uri: 'mbmesh_output/pointcloud.glb', loaded: false }") &&
                        contains(page, "{ uri: 'mbmesh_output/mesh.glb', loaded: true }") && contains(page, "function toggleModel(index, on)"),
                    "the page knows each model's URL and whether it is loaded");
    }

    // ---- with a GeoOrigin
    {
        X3DomWriterOptions options;
        options.use_geoorigin = true;
        options.geo = geo;
        options.models = {model("mbmesh_output/mesh.glb", "mesh.glb", "mbmesh_output", true)};
        const std::string page = write_page(options);

        ok &= check(contains(page, "<GeoOrigin DEF='GRID_ORIGIN' geoSystem='\"GD\" \"WE\"' geoCoords='36.600000 -122.400000 -10.00'>"),
                    "the GeoOrigin is declared, latitude first, as mbgrd2gltf does");
        ok &= check(!contains(page, "geoOrigin='GRID_ORIGIN'"), "the viewpoint does not name the GeoOrigin in an attribute, which X3DOM ignores");
    }

    // ---- log messages and labels are escaped, and the page says when some were left out
    {
        X3DomWriterOptions options;
        options.geo = geo;
        options.models = {model("a.glb", "<b>label</b> & more", "g", true)};
        options.log_messages = {"a < b & c > d \"q\" 'x' <script>alert(1)</script>"};
        options.log_messages_omitted = 7;
        const std::string page = write_page(options);

        ok &= check(contains(page, "a &lt; b &amp; c &gt; d &quot;q&quot; &#39;x&#39; &lt;script&gt;alert(1)&lt;/script&gt;"),
                    "log messages are escaped");
        ok &= check(!contains(page, "<script>alert(1)") && !contains(page, "<b>label</b>"), "nothing from a message or a label can add markup to the page");
        ok &= check(contains(page, "Processing Log (8 messages)") && contains(page, "7 further messages are not shown"),
                    "the page counts and mentions the messages that were left out");
    }

    // ---- the log in a file of its own, loaded by the page
    {
        X3DomWriterOptions options;
        options.geo = geo;
        options.models = {model("a.glb", "a.glb", "g", true)};
        options.log_messages = {"DEBUG should not be in the page"};
        options.log_url = viewer_log_url("/x/my run/mbmesh_output");
        const std::string page = write_page(options);
        ok &= check(viewer_log_url("/x/my run") == "my%20run/mbmesh.log", "the log URL is relative to the page beside the directory, encoded");
        ok &= check(!contains(page, "should not be in the page") && !contains(page, "class='log-line'>"), "the page does not hold the messages");
        ok &= check(contains(page, "var logUrl = 'mbmesh_output/mbmesh.log'") || contains(page, "var logUrl = 'my%20run/mbmesh.log'"), "the page names the log file");
        ok &= check(contains(page, "function loadLog()") && contains(page, "fetch(logUrl)"), "the page loads it");

        const std::filesystem::path log = std::filesystem::temp_directory_path() / "mbmesh_test.log";
        std::string error;
        ok &= check(write_log_file(log, {"INFO a", "DEBUG b < c"}, 3, &error), "the log file is written");
        std::ifstream input(log);
        const std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        std::filesystem::remove(log);
        ok &= check(text == "INFO a\nDEBUG b < c\n... 3 further messages are not shown\n", "one message a line, unescaped, then the omitted count");
    }

    // ---- errors
    {
        X3DomWriterOptions one;
        one.models = {model("a.glb", "a.glb", "g", true)};
        std::string error;
        ok &= check(!write_glb_x3dom_file("", one, &error) && !error.empty(), "an empty path is an error");
        ok &= check(!write_glb_x3dom_file(std::filesystem::temp_directory_path() / "x.html", X3DomWriterOptions(), &error) && !error.empty(),
                    "a page with no models is an error");
        ok &= check(!write_glb_x3dom_file("/nonexistent_directory/page.html", one, &error) && !error.empty(), "an unwritable path is an error");
    }

    // ---- where the page goes, and what it offers
    {
        const std::filesystem::path base = std::filesystem::temp_directory_path() / "mbmesh_test_viewer";
        std::filesystem::remove_all(base);
        const std::filesystem::path survey = base / "OceanImaging2018" / "20180923d1" / "multibeam";
        const std::filesystem::path output = survey / "mbmesh_output";
        touch(output / "mesh.glb");
        touch(output / "track.glb");
        touch(output / "pointcloud.glb");
        touch(output / "mesh.html");                     // not a GLB
        touch(output / "chunks" / "chunk_0_0.glb");        // repeats mesh.glb, so not listed
        touch(survey / "ZTopo.glb");
        touch(survey / "my file.glb");
        touch(survey / "notes.txt");

        ok &= check(viewer_web_root(output) == survey, "the web root is the parent of the output directory");
        ok &= check(viewer_page_path(output) == survey / "mbmesh_output.html", "the page is named after the output directory");
        ok &= check(viewer_web_root(output.string() + "/") == survey && viewer_page_path(output.string() + "/") == survey / "mbmesh_output.html",
                    "a trailing slash on the output directory makes no difference");
        ok &= check(viewer_title(survey) == "OceanImaging2018/20180923d1/multibeam", "the title is the last three directories of the web root");
        ok &= check(viewer_title("/a/b") == "a/b" && viewer_title("/a") == "a", "a shorter path gives a shorter title");

        const std::vector<X3DomModel> models = find_viewer_models(survey, output);
        std::vector<std::string> uris;
        for (const X3DomModel &m : models) {
            uris.push_back(m.uri);
        }
        const std::vector<std::string> expected = {"mbmesh_output/mesh.glb", "mbmesh_output/track.glb", "mbmesh_output/pointcloud.glb",
                                                   "ZTopo.glb", "my%20file.glb"};
        ok &= check(uris == expected, "the models are mesh, track, the rest of the output directory, then the GLBs beside it, with spaces encoded");
        ok &= check(models.size() == 5 && models[0].visible && models[1].visible && !models[2].visible && !models[3].visible && !models[4].visible,
                    "only mesh and track start visible");
        ok &= check(models.size() == 5 && models[0].group == "mbmesh_output" && models[3].group != "mbmesh_output" && !models[3].group_note.empty() && models[3].label == "ZTopo.glb" &&
                        models[4].label == "my file.glb",
                    "the groups and labels tell the output directory's GLBs from the others");

        // Without mesh.glb nothing starts visible; no GLBs beside it if the output is not in the web root's top level
        std::filesystem::remove(output / "mesh.glb");
        std::filesystem::remove(output / "track.glb");
        const std::vector<X3DomModel> without_mesh = find_viewer_models(survey, output);
        ok &= check(!without_mesh.empty() && without_mesh[0].uri == "mbmesh_output/pointcloud.glb" && !without_mesh[0].visible,
                    "only models that exist are offered");

        std::filesystem::remove_all(base);
    }

    // ---- the page inside the output directory, and the survey page with its accordions
    {
        const std::filesystem::path base = std::filesystem::temp_directory_path() / "mbmesh_test_survey";
        std::filesystem::remove_all(base);
        const std::filesystem::path survey = base / "20260101A";
        touch(survey / "multibeam" / "ZTopo.glb");
        touch(survey / "multibeam" / "mbmesh_output" / "mesh.glb");
        touch(survey / "multibeam" / "mbmesh_output" / "track.glb");
        touch(survey / "multibeam" / "mbmesh_output" / "tileset" / "tileset.json");
        touch(survey / "multibeam" / "mbmesh_output" / "tileset" / "L0" / "tile_0_0.glb");
        touch(survey / "multibeam" / "mbmesh_output" / "chunks" / "t.glb");
        // A tileset directory can have any name, as the default of mbglb2tiles used to be mbglb2tiles_output; its tiles are in directories of levels
        touch(survey / "multibeam" / "mbmesh_output" / "mbglb2tiles_output" / "tileset.json");
        touch(survey / "multibeam" / "mbmesh_output" / "mbglb2tiles_output" / "L0" / "tile_0_0.glb");
        touch(survey / "multibeam" / "mbmesh_output" / "mbglb2tiles_output" / "L1" / "tile_0_0.glb");
        touch(survey / "lidar" / "points.glb");
        touch(survey / "lidar" / "a-x" / "q.glb");
        touch(survey / "lidar" / "a" / "p.glb");
        touch(survey / "lidar" / ".hidden" / "h.glb");
        // The logs: mbmesh.log in the output directory, mbglb2tiles.log in its tileset directory, and a log beside a GLB of mbgrd2gltf
        touch(survey / "multibeam" / "mbmesh_output" / "mbmesh.log");
        touch(survey / "multibeam" / "mbmesh_output" / "mbglb2tiles_output" / "mbglb2tiles.log");
        touch(survey / "multibeam" / "ZTopo.log");

        // The page in the output directory: relative to that directory, and no GLBs from beside it
        const std::filesystem::path output = survey / "multibeam" / "mbmesh_output";
        const std::vector<X3DomModel> inner = find_viewer_models(survey / "multibeam", output, true, true);
        ok &= check(inner.size() == 3 && inner[0].uri == "tileset/tileset.json" && inner[1].uri == "mesh.glb" && inner[2].uri == "track.glb",
                    "the page inside the output directory refers to its files by their names there");
        ok &= check(viewer_inner_page_path(output) == output / "mbmesh.html", "that page is mbmesh.html in the output directory");
        ok &= check(find_viewer_models(survey / "multibeam", output, false, false).size() == 3 &&
                        find_viewer_models(survey / "multibeam", output, false, false)[2].label == "ZTopo.glb",
                    "the page beside it still offers the GLBs of its directory");

        // The survey: every GLB beneath it, in directory order, relative to the page's directory
        bool use_geoorigin = false;
        X3DomGeoOrigin survey_geo;
        const std::vector<X3DomModel> models = find_survey_models(survey, base, &use_geoorigin, &survey_geo);
        std::vector<std::string> uris;
        for (const X3DomModel &m : models) {
            uris.push_back(m.uri);
        }
        const std::vector<std::string> expected = {
            "20260101A/lidar/points.glb",
            "20260101A/lidar/a/p.glb",
            "20260101A/lidar/a-x/q.glb",
            "20260101A/multibeam/ZTopo.glb",
            "20260101A/multibeam/mbmesh_output/mbglb2tiles_output/tileset.json",
            "20260101A/multibeam/mbmesh_output/tileset/tileset.json",
            "20260101A/multibeam/mbmesh_output/mesh.glb",
            "20260101A/multibeam/mbmesh_output/track.glb"};
        ok &= check(uris == expected, "the survey lists every GLB beneath it, a directory's files before its subdirectories, without chunks or hidden directories");
        ok &= check(models.size() == 8 && models[4].is_tileset && models[4].group == "20260101A/multibeam/mbmesh_output" &&
                        models[0].group == "20260101A/lidar" && models[2].group == "20260101A/lidar/a-x",
                    "the groups are the directory paths, from the survey directory");
        ok &= check(models[4].label == "mbglb2tiles_output (OGC 3D Tiles)" && models[5].is_tileset && models[5].label == "tileset (OGC 3D Tiles)",
                    "a tileset directory of any name is one tileset, labeled by the name of the directory, and its tiles are not listed");
        ok &= check(models[3].log_label == "mbgrd2gltf log" && models[3].log_uri == "20260101A/multibeam/ZTopo.log" &&
                        models[4].log_label == "mbglb2tiles log" && models[4].log_uri == "20260101A/multibeam/mbmesh_output/mbglb2tiles_output/mbglb2tiles.log" &&
                        models[5].log_label == "mbmesh log" && models[5].log_uri == "20260101A/multibeam/mbmesh_output/mbmesh.log" &&
                        models[6].log_label == "mbmesh log" && models[0].log_uri.empty(),
                    "each model has a link to the log of the program that made it, named for the program, and none without a log file");
        ok &= check(!use_geoorigin, "GLBs without a georeference give no GeoOrigin");

        // A GLB with the georeference gives the GeoOrigin and the viewpoint
        {
            const std::string json =
                "{\"asset\":{\"version\":\"2.0\",\"extras\":{\"georeference\":{\"vertexCoordinates\":\"relativeToCenter\","
                "\"center\":[-2700000.5,-4300000.25,3800000],\"geodeticCenter\":{\"crs\":\"EPSG:4979\",\"longitude\":-122.3,"
                "\"latitude\":36.4,\"height\":-900},\"extent\":{\"west\":-122.31,\"south\":36.39,\"east\":-122.29,\"north\":36.41}}}}}   ";
            std::ofstream glb(output / "mesh.glb", std::ios::binary);
            const std::uint32_t length = static_cast<std::uint32_t>(json.size());
            const std::uint32_t total = 28 + length;
            const std::uint32_t version = 2;
            const std::uint32_t json_type = 0x4E4F534A;
            const std::uint32_t magic = 0x46546C67;
            glb.write(reinterpret_cast<const char *>(&magic), 4);
            glb.write(reinterpret_cast<const char *>(&version), 4);
            glb.write(reinterpret_cast<const char *>(&total), 4);
            glb.write(reinterpret_cast<const char *>(&length), 4);
            glb.write(reinterpret_cast<const char *>(&json_type), 4);
            glb.write(json.data(), static_cast<std::streamsize>(json.size()));
        }
        use_geoorigin = false;
        static_cast<void>(find_survey_models(survey, base, &use_geoorigin, &survey_geo));
        ok &= check(use_geoorigin && std::fabs(survey_geo.origin_lon + 122.3) < 1e-12 && std::fabs(survey_geo.origin_lat - 36.4) < 1e-12 &&
                        std::fabs(survey_geo.origin_elevation + 900.0) < 1e-9 && survey_geo.has_local_view && survey_geo.view_altitude > 1000.0,
                    "the first mesh.glb with a georeference gives the GeoOrigin and an overview above the data");

        // The view takes in every georeferenced GLB; the GeoOrigin is still that of the first with relative vertices
        {
            const std::string json =
                "{\"asset\":{\"version\":\"2.0\",\"extras\":{\"georeference\":{\"vertexCoordinates\":\"absolute\","
                "\"extent\":{\"west\":-122.21,\"south\":36.39,\"east\":-122.19,\"north\":36.41}}}}}   ";
            std::ofstream glb(survey / "lidar" / "points.glb", std::ios::binary);
            const std::uint32_t length = static_cast<std::uint32_t>(json.size());
            const std::uint32_t words[5] = {0x46546C67, 2, 28 + length, length, 0x4E4F534A};
            glb.write(reinterpret_cast<const char *>(words), sizeof(words));
            glb.write(json.data(), static_cast<std::streamsize>(json.size()));
            glb.close();
            bool wide_origin = false;
            X3DomGeoOrigin wide;
            std::vector<std::string> mismatches;
            static_cast<void>(find_survey_models(survey, base, &wide_origin, &wide, &mismatches));
            ok &= check(mismatches.size() == 1 && contains(mismatches[0], "lidar/points.glb") && contains(mismatches[0], "absolute vertices"),
                        "a GLB with absolute vertices does not agree with a GeoOrigin");

            // A GLB relative to another GeoOrigin, a second away in longitude, is reported and named
            const std::string other =
                "{\"asset\":{\"version\":\"2.0\",\"extras\":{\"georeference\":{\"vertexCoordinates\":\"relativeToCenter\","
                "\"center\":[-2700100.5,-4300000.25,3800000],\"geodeticCenter\":{\"crs\":\"EPSG:4979\",\"longitude\":-122.3012,"
                "\"latitude\":36.4,\"height\":-900},\"extent\":{\"west\":-122.31,\"south\":36.39,\"east\":-122.29,\"north\":36.41}}}}}   ";
            std::ofstream second(survey / "multibeam" / "ZTopo.glb", std::ios::binary);
            const std::uint32_t other_length = static_cast<std::uint32_t>(other.size());
            const std::uint32_t other_words[5] = {0x46546C67, 2, 28 + other_length, other_length, 0x4E4F534A};
            second.write(reinterpret_cast<const char *>(other_words), sizeof(other_words));
            second.write(other.data(), static_cast<std::streamsize>(other.size()));
            second.close();
            bool scratch_origin = false;
            X3DomGeoOrigin scratch_geo;
            mismatches.clear();
            static_cast<void>(find_survey_models(survey, base, &scratch_origin, &scratch_geo, &mismatches));
            ok &= check(mismatches.size() == 2 && contains(mismatches[1], "multibeam/ZTopo.glb") && contains(mismatches[1], "mbmesh_output/mesh.glb") &&
                            contains(mismatches[1], "100.0 m apart"),
                        "a GLB with another GeoOrigin is reported with both names and the distance");
            std::filesystem::remove(survey / "multibeam" / "ZTopo.glb");
            std::filesystem::remove(survey / "lidar" / "points.glb");
            mismatches.clear();
            touch(survey / "lidar" / "points.glb");
            static_cast<void>(find_survey_models(survey, base, &scratch_origin, &scratch_geo, &mismatches));
            ok &= check(mismatches.empty(), "GLBs that agree, and those with no georeference, report nothing");
            ok &= check(wide_origin && std::fabs(wide.origin_lon + 122.3) < 1e-12 && std::fabs(wide.center_lon + 122.25) < 1e-9 &&
                            std::fabs(wide.center_lat - 36.4) < 1e-9 && wide.view_altitude > survey_geo.view_altitude * 4.0 && wide.has_local_view,
                        "the overview is centered on, and as high as to see, all the georeferenced GLBs");
        }

        // The chunks of the runs beneath the survey, for --label-chunks: only the chunk files that say where their chunk is
        {
            const std::filesystem::path chunks = survey / "multibeam" / "mbmesh_output" / "chunks";
            const auto write_glb = [](const std::filesystem::path &path, const std::string &json_text) {
                const std::string json = json_text + std::string((4 - json_text.size() % 4) % 4, ' ');
                std::ofstream glb(path, std::ios::binary);
                const std::uint32_t length = static_cast<std::uint32_t>(json.size());
                const std::uint32_t words[5] = {0x46546C67, 2, 28 + length, length, 0x4E4F534A};
                glb.write(reinterpret_cast<const char *>(words), sizeof(words));
                glb.write(json.data(), static_cast<std::streamsize>(json.size()));
            };
            write_glb(chunks / "chunk_2_-10.glb",
                      "{\"asset\":{\"extras\":{\"MB-System\":{\"content\":{\"role\":\"chunk\",\"chunk\":[2,-10],\"chunkLabel\":{"
                      "\"corners\":[[-122.3,36.4,-900],[-122.2999,36.4,-900],[-122.2999,36.4001,-900],[-122.3,36.4001,-900]],"
                      "\"center\":[-122.29995,36.40005,-900],\"rise\":1.5,\"textSize\":0.3}}}}}}");
            write_glb(chunks / "chunk_3_-10.glb", "{\"asset\":{\"extras\":{\"MB-System\":{\"content\":{\"role\":\"chunk\",\"chunk\":[3,-10]}}}}}");
            std::size_t without = 0;
            const std::vector<X3DomChunkLabel> labels = find_survey_chunk_labels(survey, true, survey_geo, &without);
            ok &= check(labels.size() == 1 && labels[0].name == "chunk_2_-10.glb" && without == 1, "only the chunk files that say where they are are labeled, and the others are counted");
            if (labels.size() == 1) {
                // The first corner is at the GeoOrigin of the survey (-122.3, 36.4, -900), so it is at the origin of the models
                ok &= check(std::fabs(labels[0].corners[0][0]) < 1e-6 && std::fabs(labels[0].corners[0][1]) < 1e-6 && std::fabs(labels[0].corners[0][2]) < 1e-6,
                            "the corner at the GeoOrigin is at the origin of the model coordinates");
                const double east = std::hypot(labels[0].corners[1][0] - labels[0].corners[0][0], labels[0].corners[1][1] - labels[0].corners[0][1]);
                ok &= check(east > 7.0 && east < 12.0 && std::fabs(labels[0].rise - 1.5) < 1e-9 && std::fabs(labels[0].text_size - 0.3) < 1e-9 &&
                                std::fabs(labels[0].latitude - 36.40005) < 1e-9 && std::fabs(labels[0].longitude + 122.29995) < 1e-9,
                            "the corners are 0.0001 degrees of longitude apart, about 9 m, with the rise, text size and position of the label");
            }
            std::filesystem::remove(chunks / "chunk_2_-10.glb");
            std::filesystem::remove(chunks / "chunk_3_-10.glb");
        }

        // The accordions: one per directory, inside the one before, the survey directory open
        X3DomWriterOptions options;
        options.title = "20260101A";
        options.geo = survey_geo;
        options.nested_groups = true;
        options.models = models;
        const std::string page = write_page(options);
        const std::size_t survey_at = page.find("<summary><b>20260101A</b></summary>");
        const std::size_t lidar_at = page.find("<summary><b>lidar</b></summary>");
        const std::size_t multibeam_at = page.find("<summary><b>multibeam</b></summary>");
        const std::size_t output_at = page.find("<summary><b>mbmesh_output</b></summary>");
        ok &= check(survey_at != std::string::npos && survey_at < lidar_at && lidar_at < multibeam_at && multibeam_at < output_at,
                    "the page has an accordion for each directory, in order");
        ok &= check(contains(page, "<details class='group-accordion' open><summary><b>20260101A</b>") &&
                        contains(page, "<details class='group-accordion'><summary><b>lidar</b>"),
                    "the survey directory starts open and the directories inside it closed");
        std::size_t opened = 0, closed = 0;
        for (std::size_t at = page.find("<details class='group-accordion'"); at != std::string::npos; at = page.find("<details class='group-accordion'", at + 1)) {
            opened++;
        }
        for (std::size_t at = page.find("</div></details>"); at != std::string::npos; at = page.find("</div></details>", at + 1)) {
            closed++;
        }
        ok &= check(opened == 6 && closed == 6, "every accordion is closed again");
        ok &= check(contains(page, "id='model-checkbox-7' type='checkbox' checked onclick='toggleModel(7, this.checked);'") &&
                        contains(page, "id='model-checkbox-6' type='checkbox' onclick='toggleModel(6, this.checked);'") &&
                        contains(page, "{ uri: '20260101A/multibeam/mbmesh_output/track.glb', loaded: true }") &&
                        contains(page, "<Inline id='inline_7' url='20260101A/multibeam/mbmesh_output/track.glb'"),
                    "each model has its check box and its place in the model list");

        // User GeoViewpoints given for the survey page are in its menu even without a georeference
        {
            X3DomUserViewpoint user;
            std::string viewpoint_error;
            ok &= check(parse_user_viewpoint("-122.4,36.6,50,-122.39,36.61,10", &user, &viewpoint_error), "the test viewpoint parses");
            X3DomWriterOptions plain;
            plain.nested_groups = true;
            plain.models = models;
            plain.user_viewpoints = {user};
            const std::string plain_page = write_page(plain);
            ok &= check(contains(plain_page, "<option value='USER_GEOVIEW_1'>User GeoViewpoint 1") &&
                            !contains(plain_page, "<option value='OVERVIEW_LOCAL'>") && contains(plain_page, "<GeoViewpoint DEF='USER_GEOVIEW_1'"),
                        "the survey page offers its user GeoViewpoints in the menu, with no georeference");
        }

        std::filesystem::remove_all(base);
    }

    std::printf(ok ? "x3dom writer tests passed\n" : "x3dom writer tests FAILED\n");
    return ok ? 0 : 1;
}
