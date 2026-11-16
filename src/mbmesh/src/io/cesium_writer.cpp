#include "io/cesium_writer.h"

#include <fstream>
#include <sstream>

namespace {

std::string escape_html(const std::string &text) {
    std::string out;
    for (const char c : text) {
        switch (c) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        case '\'': out += "&#39;"; break;
        case '"': out += "&quot;"; break;
        default: out += c;
        }
    }
    return out;
}

// A string for a JavaScript literal in single quotes inside a <script>.
std::string escape_js(const std::string &text) {
    std::string out;
    for (const char c : text) {
        switch (c) {
        case '\\': out += "\\\\"; break;
        case '\'': out += "\\'"; break;
        case '\n': out += "\\n"; break;
        case '<': out += "\\x3c"; break;
        default: out += c;
        }
    }
    return out;
}

// The script of the scale object. The center of rotation of Cesium is not a point that a page can ask for, so the axes are put at the point of the
// data under the middle of the view (a pick of the depth there), and at the center of the tileset where there is none.
const char *scale_axes_script() {
    return R"JS(
    // The scale object: axes East (red), North (green) and Up (blue) where the middle of the view meets the data, as long on the screen at any
    // distance, with their length in meters, a round number, in the label. The length is true at the axes; the perspective makes other distances differ.
    var scaleAxesOn = true, scaleAxesLength = 0, scaleAxesAnchor = null, scaleAxesPickedAt = 0, scaleAxesPosition = null, scaleAxesDirection = null;
    var scaleAxesLines = viewer.scene.primitives.add(new Cesium.PolylineCollection());
    var scaleAxesLabels = viewer.scene.primitives.add(new Cesium.LabelCollection());
    var scaleAxesColors = [Cesium.Color.RED, Cesium.Color.LIME, Cesium.Color.DEEPSKYBLUE];
    var scaleAxesLetters = ['E', 'N', 'U'];
    var scaleAxesLine = [], scaleAxesLetter = [];
    for (var axis = 0; axis < 3; axis++) {
      scaleAxesLine.push(scaleAxesLines.add({ positions: [Cesium.Cartesian3.ZERO, Cesium.Cartesian3.ZERO], width: 4,
        material: Cesium.Material.fromType('Color', { color: scaleAxesColors[axis] }),
        // Where the surface is in front of an axis, it is still seen, fainter.
        depthFailMaterial: Cesium.Material.fromType('Color', { color: scaleAxesColors[axis].withAlpha(0.45) }) }));
      scaleAxesLetter.push(scaleAxesLabels.add({ text: scaleAxesLetters[axis], font: 'bold 16px sans-serif', fillColor: scaleAxesColors[axis],
        outlineColor: Cesium.Color.BLACK, outlineWidth: 3, style: Cesium.LabelStyle.FILL_AND_OUTLINE,
        horizontalOrigin: Cesium.HorizontalOrigin.CENTER, verticalOrigin: Cesium.VerticalOrigin.CENTER,
        disableDepthTestDistance: Number.POSITIVE_INFINITY }));
    }
    var scaleAxesText = scaleAxesLabels.add({ text: '', font: 'bold 16px sans-serif', fillColor: Cesium.Color.WHITE, outlineColor: Cesium.Color.BLACK,
      outlineWidth: 3, style: Cesium.LabelStyle.FILL_AND_OUTLINE, horizontalOrigin: Cesium.HorizontalOrigin.CENTER,
      verticalOrigin: Cesium.VerticalOrigin.TOP, pixelOffset: new Cesium.Cartesian2(0, 8), disableDepthTestDistance: Number.POSITIVE_INFINITY });

    function roundLength(raw) {
      var power = Math.pow(10, Math.floor(Math.log(raw) / Math.LN10 + 1e-9));
      var mantissa = raw / power;
      return (mantissa >= 5 ? 5 : mantissa >= 2 ? 2 : 1) * power;
    }

    function lengthText(meters) {
      if (meters >= 1000) {
        return (meters / 1000) + ' km';
      }
      return meters >= 1 ? meters + ' m' : Math.round(meters * 100) + ' cm';
    }

    // Called before every frame, so the axes follow the camera as it moves: nothing is done if the camera has not moved, and the point under the middle of
    // the view, which costs a read of the depth buffer, is picked at most four times a second unless force is true.
    function updateScaleAxes(force) {
      if (!scaleAxesOn) {
        return;
      }
      var scene = viewer.scene, camera = viewer.camera;
      if (!force && scaleAxesPosition && Cesium.Cartesian3.equals(camera.positionWC, scaleAxesPosition) && Cesium.Cartesian3.equals(camera.directionWC, scaleAxesDirection)) {
        return;
      }
      scaleAxesPosition = Cesium.Cartesian3.clone(camera.positionWC, scaleAxesPosition);
      scaleAxesDirection = Cesium.Cartesian3.clone(camera.directionWC, scaleAxesDirection);
      var anchor = scaleAxesAnchor;
      var now = Date.now();
      if (force || !anchor || now - scaleAxesPickedAt > 250) {
        scaleAxesPickedAt = now;
        try {
          if (scene.pickPositionSupported) {
            anchor = scene.pickPosition(new Cesium.Cartesian2(scene.canvas.clientWidth / 2, scene.canvas.clientHeight / 2));
          }
        } catch (e) {
          anchor = undefined;
        }
        if (!Cesium.defined(anchor)) {
          anchor = scaleAxesAnchor || (tileset ? tileset.boundingSphere.center : null);
        }
      }
      if (!anchor) {
        return;
      }
      scaleAxesAnchor = anchor;
      var distance = Cesium.Cartesian3.distance(camera.positionWC, anchor);
      var fovy = camera.frustum.fovy || 0.785398;
      if (!(distance > 0)) {
        return;
      }
      // About a sixth of the height of the view, rounded down to 1, 2 or 5 times a power of ten.
      var length = roundLength(distance * Math.tan(fovy / 2) / 3);
      var frame = Cesium.Transforms.eastNorthUpToFixedFrame(anchor);
      // The origin of the axes is a little above the surface that was picked, so that the horizontal axes do not lie in it and sink into a rising slope.
      var up = Cesium.Matrix4.getColumn(frame, 2, new Cesium.Cartesian4());
      var origin = Cesium.Cartesian3.add(anchor, new Cesium.Cartesian3(up.x * length * 0.1, up.y * length * 0.1, up.z * length * 0.1), new Cesium.Cartesian3());
      var tips = [];
      for (var axis = 0; axis < 3; axis++) {
        var direction = Cesium.Matrix4.getColumn(frame, axis, new Cesium.Cartesian4());
        var tip = Cesium.Cartesian3.add(origin, new Cesium.Cartesian3(direction.x * length, direction.y * length, direction.z * length), new Cesium.Cartesian3());
        tips.push(tip);
        scaleAxesLine[axis].positions = [origin, tip];
        scaleAxesLetter[axis].position = Cesium.Cartesian3.add(origin, new Cesium.Cartesian3(direction.x * length * 1.1, direction.y * length * 1.1, direction.z * length * 1.1), new Cesium.Cartesian3());
      }
      scaleAxesText.position = Cesium.Cartesian3.midpoint(origin, tips[0], new Cesium.Cartesian3());
      if (length !== scaleAxesLength) {
        scaleAxesLength = length;
        scaleAxesText.text = lengthText(length);
      }
    }

    function setScaleAxes(on) {
      scaleAxesOn = on;
      scaleAxesLines.show = on;
      scaleAxesLabels.show = on;
      if (on) {
        updateScaleAxes(true);
      }
    }

    viewer.scene.preRender.addEventListener(function () { updateScaleAxes(false); });
    viewer.camera.moveEnd.addEventListener(function () { updateScaleAxes(true); });

)JS";
}

}  // namespace

