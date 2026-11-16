#include "io/cesium_writer.h"

#include <fstream>
#include <iomanip>
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
// The user viewpoints as the elements of a JavaScript array: the camera position, longitude, latitude and height, and the orientation as an
// axis and an angle in the local frame of the camera (X east, Y up, Z south, the camera looks along its -Z), as a GeoViewpoint takes it.
std::string user_viewpoints_script(const std::vector<X3DomUserViewpoint> &viewpoints) {
    std::ostringstream out;
    out << std::setprecision(12);
    bool first = true;
    for (const X3DomUserViewpoint &viewpoint : viewpoints) {
        double axis[3];
        double angle = 0.0;
        std::string error;
        if (!geoviewpoint_orientation(viewpoint, axis, &angle, &error)) {
            continue;   // the program checks each one when it reads the option
        }
        out << (first ? "" : ", ") << "{ position: [" << viewpoint.longitude << ", " << viewpoint.latitude << ", " << viewpoint.elevation
            << "], axis: [" << axis[0] << ", " << axis[1] << ", " << axis[2] << "], angle: " << angle << " }";
        first = false;
    }
    return out.str();
}

// The script of the viewpoints of the user, and of the button that gives the option for the view on the screen.
const char *viewpoint_script() {
    return R"JS(    // A viewpoint of the user is the camera position and its orientation as an axis and an angle in the local frame there, X east, Y up and Z south, with
    // the camera looking along its -Z, as the GeoViewpoint of X3DOM has it: this takes that to the ECEF direction and up that Cesium wants.
    function setUserViewpoint(viewpoint, animate) {
      if (!viewpoint) {
        return;
      }
      var destination = Cesium.Cartesian3.fromDegrees(viewpoint.position[0], viewpoint.position[1], viewpoint.position[2]);
      var rotation = Cesium.Matrix3.fromQuaternion(Cesium.Quaternion.fromAxisAngle(new Cesium.Cartesian3(viewpoint.axis[0], viewpoint.axis[1], viewpoint.axis[2]), viewpoint.angle));
      var frame = Cesium.Transforms.eastNorthUpToFixedFrame(destination);
      var toEcef = function (local) {
        var enu = Cesium.Matrix3.multiplyByVector(rotation, local, new Cesium.Cartesian3());
        return Cesium.Matrix4.multiplyByPointAsVector(frame, new Cesium.Cartesian3(enu.x, -enu.z, enu.y), new Cesium.Cartesian3());
      };
      var orientation = { direction: toEcef(new Cesium.Cartesian3(0, 0, -1)), up: toEcef(new Cesium.Cartesian3(0, 1, 0)) };
      if (animate) {
        viewer.camera.flyTo({ destination: destination, orientation: orientation, duration: 1.0 });
      } else {
        viewer.camera.setView({ destination: destination, orientation: orientation });
      }
    }

    // The option for the view on the screen: the camera, its orientation in the local frame as an axis and an angle, and the point of the data in the
    // middle of the view, which is what the --geoviewpoint of the X3DOM pages has, so that an option copied from one page works in the other.
    function copyViewpoint() {
      var camera = viewer.camera;
      var position = Cesium.Cartographic.fromCartesian(camera.positionWC);
      var center = null;
      try {
        var canvas = viewer.scene.canvas;
        center = viewer.scene.pickPosition(new Cesium.Cartesian2(canvas.clientWidth / 2, canvas.clientHeight / 2));
      } catch (e) {
        center = null;
      }
      if (!center && tileset) {
        center = tileset.boundingSphere.center;
      }
      if (!center) {
        return;
      }
      var target = Cesium.Cartographic.fromCartesian(center);
      var toLocal = Cesium.Matrix4.inverseTransformation(Cesium.Transforms.eastNorthUpToFixedFrame(camera.positionWC), new Cesium.Matrix4());
      var local = function (vector) {
        var enu = Cesium.Matrix4.multiplyByPointAsVector(toLocal, vector, new Cesium.Cartesian3());
        return [enu.x, enu.z, -enu.y];
      };
      // The camera's axes in the local frame: its X to the right, its Y up and its Z backwards.
      var axes = [local(camera.rightWC), local(camera.upWC), local(Cesium.Cartesian3.negate(camera.directionWC, new Cesium.Cartesian3()))];
      var o = function (i, j) { return axes[j][i]; };
      var angle = Math.acos(Math.max(-1, Math.min(1, (o(0, 0) + o(1, 1) + o(2, 2) - 1) / 2)));
      var axis = [o(2, 1) - o(1, 2), o(0, 2) - o(2, 0), o(1, 0) - o(0, 1)];
      var length = Math.sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
      if (length < 1e-9) {
        axis = [0, 0, 1];
        angle = 0;
      } else {
        axis = [axis[0] / length, axis[1] / length, axis[2] / length];
      }
      var degrees = Cesium.Math.toDegrees;
      var numbers = [degrees(position.longitude), degrees(position.latitude), position.height, axis[0], axis[1], axis[2], angle,
                     degrees(target.longitude), degrees(target.latitude), target.height];
      var digits = [7, 7, 3, 8, 8, 8, 8, 7, 7, 3];
      var text = '--geoviewpoint=' + numbers.map(function (value, index) { return value.toFixed(digits[index]); }).join(',');
      var failed = function () {
        window.prompt('Copy the option:', text);
      };
      if (navigator.clipboard && window.isSecureContext) {
        navigator.clipboard.writeText(text).then(function () {
          showStatus('The --geoviewpoint option was copied to the clipboard', false);
          setTimeout(function () { showStatus('', false); }, 4000);
        }, failed);
      } else {
        failed();
      }
    }

)JS";
}

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
        << "      <option value='side'>Low, 15&deg; from the south</option>\n";
    for (std::size_t i = 0; i < options.user_viewpoints.size(); i++) {
        out << "      <option value='user" << i << "'>User viewpoint " << (i + 1) << "</option>\n";
    }
    out << "    </select></label>\n"
        << "    <label><button onclick='copyViewpoint();' title='Puts the --geoviewpoint option for the view on the screen on the clipboard'>Copy viewpoint</button></label>\n";
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
        << "    var userViewpoints = [" << user_viewpoints_script(options.user_viewpoints) << "];\n"
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
        << "      if (name.indexOf('user') === 0) {\n"
        << "        setUserViewpoint(userViewpoints[Number(name.substring(4))], animate);\n"
        << "        return;\n"
        << "      }\n"
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
        << viewpoint_script()
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
