#pragma once

#include <string>
#include <vector>

#include "io/cesium_writer.h"
#include "io/x3dom_writer.h"

// What mbmesh and mbglb2tiles have for the X3DOM pages, as what the Cesium page takes: the models, and the viewpoints of the user.

// The models of an X3DOM page as those of a Cesium page. A tileset keeps its lift, which the Cesium page takes off again; the chunk outlines and labels have no
// counterpart.
inline std::vector<CesiumModel> cesium_models(const std::vector<X3DomModel> &models) {
    std::vector<CesiumModel> out;
    out.reserve(models.size());
    for (const X3DomModel &model : models) {
        CesiumModel converted;
        converted.uri = model.uri;
        converted.label = model.label;
        converted.group = model.group;
        converted.group_note = model.group_note;
        converted.visible = model.visible;
        converted.is_tileset = model.is_tileset;
        converted.max_screen_space_error = model.max_screen_space_error;
        converted.lift_meters = model.lift_meters;
        converted.height_colors = model.height_colors;
        converted.log_uri = model.log_uri;
        converted.log_label = model.log_label;
        out.push_back(converted);
    }
    return out;
}

// A viewpoint of the user (--geoviewpoint) for the Cesium page. Returns false and says why in error if its orientation cannot be found, as when the camera
// is at the point it looks at.
[[nodiscard]] inline bool cesium_user_viewpoint(const X3DomUserViewpoint &viewpoint, CesiumUserViewpoint *converted, std::string *error) {
    double axis[3];
    double angle = 0.0;
    if (!geoviewpoint_orientation(viewpoint, axis, &angle, error)) {
        return false;
    }
    converted->longitude = viewpoint.longitude;
    converted->latitude = viewpoint.latitude;
    converted->height = viewpoint.elevation;
    for (int i = 0; i < 3; i++) {
        converted->axis[i] = axis[i];
    }
    converted->angle = angle;
    return true;
}

[[nodiscard]] inline bool cesium_user_viewpoints(const std::vector<X3DomUserViewpoint> &viewpoints, std::vector<CesiumUserViewpoint> *converted, std::string *error) {
    for (const X3DomUserViewpoint &viewpoint : viewpoints) {
        CesiumUserViewpoint one;
        if (!cesium_user_viewpoint(viewpoint, &one, error)) {
            return false;
        }
        converted->push_back(one);
    }
    return true;
}
