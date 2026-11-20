// The CesiumJS page: its contents, the globe hidden at the start with a check box that shows it, tilesets with the lift taken off again and GLBs placed by the
// GeoOrigin, the viewpoints, the optional parts, and the names of the page.

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

CesiumModel tileset_model() {
    CesiumModel tileset;
    tileset.uri = "ZTopo_tileset/tileset.json";
    tileset.label = "ZTopo_tileset (OGC 3D Tiles)";
    tileset.visible = true;
    tileset.is_tileset = true;
    tileset.max_screen_space_error = 1.0;
    tileset.lift_meters = 857.0;
    tileset.height_colors = true;
    tileset.log_uri = "ZTopo_tileset/mbglb2tiles.log";
    tileset.log_label = "mbglb2tiles log";
    return tileset;
}

}  // namespace

int main() {
    bool ok = true;

    CesiumPageOptions options;
    options.title = "OceanImaging2018/20180925d1/multibeam";
    options.models = {tileset_model()};
    const std::string page = cesium_page_text(options);

    ok &= check(has(page, "<script src='https://cesium.com/downloads/cesiumjs/releases/1.114/Build/Cesium/Cesium.js'></script>") &&
                    has(page, "Build/Cesium/Widgets/widgets.css"),
                "the page loads CesiumJS from its web site by default");
    ok &= check(has(page, "{ uri: 'ZTopo_tileset/tileset.json', tileset: true, visible: true, lift: 857, sse: 1, shade: false, colors: true }") && has(page, "Cesium.Cesium3DTileset.fromUrl(model.uri"),
                "the page loads the tileset, with its lift and its screen-space error");
    ok &= check(has(page, "viewer.scene.globe.show = false;") && has(page, "<input id='globe-checkbox' type='checkbox' onclick='setGlobe(this.checked);'>") &&
                    has(page, "function setGlobe(on)"),
                "the globe is hidden at the start, and a check box shows it");
    ok &= check(has(page, "globe.translucency.enabled = true") && has(page, "enableCollisionDetection = false"),
                "the globe is translucent and the camera can go below it");
    ok &= check(has(page, "Cartesian3.multiplyByScalar(up, -model.lift"), "the lift of a tileset is taken off again");
    ok &= check(has(page, "<input id='model-checkbox-0' type='checkbox' checked onclick='setModel(0, this.checked);'> ZTopo_tileset (OGC 3D Tiles)") &&
                    has(page, "<a class='log' href='ZTopo_tileset/mbglb2tiles.log' target='_blank'>mbglb2tiles log</a>"),
                "the model has a check box, and a link to its log named for the program");
    ok &= check(has(page, "<input id='color-checkbox-0' type='checkbox' checked onclick='setModelColors(0, this.checked);'> color by height") &&
                    has(page, "tileset.customShader") == false && has(page, "model.primitive.customShader = on ? undefined : new Cesium.CustomShader("),
                "the colors of a tileset have a check box, and gray comes from a custom shader of the tileset, which keeps the shading that REPLACE loses");
    ok &= check(!has(page, "Cesium3DTileColorBlendMode.REPLACE"), "the REPLACE blend mode is not used");
    ok &= check(has(page, "var userViewpoints = [];") && !has(page, "value='user0'") && has(page, "function copyViewpoint()") && has(page, "'--geoviewpoint='"),
                "without viewpoints of the user the menu has none, and the page can give the option for a view");
    ok &= check(has(page, "<select id='viewpoint-select'") && has(page, "value='down'") && has(page, "function setViewpoint(name, animate)") &&
                    has(page, "setViewpoint('down', false);") && has(page, "viewSphere = viewSphere ? Cesium.BoundingSphere.union"),
                "the page has the viewpoint menu, and opens looking down at what is shown");
    ok &= check(has(page, "<input id='scale-axes-checkbox' type='checkbox' checked onclick='setScaleAxes(this.checked);'>") && has(page, "function updateScaleAxes(force)") &&
                    has(page, "scene.preRender.addEventListener(function () { updateScaleAxes(false); })") && has(page, "function roundLength(raw)") &&
                    has(page, "var scaleAxesExaggeration = 1;"),
                "the page has the scale object and its check box");

    // Viewpoints of the user
    CesiumPageOptions with_views = options;
    CesiumUserViewpoint down;   // straight down, 100 m above the point
    down.longitude = -122.304;
    down.latitude = 36.3928;
    down.height = -740.0;
    down.axis[0] = 1.0;
    down.axis[1] = 0.0;
    down.axis[2] = 0.0;
    down.angle = -1.5708;
    CesiumUserViewpoint other_view = down;
    other_view.longitude = -122.3041;
    with_views.user_viewpoints = {down, other_view};
    const std::string views_page = cesium_page_text(with_views);
    ok &= check(has(views_page, "<option value='user0'>User viewpoint 1</option>") && has(views_page, "<option value='user1'>User viewpoint 2</option>") &&
                    has(views_page, "{ position: [-122.304, 36.3928, -740], axis: [1, 0, 0], angle: -1.5708 }") && has(views_page, "function setUserViewpoint("),
                "the viewpoints of the user are in the menu, with the camera and its orientation as an axis and an angle");

    // GLBs, in groups, with the GeoOrigin that places them, and a vertical exaggeration
    CesiumPageOptions survey;
    survey.title = "survey";
    survey.model_origin[0] = -2746706.39625;
    survey.model_origin[1] = -4344091.79839;
    survey.model_origin[2] = 3762835.17592;
    survey.vertical_exaggeration = 2.0;
    CesiumModel mesh;
    mesh.uri = "multibeam/mbmesh_output/mesh.glb";
    mesh.label = "mesh.glb";
    mesh.group = "multibeam";
    mesh.group_note = "mbmesh_output";
    mesh.log_uri = "multibeam/mbmesh_output/mbmesh.log";
    mesh.log_label = "mbmesh log";
    CesiumModel track = mesh;
    track.uri = "multibeam/mbmesh_output/track.glb";
    track.label = "track.glb";
    track.group_note.clear();
    track.visible = true;
    CesiumModel lidar = mesh;
    lidar.uri = "lidar/mbmesh_output/mesh.glb";
    lidar.group = "lidar";
    survey.models = {tileset_model(), mesh, track, lidar};
    survey.models[0].group = "multibeam";
    const std::string survey_page = cesium_page_text(survey);
    ok &= check(has(survey_page, "{ uri: 'multibeam/mbmesh_output/mesh.glb', tileset: false, visible: false, lift: 0, sse: 0, shade: false, colors: false }") &&
                    has(survey_page, "Cesium.Model.fromGltfAsync(options)") &&
                    has(survey_page, "modelMatrix: Cesium.Matrix4.fromTranslation(modelOrigin), upAxis: Cesium.Axis.Z, forwardAxis: Cesium.Axis.X"),
                "a GLB is a model placed by the translation of the GeoOrigin alone, with the axes of glTF left as they are");
    ok &= check(has(survey_page, "var modelOrigin = new Cesium.Cartesian3(-2746706.396, -4344091.798, 3762835.176);"), "the GeoOrigin is the origin of the models");
    ok &= check(has(survey_page, "<div class='group'>multibeam</div>") && has(survey_page, "<div class='note'>mbmesh_output</div>") && has(survey_page, "<div class='group'>lidar</div>") &&
                    survey_page.find("<div class='group'>multibeam</div>") == survey_page.rfind("<div class='group'>multibeam</div>"),
                "the models are listed under their groups, the heading once for each group");
    ok &= check(has(survey_page, "<input id='model-checkbox-2' type='checkbox' checked onclick='setModel(2, this.checked);'> track.glb") &&
                    has(survey_page, "<input id='model-checkbox-1' type='checkbox' onclick='setModel(1, this.checked);'> mesh.glb"),
                "a model that is shown at the start is checked, the others are loaded when they are switched on");
    ok &= check(!has(survey_page, "color-checkbox-1") && has(survey_page, "color-checkbox-0"), "only a tileset with colors has the color check box");
    ok &= check(has(survey_page, "var scaleAxesExaggeration = 2;") && has(survey_page, "' (up '"), "the scale object says what the Up axis stands for under a vertical exaggeration");

    // Nothing shown at the start: the first tileset, else the first model, is
    CesiumPageOptions nothing;
    nothing.title = "survey";
    CesiumModel first = mesh;
    CesiumModel second = tileset_model();
    second.visible = false;
    nothing.models = {first, second};
    const std::string nothing_page = cesium_page_text(nothing);
    ok &= check(has(nothing_page, "{ uri: 'ZTopo_tileset/tileset.json', tileset: true, visible: true,") && has(nothing_page, "<input id='model-checkbox-1' type='checkbox' checked"),
                "if nothing is shown at the start the first tileset is");
    nothing.models = {first};
    ok &= check(has(cesium_page_text(nothing), "visible: true,"), "if nothing is shown at the start and there is no tileset the first model is");

    // A GLB without normals is shaded from the geometry
    CesiumPageOptions grid;
    grid.title = "grid";
    CesiumModel grid_model;
    grid_model.uri = "grid.glb";
    grid_model.label = "grid.glb";
    grid_model.visible = true;
    grid_model.shade_without_normals = true;
    grid_model.height_colors = true;
    grid.models = {grid_model};
    const std::string grid_page = cesium_page_text(grid);
    ok &= check(has(grid_page, "{ uri: 'grid.glb', tileset: false, visible: true, lift: 0, sse: 0, shade: true, colors: true }") && has(grid_page, "options.customShader = new Cesium.CustomShader(") &&
                    has(grid_page, "vec3 n = normalize(cross(dFdx(p), dFdy(p)))") && has(grid_page, "mix(vec3(0.7), material.diffuse, u_colors)") &&
                    has(grid_page, "<input id='color-checkbox-0' type='checkbox' checked onclick='setModelColors(0, this.checked);'> color by height") &&
                    has(grid_page, "model.primitive.customShader.setUniform('u_colors', on ? 1.0 : 0.0)"),
                "a GLB without normals is shaded with a normal found from the position of neighboring pixels, its vertex colors have a check box, and gray replaces them");

    // The options change the page
    CesiumPageOptions plain = options;
    plain.scale_axes = false;
    const std::string without_axes = cesium_page_text(plain);
    ok &= check(!has(without_axes, "scale-axes") && !has(without_axes, "updateScaleAxes") && !has(without_axes, "PolylineCollection"), "scale_axes=false leaves the scale object out");
    options.cesium_url = "./cesium/";
    options.models[0].height_colors = false;
    options.models[0].log_uri.clear();
    options.models[0].lift_meters = 0.0;
    options.show_globe = true;
    options.title = "a <b> & 'c'";
    const std::string other = cesium_page_text(options);
    ok &= check(has(other, "<script src='./cesium/Cesium.js'></script>") && has(other, "href='./cesium/Widgets/widgets.css'"),
                "a given CesiumJS directory is used, without a doubled slash");
    ok &= check(!has(other, "id='color-checkbox") && !has(other, "class='log'"), "without colors and log the page has neither");
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
