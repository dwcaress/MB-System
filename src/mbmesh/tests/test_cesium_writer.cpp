// The CesiumJS page of a tileset: its contents, the globe hidden at the start with a check box that shows it, the lift taken off again, the
// optional parts, and the names of the page.

#include "io/cesium_writer.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

bool check(bool condition, const std::string &message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message.c_str());
    }
    return condition;
}

bool has(const std::string &text, const std::string &part) {
    return text.find(part) != std::string::npos;
}

}  // namespace

int main() {
    bool ok = true;

    CesiumPageOptions options;
    options.title = "OceanImaging2018/20180925d1/multibeam";
    options.tileset_uri = "ZTopo_tileset/tileset.json";
    options.log_url = "ZTopo_tileset/mbglb2tiles.log";
    options.lift_meters = 857.0;
    options.max_screen_space_error = 1.0;
    options.height_colors = true;
    const std::string page = cesium_page_text(options);

    ok &= check(has(page, "<script src='https://cesium.com/downloads/cesiumjs/releases/1.114/Build/Cesium/Cesium.js'></script>") &&
                    has(page, "Build/Cesium/Widgets/widgets.css"),
                "the page loads CesiumJS from its web site by default");
    ok &= check(has(page, "var tilesetUri = 'ZTopo_tileset/tileset.json';") && has(page, "Cesium.Cesium3DTileset.fromUrl(tilesetUri"),
                "the page loads the tileset");
    ok &= check(has(page, "viewer.scene.globe.show = false;") && has(page, "<input id='globe-checkbox' type='checkbox' onclick='setGlobe(this.checked);'>") &&
                    has(page, "function setGlobe(on)"),
                "the globe is hidden at the start, and a check box shows it");
    ok &= check(has(page, "globe.translucency.enabled = true") && has(page, "enableCollisionDetection = false"),
                "the globe is translucent and the camera can go below it");
    ok &= check(has(page, "var liftMeters = 857;") && has(page, "Cartesian3.multiplyByScalar(up, -liftMeters"), "the lift of the tileset is taken off again");
    ok &= check(has(page, "var maximumScreenSpaceError = 1;"), "the maximumScreenSpaceError is the option's");
    ok &= check(has(page, "<input id='color-checkbox' type='checkbox' checked onclick='setColors(this.checked);'>"), "the colors have a check box");
    ok &= check(has(page, "tileset.customShader = on ? undefined : grayShader;") && !has(page, "Cesium3DTileColorBlendMode.REPLACE"),
                "gray comes from a custom shader, which keeps the shading that the REPLACE blend mode loses");
    ok &= check(has(page, "<a href='ZTopo_tileset/mbglb2tiles.log' target='_blank'>log</a>"), "the log is linked");

    ok &= check(has(page, "var userViewpoints = [];") && !has(page, "value='user0'") && has(page, "function copyViewpoint()") && has(page, "'--geoviewpoint='"),
                "without viewpoints of the user the menu has none, and the page can give the option for a view");
    CesiumPageOptions with_views = options;
    X3DomUserViewpoint down;   // straight down, 100 m above the point
    down.longitude = -122.304;
    down.latitude = 36.3928;
    down.elevation = -740.0;
    down.center_longitude = -122.304;
    down.center_latitude = 36.3928;
    down.center_elevation = -840.0;
    down.has_orientation = true;
    down.axis[0] = 1.0;
    down.axis[1] = 0.0;
    down.axis[2] = 0.0;
    down.angle = -1.5708;
    X3DomUserViewpoint oblique = down;   // from the position looking toward the center, with no orientation
    oblique.has_orientation = false;
    oblique.longitude = -122.3041;
    with_views.user_viewpoints = {down, oblique};
    const std::string views_page = cesium_page_text(with_views);
    ok &= check(has(views_page, "<option value='user0'>User viewpoint 1</option>") && has(views_page, "<option value='user1'>User viewpoint 2</option>") &&
                    has(views_page, "{ position: [-122.304, 36.3928, -740], axis: [1, 0, 0], angle: -1.5708 }") && has(views_page, "function setUserViewpoint("),
                "the viewpoints of the user are in the menu, with the camera and its orientation as an axis and an angle");
    ok &= check(has(page, "<select id='viewpoint-select'") && has(page, "value='down'") && has(page, "function setViewpoint(name, animate)") &&
                    has(page, "setViewpoint('down', false);"),
                "the page has the viewpoint menu, and opens looking down");
    ok &= check(has(page, "<input id='scale-axes-checkbox' type='checkbox' checked onclick='setScaleAxes(this.checked);'>") && has(page, "function updateScaleAxes(force)") &&
                    has(page, "scene.preRender.addEventListener(function () { updateScaleAxes(false); })") && has(page, "function roundLength(raw)"),
                "the page has the scale object and its check box");

    // The options change the page
    CesiumPageOptions plain = options;
    plain.scale_axes = false;
    const std::string without_axes = cesium_page_text(plain);
    ok &= check(!has(without_axes, "scale-axes") && !has(without_axes, "updateScaleAxes") && !has(without_axes, "PolylineCollection"), "scale_axes=false leaves the scale object out");
    options.cesium_url = "./cesium/";
    options.height_colors = false;
    options.log_url.clear();
    options.show_globe = true;
    options.lift_meters = 0.0;
    options.title = "a <b> & 'c'";
    const std::string other = cesium_page_text(options);
    ok &= check(has(other, "<script src='./cesium/Cesium.js'></script>") && has(other, "href='./cesium/Widgets/widgets.css'"),
                "a given CesiumJS directory is used, without a doubled slash");
    ok &= check(!has(other, "color-checkbox") && !has(other, ">log</a>"), "without colors and log the page has neither");
    ok &= check(has(other, "viewer.scene.globe.show = true;") && has(other, "id='globe-checkbox' type='checkbox' checked"), "the globe can be shown at the start");
    ok &= check(has(other, "a &lt;b&gt; &amp; &#39;c&#39;") && !has(other, "<b> &"), "the title is escaped");

    // The file and its name
    ok &= check(cesium_page_path("survey/multibeam/ZTopo_tileset") == std::filesystem::path("survey/multibeam/ZTopo_tileset_cesium.html") &&
                    cesium_page_path("survey/multibeam/ZTopo_tileset/") == std::filesystem::path("survey/multibeam/ZTopo_tileset_cesium.html"),
                "the page is beside the output directory, named after it");
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "mbmesh_test_cesium.html";
    std::string error;
    ok &= check(write_cesium_page(path, options, &error), "the page is written: " + error);
    std::ifstream in(path);
    const std::string written((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    ok &= check(written == other, "the file holds the text");
    std::filesystem::remove(path);
    ok &= check(!write_cesium_page("/nonexistent_directory/x.html", options, &error) && !error.empty(), "a page that cannot be written is an error");

    if (ok) {
        std::printf("ALL CHECKS PASSED\n");
    }
    return ok ? 0 : 1;
}
