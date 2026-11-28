#pragma once

// The Haxby color table that mbm_grdplot uses by default, and the coloring of heights by it, shared by mbmesh and mbgrd2gltf so that a mesh from either has the
// same colors: 11 colors from dark blue to white, changing linearly between 11 heights that are chosen from the range of the heights as "mbm_grdplot -G5" does.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace mbdraco {

// The Haxby color table that mbm_grdplot uses (its palette 1), from low to high: 11 colors.
inline constexpr int haxby_color_count = 11;
inline constexpr double haxby_colors[haxby_color_count][3] = {
    {37, 57, 175}, {40, 127, 251}, {50, 190, 255}, {106, 235, 255}, {138, 236, 174}, {205, 255, 162},
    {240, 236, 121}, {255, 189, 87}, {255, 161, 68}, {255, 186, 133}, {255, 255, 255},
};

// The color of the palette at x, from 0 (first color) to 10 (last), interpolated between the colors.
[[nodiscard]] inline std::array<double, 3> haxby_palette_color(double x) {
    x = std::min(static_cast<double>(haxby_color_count - 1), std::max(0.0, x));
    const int i1 = std::min(haxby_color_count - 2, static_cast<int>(x));
    std::array<double, 3> color;
    for (int c = 0; c < 3; c++) {
        color[c] = haxby_colors[i1][c] + (haxby_colors[i1 + 1][c] - haxby_colors[i1][c]) * (x - i1);
    }
    return color;
}

// The bounds of the color table that "mbm_grdplot -G5" makes without -S: 11 equal steps from a rounded value below the
// minimum, the step chosen as mbm_grdplot does from the range of the values.
inline void linear_color_bounds(double minimum, double maximum, std::vector<double> *bounds) {
    bounds->clear();
    const double range = maximum - minimum;
    if (!(range > 0.0)) {
        bounds->push_back(minimum);
        return;
    }
    const int steps = haxby_color_count - 1;
    double contour_interval = std::pow(10.0, std::trunc(std::log10(range) + 0.5)) / 10.0;
    if (range / contour_interval < 10.0) {
        contour_interval /= 4.0;
    } else if (range / contour_interval < 20.0) {
        contour_interval /= 2.0;
    }
    const double start_interval = contour_interval / 2.0;
    double multiplier = std::trunc(range / steps / start_interval) + 1.0;
    double color_interval = multiplier * start_interval;
    const auto first_bound = [&](double interval) {
        return minimum < 0.0 ? interval * (std::trunc(minimum / interval) - 1.0) : interval * std::trunc(minimum / interval);
    };
    double start = first_bound(color_interval);
    if (start + color_interval * steps < maximum) {
        multiplier += 1.0;
        color_interval = multiplier * start_interval;
        start = first_bound(color_interval);
    }
    for (int i = 0; i <= steps; i++) {
        bounds->push_back(start + color_interval * i);
    }
}

// The color, 0 to 255, of a value on a table of bounds with a color at each: linear between the two bounds around it, the last color
// above the last bound.
[[nodiscard]] inline std::array<double, 3> color_for_value(double value, const std::vector<double> &bounds, const std::vector<std::array<double, 3>> &bound_colors) {
    const int bound_count = static_cast<int>(bounds.size());
    std::array<double, 3> color = bound_colors.back();
    if (bound_count > 1 && value < bounds.back()) {
        const int upper = static_cast<int>(std::upper_bound(bounds.begin(), bounds.end(), value) - bounds.begin());
        const int lower = std::max(0, upper - 1);
        const int next = std::min(bound_count - 1, upper);
        const double fraction = next > lower ? (value - bounds[lower]) / (bounds[next] - bounds[lower]) : 0.0;
        for (int c = 0; c < 3; c++) {
            color[c] = bound_colors[lower][c] + (bound_colors[next][c] - bound_colors[lower][c]) * fraction;
        }
    }
    return color;
}

[[nodiscard]] inline std::vector<std::array<double, 3>> palette_colors_for_bounds(int bound_count) {
    std::vector<std::array<double, 3>> bound_colors;
    for (int i = 0; i < bound_count; i++) {
        bound_colors.push_back(haxby_palette_color(bound_count > 1 ? (haxby_color_count - 1.0) * i / (bound_count - 1.0) : haxby_color_count / 2.0));
    }
    return bound_colors;
}


// The bounds of the table for heights from minimum to maximum (the same as haxby_height_bounds of mbmesh).
[[nodiscard]] inline std::vector<double> haxby_bounds(double minimum, double maximum) {
    std::vector<double> bounds;
    linear_color_bounds(minimum, maximum, &bounds);
    return bounds;
}

// The color of one height, red, green and blue bytes, on a table of bounds (from haxby_bounds).
[[nodiscard]] inline std::array<std::uint8_t, 3> haxby_color_bytes(double height, const std::vector<double> &bounds, const std::vector<std::array<double, 3>> &bound_colors) {
    const std::array<double, 3> color = color_for_value(height, bounds, bound_colors);
    std::array<std::uint8_t, 3> out;
    for (int c = 0; c < 3; c++) {
        out[c] = static_cast<std::uint8_t>(std::lround(std::min(255.0, std::max(0.0, color[c]))));
    }
    return out;
}

}  // namespace mbdraco
