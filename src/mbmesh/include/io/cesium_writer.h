#pragma once

#include <filesystem>
#include <string>

// A viewer page of an OGC 3D Tiles tileset for CesiumJS, which is the reference renderer of 3D Tiles (the X3DOM pages need a build of X3DOM that has
// GeoTileset). The data of a survey are below the ellipsoid, which the globe of Cesium hides, so the page starts with the globe hidden and has a
// check box that shows it, with the globe translucent so that the data can be seen through it.
struct CesiumPageOptions {
    // Shown in the panel and as the title of the page.
    std::string title;
    // The tileset.json, relative to the page, with characters that need it percent-encoded.
    std::string tileset_uri;
    // The log of the program, relative to the page; a link to it is shown if it is not empty.
    std::string log_url;
    // The directory of a build of CesiumJS, with Cesium.js and Widgets/widgets.css in it; the release that the macro mbm_tmap uses from its
    // web site if it is empty.
    std::string cesium_url;
    // How far the tileset is lifted above the ellipsoid by the transform of its tileset.json (--lift), which X3DOM needs and Cesium does not: the
    // page moves the tileset back down by this much, so that it is where it was measured.
    double lift_meters = 0.0;
    // The maximumScreenSpaceError of the Cesium3DTileset, in pixels; zero leaves Cesium's own default of 16.
    double max_screen_space_error = 0.0;
    // The tiles have a color for each vertex; the page offers a check box that turns the colors off, which shows the tiles in one gray.
    bool height_colors = false;
    // The scale object, as the X3DOM pages have: axes East (red), North (green) and Up (blue) at the point of the data in the middle of the view, which
    // the page resizes whenever the camera moves so that they stay about a sixth of the height of the view, with their length, rounded down to 1, 2 or
    // 5 times a power of ten, in the label. A check box switches it off.
    bool scale_axes = true;
    // Whether the globe is there when the page opens, which it is not by default; the check box shows or hides it either way.
    bool show_globe = false;
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