std::filesystem::path cesium_page_path(const std::filesystem::path &output_directory) {
    std::filesystem::path directory = output_directory;
    if (!directory.has_filename()) {
        directory = directory.parent_path();
    }
    return directory.parent_path() / (directory.filename().string() + "_cesium.html");
}

std::string cesium_page_text(const CesiumPageOptions &options) {
    std::string cesium = options.cesium_url.empty() ? std::string(default_cesium_url()) : options.cesium_url;
    while (cesium.size() > 1 && cesium.back() == '/') {
        cesium.pop_back();
    }
    std::ostringstream out;
    out << "<!DOCTYPE html>\n"
        << "<html lang='en'>\n"
        << "<head>\n"
        << "  <meta charset='UTF-8'>\n"
        << "  <title>" << escape_html(options.title) << " (Cesium)</title>\n"
        << "  <script src='" << escape_html(cesium) << "/Cesium.js'></script>\n"
        << "  <link href='" << escape_html(cesium) << "/Widgets/widgets.css' rel='stylesheet'>\n"
        << "  <style>\n"
        << "    html, body, #cesiumContainer { width: 100%; height: 100%; margin: 0; padding: 0; overflow: hidden; font-family: sans-serif; }\n"
        << "    #panel { position: absolute; top: 10px; left: 10px; z-index: 20; background: rgba(255,255,255,0.88); padding: 8px 12px;\n"
        << "             border-radius: 4px; font-size: 14px; max-width: 22em; }\n"
        << "    #panel label { display: block; margin-top: 4px; }\n"
        << "    #status { font-size: smaller; margin-top: 6px; }\n"
        << "    #status.error { color: #b00020; }\n"
        << "  </style>\n"
        << "</head>\n"
        << "<body>\n"
        << "  <div id='cesiumContainer'></div>\n"
        << "  <div id='panel'>\n"
        << "    <b>" << escape_html(options.title) << "</b>\n"
        << "    <label><input id='globe-checkbox' type='checkbox'" << (options.show_globe ? " checked" : "")
        << " onclick='setGlobe(this.checked);'> show globe (translucent)</label>\n";
    out << "    <label>Viewpoint: <select id='viewpoint-select' onchange='setViewpoint(this.value, true);'>\n"
        << "      <option value='down'>Overview, looking down</option>\n"
        << "      <option value='oblique'>Oblique, 45&deg; from the south</option>\n"
        << "      <option value='side'>Low, 15&deg; from the south</option>\n"
        << "    </select></label>\n";
    if (options.height_colors) {
        out << "    <label><input id='color-checkbox' type='checkbox' checked onclick='setColors(this.checked);'> color by height</label>\n";
    }
    if (!options.log_url.empty()) {
        out << "    <label><a href='" << escape_html(options.log_url) << "' target='_blank'>log</a></label>\n";
    }
    if (options.scale_axes) {
        out << "    <label><input id='scale-axes-checkbox' type='checkbox' checked onclick='setScaleAxes(this.checked);'> scale axes</label>\n";
    }
    out << "    <div id='status'>loading the tileset...</div>\n"
        << "  </div>\n"
        << "  <script>\n"
        << "    // The tileset of OGC 3D Tiles in CesiumJS. The data are below the ellipsoid, which the globe would hide, so the globe starts hidden.\n"
        << "    var tilesetUri = '" << escape_js(options.tileset_uri) << "';\n"
        << "    var liftMeters = " << options.lift_meters << ";\n"
        << "    var maximumScreenSpaceError = " << options.max_screen_space_error << ";\n"
        << "    var tileset = null;\n"
        << "\n"
        << "    function showStatus(text, isError) {\n"
        << "      var status = document.getElementById('status');\n"
        << "      status.textContent = text;\n"
        << "      status.className = isError ? 'error' : '';\n"
        << "    }\n"
        << "\n"
        << "    var viewer = new Cesium.Viewer('cesiumContainer', {\n"
        << "      baseLayer: new Cesium.ImageryLayer(new Cesium.OpenStreetMapImageryProvider({ url: 'https://tile.openstreetmap.org/' })),\n"
        << "      baseLayerPicker: false, geocoder: false, homeButton: false, sceneModePicker: false, navigationHelpButton: false,\n"
        << "      animation: false, timeline: false, fullscreenButton: false, infoBox: false, selectionIndicator: false\n"
        << "    });\n"
        << "    // Data below the globe: the camera must be able to go there, and the globe, when it is shown, lets them be seen through it.\n"
        << "    viewer.scene.screenSpaceCameraController.enableCollisionDetection = false;\n"
        << "    viewer.scene.globe.translucency.enabled = true;\n"
        << "    viewer.scene.globe.translucency.frontFaceAlpha = 0.45;\n"
        << "    viewer.scene.globe.show = " << (options.show_globe ? "true" : "false") << ";\n"
        << "    viewer.scene.backgroundColor = Cesium.Color.fromCssColorString('#1c1f24');\n"
        << "\n"
        << "    function setGlobe(on) {\n"
        << "      viewer.scene.globe.show = on;\n"
        << "    }\n"
        << "\n"
        << "    // The viewpoints look at the tileset from above, and from the south at 45 and at 15 degrees, with North at the top of the view that looks down.\n"
        << "    // The distance fits the bounding sphere of the tileset in the view.\n"
        << "    var viewpoints = { down: -90, oblique: -45, side: -15 };\n"
        << "    function setViewpoint(name, animate) {\n"
        << "      if (!tileset || !(name in viewpoints)) {\n"
        << "        return;\n"
        << "      }\n"
        << "      var sphere = tileset.boundingSphere;\n"
        << "      var offset = new Cesium.HeadingPitchRange(0, Cesium.Math.toRadians(viewpoints[name]), sphere.radius * (name === 'down' ? 2.2 : 2.5));\n"
        << "      if (animate) {\n"
        << "        viewer.camera.flyToBoundingSphere(sphere, { offset: offset, duration: 1.0 });\n"
        << "      } else {\n"
        << "        viewer.camera.viewBoundingSphere(sphere, offset);\n"
        << "        viewer.camera.lookAtTransform(Cesium.Matrix4.IDENTITY);\n"
        << "      }\n"
        << "    }\n"
        << "\n"
        << "    // The colors are the COLOR_0 of the tiles; a gray base color replaces them. A custom shader sets the base color before the lighting, which\n"
        << "    // keeps the relief shaded: the REPLACE blend mode of a style paints the final color and loses it.\n"
        << "    var grayShader = new Cesium.CustomShader({\n"
        << "      fragmentShaderText: 'void fragmentMain(FragmentInput fsInput, inout czm_modelMaterial material) { material.diffuse = vec3(0.75); }'\n"
        << "    });\n"
        << "    function setColors(on) {\n"
        << "      if (!tileset) {\n"
        << "        return;\n"
        << "      }\n"
        << "      tileset.customShader = on ? undefined : grayShader;\n"
        << "    }\n"
        << "\n"
        << (options.scale_axes ? scale_axes_script() : "")
        << "    Cesium.Cesium3DTileset.fromUrl(tilesetUri, maximumScreenSpaceError > 0 ? { maximumScreenSpaceError: maximumScreenSpaceError } : {})\n"
        << "      .then(function (loaded) {\n"
        << "        tileset = loaded;\n"
        << "        viewer.scene.primitives.add(tileset);\n"
        << "        if (liftMeters !== 0) {\n"
        << "          // The transform of tileset.json lifts the tiles by liftMeters along the local vertical, for X3DOM: take it off again.\n"
        << "          var center = tileset.boundingSphere.center;\n"
        << "          var up = Cesium.Ellipsoid.WGS84.geodeticSurfaceNormal(center, new Cesium.Cartesian3());\n"
        << "          tileset.modelMatrix = Cesium.Matrix4.fromTranslation(Cesium.Cartesian3.multiplyByScalar(up, -liftMeters, new Cesium.Cartesian3()));\n"
        << "        }\n"
        << "        setViewpoint('down', false);\n"
        << "        showStatus('', false);\n"
        << (options.scale_axes ? "        setTimeout(function () { updateScaleAxes(true); }, 1500);\n" : "")
        << "      })\n"
        << "      .catch(function (error) {\n"
        << "        showStatus('Cannot load ' + tilesetUri + ': ' + error + '. The page has to be served over HTTP, not opened as a file.', true);\n"
        << "      });\n"
        << "  </script>\n"
        << "</body>\n"
        << "</html>\n";
    return out.str();
}

bool write_cesium_page(const std::filesystem::path &html_path, const CesiumPageOptions &options, std::string *error) {
    std::ofstream file(html_path);
    if (!file) {
        if (error != nullptr) {
            *error = "Unable to create Cesium page: " + html_path.string();
        }
        return false;
    }
    file << cesium_page_text(options);
    file.close();
    if (!file) {
        if (error != nullptr) {
            *error = "Error while writing Cesium page: " + html_path.string();
        }
        return false;
    }
    return true;
}
