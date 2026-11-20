#include "scale_axes.h"

#include <cstdio>
#include <sstream>
#include <string>

namespace {

int failures = 0;

void check(bool ok, const char *what) {
    if (!ok) {
        std::printf("FAILED: %s\n", what);
        failures++;
    }
}

bool contains(const std::string &text, const std::string &part) {
    return text.find(part) != std::string::npos;
}

} // namespace

int main() {
    // The hook that each viewpoint carries calls the function that the script defines.
    const std::string script = scale_axes::script();
    check(contains(scale_axes::viewpoint_hook(), "onviewpointChanged='updateScaleAxes()'") && contains(script, "function updateScaleAxes()"),
          "the viewpoint hook calls updateScaleAxes, which the script defines");
    check(contains(script, "function setScaleAxes(on)") && contains(script, "function roundLength(raw)") && contains(script, "function lengthText(meters)"),
          "the script defines setScaleAxes, roundLength and lengthText");
    check(contains(script, "scaleAxesExaggeration"), "the script has a vertical exaggeration that a page can set");
    // What the page must have for the script: the origin of the models and the conversion to geodetic.
    check(contains(script, "modelOrigin") && contains(script, "ecefToGeodetic(") && contains(script, "getElementById('x3dElement')"),
          "the script uses the modelOrigin, ecefToGeodetic and x3dElement of the page");

    std::ostringstream scene;
    scale_axes::write_scene(scene);
    const std::string nodes = scene.str();
    check(contains(nodes, "<switch id='scale_axes_switch'") && contains(nodes, "<matrixTransform id='scale_axes_matrix'") && contains(nodes, "<text id='scale_axes_length'"),
          "the scene has the switch, the matrix and the length label that the script finds by id");
    check(contains(nodes, "string='E'") && contains(nodes, "string='N'") && contains(nodes, "string='U'"), "the axes are lettered E, N and U");

    std::ostringstream row;
    scale_axes::write_checkbox_row(row);
    check(contains(row.str(), "id='scale-axes-checkbox' type='checkbox' checked onclick='setScaleAxes(this.checked);'") && contains(row.str(), "<tr>"),
          "the check box is a table row that calls setScaleAxes");

    std::printf("scale axes: %zu bytes of script, %zu of scene\n", script.size(), nodes.size());
    return failures == 0 ? 0 : 1;
}
