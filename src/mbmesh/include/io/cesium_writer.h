#pragma once

#include <filesystem>
#include <string>
#include <vector>

// A viewer page for CesiumJS of what mbgrd2gltf, mbmesh and mbglb2tiles make: OGC 3D Tiles tilesets, which CesiumJS is the reference renderer of (the X3DOM
// pages need a build of X3DOM that has GeoTileset), and GLBs. The data of a survey are below the ellipsoid, which the globe of Cesium hides, so the page
// starts with the globe hidden and has a check box that shows it, with the globe translucent so that the data can be seen through it.
//
// The writer knows nothing of the programs: each fills in the options. It is a source of the three programs, so it uses nothing but the standard library.

// A viewpoint of the user (--geoviewpoint): the camera position, geodetic, and its orientation as an axis and an angle in the local frame of the camera
// (X east, Y up, Z south; the camera looks along its -Z), as a GeoViewpoint takes it. The height is a true one.
struct CesiumUserViewpoint {
    double longitude = 0.0;
    double latitude = 0.0;
    double height = 0.0;
    double axis[3] = {0.0, 0.0, 1.0};
    double angle = 0.0;
};

// One thing the page can show, with a check box to switch it on and off.
struct CesiumModel {
    std::string uri;         // relative to the page, with characters that need it percent-encoded: a tileset.json or a GLB
    std::string label;       // shown beside the check box
    std::string group;       // models with the same group are listed together under that heading; may be empty
    std::string group_note;  // small print under the heading, for the first model of a group; may be empty
    bool visible = false;    // shown when the page opens; others are loaded when first switched on
    // The uri is an OGC 3D Tiles tileset.json, loaded as a Cesium3DTileset. Otherwise it is a GLB whose vertices are ECEF meters, or offsets from the GeoOrigin
    // (CesiumPageOptions::model_origin), as the GLBs of mbgrd2gltf and mbmesh are.
    bool is_tileset = false;
    // For a GLB: it has no normals (the mesh of mbgrd2gltf has none), which CesiumJS draws unlit, in one flat color; the page shades it with a normal found
    // from the change of position between neighboring pixels, and a light, so that the relief shows.
    bool shade_without_normals = false;
    // For a tileset: the maximumScreenSpaceError of the Cesium3DTileset, in pixels; zero leaves Cesium's own default of 16.
    double max_screen_space_error = 0.0;
    // For a tileset: how far its tileset.json lifts it above the ellipsoid (--lift), which X3DOM needs and Cesium does not: the page moves the tileset back
    // down by this much, so that it is where it was measured. The GLBs are not lifted.
    double lift_meters = 0.0;
    // For a tileset: its tiles have a color for each vertex, and the page offers a check box that shows them in one shaded gray instead.
    bool height_colors = false;
    // The log of the program that made the model: a link to the right of the label, named for the program ("mbmesh log"). Both are empty without one.
    std::string log_uri;     // relative to the page, with characters that need it percent-encoded
    std::string log_label;
};

struct CesiumPageOptions {
    // Shown in the panel and as the title of the page.
    std::string title;
    // The directory of a build of CesiumJS, with Cesium.js and Widgets/widgets.css in it; the release that the macro mbm_tmap uses from its
    // web site if it is empty.
    std::string cesium_url;
    // The scale object, as the X3DOM pages have: axes East (red), North (green) and Up (blue) at the point of the data in the middle of the view, which
    // the page resizes whenever the camera moves so that they stay about a sixth of the height of the view, with their length, rounded down to 1, 2 or
    // 5 times a power of ten, in the label. A check box switches it off.
    bool scale_axes = true;
    // The vertical exaggeration of the GLBs (mbgrd2gltf --exaggeration): the label of the scale object says what the Up axis stands for as well.
    double vertical_exaggeration = 1.0;
    // Whether the globe is there when the page opens, which it is not by default; the check box shows or hides it either way.
    bool show_globe = false;
    // Viewpoints of the user, which the Viewpoint menu offers as "User viewpoint 1", "User viewpoint 2" and so on. The heights are true ones, which the page
    // uses as they are: it has no lift. The "Copy viewpoint" button gives the option for the view on the screen.
    std::vector<CesiumUserViewpoint> user_viewpoints;
    // The log of the program that made what the page shows, for a page of one run: a link under the title, named log_label ("mbmesh log"). Both are empty
    // without one; a model has a log of its own (CesiumModel::log_uri).
    std::string log_uri;     // relative to the page, with characters that need it percent-encoded
    std::string log_label;
    // The WGS 84 ECEF position that the vertices of the GLBs are relative to, the GeoOrigin; zeros if they are absolute ECEF.
    double model_origin[3] = {0.0, 0.0, 0.0};
    // What the page shows. The view of the page when it opens, and its Viewpoint menu, fit the models that are visible.
    std::vector<CesiumModel> models;
};

// The default of CesiumPageOptions::cesium_url.
inline const char *default_cesium_url() {
    return "https://cesium.com/downloads/cesiumjs/releases/1.114/Build/Cesium";
}

// The text of the page.
[[nodiscard]] std::string cesium_page_text(const CesiumPageOptions &options);

// Writes the page. Returns false and says why in error.
[[nodiscard]] bool write_cesium_page(const std::filesystem::path &html_path, const CesiumPageOptions &options, std::string *error);

// The page of a tileset beside its output directory, named after it, <root>/<directory>_cesium.html, as viewer_page_path() gives <directory>.html.
[[nodiscard]] std::filesystem::path cesium_page_path(const std::filesystem::path &output_directory);
