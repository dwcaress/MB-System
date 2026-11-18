#pragma once

#include <ostream>
#include <string>

// The scale object of the X3DOM pages of mbgrd2gltf, mbmesh and mbglb2tiles, held in one header so that the pages agree: axes East (red),
// North (green) and Up (blue) at the center of rotation, which the page resizes whenever a viewpoint changes so that they stay about a sixth
// of the height of the view, with their length in the label, rounded down to 1, 2 or 5 times a power of ten, in meters. The technique is that of
// the platforms of STOQS (a GeoLocation with an axes triad, resized by a viewpointChanged handler, with the length as its label).
//
// A page that uses it
//   - puts viewpoint_hook() in each of its Viewpoint and GeoViewpoint elements,
//   - writes the scene block with write_scene() inside its <scene>,
//   - writes write_checkbox_row() in its control table (a <tr> of one cell that spans two columns),
//   - and writes script() into its <script> after it has defined
//       modelOrigin    the WGS 84 ECEF position that the models are relative to (zeros if they are absolute), and
//       ecefToGeodetic(x, y, z), ECEF meters to [longitude, latitude, height],
//     and has an <x3d id='x3dElement'>.
// The length is true at the axes: in a perspective view other distances differ. A page with a vertical exaggeration sets the JavaScript variable
// scaleAxesExaggeration after the script (mbgrd2gltf does, with --exaggeration), and the label then says what the Up axis stands for as well.
namespace scale_axes {

// The attribute that each viewpoint carries to tell the page that the camera moved.
inline std::string viewpoint_hook() {
    return " onviewpointChanged='updateScaleAxes()'";
}

// The check box that switches the scale object on and off, as a row of the control table.
inline void write_checkbox_row(std::ostream &out) {
    out << "          <tr>\n"
        << "            <td colspan='2'>\n"
        << "              <input id='scale-axes-checkbox' type='checkbox' checked onclick='setScaleAxes(this.checked);'>\n"
        << "              </input> scale axes\n"
        << "            </td>\n"
        << "          </tr>\n";
}

// The scene nodes: a triad of unit length, in the frame East (X), Up (Y), South (Z) that X3DOM uses at a point of the Earth, which the page moves to
// the center of rotation with a matrix, scaled by the length that its label says, in meters. The letters face the viewer.
inline void write_scene(std::ostream &out) {
    const auto axis = [&](const char *translation, const std::string &rotation, const char *color) {
        out << "            <transform translation='" << translation << "'" << (rotation.empty() ? std::string() : " rotation='" + rotation + "'") << ">\n"
            << "              <shape><appearance><material diffuseColor='0 0 0' emissiveColor='" << color << "'></material></appearance>\n"
            << "                <cylinder height='1' radius='0.008'></cylinder></shape>\n"
            << "            </transform>\n";
    };
    const auto letter = [&](const char *translation, const char *text, const char *color) {
        out << "            <transform translation='" << translation << "' scale='0.09 0.09 0.09'><billboard axisOfRotation='0 0 0'><shape>\n"
            << "              <appearance><material diffuseColor='0 0 0' emissiveColor='" << color << "'></material></appearance>\n"
            << "              <text string='" << text << "'><fontstyle size='2' justify='MIDDLE'></fontstyle></text></shape></billboard></transform>\n";
    };
    out << "        <!-- The scale object: axes East (red), North (green) and Up (blue) at the center of rotation, with the length of each. -->\n"
        << "        <switch id='scale_axes_switch' whichChoice='0'>\n"
        << "          <matrixTransform id='scale_axes_matrix' matrix='1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1'>\n"
        << "            <group>\n";
    axis("0.5 0 0", "0 0 1 -1.5708", "0.9 0.1 0.1");
    axis("0 0.5 0", "", "0.1 0.2 0.95");
    axis("0 0 -0.5", "1 0 0 -1.5708", "0.1 0.65 0.1");
    letter("1.08 0 0", "E", "0.9 0.1 0.1");
    letter("0 1.08 0", "U", "0.1 0.2 0.95");
    letter("0 0 -1.08", "N", "0.1 0.65 0.1");
    // The length label is white, with the same text in black just behind it and a little to the side, so that it reads on a light surface as well. It is
    // moved toward the viewer (the local Z of a billboard) so that a surface that rises toward the camera does not hide part of it.
    out << "            <transform translation='0.5 0 0.16' scale='0.1 0.1 0.1'><billboard axisOfRotation='0 0 0'><transform translation='0 0 6'>\n"
        << "              <transform translation='0.07 -0.07 -0.02'><shape>\n"
        << "                <appearance><material diffuseColor='0 0 0' emissiveColor='0 0 0'></material></appearance>\n"
        << "                <text id='scale_axes_shadow' string='  '><fontstyle size='2' justify='MIDDLE'></fontstyle></text></shape></transform>\n"
        << "              <shape>\n"
        << "                <appearance><material diffuseColor='0 0 0' emissiveColor='1 1 1'></material></appearance>\n"
        << "                <text id='scale_axes_length' string='  '><fontstyle size='2' justify='MIDDLE'></fontstyle></text></shape>\n"
        << "            </transform></billboard></transform>\n"
        << "            </group>\n"
        << "          </matrixTransform>\n"
        << "        </switch>\n";
}

// The script of the scale object.
inline const char *script() {
    return R"JS(
      // The scale object: axes at the center of rotation, East, North and Up, as long on the screen at any distance, with their length in
      // meters, a round number, in the label. The axes are the unit triad of the scene, moved and scaled with a matrix in the frame that
      // X3DOM uses at a point of the Earth (X east, Y up, Z south); the page calls this whenever the viewpoint changes.
      // scaleAxesExaggeration is the vertical exaggeration of the models, 1 unless the page sets it: the Up axis is then that many times as long as the
      // meters it stands for, and the label says both.
      var scaleAxesOn = true, scaleAxesMatrix = '', scaleAxesLength = 0, scaleAxesExaggeration = 1;

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

      function updateScaleAxes() {
        var node = document.getElementById('scale_axes_matrix');
        if (!scaleAxesOn || !node) {
          return;
        }
        var runtime = document.getElementById('x3dElement').runtime;
        if (!runtime || !runtime.canvas || !runtime.canvas.doc || !runtime.canvas.doc._scene) {
          return;
        }
        var camera, center, fov = 0.785398;
        try {
          var viewpoint = runtime.canvas.doc._scene.getViewpoint();
          camera = runtime.viewMatrix().inverse().e3();
          center = viewpoint.getCenterOfRotation();
          fov = viewpoint._vf.fieldOfView || fov;
        } catch (e) {
          return;
        }
        if (!camera || !center) {
          return;
        }
        var distance = Math.sqrt(Math.pow(camera.x - center.x, 2) + Math.pow(camera.y - center.y, 2) + Math.pow(camera.z - center.z, 2));
        if (!(distance > 0)) {
          return;
        }
        // About a sixth of the height of the view, rounded down to 1, 2 or 5 times a power of ten.
        var length = roundLength(distance * Math.tan(fov / 2) / 3);
        var at = ecefToGeodetic(center.x + modelOrigin[0], center.y + modelOrigin[1], center.z + modelOrigin[2]);
        var phi = at[1] * Math.PI / 180, lambda = at[0] * Math.PI / 180;
        var east = [-Math.sin(lambda), Math.cos(lambda), 0];
        var north = [-Math.sin(phi) * Math.cos(lambda), -Math.sin(phi) * Math.sin(lambda), Math.cos(phi)];
        var up = [Math.cos(phi) * Math.cos(lambda), Math.cos(phi) * Math.sin(lambda), Math.sin(phi)];
        var m = [];
        [east, up, [-north[0], -north[1], -north[2]]].forEach(function (axis) {
          m.push(axis[0] * length, axis[1] * length, axis[2] * length, 0);
        });
        m.push(center.x, center.y, center.z, 1);
        var text = m.map(function (value) { return value.toFixed(5); }).join(' ');
        if (text !== scaleAxesMatrix) {
          scaleAxesMatrix = text;
          node.setAttribute('matrix', text);
        }
        if (length !== scaleAxesLength) {
          scaleAxesLength = length;
          var label = lengthText(length);
          if (scaleAxesExaggeration !== 1) {
            label += ' (up ' + lengthText(Math.round(length / scaleAxesExaggeration * 100) / 100) + ')';
          }
          document.getElementById('scale_axes_length').setAttribute('string', label);
          document.getElementById('scale_axes_shadow').setAttribute('string', label);
        }
      }

      function setScaleAxes(on) {
        scaleAxesOn = on;
        document.getElementById('scale_axes_switch').setAttribute('whichChoice', on ? 0 : -1);
        if (on) {
          updateScaleAxes();
        }
      }

      // The first placement, once the scene has loaded, and again after a short while in case no viewpoint change comes.
      setTimeout(updateScaleAxes, 1500);
      setTimeout(updateScaleAxes, 5000);
)JS";
}

} // namespace scale_axes
